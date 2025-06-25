/*#include <iostream>
#include <vector>
extern "C" {
  #include "fann.h"
}

int main() {
    const char* annFile = "../Weights/32.ann";
    struct fann *ann = fann_create_from_file(annFile);
    if (!ann) {
        std::cerr << "Error: cannot load ANN file " << annFile << std::endl;
        return 1;
    }

    unsigned int num_input = fann_get_num_input(ann);
    std::cout << "Loaded ANN with input dimensions: " << num_input << std::endl;

    if (num_input != 25) {
        std::cerr << "Error: expected 25 inputs, ANN has " << num_input << std::endl;
        fann_destroy(ann);
        return 2;
    }

    // 模拟输入：你可以替换为实际数据
    std::vector<fann_type> input(25, 0.5f);

    fann_type* output = fann_run(ann, input.data());
    unsigned int num_output = fann_get_num_output(ann);
    std::cout << "ANN output (" << num_output << " dims): ";
    for (unsigned int i = 0; i < num_output; ++i) {
        std::cout << output[i] << " ";
    }
    std::cout << std::endl;

    fann_destroy(ann);
    system("pause");
    return 0;
}*/