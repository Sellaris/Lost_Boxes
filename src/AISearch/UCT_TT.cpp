#include "UCT_TT.h"
#include "Estimate.h"
#include "Params.h"
#include "Node_TT.h"
#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <ctime>
#include <mutex>
#include "NodeArena.h"

// 为多线程版本准备独立 RNG
thread_local std::mt19937_64 rng_tt(std::chrono::system_clock::now().time_since_epoch().count());

//--------------------------------------------------
//   基础工具函数  —— 与 UCT.cpp 等价
//--------------------------------------------------

// Process_UCT_TT: 单次节点搜索
float Process_UCT_TT(Node_TT &B, int &Total) {
    B.Times++;
    if (B.Board_Winner != 0) {
        B._Avg_Value = (B.Board_Winner == B.Owner) ? 0.f : 1.f;
        Total++;
        return B._Avg_Value;
    }

    if (B._Exist_Child < B.Total_Child) {
        Total++;
        B._Child_NODES[B._Exist_Child] = B.expand_UCT_Node();
        B._Exist_Child++;
        B.refresh_Avg_Value();
        return B._Avg_Value;
    } else {
        int Best_Node = 0;
        double BEST_UCB = 0;
        double UCB_Value[60];
        for (int i = 0; i < B.Total_Child; ++i) {
            UCB_Value[i] = B._Child_NODES[i]->TAKE_UCB_Value(Total);
            if (UCB_Value[i] >= BEST_UCB) {
                Best_Node = i;
                BEST_UCB = UCB_Value[i];
            }
        }
        Process_UCT_TT(*B._Child_NODES[Best_Node], Total);
        B.refresh_Avg_Value();
        return B._Avg_Value;
    }
}

//--------------------------------------------------
//   核心落子函数
//--------------------------------------------------
void Move_UCT_TT(Board &CB, int Player, std::vector<POS> &pace, int total_moves) {
    Node_TT UCT_B(Player, CB._MAP_, true);

    int UCT__LIMIT_TIME = total_moves < 10 ? UCT__LIMIT_TIME1 :
                           (total_moves < 20 ? UCT__LIMIT_TIME2 :
                           (total_moves < 30 ? UCT__LIMIT_TIME3 : UCT__LIMIT_TIME4));

    if (UCT_B.Board_Winner == 0) {
        if (UCT_B.Total_Child == 0) {
            return;
        }

        // 每线程一棵树
        std::vector<Node_TT> thread_trees;
        thread_trees.reserve(THREAD_COUNT);
        for (int i = 0; i < THREAD_COUNT; ++i) {
            thread_trees.emplace_back(UCT_B);
        }

        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        // 汇总数组，线程安全更新
        std::vector<long long> total_visits_per_move(UCT_B.Total_Child, 0);
        std::vector<double> combined_value_sum(UCT_B.Total_Child, 0.0);
        std::mutex agg_mutex;

        auto start_time = std::chrono::steady_clock::now();
        auto uct_worker = [&](Node_TT &tree_root) {
            int total_sims_thread = 0;
            while (true) {
                auto current_time = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed_seconds = current_time - start_time;
                if (elapsed_seconds.count() >= UCT__LIMIT_TIME) {
                    break;
                }
                Process_UCT_TT(tree_root, total_sims_thread);
            }
            // 线程结束前将统计数据写入全局数组
            {
                std::lock_guard<std::mutex> lk(agg_mutex);
                for (int i = 0; i < tree_root._Exist_Child; ++i) {
                    total_visits_per_move[i] += tree_root._Child_NODES[i]->Times;
                    combined_value_sum[i]   += tree_root._Child_NODES[i]->_Avg_Value * tree_root._Child_NODES[i]->Times;
                }
            }

            // 一次性释放当前线程所有 Node_TT 内存
            get_node_arena().reset();

        };

        for (int i = 0; i < THREAD_COUNT; ++i) {
            threads.emplace_back(uct_worker, std::ref(thread_trees[i]));
        }
        for (auto &th : threads) {
            th.join();
        }

        int best_node_idx = 0;
        float best_avg_value = -1.0f;
        for (int i = 0; i < UCT_B.Total_Child; ++i) {
            float current_avg_value = 0.0f;
            if (total_visits_per_move[i] > 0) {
                current_avg_value = static_cast<float>(combined_value_sum[i] / total_visits_per_move[i]);
            }
            if (current_avg_value >= best_avg_value) {
                best_avg_value = current_avg_value;
                best_node_idx = i;
            }
        }

        CB.move(Player, {UCT_B.Node_Moves[best_node_idx].first, UCT_B.Node_Moves[best_node_idx].second}, total_moves + pace.size() + 1);
        pace.emplace_back(UCT_B.Node_Moves[best_node_idx].first, UCT_B.Node_Moves[best_node_idx].second);
        print_tt_stats_TT();
        get_node_arena().reset();
    } else {
        CB._TAKE_All_C_TYPE_Boxes(Player, pace, total_moves);
        Situation_Move_latter_UCT_TT(CB, Player, pace, total_moves);
        print_tt_stats_TT();
        get_node_arena().reset();
    }
}

//--------------------------------------------------
//   前期牺牲策略（复制原版实现）
//--------------------------------------------------
void _Sacrifice_Move_UCT_TT(Board &CB, int Player, std::vector<POS> &pace, int total_moves) {
    Board_Box Dead(CB);
    bool Dead_Chain = Dead.TAKE_Dead_Chain_Exist();
    bool Dead_Circle = Dead.TAKE_Dead_Circle_Exist();
    if (Dead_Circle || Dead_Chain) {
        int Box_Num_Sacrifice = Dead_Chain ? 2 : 4;
        int move_count = total_moves + pace.size();
        Dead._TAKE_All_C_Boxes(Player, move_count, true);
        POS Box_Num = Dead.TAKE_Early_Rational_Box_Num();
        if (Box_Num.first - Box_Num.second <= Box_Num_Sacrifice) {
            CB._TAKE_All_C_TYPE_Boxes(Player, pace, total_moves);
            Move_UCT_TT(CB, Player, pace, total_moves);
        } else {
            POS DCMove;
            if (Box_Num_Sacrifice == 2) {
                while (true) {
                    Board Test = CB;
                    Test._TAKE_C_Box(Player);
                    Board_Box Dead(Test);
                    if (Dead.TAKE_Dead_Chain_Exist()) {
                        POS t = CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                        pace.emplace_back(t);
                    } else {
                        break;
                    }
                }
                DCMove = CB.TAKE_Double_LINK_Position(Player);
            } else {
                while (true) {
                    Board Test = CB;
                    Test._TAKE_C_Box(Player);
                    Board_Box Dead(Test);
                    if (Dead.TAKE_Dead_Circle_Exist()) {
                        POS t = CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                        pace.emplace_back(t);
                    } else {
                        break;
                    }
                }
                DCMove = CB.TAKE_Double_LINK_Position(Player);
            }
            CB.move(Player, {DCMove.first, DCMove.second}, total_moves + pace.size() + 1);
            pace.emplace_back(DCMove);
            while (true) {
                if (!CB.TAKE__C_Box_Type_Limit(Player, pace, total_moves)) break;
            }
        }
    } else {
        CB._TAKE_All_C_TYPE_Boxes(Player, pace, total_moves);
        Move_UCT_TT(CB, Player, pace, total_moves);
    }
}

//--------------------------------------------------
//   后期策略
//--------------------------------------------------
void Situation_Move_latter_UCT_TT(Board &CB, int Player, std::vector<POS> &pace, int total_moves) {
    if (CB.TAKE_Long_C_Box_Type_Exist()) {
        Board_Box Dead(CB);
        int Box_Num_Sacrifice = 0;
        if (Dead.TAKE_Dead_Chain_Exist()) Box_Num_Sacrifice = 2;
        if (Dead.TAKE_Dead_Circle_Exist()) Box_Num_Sacrifice = 4;
        int move_count = total_moves + pace.size();
        Dead._TAKE_All_C_Boxes(Player, move_count, true);
        POS Box_Num = Dead.TAKE_Rational_StateBox_Num();
        if (Box_Num.first - Box_Num.second <= Box_Num_Sacrifice || Dead.Winner_TAKE_() != EMPTY) {
            CB._TAKE_All_C_TYPE_Boxes(Player, pace, total_moves);
            if (Dead.Winner_TAKE_() == EMPTY) Situation_Move_latter_UCT_TT(CB, Player, pace, total_moves);
        } else {
            POS DCMove;
            if (Box_Num_Sacrifice == 2) {
                while (true) {
                    Board Test = CB;
                    Test._TAKE_C_Box(Player);
                    Board_Box Dead(Test);
                    if (Dead.TAKE_Dead_Chain_Exist()) {
                        POS tmp = CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                        pace.emplace_back(tmp);
                    } else break;
                }
                DCMove = CB.TAKE_Double_LINK_Position(Player);
            } else {
                while (true) {
                    Board Test = CB;
                    Test._TAKE_C_Box(Player);
                    Board_Box Dead(Test);
                    if (Dead.TAKE_Dead_Circle_Exist()) {
                        POS t = CB._TAKE_C_Box(Player, total_moves + pace.size() + 1);
                        pace.emplace_back(t);
                    } else break;
                }
                DCMove = CB.TAKE_Double_LINK_Position(Player);
            }
            CB.move(Player, DCMove, total_moves + pace.size() + 1);
            pace.emplace_back(DCMove);
            while (true) {
                if (!CB.TAKE__C_Box_Type_Limit(Player, pace, total_moves)) break;
            }
        }
    } else {
        CB._TAKE_All_C_TYPE_Boxes(Player, pace, total_moves);
        Board_Box Test(CB);
        POS p = Test.openPolicy();
        CB.move(Player, p, total_moves + pace.size() + 1);
        pace.emplace_back(p);
    }
}

//--------------------------------------------------
//   阶段分流
//--------------------------------------------------
void Turn_Move_Game_UCT_TT(Board &CB, int Player, int *status, std::vector<POS> &pace, int total_moves) {
    Board Test = CB;
    int move_count = total_moves + pace.size();
    Test._TAKE_All_C_Boxes(Player, move_count, true);
    bool LatterSituation = (Test.TAKE_Num_of_FILTER_Move() == 0);

    if (!LatterSituation) {
        _Sacrifice_Move_UCT_TT(CB, Player, pace, total_moves);
    } else {
        Situation_Move_latter_UCT_TT(CB, Player, pace, total_moves);
    }
    *status = 1;
} 