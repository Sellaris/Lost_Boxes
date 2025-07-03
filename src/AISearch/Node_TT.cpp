#include "Node_TT.h"
#include "Estimate.h"
#include "Params.h"
#include "UCT.h"
#include <cstring>
#include <cmath>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <vector>
#include <limits>
#include <atomic>
#include <iostream>
#include <deque>
#include "NodeArena.h"
// ---------------- 置换表相关全局结构 ----------------
namespace {
    // 只缓存评估结果而不共享节点实例，避免多线程竞争
    static std::unordered_map<std::string, float> g_eval_table_tt;
    static std::deque<std::string> g_key_order_tt;
    constexpr size_t TT_MAX_SIZE = TT_MAX_SIZE_; // 约 40 万局面，控制内存
    static std::mutex g_tt_mutex_tt;

    // 统计信息
    std::atomic<int> tt_probes_tt{0};
    std::atomic<int> tt_hits_tt{0};
    std::atomic<int> tt_inserts_tt{0};

    // 生成转置表 key：棋盘 + 当前落子方
    std::string generate_key_tt(const Board &b, int player) {
        std::string key;
        key.reserve(LEN * LEN + 1);
        for (int i = 0; i < LEN; ++i) {
            for (int j = 0; j < LEN; ++j) {
                key.push_back(static_cast<char>(b._MAP_[i][j]));
            }
        }
        key.push_back(static_cast<char>(player));
        return key;
    }
}

// ---------------- 构造函数 ----------------
Node_TT::Node_TT(){
    Owner = Times = Board_Winner = _Exist_Child = Total_Child = score = 0;
    search_depth = -1;
    bound_type = EXACT;
    best_move = {-1, -1};
    _Avg_Value = 0.f;
}

Node_TT::Node_TT(const Node_TT& other) : Board(other) {
    // 基础信息复制
    Owner         = other.Owner;
    Times         = other.Times;
    Board_Winner  = other.Board_Winner;
    Total_Child   = other.Total_Child;
    _Avg_Value    = other._Avg_Value;
    search_depth  = other.search_depth;
    score         = other.score;
    bound_type    = other.bound_type;
    best_move     = other.best_move;

    // 拷贝走法列表
    memcpy(Node_Moves, other.Node_Moves, sizeof(Node_Moves));
    memcpy(_MOVE_NUMBERS_, other._MOVE_NUMBERS_, sizeof(_MOVE_NUMBERS_));

    // 为避免跨线程数据竞争，不拷贝子节点指针。
    _Exist_Child = 0;
    for (auto &ptr : _Child_NODES) ptr = nullptr;
}

Node_TT::Node_TT(int Player, int ARRAY[LEN][LEN], bool GetCBox) : Board(ARRAY) {
    if (GetCBox) {
        int move_count = 0; // C 类型格编号为 0，不计入总步数
        _TAKE_All_C_Boxes(Player, move_count, false);
    }
    Owner = Player;
    Times = 0;
    search_depth = -1;
    score = 0;
    bound_type = EXACT;
    best_move = {-1, -1};

    Board Win(_MAP_);
    Board_Winner = TAKE_Winner_OF_Board(Win, -Player);

    _Exist_Child = 0;
    int free_edge = Win.TAKE__Free_Edge_Num();

    Board_Box test(_MAP_);
    if (free_edge >= UCT__RANGE_FILTER) {
        Total_Child = test.TAKE__Free__Moves(Node_Moves);
    } else {
        Total_Child = test.TAKE_Filter_Moves(Node_Moves);
    }
}

// ---------------- 节点辅助函数 ----------------
float Node_TT::refresh_Avg_Value() {
    float value = 0.f;
    for (int i = 0; i < _Exist_Child; ++i) {
        value += _Child_NODES[i]->_Avg_Value * _Child_NODES[i]->Times / Times;
    }
    return _Avg_Value = 1.f - value;
}

float Node_TT::TAKE_UCB_Value(int Total) {
    if (Times == 0) return std::numeric_limits<float>::infinity();
    return _Avg_Value + sqrt((log(Total) * 2) / Times);
}

// ---------------- 生成 / 获取 子节点 ----------------
Node_TT* Node_TT::expand_UCT_Node() {
    // 1. 先生成走子后的棋盘
    Board cb(_MAP_);
    cb.move(Owner, {Node_Moves[_Exist_Child].first, Node_Moves[_Exist_Child].second}, 0);
    int child_player = -Owner;

    // 2. 生成 key 查找 / 插入置换表
    std::string key = generate_key_tt(cb, child_player);

    // 2. 创建全新的子节点（线程独立）
    tt_probes_tt++;
    Node_TT* child_node = new Node_TT(child_player, cb._MAP_, true);

    // 3. 评估值缓存查询 / 写入
    float norm = 0.f;
    {
        std::lock_guard<std::mutex> lk(g_tt_mutex_tt);
        auto it = g_eval_table_tt.find(key);
        if (it != g_eval_table_tt.end()) {
            norm = it->second;
            tt_hits_tt++;
        } else {
            Board_Box bb(cb._MAP_);
            int eval = bb.evaluate_v2();
            norm = static_cast<float>(eval) / 8.0f;
            g_eval_table_tt[key] = norm;
            g_key_order_tt.push_back(key);
            if(g_eval_table_tt.size() > TT_MAX_SIZE){
                const std::string &old_key = g_key_order_tt.front();
                g_key_order_tt.pop_front();
                g_eval_table_tt.erase(old_key);
            }
            tt_inserts_tt++;
        }
    }

    child_node->_Avg_Value = norm;

    return child_node;
}

void print_tt_stats_TT() {
    int probes = tt_probes_tt.load();
    int hits   = tt_hits_tt.load();
    int inserts= tt_inserts_tt.load();
    float hit_rate = probes ? static_cast<float>(hits) / probes * 100.f : 0.f;
    std::cerr << "[UCT_TT 置换表] 查询:" << probes << " 命中:" << hits
              << " 插入:" << inserts << " 命中率:" << hit_rate << "%\n";
} 

// ---------------- 析构函数 ----------------
Node_TT::~Node_TT() {
    // 由线程本地 Arena 统一回收，无需逐个 delete
}

// ---------------- 自定义内存管理 ----------------
void* Node_TT::operator new(std::size_t size) {
    return get_node_arena().alloc(size);
}

void Node_TT::operator delete(void* /*ptr*/, std::size_t /*size*/) {
    // 内存由 Arena 统一回收，此处无需处理
}

void Node_TT::operator delete(void* /*ptr*/) noexcept {
    // 与上同
}

// 构建期检查，确保节点大小不超过固定块
static_assert(sizeof(Node_TT) <= Arena::BLOCK_SIZE, "Node_TT size exceeds Arena block size (2 KiB)");