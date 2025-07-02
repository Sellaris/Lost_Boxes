/*
 * UCT_TT.h  ——  UCT + 置换表 + 估值 函数声明
 * 同 UCT.h 逻辑，但内部节点与评估替换为 Node_TT 与 evaluate_v2。
 */
#ifndef DAB_UCT_TT_H
#define DAB_UCT_TT_H
#include "Node_TT.h"
#include "board.h"
#include <vector>

// 处理单节点的 UCT 过程
float Process_UCT_TT(Node_TT &B, int &Total);

// 使用 UCT_TT 选择落子（核心）
void Move_UCT_TT(Board &CB, int Player, std::vector<POS> &pace, int total_moves);

// 前期牺牲策略（与原版一致，内部调用 Move_UCT_TT）
void _Sacrifice_Move_UCT_TT(Board &CB, int Player, std::vector<POS> &pace, int total_moves);

// 后期决策
void Situation_Move_latter_UCT_TT(Board &CB, int Player, std::vector<POS> &pace, int total_moves);

// 根据棋局阶段自动调用
void Turn_Move_Game_UCT_TT(Board &CB, int Player, int *status, std::vector<POS> &pace, int total_moves);

#endif // DAB_UCT_TT_H 