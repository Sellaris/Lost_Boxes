#ifndef DAB_NODE_PLUS_H
#define DAB_NODE_PLUS_H
#include "board.h"
#include "Params.h"
//====================Node_Plus类声明====================
// Node_Plus: UCT树节点，继承自Board
// 成员变量:
//   Owner: 所属玩家
//   Times: 被尝试次数
//   Board_Winner: 局面胜者
//   _Exist_Child: 当前子节点数
//   Total_Child: 子节点总数
//   _Avg_Value: 平均收益
//   Node_Plus_Moves: 所有可能招式
//   _Child_NODES: 子节点指针数组
class Node_Plus:public Board{
    public:
        int Owner,Times,Board_Winner,_Exist_Child,Total_Child;
        int search_depth; // 为置换表增加搜索深度记录
        int score;        // TT中存储的分数
        Bound bound_type; // TT中存储分数的类型(EXACT, LOWER, UPPER)
        POS best_move;    // TT中存储的最佳走法
        float _Avg_Value;
        POS Node_Plus_Moves[60];
        Node_Plus*_Child_NODES[60];  
        
          
        void PruneEdges();
        // Node_Plus(): 默认构造
        Node_Plus();                                 // 默认构造
        Node_Plus(const Node_Plus& other);             // 拷贝构造
        Node_Plus(int Player, int ARRAY[LEN][LEN], bool GetCBox); // 参数构造
        ~Node_Plus();                 // 新增：递归释放子节点
        // Node_Plus(Node_Plus&other): 拷贝构造
        //Node_Plus(const Node_Plus&other);
        // Node_Plus(int Player,int ARRAY[LEN][LEN],bool GetCBox): 参数构造
        //Node_Plus(int Player,int ARRAY[LEN][LEN],bool GetCBox);
        // refresh_Avg_Value: 更新平均收益
        // 输出: 返回新平均值
        float refresh_Avg_Value();
        // TAKE_UCB_Value: 获取UCB值
        // 输入: Total-总次数
        // 输出: 返回UCB值
        float TAKE_UCB_Value(int Total);
        // expand_UCT_Node_Plus: 扩展节点
        // 输出: 返回新子节点指针
        Node_Plus*expand_UCT_Node_Plus();
            // 自定义内存管理：线程本地 Arena
        static void* operator new(std::size_t size);
        static void operator delete(void* ptr, std::size_t size);
        static void operator delete(void* ptr) noexcept;
    private:
    /// 随机从一组可下边中选一条
    POS RandomEdge(const std::vector<POS>& edges);

    /// 标记除 keep 以外的所有可下边为等价边
    void MarkEquivalenceEdges(const std::vector<POS>& edges, const POS& keep);

    /// 处理棋盘四角的对称等价边
    void MarkCornerEquivalence();

    /// 真正从 Node_Plus_Moves 中移除已标记的等价边
    void RemoveMarkedEdges();

};
#endif
