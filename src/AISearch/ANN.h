// ANN.h
#ifndef ANN_H
#define ANN_H

#include <vector>
#include "fann.h"

extern "C" {
    #include "fann.h"
}

struct FannDeleter {
    void operator()(struct fann* ann) const {
        if (ann) fann_destroy(ann);
    }
};

inline float processANN( std::vector<fann_type> input) {
    const char* annFile = "D:\\LAST_DAB\\src\\Weights\\32.ann";

    std::unique_ptr<struct fann, FannDeleter> ann(fann_create_from_file(annFile));
    
    if (!ann) {
        std::cerr << "Error: cannot load ANN file " << annFile << std::endl;
        return 1.0f;
    }

    fann_type* output = fann_run(ann.get(), input.data());
    return output[0];
}

#endif // ANN_H
