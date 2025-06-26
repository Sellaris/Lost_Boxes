#ifndef DAB_FEATURE_EXTRACTOR_H
#define DAB_FEATURE_EXTRACTOR_H

#include "board.h"
#include <vector>

// fann_type is defined in fann.h, but to avoid including the full header here,
// we can forward-declare it or use a known type like float.
// FANN uses float by default for fann_type unless compiled with FIXEDFANN.
using fann_type = float;

struct Info {
    int Point[5] = {0};
    int Link[11] = {0};
    int Loop[5] = {0};
    int Halfheart = 0;
    int Fourlink = 0;
};

Info GetInfo(const Board& board, int currentPlayer);

std::vector<fann_type> extract_features(const Board& board, int currentPlayer);

#endif // DAB_FEATURE_EXTRACTOR_H