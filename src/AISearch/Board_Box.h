// Board_Box_Plus.h
#ifndef BOARD_BOX_H
#define BOARD_BOX_H

#include "board.h"
#include <vector>
#include <tuple>
#include <functional>

using POS = std::pair<int,int>;

class Board_Box_Plus {
public:
    explicit Board_Box_Plus(const Board& b);

    /**
     * 将当前局面中的所有链（chain）和环（cycle）对应的“可下边”集合分别
     * 填入 chains, cycles 中。每条链或环由一系列 POS（缺口边坐标）组成。
     */
    void GetChainsAndCycles(
        std::vector<std::vector<POS>>& chains,
        std::vector<std::vector<POS>>& cycles
    );

private:
    const Board& _board;

    /// 返回中心格 (bx,by) 已画边数（即自由度外的边数）
    int CountDrawnEdges(int bx, int by) const;

    /// 当 CountDrawnEdges(bx,by)==3 时，返回那格唯一的“缺口边”
    POS MissingEdge(int bx, int by) const;

    /**
     * 对于中间箱（CountDrawnEdges==2），给定已知缺口 e0，返回另一条缺口 e1
     * 并告诉你对应的下一个中心箱坐标 (nbx,nby)。
     */
    std::tuple<int,int,POS> TraceNext(int bx, int by, const POS& e0) const;
};

#endif // BOARD_BOX_H