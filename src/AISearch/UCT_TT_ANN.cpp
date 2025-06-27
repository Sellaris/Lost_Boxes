#include "UCT_TT_ANN.h"
#include "Estimate.h"
#include "Node.h"
#include <string>
#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <cmath>
extern "C" {
  #include "fann.h"
}
#include "FeatureExtractor.h"
#include "ANN.h"

// 为TT_ANN版本的UCT提供独立的线程局部随机数生成器
thread_local std::mt19937_64 rng_tt_ANN(std::chrono::system_clock::now().time_since_epoch().count());

namespace { // 匿名命名空间，用于内部实现

// --- 置换表与内存管理 ---
// 使用内存池(g_node_pool)持有所有节点的所有权
static std::vector<std::unique_ptr<Node>> g_node_pool;
// 置换表(g_trans_table)存储指向内存池中节点的非拥有指针
static std::unordered_map<std::string, Node*> g_trans_table;
// 用于保护共享资源(g_node_pool, g_trans_table)的互斥锁
static std::mutex g_tt_mutex;

// --- 前向声明 ---
float Process_UCT_TT_ANN(Node& b, int& total, int total_moves);
Node* expand_UCT_Node_TT_ANN(Node& parent_node, int total_moves);
void Move_UCT_TT_ANN(Board& cb, int player, std::vector<POS>& pace, int total_moves);
void _Sacrifice_Move_UCT_TT_ANN(Board& CB, int Player, std::vector<POS>& pace, int total_moves);
void Situation_Move_latter_TT_ANN(Board& CB, int Player, std::vector<POS>& pace, int total_moves);
int Filter_Turn_RAND_TT_ANN(Board&CB,int Player);
float Evalution_TAKE_Filter_OF_MC_TT_ANN(Board&CB,int NextPlayer,int _Winner);

std::atomic<int> tt_probes{0};      // 置换表查询次数
std::atomic<int> tt_hits{0};         // 命中次数
std::atomic<int> tt_inserts{0};      // 插入次数
std::atomic<int64_t> tt_total_nodes{0}; // 总节点数
std::atomic<int> tt_searches{0};     // 搜索次数


// --- 核心工具函数 ---
/**
 * @brief 为棋盘状态和玩家生成唯一的键。
 * @param b 棋盘
 * @param player 当前玩家
 * @return 返回字符串键
 */
std::string generate_key(const Board &b, int player) {
    std::string key;
    key.reserve(LEN * LEN + 1);
    for (int i = 0; i < LEN; ++i) {
        for (int j = 0; j < LEN; ++j) {
            key.push_back(static_cast<char>(b._MAP_[i][j]));
        }
    }
    key.push_back(static_cast<char>(player)); // 关键修复：加入玩家信息
    return key;
}

/**
 * @brief 计算节点的UCB值 (从Node.cpp改编)
 * @param node 目标节点
 * @param total_parent_visits 父节点的访问次数
 * @return float UCB值
 */
float take_ucb_value(const Node* node, int total_parent_visits) {
    if (node->Times == 0) {
        return 1e9; // 优先探索未访问过的节点
    }
    return node->_Avg_Value + C * sqrt(log(static_cast<double>(total_parent_visits)) / node->Times);
}

/**
 * @brief 更新节点的平均价值 (从Node.cpp改编)
 * @param node 要更新的节点
 */
void refresh_avg_value(Node* node) {
    if (node->_Exist_Child == 0) return;
    
    double children_weighted_value = 0;
    int children_total_visits = 0;

    for(int i = 0; i < node->_Exist_Child; ++i){
        Node* child = node->_Child_NODES[i];
        if (child->Times > 0) {
            children_weighted_value += (1.0 - child->_Avg_Value) * child->Times;
            children_total_visits += child->Times;
        }
    }

    if (children_total_visits > 0) {
         node->_Avg_Value = children_weighted_value / children_total_visits;
    }
}

int Filter_Turn_RAND_TT_ANN(Board&CB,int Player){
    POS _Moves[60];
    int move_count = 0; // 模拟不需要编号
    CB._TAKE_All_C_Boxes(Player, move_count, false);
    Board_Box Test=CB;
    int Move_Num;
    int _Free_Edge=CB.TAKE__Free_Edge_Num();
    if(_Free_Edge<UCT__RANGE_FILTER)Move_Num=Test.TAKE_Filter_Moves(_Moves);
    else Move_Num=Test.TAKE__Free__Moves(_Moves);
    if(Move_Num!=0){
        std::uniform_int_distribution<int> dist(0, Move_Num - 1);
        int Rnd = dist(rng_tt_ANN);
        CB.move(Player,{_Moves[Rnd].first,_Moves[Rnd].second}, 0);
        return -Player;
    }else return Player;
}

int TAKE_Filter_Winner_OF_MC_TT_ANN(Board&CB,int NextPlayer){
    int player=NextPlayer;
    while(CB.TAKE__Free_Edge_Num()!=0){
        player=Filter_Turn_RAND_TT_ANN(CB,player);
    }
    return TAKE_Winner_OF_Board(CB,-player);
}

int TAKE_Filter_Winner_OF_MC_TT_ANN(Board&CB,int NextPlayer,int&Rating){
    int player=NextPlayer;
    while(CB.TAKE__Free_Edge_Num()!=0){
        player=Filter_Turn_RAND_TT_ANN(CB,player);
    }
    return TAKE_Winner_OF_Board(CB,-player,-NextPlayer,Rating);
}

void multi_thread_func_tt(Board&_MCB,std::atomic<int>&_MCE,int NextPlayer,int _Winner,int UCT__TIMES__MC,int thread_id){
    int local__MCE=0;
    for(int i=thread_id;i<UCT__TIMES__MC;i+=THREAD_COUNT){
        int CNT=0,WIN=0;
        if(TAKE_Filter_Winner_OF_MC_TT_ANN(_MCB,NextPlayer,CNT)==_Winner)WIN++;
        local__MCE+=WIN+CNT-13;
    }
    _MCE+=local__MCE;
}

float Evalution_TAKE_Filter_OF_MC_TT_ANN(Board&CB,int NextPlayer,int _Winner){
    Board _MCB=CB;
    std::atomic<int>_MCE(0);
    std::vector<std::thread>threads;
    threads.reserve(THREAD_COUNT);
    for(int i=0;i<THREAD_COUNT;i++){
        threads.emplace_back(std::thread(multi_thread_func_tt,std::ref(_MCB),std::ref(_MCE),NextPlayer,_Winner,UCT__TIMES__MC,i));
    }
    for(auto&th:threads)th.join();
    return static_cast<float>(_MCE.load())/static_cast<float>(UCT__TIMES__MC);
}
float Evalution_TAKE_Filter_OF_TT_ANN(Board&CB,int NextPlayer,int _Winner){
    //std::vector<fann_type> input(25, 0.5f);
    std::vector<fann_type> input = extract_features(CB, NextPlayer);
    //std::cout<<"input: ";
    //for(fann_type i:input) std::cout<<std::fixed<<std::setprecision(4)<<i<<" ";
    //std::cout<<std::endl;
    float output = processANN(input);//test
    return output;
}

/**
 * @brief 扩展UCT节点，集成置换表查找
 * @param parent_node 父节点
 * @return Node* 指向新的或已存在的子节点
 */
Node* expand_UCT_Node_TT_ANN(Node& parent_node, int total_moves) {
    ++tt_probes;

    // 模拟落子以获取子节点状态
    Board child_board(parent_node._MAP_);
    const POS& move_to_make = parent_node.Node_Moves[parent_node._Exist_Child];
    child_board.move(parent_node.Owner, move_to_make, 0);
    
    int child_player = -parent_node.Owner;
    int dummy_move_count = 0;
    child_board._TAKE_All_C_Boxes(child_player, dummy_move_count, false);

    // 为子节点状态生成键并查询置换表
    std::string key = generate_key(child_board, child_player);
    
    std::lock_guard<std::mutex> lk(g_tt_mutex);
    auto it = g_trans_table.find(key);

    if (it != g_trans_table.end()) {
        ++tt_hits;
        return it->second; // 节点已存在，直接复用
    }

    // 节点不存在，创建新节点
    auto new_node_ptr = std::make_unique<Node>(child_player, child_board._MAP_, false);
    Node* child_node = new_node_ptr.get();

    // 使用蒙特卡洛模拟进行初始评估
    Board eval_board(child_node->_MAP_);
    float eval_score = 0;
   //std::cout<<"total_moves: "<<total_moves<<"\n";
    if(total_moves<18){
       eval_score=Evalution_TAKE_Filter_OF_MC_TT_ANN(eval_board, child_player, parent_node.Owner);
       child_node->_Avg_Value = eval_score;
    }else{
        eval_score=Evalution_TAKE_Filter_OF_TT_ANN(eval_board, child_player, parent_node.Owner);
        child_node->_Avg_Value = eval_score/2.0f +0.5f;
    }

    // 存入内存池和置换表
    g_node_pool.push_back(std::move(new_node_ptr));
    g_trans_table[key] = child_node;

    ++tt_inserts;
    tt_total_nodes += g_node_pool.size();

    return child_node;
}
// 新增打印函数
void print_tt_stats() {
    static int call_count = 0;
    ++call_count;
    
    float hit_rate = tt_probes > 0 ? 
        static_cast<float>(tt_hits) / tt_probes : 0.0f;
    
    int64_t total_simulations = 
        static_cast<int64_t>(tt_searches.load()) * 
        THREAD_COUNT ;
    std::cerr << "[置换表状态]" << std::endl;
    std::cerr << " 查询次数: " << tt_probes << std::endl;
    std::cerr << " 命中次数: " << tt_hits << std::endl;
    std::cerr << " 命中率: " << hit_rate * 100 << "%\n";
    std::cerr << " 当前节点数: " << g_node_pool.size() << std::endl;
    std::cerr << " 搜索次数 " << total_simulations << "\n\n";
}

/**
 * @brief 带置换表的UCT搜索主流程 (从UCT.cpp改编)
 * @param b 当前节点
 * @param total 总模拟次数
 * @return float 评估价值
 */
float Process_UCT_TT_ANN(Node& b, int& total, int total_moves) {
    b.Times++;

    ++total;
    ++tt_searches;


    if (b.Board_Winner != 0) {
        b._Avg_Value = (b.Board_Winner == b.Owner) ? 0.0f : 1.0f;
        total++;
        return 1.0f - b._Avg_Value;
    }

    if (b.Total_Child == 0) { // 叶子节点且无可用移动
        total++;
        return 1.0f - b._Avg_Value;
    }

    if (b._Exist_Child < b.Total_Child) {
        Node* new_child = expand_UCT_Node_TT_ANN(b, total_moves);
        b._Child_NODES[b._Exist_Child] = new_child;
        b._Exist_Child++;
        total++;
        refresh_avg_value(&b);
        return 1.0f - new_child->_Avg_Value;
    }
    
    // 选择最佳子节点
    int best_node_idx = -1;
    double best_ucb = -1.0;
    for (int i = 0; i < b.Total_Child; ++i) {
        double ucb = take_ucb_value(b._Child_NODES[i], b.Times);
        if (ucb > best_ucb) {
            best_ucb = ucb;
            best_node_idx = i;
        }
    }

    if (best_node_idx != -1) {
        Process_UCT_TT_ANN(*b._Child_NODES[best_node_idx], total, total_moves);
        refresh_avg_value(&b);
    }
    
    return 1.0 - b._Avg_Value;
}

/**
 * @brief 使用带置换表的UCT算法进行移动决策 (从UCT.cpp改编)
 */
void Move_UCT_TT_ANN(Board& cb, int player, std::vector<POS>& pace, int total_moves) {
    // 强制吃C型格的逻辑已经移到Turn_Move_Game_TT_ANN中，这里只负责MCTS搜索和落子
    // 确保在进入此函数时，所有C型格都已被处理

    std::string root_key = generate_key(cb, player);
    Node* root_node = nullptr;

    { // 加锁以安全地访问或创建根节点
        std::lock_guard<std::mutex> lk(g_tt_mutex);
        auto it = g_trans_table.find(root_key);
        if (it != g_trans_table.end()) {
            root_node = it->second;
        } else {
            auto new_root_ptr = std::make_unique<Node>(player, cb._MAP_, false); // false, C型格已在外层处理
            root_node = new_root_ptr.get();
            g_node_pool.push_back(std::move(new_root_ptr));
            g_trans_table[root_key] = root_node;
        }
    }
    int UCT__LIMIT_TIME = total_moves < 10 ? UCT__LIMIT_TIME1 :
    (total_moves < 20 ? UCT__LIMIT_TIME2 : 
    (total_moves < 30 ?UCT__LIMIT_TIME3 : UCT__LIMIT_TIME4 )); // 根据总移动数调整时间限制
    if (root_node->Board_Winner == 0 && root_node->Total_Child > 0) {
        int total_sims = 0;
        clock_t start = clock();
        for (int i = 0; i < UCT__ALL_TIMES; ++i) {
            Process_UCT_TT_ANN(*root_node, total_sims, total_moves);
            if ((static_cast<double>(clock() - start) / CLOCKS_PER_SEC) >= UCT__LIMIT_TIME) break;
        }

        int best_node_idx = -1;
        int max_visits = -1;
        for (int i = 0; i < root_node->_Exist_Child; ++i) {
            if (root_node->_Child_NODES[i]->Times > max_visits) {
                max_visits = root_node->_Child_NODES[i]->Times;
                best_node_idx = i;
            }
        }
        
        if (best_node_idx != -1) {
            const POS& best_move = root_node->Node_Moves[best_node_idx];
            cb.move(player, best_move, total_moves + pace.size() + 1);
            pace.emplace_back(best_move);
        } else {
            // 如果由于某种原因没有可选的子节点（例如时间太短），则不走棋
        }
    }    
    print_tt_stats();

}

void _Sacrifice_Move_UCT_TT_ANN(Board&CB,int Player,vector<POS>&pace, int total_moves){
    Board_Box Dead(CB);
    bool Dead_Chain=Dead.TAKE_Dead_Chain_Exist();
    bool Dead_Circle=Dead.TAKE_Dead_Circle_Exist();
    if(Dead_Circle||Dead_Chain){
        int Box_Num_Sacrifice;
        switch(Dead_Chain){
            case true:Box_Num_Sacrifice=2;break;
            default:Box_Num_Sacrifice=4;break;
        }
        int move_count = total_moves + pace.size();
        Dead._TAKE_All_C_Boxes(Player, move_count, true);
        POS Box_Num=Dead.TAKE_Early_Rational_Box_Num();
        if(Box_Num.first-Box_Num.second<=Box_Num_Sacrifice){
            CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
            Move_UCT_TT_ANN(CB,Player,pace, total_moves);
        }else{
            POS DCMove;
            switch(Box_Num_Sacrifice){
                case 2:
                    while(true){
                        Board Test=CB;
                        Test._TAKE_C_Box(Player);
                        Board_Box Dead(Test);
                        if(Dead.TAKE_Dead_Chain_Exist()){
                            POS t=CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                            pace.emplace_back(t);
                        }else break;
                    }
                    DCMove=CB.TAKE_Double_LINK_Position(Player);
                    break;
                default:
                    while(true){
                        Board Test=CB;
                        Test._TAKE_C_Box(Player);
                        Board_Box Dead(Test);
                        if(Dead.TAKE_Dead_Circle_Exist()){
                            POS t=CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                            pace.emplace_back(t);
                        }else break;
                    }
                    DCMove=CB.TAKE_Double_LINK_Position(Player);
                    break;
            }
            CB.move(Player,{DCMove.first,DCMove.second}, total_moves + pace.size() + 1);
            pace.emplace_back(DCMove);
            while(true){
                if(!CB.TAKE__C_Box_Type_Limit(Player,pace, total_moves))break;
            }
        }
    }else{
        CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
        Move_UCT_TT_ANN(CB,Player,pace, total_moves);
    }
}

void Situation_Move_latter_TT_ANN(Board&CB,int Player,vector<POS>&pace, int total_moves){
    if(CB.TAKE_Long_C_Box_Type_Exist()){
        Board_Box Dead(CB);
        int Box_Num_Sacrifice=0;
        if(Dead.TAKE_Dead_Chain_Exist())Box_Num_Sacrifice=2;
        if(Dead.TAKE_Dead_Circle_Exist())Box_Num_Sacrifice=4;
        int move_count = total_moves + pace.size();
        Dead._TAKE_All_C_Boxes(Player, move_count, true);
        POS Box_Num=Dead.TAKE_Rational_StateBox_Num();
        if(Box_Num.first-Box_Num.second<=Box_Num_Sacrifice||Dead.Winner_TAKE_()!=EMPTY){
            CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
            if(Dead.Winner_TAKE_()==EMPTY)Situation_Move_latter_TT_ANN(CB,Player,pace, total_moves);
        }else{
            POS DCMove;
            switch(Box_Num_Sacrifice){
                case 2:
                    while(true){
                        Board Test=CB;
                        Test._TAKE_C_Box(Player);
                        Board_Box Dead(Test);
                        if(Dead.TAKE_Dead_Chain_Exist()){
                            POS tmp=CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                            pace.emplace_back(tmp);
                        }else break;
                    }
                    DCMove=CB.TAKE_Double_LINK_Position(Player);
                    break;
                default:
                    while(true){
                        Board Test=CB;
                        Test._TAKE_C_Box(Player);
                        Board_Box Dead(Test);
                        if(Dead.TAKE_Dead_Circle_Exist()){
                            POS t=CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                            pace.emplace_back(t);
                        }else break;
                    }
                    DCMove=CB.TAKE_Double_LINK_Position(Player);
                    break;
            }
            CB.move(Player,DCMove, total_moves + pace.size() + 1);
            pace.emplace_back(DCMove);
            while(true){
                if(!CB.TAKE__C_Box_Type_Limit(Player,pace, total_moves))break;
            }
        }
    }else{
        CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
        Board_Box Test(CB);
        POS p=Test.openPolicy();
        CB.move(Player,p, total_moves + pace.size() + 1);
        pace.emplace_back(p);
    }
}

} // 匿名命名空间结束

// --- 公共接口实现 ---
void Clear_UCT_TT_ANN() {
    std::lock_guard<std::mutex> lk(g_tt_mutex);
    g_trans_table.clear();
    g_node_pool.clear();
    
        // 重置调试统计
    tt_probes = 0;
    tt_hits = 0;
    tt_inserts = 0;
    tt_total_nodes = 0;
    tt_searches = 0;
    std::cerr << "[置换表已清空]" << std::endl;
}

void Turn_Move_Game_TT_ANN(Board &CB, int Player, int *status, std::vector<POS> &pace, int total_moves) {
    // 采用与旧版AI相同的逻辑分发器
    Board local_board = CB; // 创建一个本地副本进行操作，不修改原始棋盘
    Board Test = local_board;
    int move_count_dummy = 0; // 这个仅用于探测，不影响主棋盘
    Test._TAKE_All_C_Boxes(Player, move_count_dummy, false); // 在副本上探测吃C

    bool LatterSituation = (Test.TAKE_Num_of_FILTER_Move() == 0);

    if(!LatterSituation) {
        _Sacrifice_Move_UCT_TT_ANN(local_board, Player, pace, total_moves);
    } else {
        Situation_Move_latter_TT_ANN(local_board, Player, pace, total_moves);
    }
    
    *status = 1; // 标记为已完成决策
} 