#ifndef DAB_BOARD_H
#define DAB_BOARD_H
#include "Params.h"
#include <functional>
#include <vector>
using std::vector;
//====================Board类声明====================
// Board: 棋盘类，包含棋盘状态与操作
class Board{
  public:
    int _MAP_[11][11]{}, BOX_of_BLACK = 0, BOX_of_WHITE = 0;
    int _MOVE_NUMBERS_[11][11]{};
    // Board(): 默认构造
    Board();
    // Board(int ARRAY[LEN][LEN]): 参数构造
    Board(int ARRAY[LEN][LEN]);
    // setBoard: 设置局面
    // 输入: ARRAY-棋盘数组
    void setBoard(int ARRAY[LEN][LEN]);
    // move: 下棋
    // 输入: player-玩家, l-落子
    // 输出: 返回操作结果
    int move(int player, POS l, int move_num = 0);
    // undo_move_sim: 模拟悔棋
    // 输入: l-落子
    void undo_move_sim(POS l);
    // _TAKE_C_Box: 吃C型格
    // 输入: player-玩家
    // 输出: 返回操作坐标
    POS _TAKE_C_Box(int player, int move_num = 0);
    // _TAKE_All_C_TYPE_Boxes: 吃掉所有C型格
    // 输入: Player-玩家, pace-落子序列
    void _TAKE_All_C_TYPE_Boxes(int Player, vector<POS> &pace, int total_moves);
    // _TAKE_All_C_Boxes: 吃掉所有C型格(无序列)
    // 输入: Player-玩家
    void _TAKE_All_C_Boxes(int Player, int& move_count, bool do_numbering);
    // TAKE__Free___BOX__TRUE: 判断是否为自由格
    // 输入: bx,xy-坐标
    // 输出: 返回bool
    bool TAKE__Free___BOX__TRUE(int bx, int xy);
    // TAKE_Boxes_OF_Players: 得到玩家占领格子数
    // 输入: player-玩家
    // 输出: 返回数量
    int TAKE_Boxes_OF_Players(int player) const;
    // TAKE__Free_dom: 得到自由度
    // 输入: x,y-坐标
    // 输出: 返回自由度
    int TAKE__Free_dom(int x, int y);
    int TAKE__Free_dom(POS l);
    // TAKE__Free_Edge_Num: 得到自由边数量
    int TAKE__Free_Edge_Num();
    // TAKE__C_Type__BOX__TRUE: 判断是否有C型格
    bool TAKE__C_Type__BOX__TRUE(int bx, int by);
    // TAKE_Long_C_Type__BOX__TRUE: 判断是否为长死格起点
    bool TAKE_Long_C_Type__BOX__TRUE(int bx, int by);
    // TAKE_Long_C_Box_Type_Exist: 判断是否存在长C型链
    bool TAKE_Long_C_Box_Type_Exist();
    // EARN_BOX_TRUE: 是否赢得格子
    bool EARN_BOX_TRUE(POS l);
    // Safe_Edge_TRUE: 是否有安全边
    bool Safe_Edge_TRUE();
    // is_Free_Line: 是否为自由边
    bool is_Free_Line(POS l) const;
    bool is_Free_Line(int i, int j) const;
    // MAKE_C_Box_TRUE: 是否制造C型格
    bool MAKE_C_Box_TRUE(POS l);
    // Winner_TAKE_: 返回赢家
    int Winner_TAKE_() const;
    // END_TRUE: 判断游戏是否结束
    bool END_TRUE() const;
    // Public_Side_GET: 获得两个格子的公共边
    POS Public_Side_GET(POS a, POS b);
    // TAKE_Double_LINK_Position: 查找doublecross制作方法
    POS TAKE_Double_LINK_Position(int Player);
    // TAKE__C_Box_Type_Limit: 限制版吃C型格
    bool TAKE__C_Box_Type_Limit(int Player, vector<POS> &pace, int total_moves);
    // TAKE_Num_of_FILTER_Move: 得到所有过滤可行边数量
    int TAKE_Num_of_FILTER_Move();
};
// Odd_TRUE: 判断奇数
bool Odd_TRUE(int num);
// Even_TRUE: 判断偶数
bool Even_TRUE(int num);
// COPY_BOARD: 棋盘复制
void COPY_BOARD(int SOUR[LEN][LEN], int TAR[LEN][LEN]);

#endif