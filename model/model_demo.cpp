//
// Created by Yongliang Yang on 2023/5/23.
//

# include <torch/script.h>
#include <iostream>
#include <memory>
#include <vector>
#include <ATen/ATen.h>

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cerr << "usage: RS-Server <path-to-exported-script-module>\n";
        return -1;
    }


    torch::jit::script::Module module;
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load(argv[1]);
    }
    catch (const c10::Error& e) {
        std::cerr << "error loading the model\n";
        return -1;
    }

    std::vector<torch::jit::IValue> inputs;

    std::vector<float> value{0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33, 0.33};
    auto opts = torch::TensorOptions().dtype(torch::kFloat32);
    auto input_tensor = torch::from_blob(value.data(), {int64_t(value.size())}, opts).reshape({1, 10});

    //inputs.push_back(input_tensor.reshape({1, 10}));

    at::Tensor output = module.forward({input_tensor}).toTensor();
    std::cout << output << '\n';

    std::vector<float> v(output.data_ptr<float>(), output.data_ptr<float>() + output.numel());

    std::cout << v << '\n';

    std::cout << "ok\n";
}
