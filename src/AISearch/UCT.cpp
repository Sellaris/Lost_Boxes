#include "Params.h"
#include "board.h"
#include "Node.h"
#include "Estimate.h"
#include "UCT.h"
#include <iostream>
#include <atomic>
#include <random>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include "NodeArena.h"
thread_local std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count()*_threadid);


/**
 * 判断棋盘胜利者
 * 检查当前棋盘状态是否已经结束并返回胜利者
 * 输入：CB - 棋盘对象，Player_L - 玩家标识
 * 输出：胜利者标识（EMPTY表示未结束）
 * 算法：检查是否还有可用移动，如果没有则判断胜利者
 */
int TAKE_Winner_OF_Board(Board&CB,int Player_L){
    Board_Box Advanced(CB);
    return Advanced.TAKE_Num_of_FILTER_Move()!=0?EMPTY:Advanced.TAKE_Winner_OF_Board(Player_L);
}

/**
 * 判断棋盘胜利者并计算评分
 * 检查当前棋盘状态并计算父节点拥有者的格子数
 * 输入：CB - 棋盘对象，Player_L - 玩家标识，Father_O - 父节点拥有者，Rating - 评分引用
 * 输出：胜利者标识（0表示未结束）
 * 算法：检查是否还有可用移动，计算父节点拥有者的格子数作为评分
 */
int TAKE_Winner_OF_Board(Board&CB,int Player_L,int Father_O,int&Rating){
    Board_Box Advanced(CB);
    if(Advanced.TAKE_Num_of_FILTER_Move()!=0){
        Rating=Advanced.TAKE_Boxes_OF_Players(Father_O);
        return 0;
    }
    return Advanced.TAKE_Winner_OF_Board(Player_L,Father_O,Rating);
}

/**
 * 蒙特卡洛模拟获取胜利者
 * 使用随机策略进行蒙特卡洛模拟直到游戏结束
 * 输入：CB - 棋盘对象，NextPlayer - 下一个玩家
 * 输出：胜利者标识
 * 算法：循环执行随机移动直到没有可用移动，然后判断胜利者
 */
int TAKE_Filter_Winner_OF_MC(Board&CB,int NextPlayer){
    int player=NextPlayer;
    while(CB.TAKE__Free_Edge_Num()!=0){
        player=Filter_Turn_RAND(CB,player);
    }
    return TAKE_Winner_OF_Board(CB,-player);
}

/**
 * 蒙特卡洛模拟获取胜利者并计算评分
 * 使用随机策略进行蒙特卡洛模拟并计算父节点拥有者的格子数
 * 输入：CB - 棋盘对象，NextPlayer - 下一个玩家，Rating - 评分引用
 * 输出：胜利者标识
 * 算法：循环执行随机移动直到没有可用移动，然后判断胜利者并计算评分
 */
int TAKE_Filter_Winner_OF_MC(Board&CB,int NextPlayer,int&Rating){
    int player=NextPlayer;
    while(CB.TAKE__Free_Edge_Num()!=0){
        player=Filter_Turn_RAND(CB,player);
    }
    return TAKE_Winner_OF_Board(CB,-player,-NextPlayer,Rating);
}

/*
// * 多线程蒙特卡洛模拟函数
// * 在指定线程中执行蒙特卡洛模拟
// * 输入：_MCB - 棋盘副本，_MCE - 原子计数器，NextPlayer - 下一个玩家，_Winner - 目标胜利者，UCT__TIMES__MC - 模拟次数，thread_id - 线程ID
// * 输出：无
// * 算法：根据线程ID分配模拟任务，执行指定次数的蒙特卡洛模拟并累加结果
 
void multi_thread_func(Board _MCB,std::atomic<int>&_MCE,int NextPlayer,int _Winner,int UCT__TIMES__MC,int thread_id){
    int local__MCE=0;
    for(int i=thread_id;i<UCT__TIMES__MC;i+=THREAD_COUNT){
        int CNT=0,WIN=0;
        Board COPY_LOCAL_MCB= _MCB;
        if(TAKE_Filter_Winner_OF_MC(COPY_LOCAL_MCB,NextPlayer,CNT)==_Winner)WIN++;
        local__MCE+=WIN+CNT-13;
    }
    _MCE+=local__MCE;
}


// * 蒙特卡洛评估函数
// * 使用多线程进行蒙特卡洛模拟评估
// * 输入：CB - 棋盘对象，NextPlayer - 下一个玩家，_Winner - 目标胜利者
// * 输出：评估分数
// * 算法：创建多个线程并行执行蒙特卡洛模拟，最后计算平均分数
 
float Evalution_TAKE_Filter_OF_MC(Board&CB,int NextPlayer,int _Winner){
    //Board _MCB=CB;
    std::atomic<int>_MCE(0);
    std::vector<std::thread>threads;
    threads.reserve(THREAD_COUNT);
    for(int i=0;i<THREAD_COUNT;i++){
        threads.emplace_back(std::thread(multi_thread_func,CB,std::ref(_MCE),NextPlayer,_Winner,UCT__TIMES__MC,i));
    }
    for(auto&th:threads)th.join();
    return static_cast<float>((float)_MCE.load())/(float)static_cast<float>(UCT__TIMES__MC);
}
*/
/**
 * 蒙特卡洛评估函数（单线程版）
 * 输入：CB - 棋盘对象，NextPlayer - 下一个玩家，_Winner - 目标胜利者
 * 输出：评估分数
 * 算法：顺序执行指定次数的蒙特卡洛模拟，最后计算平均分数
 */
float Evalution_TAKE_Filter_OF_MC(Board&CB, int NextPlayer, int _Winner) {
    int MCE = 0;
    for (int i = 0; i < UCT__TIMES__MC; ++i) {
        int CNT = 0, WIN = 0;
        Board COPY_LOCAL_MCB = CB;
        if (TAKE_Filter_Winner_OF_MC(COPY_LOCAL_MCB, NextPlayer, CNT) == _Winner) WIN++;
        MCE += WIN + CNT - 13;
    }
    return static_cast<float>(MCE) / static_cast<float>(UCT__TIMES__MC);
}
/**
 * UCT树搜索处理函数
 * 执行UCT算法的核心搜索逻辑
 * 输入：B - 当前节点，Total - 总访问次数引用
 * 输出：当前节点的平均价值
 * 算法：根据节点状态选择扩展、模拟或选择操作，递归处理子节点
 */
float Process_UCT(Node&B,int&Total){
    B.Times++;
    if(B.Board_Winner!=0){
        B._Avg_Value=B.Board_Winner==B.Owner?0:1;
        Total++;
        return B._Avg_Value;
    }
    if(B._Exist_Child<B.Total_Child){
        Total++;
        B._Child_NODES[B._Exist_Child]=B.expand_UCT_Node();
        B._Exist_Child++;
        B.refresh_Avg_Value();
        return B._Avg_Value;
    }else{
        int Best_Node=0;
        double BEST_UCB=0;
        double UCB_Value[60];
        for(int i=0;i<B.Total_Child;i++){
            UCB_Value[i]=B._Child_NODES[i]->TAKE_UCB_Value(Total);
            if(UCB_Value[i]>=BEST_UCB){
                Best_Node=i;
                BEST_UCB=UCB_Value[i];
            }
        }
        Process_UCT(*B._Child_NODES[Best_Node],Total);
        B.refresh_Avg_Value();
        return B._Avg_Value;
    }
}

/**
 * UCT移动决策函数
 * 使用UCT算法为当前局面选择最佳移动
 * 输入：CB - 棋盘对象，Player - 当前玩家，pace - 移动记录向量
 * 输出：无
 * 算法：创建UCT根节点，执行指定次数的搜索，选择最佳子节点对应的移动
 */
void Move_UCT(Board&CB,int Player,vector<POS>&pace, int total_moves){
    Node UCT_B(Player,CB._MAP_,true);
    int UCT__LIMIT_TIME = total_moves <= 10 ? UCT__LIMIT_TIME1 :
                           (total_moves <= 20 ? UCT__LIMIT_TIME2 :
                           (total_moves <= 30 ? UCT__LIMIT_TIME3 : UCT__LIMIT_TIME4));

    if(UCT_B.Board_Winner==0){
        if (UCT_B.Total_Child == 0) {
            return;
        }
        std::vector<Node> thread_trees;
        thread_trees.reserve(THREAD_COUNT);
        std::vector<long long> total_visits_per_move(UCT_B.Total_Child, 0);
        std::vector<double> combined_value_sum(UCT_B.Total_Child, 0.0);
        std::mutex agg_mutex;
        for(int i = 0; i < THREAD_COUNT; ++i) {
            thread_trees.emplace_back(UCT_B);
        }
        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        auto start_time = std::chrono::steady_clock::now();
        auto uct_worker = [&](Node& tree_root) {
            int total_sims_thread = 0;
            while (true) {
                auto current_time = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed_seconds = current_time - start_time;
                if (elapsed_seconds.count() >= UCT__LIMIT_TIME) {
                    break;
                }
                Process_UCT(tree_root, total_sims_thread);
            }
            {
                std::lock_guard<std::mutex> lk(agg_mutex);
                for (int i = 0; i < tree_root._Exist_Child; ++i) {
                    total_visits_per_move[i] += tree_root._Child_NODES[i]->Times;
                    combined_value_sum[i] += tree_root._Child_NODES[i]->_Avg_Value * tree_root._Child_NODES[i]->Times;
                }
            }
            get_node_arena().reset();
        };
        for(int i = 0; i < THREAD_COUNT; ++i){
            threads.emplace_back(uct_worker, std::ref(thread_trees[i]));
        }
        for(auto& th : threads) {
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
        CB.move(Player,{UCT_B.Node_Moves[best_node_idx].first,UCT_B.Node_Moves[best_node_idx].second}, total_moves + pace.size() + 1);
        pace.emplace_back(UCT_B.Node_Moves[best_node_idx].first,UCT_B.Node_Moves[best_node_idx].second);
        get_node_arena().reset();
    }else{
        CB._TAKE_All_C_TYPE_Boxes(Player,pace, total_moves);
        Situation_Move_latter(CB,Player,pace, total_moves);
        get_node_arena().reset();
    }
}




/**
 * 牺牲移动UCT决策函数
 * 在存在死链或死环时进行牺牲策略的UCT决策
 * 输入：CB - 棋盘对象，Player - 当前玩家，pace - 移动记录向量
 * 输出：无
 * 算法：检测死链或死环，根据牺牲后的理性状态决定是否牺牲，执行相应的移动策略
 */
void _Sacrifice_Move_UCT(Board&CB,int Player,vector<POS>&pace, int total_moves){
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
            Move_UCT(CB,Player,pace, total_moves);
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
        Move_UCT(CB,Player,pace, total_moves);
    }
}

/**
 * 后期局面移动函数
 * 处理游戏后期只有长链和环的情况
 * 输入：CB - 棋盘对象，Player - 当前玩家，pace - 移动记录向量
 * 输出：无
 * 算法：检测长链类型，根据牺牲策略决定是否牺牲，执行相应的移动
 */
void Situation_Move_latter(Board&CB,int Player,vector<POS>&pace, int total_moves){
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
            if(Dead.Winner_TAKE_()==EMPTY)Situation_Move_latter(CB,Player,pace, total_moves);
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

/**
 * 游戏回合移动函数
 * 根据游戏阶段自动选择合适的移动策略
 * 输入：CB - 棋盘对象，Player - 当前玩家，status - 状态指针，pace - 移动记录向量
 * 输出：无
 * 算法：判断是否为后期局面，选择相应的移动策略
 */
void Turn_Move_Game(Board&CB,int Player,int*status,vector<POS>&pace, int total_moves){
    Board Test=CB;
    int move_count = total_moves + pace.size();
    Test._TAKE_All_C_Boxes(Player, move_count, true);
    bool LatterSituation=(Test.TAKE_Num_of_FILTER_Move()==0);
    if(!LatterSituation)_Sacrifice_Move_UCT(CB,Player,pace, total_moves);
    else Situation_Move_latter(CB,Player,pace, total_moves);
    *status=1;
}

/**
 * 随机过滤移动函数
 * 在蒙特卡洛模拟中执行随机移动
 * 输入：CB - 棋盘对象，Player - 当前玩家
 * 输出：下一个玩家标识
 * 算法：根据自由边数量选择移动策略，执行随机移动
 */
int Filter_Turn_RAND(Board&CB,int Player){
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
        int Rnd = dist(rng);
        CB.move(Player,{_Moves[Rnd].first,_Moves[Rnd].second}, 0);
        return -Player;
    }else return Player;
} 