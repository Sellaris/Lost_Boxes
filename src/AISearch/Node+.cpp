#include "Node+.h"
#include "UCT+.h"
#include "Estimate.h"
#include <cmath>
#include "NodeArena.h"
#include "Board_Box.h"
#include <cstring>
#include <assert.h>
thread_local std::mt19937_64 rng_PLUS(std::chrono::system_clock::now().time_since_epoch().count()*_threadid);

/**
 * 默认构造函数
 * 初始化棋盘状态，设置所有成员变量为默认值
 * 输入：无
 * 输出：初始化完成的Node_Plus对象
 * 算法：直接初始化所有成员变量为0或默认值
 */
Node_Plus::Node_Plus(){
    Owner=Times=Board_Winner=_Exist_Child=Total_Child=score=0;
    search_depth=-1;
    bound_type = EXACT;
    best_move = {-1, -1};
    _Avg_Value=0;
    std::memset(_MOVE_NUMBERS_, 0, sizeof(_MOVE_NUMBERS_));
    std::memset(_Child_NODES,   0, sizeof(_Child_NODES));     

}

/**
 * 拷贝构造函数
 * 从另一个Node_Plus对象复制所有数据
 * 输入：other - 要复制的Node_Plus对象
 * 输出：新创建的Node_Plus对象
 * 算法：调用Board拷贝构造函数，然后复制所有成员变量
 */
Node_Plus::Node_Plus(const Node_Plus&other):Board(other){
    Owner=other.Owner;
    Times=other.Times;
    Board_Winner=other.Board_Winner;
    _Exist_Child=other._Exist_Child;
    Total_Child=other.Total_Child;
    _Avg_Value=other._Avg_Value;
    for(int i=0;i<60;++i){
        Node_Plus_Moves[i]=other.Node_Plus_Moves[i];
        _Child_NODES[i]=other._Child_NODES[i];
    }
    memcpy(_MOVE_NUMBERS_, other._MOVE_NUMBERS_, sizeof(_MOVE_NUMBERS_));
}

/**
 * 带参数的构造函数
 * 根据当前棋盘状态和玩家创建新节点
 * 输入：Player - 当前玩家，Array - 棋盘数组，GetCBox - 是否获取C型格
 * 输出：新创建的Node_Plus对象
 * 算法：先处理C型格，然后初始化节点数据，计算可能的移动数量
 */
Node_Plus::Node_Plus(int Player,int Array[LEN][LEN],bool GetCBox):Board(Array){
    if(GetCBox){
        // C型格属于规则内的强制移动，不计入总步数，因此编号为0
        int move_count = 0; 
        _TAKE_All_C_Boxes(Player, move_count, false);
    }
    Owner=Player;
    Times=0;
    search_depth = -1;
    score = 0;
    bound_type = EXACT;
    best_move = {-1, -1};
    std::memset(_MOVE_NUMBERS_, 0, sizeof(_MOVE_NUMBERS_));
    std::memset(_Child_NODES,   0, sizeof(_Child_NODES));     
    Board Win(_MAP_);
    Board_Winner=TAKE_Winner_OF_Board_PLUS(Win,-Player);
    _Exist_Child=0;
    int F=Win.TAKE__Free_Edge_Num();
    if(F>=UCT__RANGE_FILTER){
        Board_Box Test(_MAP_);
        Total_Child=Test.TAKE__Free__Moves(Node_Plus_Moves);
    }else{
        Board_Box Test(_MAP_);
        Total_Child=Test.TAKE_Filter_Moves(Node_Plus_Moves);
    }
    PruneEdges();  
}

/**
 * 刷新平均价值
 * 根据子节点的平均价值计算当前节点的平均价值
 * 输入：无
 * 输出：更新后的平均价值
 * 算法：遍历所有子节点，按访问次数加权计算平均价值，当前节点价值为1减去子节点加权平均
 */
float Node_Plus::refresh_Avg_Value(){
    float _Value=0;
    for(int i=0;i<_Exist_Child;i++){
        _Value+=_Child_NODES[i]->_Avg_Value*_Child_NODES[i]->Times/Times;
    }
    return _Avg_Value=1-_Value;
}

/**
 * 计算UCB值
 * 使用UCB1公式计算节点的UCB值用于树搜索
 * 输入：Total - 总访问次数
 * 输出：UCB值
 * 算法：UCB1公式：平均价值 + sqrt(2*ln(总次数)/当前次数)
 */
float Node_Plus::TAKE_UCB_Value(int Total){
    return _Avg_Value+sqrt((log(Total)*2)/Times);
}

/**
 * 扩展UCT节点
 * 创建新的子节点并进行蒙特卡洛评估
 * 输入：无
 * 输出：新创建的子节点指针
 * 算法：根据当前可用移动创建新棋盘，执行移动，然后进行蒙特卡洛评估初始化价值
 */
Node_Plus*Node_Plus::expand_UCT_Node_Plus(){
    Board CB(_MAP_);
    CB.move(Owner,{Node_Plus_Moves[_Exist_Child].first,Node_Plus_Moves[_Exist_Child].second}, 0);
    Node_Plus*NEW_BOARD=new Node_Plus(-Owner,CB._MAP_,true);
    CB.setBoard(NEW_BOARD->_MAP_);
    NEW_BOARD->_Avg_Value=(Evalution_TAKE_Filter_OF_MC_PLUS(CB,-Owner,Owner)+13)/26.0f;
    return NEW_BOARD;
} 

// ---------------- 析构函数 ----------------
Node_Plus::~Node_Plus() {
    // 由线程本地 Arena 统一回收，无需逐个 delete
}

// ---------------- 自定义内存管理 ----------------
void* Node_Plus::operator new(std::size_t size) {
    return get_node_arena().alloc(size);
}

void Node_Plus::operator delete(void* /*ptr*/, std::size_t /*size*/) {
    // 内存由 Arena 统一回收，此处无需处理
}

void Node_Plus::operator delete(void* /*ptr*/) noexcept {
    // 与上同
}

// 构建期检查，确保节点大小不超过固定块
static_assert(sizeof(Node_Plus) <= Arena::BLOCK_SIZE, "Node_Plus size exceeds Arena block size (2 KiB)");


// ---------------- 等价裁边模块 ----------------
POS Node_Plus::RandomEdge(const std::vector<POS>& edges) {
    std::uniform_int_distribution<size_t> dist(0, edges.size() - 1);
    return edges[ dist(rng_PLUS) ];
}

void Node_Plus::MarkEquivalenceEdges(const std::vector<POS>& edges, const POS& keep) {
    for (auto& e : edges) {
        if (e != keep) {
            // 1 表示"等价边"，后面统一从候选中剔除
            _MOVE_NUMBERS_[ e.first ][ e.second ] = 1;
        }
    }
}

void Node_Plus::MarkCornerEquivalence() {
    // 棋盘四角对称边对 (假设棋盘 0–10)
    const std::pair<POS,POS> corners[4] = {
        {{0,1},{1,0}}, {{0,9},{1,10}}, {{10,1},{9,0}}, {{10,9},{9,10}}
    };
    
    // 对应的角落格子坐标
    const POS corner_boxes[4] = {
        {1,1}, {1,9}, {9,1}, {9,9}
    };
    
    for (int i = 0; i < 4; ++i) {
        auto& pr = corners[i];
        POS a = pr.first, b = pr.second;
        POS corner_box = corner_boxes[i];
        
        // 检查角落格子的四条边是否都未被下过
        if (TAKE__Free_dom(corner_box.first, corner_box.second) == 4) {
            bool hasA = false, hasB = false;
            for (int j = 0; j < Total_Child; ++j) {
                if (Node_Plus_Moves[j] == a) hasA = true;
                if (Node_Plus_Moves[j] == b) hasB = true;
            }
            if (hasA && hasB) {
                // 随机保留一条，另一条标记
                POS keep = (rng_PLUS() & 1) ? a : b;
                POS drop = (keep == a ? b : a);
                _MOVE_NUMBERS_[ drop.first ][ drop.second ] = 1;
            }
        }
    }
}

void Node_Plus::RemoveMarkedEdges() {
    int w = 0;
    for (int r = 0; r < Total_Child; ++r) {
        POS e = Node_Plus_Moves[r];
        if (_MOVE_NUMBERS_[ e.first ][ e.second ] == 0) {
            Node_Plus_Moves[w++] = e;
        }
    }
    Total_Child = w;
}

void Node_Plus::PruneEdges() {
    // 1. 检测所有链和环
    Board_Box_Plus detector(*this);
    std::vector<std::vector<POS>> chains, cycles;
    detector.GetChainsAndCycles(chains, cycles);

    // 2. 对每条链/环保留一条，其余标记为等价边
    for (auto& ch : chains) {
        POS keep = RandomEdge(ch);
        MarkEquivalenceEdges(ch, keep);
    }
    for (auto& cy : cycles) {
        POS keep = RandomEdge(cy);
        MarkEquivalenceEdges(cy, keep);
    }

    // 3. 特殊处理四角
    MarkCornerEquivalence();

    // 4. 从 Node_Plus_Moves 中移除所有被标记的等价边
    RemoveMarkedEdges();
    assert(Total_Child > 0 && "PruneEdges() removed all candidate moves");

}