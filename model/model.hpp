//
// Created by Yongliang Yang on 2023/5/25.
//

#include <torch/script.h>
#include <iostream>
#include <memory>
#include <vector>
#include <ATen/ATen.h>

namespace rs {
    class RS_Model{
    public:
        RS_Model(const char* model_path, at::IntArrayRef& input_tensor_shape) : _input_tensor_shape(input_tensor_shape){
            try {
                // Deserialize the ScriptModule from a file using torch::jit::load().
                _model = torch::jit::load(model_path);
            }
            catch (const c10::Error& e) {
                std::cerr << "error loading the model\n";
            }
        }
        std::vector<float> inference(std::vector<float>& input_value) {
            // 将vector转换为tensor
            auto _opts = torch::TensorOptions().dtype(torch::kFloat32);
            auto input_tensor = torch::from_blob(input_value.data(), {int64_t(input_value.size())}, _opts).reshape(_input_tensor_shape);
            at::Tensor output = _model.forward({input_tensor}).toTensor();

            // 将tensor转换为vector
            std::vector<float> v(output.data_ptr<float>(), output.data_ptr<float>() + output.numel());
            return v;
        }
    private:
        // model
        torch::jit::script::Module _model;
        //std::string _model_path;
        // 用于初始化输入到模型的维度信息
        at::IntArrayRef _input_tensor_shape;

    };
}
