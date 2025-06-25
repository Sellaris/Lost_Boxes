/*
 * 新版 UCT + 置换表
 * 与旧版 UCT 接口保持一致，互不影响。
 */
#ifndef DAB_UCT_TT_ANN_H
#define DAB_UCT_TT_ANN_H
#include "UCT.h"  // 复用现有结构体、常量与辅助函数
#include <unordered_map>

// 使用置换表改进的 AI 决策接口
// 与 Turn_Move_Game 保持相同签名以便在主程序中灵活切换。
void Turn_Move_Game_TT_ANN(Board &CB, int Player, int *status, std::vector<POS> &pace, int total_moves);

// 清理UCT置换表，释放所有缓存的节点。
// 在新的一局游戏开始前调用，以避免使用旧的对局数据。
void Clear_UCT_TT_ANN();

#endif // DAB_UCT_TT_ANN_H 