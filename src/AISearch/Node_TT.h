/*
 * Node_TT.h  ——  结合置换表与估值版 UCT 节点
 * 基本字段与 Node 保持一致，但在 expand_UCT_Node 中使用静态置换表并改用
 * 局面估值函数 evaluate_v2 替代蒙特卡洛模拟。
 */
#ifndef DAB_NODE_TT_H
#define DAB_NODE_TT_H
#include "board.h"
#include "Params.h"
#include <utility>

class Node_TT : public Board {
public:
    int Owner, Times, Board_Winner, _Exist_Child, Total_Child;
    int search_depth; // TT 记录的最大搜索深度
    int score;        // TT 记录的分数
    Bound bound_type; // TT 记录的类型
    POS best_move;    // TT 记录的最佳走法
    float _Avg_Value; // UCT 平均值
    POS Node_Moves[60];
    Node_TT* _Child_NODES[60]{}; // 指向子节点

    Node_TT();                                 // 默认构造
    Node_TT(const Node_TT& other);             // 拷贝构造
    Node_TT(int Player, int ARRAY[LEN][LEN], bool GetCBox); // 参数构造
    ~Node_TT();                 // 新增：递归释放子节点

    float refresh_Avg_Value();                 // 更新平均收益
    float TAKE_UCB_Value(int Total);           // 计算 UCB 值
    Node_TT* expand_UCT_Node();                // 扩展子节点（集成置换表）
};

// 打印置换表统计信息（在 Node_TT.cpp 实现）
void print_tt_stats_TT();

#endif // DAB_NODE_TT_H 