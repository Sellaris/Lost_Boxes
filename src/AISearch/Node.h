#ifndef DAB_NODE_H
#define DAB_NODE_H
#include "board.h"
#include "Params.h"
//====================Node类声明====================
// Node: UCT树节点，继承自Board
// 成员变量:
//   Owner: 所属玩家
//   Times: 被尝试次数
//   Board_Winner: 局面胜者
//   _Exist_Child: 当前子节点数
//   Total_Child: 子节点总数
//   _Avg_Value: 平均收益
//   Node_Moves: 所有可能招式
//   _Child_NODES: 子节点指针数组
class Node:public Board{
    public:
        int Owner,Times,Board_Winner,_Exist_Child,Total_Child;
        int search_depth; // 为置换表增加搜索深度记录
        float _Avg_Value;
        POS Node_Moves[60];
        Node*_Child_NODES[60];
    // Node(): 默认构造
    Node();
    // Node(Node&other): 拷贝构造
    Node(const Node&other);
    // Node(int Player,int ARRAY[LEN][LEN],bool GetCBox): 参数构造
    Node(int Player,int ARRAY[LEN][LEN],bool GetCBox);
    // refresh_Avg_Value: 更新平均收益
    // 输出: 返回新平均值
    float refresh_Avg_Value();
    // TAKE_UCB_Value: 获取UCB值
    // 输入: Total-总次数
    // 输出: 返回UCB值
    float TAKE_UCB_Value(int Total);
    // expand_UCT_Node: 扩展节点
    // 输出: 返回新子节点指针
    Node*expand_UCT_Node();
};
#endif
