//
// Created by Yongliang Yang on 2023/5/22.
//
/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <torch/script.h>
#include <iostream>
#include <memory>
#include <vector>
#include <ATen/ATen.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "model/model.hpp"
#include "utils/thread_pool.hpp"
#include "utils/utils.hpp"


#include "rs.grpc.pb.h"


ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::Status;
using rs::RSer;
using rs::userRequest;
using rs::itemsString;

class ServerImpl final {
public:
    ~ServerImpl() {
        server_->Shutdown();
        // Always shutdown the completion queue after the server.
        cq_->Shutdown();
    }
    ServerImpl(std::shared_ptr<rs::RS_Model> model, std::shared_ptr<ThreadPool> thread_pool) :
        _model(model), _thread_pool(thread_pool) {}

    // There is no shutdown handling in this code.
    void Run(uint16_t port) {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

        ServerBuilder builder;
        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        // Register "service_" as the instance through which we'll communicate with
        // clients. In this case it corresponds to an *asynchronous* service.
        builder.RegisterService(&service_);
        // Get hold of the completion queue used for the asynchronous communication
        // with the gRPC runtime.
        cq_ = builder.AddCompletionQueue();
        // Finally assemble the server.
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        // Proceed to the server's main loop.
        HandleRpcs();
    }

private:
    // Class encompasing the state and logic needed to serve a request.
    class CallData {
    public:
        // Take in the "service" instance (in this case representing an asynchronous
        // server) and the completion queue "cq" used for asynchronous communication
        // with the gRPC runtime.
        CallData(RSer::AsyncService* service, ServerCompletionQueue* cq,
                 std::shared_ptr<rs::RS_Model> model, std::shared_ptr<ThreadPool> thread_pool)
                : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), __model(model), __thread_pool(thread_pool) {
            // Invoke the serving logic right away.
            Proceed();
        }

        void Proceed() {
            if (status_ == CREATE) {
                // Make this instance progress to the PROCESS state.
                status_ = PROCESS;

                // As part of the initial CREATE state, we *request* that the system
                // start processing SayHello requests. In this request, "this" acts are
                // the tag uniquely identifying the request (so that different CallData
                // instances can serve different requests concurrently), in this case
                // the memory address of this CallData instance.
                service_->RequestCallRsItems(&ctx_, &request_, &responder_, cq_, cq_,
                                          this);
            } else if (status_ == PROCESS) {
                // Spawn a new CallData instance to serve new clients while we process
                // the one for this CallData. The instance will deallocate itself as
                // part of its FINISH state.
                new CallData(service_, cq_, __model, __thread_pool);

                // The actual processing.
                const std::string& embedding_string = request_.vector();
                std::string item_list = __model->rs_handle(embedding_string);
                std::cout << "finish inference: " << item_list << '\n';
                /***
                 * 模型的推理在主线程进行，结果得会写提交到线程池执行
                 */
                std::function<void(std::string& response)> write_response = [_this=this](std::string& response) {
                    std::cout << "handle the rpc..." << '\n';
                    _this->reply_.set_items(response);
                    _this->status_ = FINISH;
                    // And we are done! Let the gRPC runtime know we've finished, using the
                    // memory address of this instance as the uniquely identifying tag for
                    // the event.
                    _this->responder_.Finish(_this->reply_, Status::OK, _this);
                };
                __thread_pool->submit(write_response, item_list);
                /*** 下面这段代码是gRPC官方提供的异步服务器的处理代码，单线程处理

                //reply_.set_items(prefix + request_.vector());

                // And we are done! Let the gRPC runtime know we've finished, using the
                // memory address of this instance as the uniquely identifying tag for
                // the event.
                status_ = FINISH;
                responder_.Finish(reply_, Status::OK, this);
                */
            } else {
                GPR_ASSERT(status_ == FINISH);
                // Once in the FINISH state, deallocate ourselves (CallData).
                delete this;
            }
        }

    private:
        // The means of communication with the gRPC runtime for an asynchronous
        // server.
        RSer::AsyncService* service_;
        // The producer-consumer queue where for asynchronous server notifications.
        ServerCompletionQueue* cq_;
        // Context for the rpc, allowing to tweak aspects of it such as the use
        // of compression, authentication, as well as to send metadata back to the
        // client.
        ServerContext ctx_;

        // What we get from the client.
        userRequest request_;
        // What we send back to the client.
        itemsString reply_;

        // The means to get back to the client.
        ServerAsyncResponseWriter<itemsString> responder_;

        std::shared_ptr<rs::RS_Model> __model;
        std::shared_ptr<ThreadPool> __thread_pool;

        // Let's implement a tiny state machine with the following states.
        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;  // The current serving state.
    };

    // This can be run in multiple threads if needed.
    void HandleRpcs() {
        // Spawn a new CallData instance to serve new clients.
        new CallData(&service_, cq_.get(), _model, _thread_pool);
        void* tag;  // uniquely identifies a request.
        bool ok;
        while (true) {
            // Block waiting to read the next event from the completion queue. The
            // event is uniquely identified by its tag, which in this case is the
            // memory address of a CallData instance.
            // The return value of Next should always be checked. This return value
            // tells us whether there is any kind of event or cq_ is shutting down.
            GPR_ASSERT(cq_->Next(&tag, &ok));
            GPR_ASSERT(ok);

            //现在的逻辑是模型的推理在主线程执行，结果的回写提交到线程池执行，也可以在这里将整个任务提交到线程池执行
            static_cast<CallData*>(tag)->Proceed();
        }
    }

    std::unique_ptr<ServerCompletionQueue> cq_;
    RSer::AsyncService service_;
    std::unique_ptr<Server> server_;
    std::shared_ptr<rs::RS_Model> _model;
    std::shared_ptr<ThreadPool> _thread_pool;
};

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);

    // 初始化模型
    long int arr[2] = {1, 10};
    c10::IntArrayRef ref(arr, 2);
    std::shared_ptr<rs::RS_Model> model = std::make_shared<rs::RS_Model>(argv[1], ref);
    std::shared_ptr<ThreadPool> thread_pool = std::make_shared<ThreadPool>();
    thread_pool->init();
//    std::string str = "0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33";
//    std::vector<float> input_value = rs::convert_string_to_vector<float>(str, ',');
//
//    rs::RS_Model _model(argv[1], ref);
//    std::vector<float> out = _model.inference(input_value);
//    std::cout << "inference result: " << out << '\n';
//
//    std::vector<int> ids_list = rs::argsort<float>(out, true);
//    std::cout << "arg_sort result: " << ids_list << '\n';
//
//    std::string ids_str = rs::convert_verctor_string<int>(ids_list, ',');
//    std::cout << "idx str: " << ids_str << std::endl;
    ServerImpl server(model, thread_pool);
    server.Run(absl::GetFlag(FLAGS_port));
    return 0;
}
