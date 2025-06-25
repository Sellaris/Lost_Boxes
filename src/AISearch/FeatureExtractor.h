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

/**
 * @brief Extracts a 25-dimensional feature vector from the board state for ANN evaluation.
 *
 * The feature extraction logic is a C++ translation of the original Go implementation,
 * calculating features like node degrees, chains (Links), and loops.
 *
 * The 25 features are:
 * - 5 "Point" features (move count, nodes by degree 1-4)
 * - 11 "Link" features (open chains of length 2-12)
 * - 5 "Loop" features (closed loops of length 4-8)
 * - 1 "Halfheart" feature (loops of length > 8)
 * - 1 "Fourlink" feature (edges connecting two degree-4 nodes)
 * - 2 score features (current player's score, opponent's score)
 *
 * @param board The current board state.
 * @param currentPlayer The player whose turn it is (BLACK or WHITE).
 * @param total_moves The total number of moves made so far in the game.
 * @return A std::vector<fann_type> of 25 normalized features.
 */
std::vector<fann_type> extract_features(const Board& board, int currentPlayer);

#endif // DAB_FEATURE_EXTRACTOR_H