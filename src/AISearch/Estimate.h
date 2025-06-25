#ifndef DAB_Estimate_H
#define DAB_Estimate_H
#include "board.h"
#include "Params.h"
#pragma once
//====================函数声明与类型定义====================
// INFO__BOX_: 用于鉴定格子类型的类
// Type: [0]已被占领 [1]死格 [2]链格 [3]自由格
// CHAIN_TYPE_ID: 所属链类型的编号，从1开始自动编号
class INFO__BOX_ {
public:
    INFO__BOX_();
    int Type;
    int CHAIN_TYPE_ID;
};
// INFO_Chain: 包含一条链的基本信息的类
// Type: 链类型
// _BOX__NUM_OF_CHAIN: 链包含的格子数
// CHAIN_Start_Position, CHAIN_END_POS: 链的起止位置
// IF_Pre_Circle: 预备环的先决条件
class INFO_Chain {
public:
    INFO_Chain();
    ChainType Type;
    int _BOX__NUM_OF_CHAIN;
    POS CHAIN_Start_Position;
    POS CHAIN_END_POS;
    bool IF_Pre_Circle;
};
// Board_Box: 扩展自 Board 的后期棋盘类型
// Boxes_OF_Board_Box: 棋盘格子类型信息
// Chains_OF_Board_Box: 链的信息
class Board_Box : public Board {
public:
    Board_Box(Board NEW_BOARD);
    Board_Box(int _MAP_[LEN][LEN]);
    INFO__BOX_ Boxes_OF_Board_Box[_BOX_LEN + 1][_BOX_LEN + 1]; // 棋盘格子类型，编号从1开始
    INFO_Chain Chains_OF_Board_Box[25];                  // 链的信息，数量不超过全部格子数
    //====================链注册与判定====================
    // NEXT__BOX_: 查找某个链格
    // 输入: Box_Founded-已找到的格子, BOX_Lasted-上一个格子
    // 输出: 返回下一个链格
    POS NEXT__BOX_(POS Box_Founded, POS BOX_Lasted);
    // First_Empty_Chain__Num: 获得一个空的链编号
    int First_Empty_Chain__Num();
    // _BOX__ID: 得到某个格子的类型
    // 输入: bx, by-格子坐标
    // 输出: 返回类型编号
    int _BOX__ID(int bx, int by);
    // INHERIT_CHAIN: 一条链吞并另一条链
    // 输入: Inheritor_Reg_ID-继承者编号, Ancester_Reg_ID-被吞并者编号
    void INHERIT_CHAIN(int Inheritor_Reg_ID, int Ancester_Reg_ID);
    // REGISTER_Chain: 从自由格出发注册所有派生链
    // 输入: _Free_BOX_POS-自由格, NXT_POS-下一个格
    void REGISTER_Chain(POS _Free_BOX_POS, POS NXT_POS);
    // REGISTER_Circle: 从一个格出发，确定是否为一个环
    // 输入: CHAIN_Start_Position-起点, NXT_POS-下一个格
    void REGISTER_Circle(POS CHAIN_Start_Position, POS NXT_POS);
    // Search_From_Box: 注册所有派生链
    // 输入: BOX_POS-起始格
    void Search_From_Box(POS BOX_POS);
    // Search_Circle: 注册所有派生环
    // 输入: BOX_POS-起始格
    void Search_Circle(POS BOX_POS);
    // BoxesType_Define: 填充所有格子的类型信息
    void BoxesType_Define();
    // Chains_Define: 定义所有链
    // 输入: Chain_P-是否启用链增强
    void Chains_Define(bool Chain_P);
    // Reset_Chains_Info: 重定义链与格的信息
    void Reset_Chains_Info();
    // DEFINE_Dead_Chain: 定义死环或死链
    void DEFINE_Dead_Chain();
    // SEARCH_Dead_Chain: 搜索死链
    // 输入: BOX_POS-起始格
    void SEARCH_Dead_Chain(POS BOX_POS);
    // REGISTER_Dead_Chain: 注册死环或死链
    // 输入: _Free_BOX_POS-自由格, F1st_Position-第一个格
    void REGISTER_Dead_Chain(POS _Free_BOX_POS, POS F1st_Position);
    //====================链与环判别====================
    // TAKE_Dead_Chain_Exist: 判断死链是否存在
    // 输出: 返回bool
    bool TAKE_Dead_Chain_Exist();
    // TAKE_Dead_Circle_Exist: 判断死环是否存在
    // 输出: 返回bool
    bool TAKE_Dead_Circle_Exist();
    //====================链操作与策略====================
    // Captual_MIN_Chain: 吞并当前最短的长链或环
    // 输入: Player_L-后手玩家
    // 输出: 返回是否成功
    bool Captual_MIN_Chain(int Player_L);
    // TAKE_OpenMIN_Chain_Position: 获得打开最短长链的坐标
    POS TAKE_OpenMIN_Chain_Position();
    // openPolicy: 打开策略
    POS openPolicy();
    // TAKE_OpenMIN_Circle_Position: 获得打开最短环的坐标
    POS TAKE_OpenMIN_Circle_Position();
    // TAKE_3_Open_Chain_Position: 获得打开3链的坐标
    POS TAKE_3_Open_Chain_Position();
    //====================局面判定====================
    // State_IS_Rational_: 判断双方是否为理性状态
    // 输入: Box_Num-格子数
    // 输出: 返回bool
    static bool State_IS_Rational_(POS Box_Num);
    // TAKE_Early_Rational_Box_Num: UCT前的预处理
    POS TAKE_Early_Rational_Box_Num();
    // TAKE_Rational_StateBox_Num: 后期决策中
    POS TAKE_Rational_StateBox_Num();
    // TAKE_Winner_OF_Board: 得到或预测本局面下的胜利者
    // 输入: Player_L-后手方
    // 输出: 返回胜利者编号
    int TAKE_Winner_OF_Board(int Player_L);
    // TAKE_Winner_OF_Board: 得到或预测本局面下的胜利者(带父节点信息)
    // 输入: Player_L-后手方, Father_O-父节点归属, Rating-得分引用
    // 输出: 返回胜利者编号, Rating赋值
    int TAKE_Winner_OF_Board(int Player_L, int Father_O, int &Rating);
    //====================等价裁边====================
    // TAKE_Filter_Moves: 得到所有过滤可行边(不产生死链)
    // 输入: _Moves-可行边数组
    // 输出: 返回数量
    int TAKE_Filter_Moves(POS _Moves[60]);
    // TAKE__Free__Moves: 得到所有自由边
    // 输入: _Moves-可行边数组
    // 输出: 返回数量
    int TAKE__Free__Moves(POS _Moves[60]);
    // TAKE_Save_ChainEdgeBool: 一格短链的裁边
    // 输入: x, y-坐标, STATE-状态数组
    // 输出: 返回bool
    bool TAKE_Save_ChainEdgeBool(int x, int y, bool STATE[LEN][LEN]);
    // TAKE_Save_AngleEdgeBool: 边角格子的裁边
    // 输入: x, y-坐标, STATE-状态数组
    // 输出: 返回bool
    bool TAKE_Save_AngleEdgeBool(int x, int y, bool STATE[LEN][LEN]);
};
#endif
