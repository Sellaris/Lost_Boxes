#include "../AISearch/Params.h"
#include "cJSON.h"
#include <cstring>
#include <fstream>
#include <iostream>

/**
 * @brief 数据记录器类，用于记录游戏对局信息
 */
class Game_Date {
  public: cJSON *root, *piece, *objarr; const char *winner, *win, *pf, *ps;
    /**
     * @brief 构造函数，初始化JSON记录器
     * @param a 先手得分
     * @param b 后手得分
     * @param p1 先手玩家名字
     * @param p2 后手玩家名字
     * Game_Date(int a,int b,const char *p1,const char *p2);
     */    
    /**
     * @brief 录入步骤信息
     * @param v 步骤信息数组
     * void STEPsRecord(STEPs v[60]);
     */    
    /**
     * @brief 结束录入并保存JSON数据
     * void RecordEnd();
     */    
    /**
     * @brief 打印生成的JSON数据到文件
     * void DataPrint();
     */    
    /**
     * @brief 释放JSON节点内存
     * void deleterecord();
     */
    Game_Date(int a,int b,const char *p1,const char *p2);
    void STEPsRecord(STEPs v[60]);
    void RecordEnd();
    void DataPrint();
    void deleterecord();
};
