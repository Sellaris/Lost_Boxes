#ifndef DAB_UCT__H
#define DAB_UCT__H
#include "Node.h"
#include "board.h"
#include <mutex>
#include <random>
#include <thread>
//====================函数声明====================
// TAKE_Winner_OF_Board: 预测当前局面胜利者
// 输入: CB-棋盘引用, Player_L-后手玩家
// 输出: 返回胜利者编号
int TAKE_Winner_OF_Board(Board&CB,int Player_L);
// TAKE_Winner_OF_Board: 预测当前局面胜利者(带父节点信息)
// 输入: CB-棋盘引用, Player_L-后手玩家, Father_O-父节点归属, Rating-得分引用
// 输出: 返回胜利者编号, Rating赋值
int TAKE_Winner_OF_Board(Board&CB,int Player_L,int Father_O,int&Rating);
// Evalution_TAKE_Filter_OF_MC: 子节点初次评估收益
// 输入: CB-棋盘引用, NextPlayer-下一个玩家, _Winner-胜者
// 输出: 返回评估值
float Evalution_TAKE_Filter_OF_MC(Board&CB,int NextPlayer,int _Winner);
// TAKE_Filter_Winner_OF_MC: 返回模拟胜负结果
// 输入: CB-棋盘引用, NextPlayer-下一个玩家
// 输出: 返回胜负结果
int TAKE_Filter_Winner_OF_MC(Board&CB,int NextPlayer);
// TAKE_Filter_Winner_OF_MC: 返回模拟胜负结果(带分数)
// 输入: CB-棋盘引用, NextPlayer-下一个玩家, Rating-得分引用
// 输出: 返回胜负结果, Rating赋值
int TAKE_Filter_Winner_OF_MC(Board&CB,int NextPlayer,int&Rating);
// Filter_Turn_RAND: 模拟时随机占边
// 输入: CB-棋盘引用, Player-玩家
// 输出: 返回操作结果
int Filter_Turn_RAND(Board&CB,int Player);
// Process_UCT: UCT_搜索主流程
// 输入: B-根节点, Total-总次数引用
// 输出: 返回UCT_评估值, Total赋值
float Process_UCT(Node&B,int&Total);
// Move_UCT: 用UCT_算法进行移动
// 输入: CB-棋盘引用, Player-玩家, pace-落子序列
// 输出: pace赋值
void Move_UCT(Board&CB,int Player,vector<POS>&pace, int total_moves);
// UCT_Node_DEL: 释放单节点空间
// 输入: Root-节点指针
// 输出: 无
void UCT_Node_DEL(Node*Root);
// UCT_Tree_DEL: 释放整棵树空间
// 输入: Root-根节点引用
// 输出: 无
void UCT_Tree_DEL(Node&Root);
// _Sacrifice_Move_UCT: 包括UCT_搜索前的预处理
// 输入: CB-棋盘引用, Player-玩家, pace-落子序列
// 输出: pace赋值
void _Sacrifice_Move_UCT(Board&CB,int Player,vector<POS>&pace, int total_moves);
// Situation_Move_latter: 后期特殊结构体决策
// 输入: CB-棋盘引用, Player-玩家, pace-落子序列
// 输出: pace赋值
void Situation_Move_latter(Board&CB,int Player,vector<POS>&pace, int total_moves);
// Turn_Move_Game: 根据前后期自动移动
// 输入: CB-棋盘引用, Player-玩家, status-状态指针, pace-落子序列
// 输出: pace赋值
void Turn_Move_Game(Board&CB,int Player,int*status,vector<POS>&pace, int total_moves);
#endif