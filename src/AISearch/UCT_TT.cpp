#include "UCT_TT.h"
#include "Estimate.h"
#include "Node.h"
#include <string>
#include <mutex>
#include <vector>
#include <memory>
#include <atomic>
#include <chrono>
#include <thread>
#include <iostream>
#include <cmath>
#include <algorithm> // For std::max/min
#include <ctime>

// 为TT版本的UCT提供独立的线程局部随机数生成器 (保留以备将来使用或用于其他随机化)
thread_local std::mt19937_64 rng_tt(std::chrono::system_clock::now().time_since_epoch().count());

namespace { // 匿名命名空间，用于内部实现

// --- 置换表与内存管理 ---
static std::vector<std::unique_ptr<Node>> g_node_pool;
static std::unordered_map<std::string, Node*> g_trans_table;
static std::mutex g_tt_mutex;

// --- 前向声明 ---
int alpha_beta_tt(Node& node, int depth, int alpha, int beta, bool is_maximizing_player);
Node* create_child_node(Node& parent_node, const POS& move);
void Move_UCT_TT(Board& cb, int player, std::vector<POS>& pace, int total_moves);
void _Sacrifice_Move_UCT_TT(Board& CB, int Player, std::vector<POS>& pace, int total_moves);
void Situation_Move_latter_TT(Board& CB, int Player, std::vector<POS>& pace, int total_moves);

// --- 统计变量 ---
std::atomic<int> tt_probes{0};
std::atomic<int> tt_hits{0};
std::atomic<int> tt_inserts{0};
std::atomic<int64_t> tt_nodes_created{0};


// --- 核心工具函数 ---
std::string generate_key(const Board &b, int player) {
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

/**
 * @brief 创建子节点，集成置换表查找
 * @param parent_node 父节点
 * @param move 要应用的移动
 * @return Node* 指向新的或已存在的子节点
 */
Node* create_child_node(Node& parent_node, const POS& move) {
    Board child_board(parent_node._MAP_);
    child_board.move(parent_node.Owner, move, 0);
    
    int child_player = -parent_node.Owner;

    std::string key = generate_key(child_board, child_player);
    
    std::lock_guard<std::mutex> lk(g_tt_mutex);
    auto it = g_trans_table.find(key);

    if (it != g_trans_table.end()) {
        return it->second;
    }

    auto new_node_ptr = std::make_unique<Node>(child_player, child_board._MAP_, false);
    Node* child_node = new_node_ptr.get();
    
    g_node_pool.push_back(std::move(new_node_ptr));
    g_trans_table[key] = child_node;

    ++tt_inserts;
    ++tt_nodes_created;

    return child_node;
}

// 打印置换表统计信息
void print_tt_stats() {
    float hit_rate = tt_probes > 0 ? static_cast<float>(tt_hits) / tt_probes : 0.0f;
    std::cerr << "[置换表状态]" << std::endl;
    std::cerr << " 查询次数: " << tt_probes << std::endl;
    std::cerr << " 命中次数: " << tt_hits << std::endl;
    std::cerr << " 命中率: " << hit_rate * 100 << "%\n";
    std::cerr << " 当前节点数: " << g_node_pool.size() << std::endl;
    std::cerr << " 创建节点数: " << tt_nodes_created << "\n\n";
}

/**
 * @brief 使用Alpha-Beta搜索和置换表的AI核心逻辑
 * @param node 当前节点
 * @param depth 搜索深度
 * @param alpha alpha值
 * @param beta beta值
 * @param is_maximizing_player 是否是最大化玩家
 * @return int 局面估值
 */
int alpha_beta_tt(Node& node, int depth, int alpha, int beta, bool is_maximizing_player) {
    ++tt_probes;
    std::string key = generate_key(node._MAP_, node.Owner);
    
    if (node.Times > 0 && node.search_depth >= depth) { 
        ++tt_hits;
        return node._Avg_Value;
    }

    if (node.Board_Winner != 0) {
        int score = (node.Board_Winner == node.Owner) ? 10000 : -10000;
        node._Avg_Value = score;
        node.Times = 1; 
        node.search_depth = depth;
        return score;
    }

    if (depth == 0 || node.Total_Child == 0) {
        Board_Box bbox(node._MAP_);
        int score = bbox.evaluate_v2();
        node._Avg_Value = score;
        node.Times = 1; 
        node.search_depth = depth;
        return score;
    }

    if (is_maximizing_player) {
        int max_eval = -1e9;
        for (int i = 0; i < node.Total_Child; ++i) {
            Node* child_node = create_child_node(node, node.Node_Moves[i]);
            node._Child_NODES[i] = child_node;
            int eval = alpha_beta_tt(*child_node, depth - 1, alpha, beta, false);
            max_eval = std::max(max_eval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        node._Avg_Value = max_eval;
        node.Times = 1;
        node.search_depth = depth;
        g_trans_table[key] = &node;
        return max_eval;
    } else { 
        int min_eval = 1e9;
        for (int i = 0; i < node.Total_Child; ++i) {
            Node* child_node = create_child_node(node, node.Node_Moves[i]);
            node._Child_NODES[i] = child_node;
            int eval = alpha_beta_tt(*child_node, depth - 1, alpha, beta, true);
            min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        node._Avg_Value = min_eval;
        node.Times = 1;
        node.search_depth = depth;
        g_trans_table[key] = &node;
        return min_eval;
    }
}


/**
 * @brief 使用估值+UCT+TT的AI决策主函数
 */
void Move_UCT_TT(Board& cb, int player, std::vector<POS>& pace, int total_moves) {
    std::string root_key = generate_key(cb, player);
    Node* root_node = nullptr;

    {
        std::lock_guard<std::mutex> lk(g_tt_mutex);
        auto it = g_trans_table.find(root_key);
        if (it != g_trans_table.end()) {
            root_node = it->second;
        } else {
            auto new_root_ptr = std::make_unique<Node>(player, cb._MAP_, false);
            root_node = new_root_ptr.get();
            g_node_pool.push_back(std::move(new_root_ptr));
            g_trans_table[root_key] = root_node;
            ++tt_nodes_created;
        }
    }

    if (root_node->Board_Winner == 0 && root_node->Total_Child > 0) {
        // "放弃控制"逻辑的移动排序
        {
            std::vector<POS> good_moves;
            std::vector<POS> bad_sacrifices;
            std::vector<POS> other_moves;
            
            for (int i = 0; i < root_node->Total_Child; ++i) {
                POS move = root_node->Node_Moves[i];
                Board test_board = cb;
                int boxes_before = test_board.TAKE_Boxes_OF_Players(root_node->Owner);
                test_board.move(root_node->Owner, move, 0);
                int boxes_after = test_board.TAKE_Boxes_OF_Players(root_node->Owner);

                if (boxes_after > boxes_before) { 
                    good_moves.push_back(move);
                } else { 
                    Board_Box next_bbox(test_board._MAP_);
                    int opponent_boxes_before = next_bbox.TAKE_Boxes_OF_Players(-root_node->Owner);
                    std::vector<POS> dummy_pace;
                    next_bbox._TAKE_All_C_TYPE_Boxes(-root_node->Owner, dummy_pace, 0);
                    int opponent_boxes_after = next_bbox.TAKE_Boxes_OF_Players(-root_node->Owner);

                    bool is_loop_sacrifice = (opponent_boxes_after - opponent_boxes_before) > 1;

                    next_bbox.Chains_Define(true);
                    int cv_after_sacrifice = next_bbox.control_value();
                    
                    bool abandon_control = is_loop_sacrifice ? (cv_after_sacrifice <= 2) : (cv_after_sacrifice <= 4);

                    if (abandon_control) {
                        bad_sacrifices.push_back(move);
                    } else {
                        other_moves.push_back(move);
                    }
                }
            }
            
            std::vector<POS> ordered_moves;
            ordered_moves.insert(ordered_moves.end(), good_moves.begin(), good_moves.end());
            ordered_moves.insert(ordered_moves.end(), other_moves.begin(), other_moves.end());
            ordered_moves.insert(ordered_moves.end(), bad_sacrifices.begin(), bad_sacrifices.end());
            
            if(ordered_moves.size() == root_node->Total_Child){
                for (int i = 0; i < root_node->Total_Child; ++i) {
                    root_node->Node_Moves[i] = ordered_moves[i];
                }
            }
        }

        clock_t start_time = clock();
        double time_limit = total_moves < 10 ? AB_LIMIT_TIME1 :
                           (total_moves < 20 ? AB_LIMIT_TIME2 : 
                           (total_moves < 30 ? AB_LIMIT_TIME3 : AB_LIMIT_TIME4));

        int best_node_idx = -1;

        for (int depth = 1; depth <= AB_MAX_DEPTH; ++depth) {
            for(int i = 0; i < root_node->Total_Child; ++i) {
                if ((double)(clock() - start_time) / CLOCKS_PER_SEC >= time_limit) {
                    break;
                }
                Node* child = create_child_node(*root_node, root_node->Node_Moves[i]);
                root_node->_Child_NODES[i] = child;
                alpha_beta_tt(*child, depth - 1, -1e9, 1e9, false);
            }

            int current_best_node_idx = -1;
            int best_score = 1e9;
            
            for (int i = 0; i < root_node->Total_Child; ++i) {
                if (root_node->_Child_NODES[i] && root_node->_Child_NODES[i]->_Avg_Value < best_score) {
                    best_score = root_node->_Child_NODES[i]->_Avg_Value;
                    current_best_node_idx = i;
                }
            }

            if (current_best_node_idx != -1) {
                best_node_idx = current_best_node_idx;
            }

            // 添加日志：报告当前完成的搜索深度和时间
            std::cerr << "Finished search at depth " << depth 
                      << ". Time elapsed: " << (double)(clock() - start_time) / CLOCKS_PER_SEC << "s." << std::endl;

            if ((double)(clock() - start_time) / CLOCKS_PER_SEC >= time_limit) {
                std::cerr << "Time limit reached, stopping at depth " << depth << std::endl;
                break;
            }
        }

        if (best_node_idx != -1) {
            const POS& best_move = root_node->Node_Moves[best_node_idx];
            // 获取最佳分数用于日志记录
            int best_score = (root_node->_Child_NODES[best_node_idx]) ? root_node->_Child_NODES[best_node_idx]->_Avg_Value : -9999;
            // 添加日志：报告AI最终选择
            std::cerr << "==> AI choosing move (" << best_move.first << "," << best_move.second 
                      << ") with score: " << best_score << std::endl;
            cb.move(player, best_move, total_moves + pace.size() + 1);
            pace.emplace_back(best_move);
        } else if (root_node->Total_Child > 0) {
             const POS& best_move = root_node->Node_Moves[0];
             // 添加日志：报告AI选择了备用走法
             std::cerr << "==> AI choosing fallback move (" << best_move.first << "," << best_move.second << ")" << std::endl;
             cb.move(player, best_move, total_moves + pace.size() + 1);
             pace.emplace_back(best_move);
        }
    }    
    print_tt_stats();
}

void _Sacrifice_Move_UCT_TT(Board&CB,int Player,vector<POS>&pace, int total_moves){
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
            Move_UCT_TT(CB,Player,pace, total_moves);
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
            CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
        }
    }else{
        CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
        Move_UCT_TT(CB,Player,pace, total_moves);
    }
}

void Situation_Move_latter_TT(Board&CB,int Player,vector<POS>&pace, int total_moves){
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
            if(Dead.Winner_TAKE_()==EMPTY)Situation_Move_latter_TT(CB,Player,pace, total_moves);
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
            CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
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
void Clear_UCT_TT() {
    std::lock_guard<std::mutex> lk(g_tt_mutex);
    g_trans_table.clear();
    g_node_pool.clear();
    
    tt_probes = 0;
    tt_hits = 0;
    tt_inserts = 0;
    tt_nodes_created = 0;
    std::cerr << "[置换表已清空]" << std::endl;
}

void Turn_Move_Game_TT(Board &CB, int Player, int *status, std::vector<POS> &pace, int total_moves) {
    Board Test = CB;
    int move_count = total_moves + pace.size();
    Test._TAKE_All_C_Boxes(Player, move_count, true);

    bool LatterSituation = (Test.TAKE_Num_of_FILTER_Move() == 0);

    if(!LatterSituation) {
        _Sacrifice_Move_UCT_TT(CB, Player, pace, total_moves);
    } else {
        Situation_Move_latter_TT(CB, Player, pace, total_moves);
    }
    
    *status = 1;
}