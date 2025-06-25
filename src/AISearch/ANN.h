#include <memory>  // 新增头文件
#include <vector>
extern "C" {
  #include "fann.h"
}

// 自定义智能指针删除器
struct FannDeleter {
    void operator()(struct fann* ann) const {
        if (ann) fann_destroy(ann);
    }
};

float processANN(std::vector<fann_type> input) {  // 封装为独立函数
    const char* annFile = "D:\\LAST_DAB\\src\\Weights\\32.ann";

    // 使用智能指针管理资源
    std::unique_ptr<struct fann, FannDeleter> ann(
        fann_create_from_file(annFile)
    );
    
    if (!ann) {
        std::cerr << "Error: cannot load ANN file " << annFile << std::endl;
        return 1;
    }

    unsigned int num_input = fann_get_num_input(ann.get());
    std::cout << "Loaded ANN with input dimensions: " << num_input << std::endl;

    if (num_input != 25) {
        std::cerr << "Error: expected 25 inputs, ANN has " << num_input << std::endl;
        return 2;
    }

    fann_type* output = fann_run(ann.get(), input.data());

    return output[0];  
}
