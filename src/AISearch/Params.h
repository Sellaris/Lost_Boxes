/*
 *此头文件是存放一些常用的宏定义
 */
#ifndef DAB_DEFINE_H
#define DAB_DEFINE_H
#include <utility>
#include <thread>

//====================常量与类型定义====================
// WAY: 四方向数组
constexpr int WAY[4][2]={{1,0},{0,1},{-1,0},{0,-1}};
// ChainType: 链类型枚举
//   Not_Define-未定义, Long_Chain-长链, Short_Chain-短链, Circle-环, Pre_Circle-预备环, Dead_Chain-死链, Dead_Circle-死环
enum ChainType{Not_Define,Long_Chain,Short_Chain,Circle,Pre_Circle,Dead_Chain,Dead_Circle};

// TT Entry Bound Types
enum Bound {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
};

// C: UCB常数
constexpr double C=1;
// FULL_BOX_/DEAD_BOX_/CHAIN_BOX_/FREE_BOX_: 格子类型
constexpr int FULL_BOX_=0;
constexpr int DEAD_BOX_=1;
constexpr int CHAIN_BOX_=2;
constexpr int FREE_BOX_=3;

// POS: 坐标类型
using POS=std::pair<int,int>;

// STEPs: 棋谱记录结构体
//   player: 玩家
//   ACTs: 行为
struct STEPs{int player=0;POS ACTs;};

// EMPTY/BLACK/WHITE: 空/黑/白
constexpr int EMPTY=0;
constexpr int BLACK=1;
constexpr int WHITE=-1;

// DOT: 点常量
constexpr int DOT=10;

// Horizon/Vertical/OCCed_LINE: 横线/竖线/被占线
constexpr int Horizon=20;
constexpr int Vertical=30;
constexpr int OCCed_LINE=40;

// _BOX_: 格常量
constexpr int _BOX_=0;

// LEN/_BOX_LEN/_BOX_NUM: 棋盘/格子长度与数量
constexpr int LEN=11;
constexpr int _BOX_LEN=5;
constexpr int _BOX_NUM=25;

// UCT_参数    
constexpr size_t TT_MAX_SIZE_ = 10000000;// 置换表最大大小 1e7局面
const int UCT__TIMES__MC=1;
constexpr int UCT__ALL_TIMES=2000000000;
constexpr int UCT__LIMIT_TIME1=10;
constexpr int UCT__LIMIT_TIME2=40; 
constexpr int UCT__LIMIT_TIME3=60;
constexpr int UCT__LIMIT_TIME4=20;
constexpr int UCT__RANGE_FILTER=25;
//60 + 150 + 300 + 30*10 = 610
// 多线程参数
const int THREAD_COUNT=10; // 新增线程数常量

// Alpha-Beta 迭代加深参数
constexpr int AB_LIMIT_TIME1 = 10;    // 0-9步时限
constexpr int AB_LIMIT_TIME2 = 50;    // 10-19步时限
constexpr int AB_LIMIT_TIME3 = 60;    // 20-29步时限
constexpr int AB_LIMIT_TIME4 = 30;    // 30步以后时限
constexpr int AB_MAX_DEPTH = 20;     // 迭代加深最大深度

#endif