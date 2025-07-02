#include "Recorder.h"
#include <fstream>
#include <nowide/fstream.hpp>
using std::fstream;
using std::ios;
#define SJ_Addstring cJSON_AddStringToObject
#define SJ_Addnumber cJSON_Add_NumberToObject
#define SJ_Creat_Obj cJSON_CreateObject
#define SJ_AddArray cJSON_AddItemToArray
#define SJ_AddObj cJSON_AddItemToObject
/**
 * @brief 构造函数初始化游戏数据
 * @param a 红方分数
 * @param b 黑方分数
 * @param p1 红方玩家名
 * @param p2 黑方玩家名
 * @note 创建JSON根节点和棋子数组，设置玩家信息和胜负状态
 */
Game_Date::Game_Date(int a,int b,const char*p1,const char*p2){
    root=SJ_Creat_Obj();piece=cJSON_CreateArray();
    SJ_Addstring(root,"R",p1);this->pf=p1;
    SJ_Addstring(root,"B",p2);this->ps=p2;
    if(a+b==25 || a>=13 || b>=13){winner=(a>b)?pf:ps;win=(a>b)?"R":"B";}
    else{winner="未分出";win="U";}
    SJ_Addstring(root,"winner",win);
    SJ_Addnumber(root,"RScore",a);
    SJ_Addnumber(root,"BScore",b);
    time_t now=time(nullptr);tm*date=localtime(&now);
    SJ_Addstring(root,"Date","2025-08-06");//此处修改时间
    SJ_Addstring(root,"Event","2025 CCGC");
}

/**
 * @brief 记录每一步棋的信息
 * @param v 存储60步棋的数组
 * @note 遍历非空棋子，生成棋子位置描述并添加到JSON数组
 */
void Game_Date::STEPsRecord(STEPs v[60]){
    int i=0;while(i<60){
        if(v[i].player==EMPTY){i++;continue;}
        std::string REG;REG+=((v[i].player==-1)?"b(":"r(");REG+=(v[i].ACTs.second/2+'a');REG+=(6-((v[i].ACTs.first+1)/2)+'0');REG+=(v[i].ACTs.first%2==0)?",h)":",v)";
        cJSON*node=SJ_Creat_Obj();
        SJ_Addstring(node,"piece",REG.c_str());
        SJ_AddArray(piece,node);i++;
    }
}

/**
 * @brief 结束游戏记录
 * @note 将棋子数组添加到根节点
 */
void Game_Date::RecordEnd(){SJ_AddObj(root,"game",piece);}

/**
 * @brief 打印生成的游戏数据到文件
 * @note 生成文件名并写入JSON数据
 */
void Game_Date::DataPrint(){
    char*json_data=cJSON_PrintUnformatted(root);
    nowide::fstream out;char buf[64];
    sprintf(buf,"AA：%s先手，%s后手，%s胜.txt",pf,ps,winner);
    out.open(buf,ios::out);out<<json_data;out.close();
}

/**
 * @brief 释放JSON节点内存
 */
void Game_Date::deleterecord(){cJSON_Delete(root);}
