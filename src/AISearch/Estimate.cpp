#include "Estimate.h"

/**
 * @brief INFO__BOX_默认构造函数
 * @details 初始化格子信息，设置默认类型和链类型ID
 * @return 无返回值
 */
INFO__BOX_::INFO__BOX_() {
	Type=0;
	CHAIN_TYPE_ID=0;
}

/**
 * @brief INFO_Chain默认构造函数
 * @details 初始化链信息，设置默认类型、格子数和预备环标志
 * @return 无返回值
 */
INFO_Chain::INFO_Chain() {
	Type=Not_Define;
	_BOX__NUM_OF_CHAIN=0;
	IF_Pre_Circle=false;
}

/**
 * @brief Board_Box构造函数，基于Board对象
 * @param NEW_BOARD 新的棋盘对象
 * @return 无返回值
 */
Board_Box::Board_Box(Board NEW_BOARD):Board(NEW_BOARD) {
}

/**
 * @brief Board_Box构造函数，基于数组
 * @param m 棋盘数组
 * @return 无返回值
 */
Board_Box::Board_Box(int m[LEN][LEN]):Board(m) {
}

/**
 * @brief 重置链与格的信息，不使用0号空间
 * @details 初始化所有格子的链类型ID和链信息
 * @return 无返回值
 */
void Board_Box::Reset_Chains_Info() {
	for(int y=1;y<=_BOX_LEN;y++) {
		for(int x=1;x<=_BOX_LEN;x++) {
			Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID=0;
		}
	}
	for(int i=1;i<25;i++) {
		Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN=0;
		Chains_OF_Board_Box[i].Type=Not_Define;
		Chains_OF_Board_Box[i].IF_Pre_Circle=false;
	}
}

/**
 * @brief 获取过滤后的移动
 * @param _Moves 移动数组
 * @return int 移动数量
 */
int Board_Box::TAKE_Filter_Moves(POS _Moves[60]) {
	bool STATE[LEN][LEN]={false};
	int Move_Num=0;
	for(int x=0;x<11;x+=2) {
		for(int y=1;y<11;y+=2) {
			if(_MAP_[x][y]==Horizon) {
				int BoardSave[LEN][LEN];
				COPY_BOARD(_MAP_,BoardSave);
				move(BLACK,{x,y});
				if(x==0) {
					if(!TAKE_Long_C_Type__BOX__TRUE(x+1,y)) {
						Board_Box test(BoardSave);
						if(test.TAKE_Save_ChainEdgeBool(x+1,y,STATE)&&test.TAKE_Save_AngleEdgeBool(x+1,y,STATE)) {
							_Moves[Move_Num]={x,y};
							Move_Num++;
						}
					}
				} else if(x==LEN-1) {
					if(!TAKE_Long_C_Type__BOX__TRUE(x-1,y)) {
						Board_Box test(BoardSave);
						if(test.TAKE_Save_ChainEdgeBool(x-1,y,STATE)&&test.TAKE_Save_AngleEdgeBool(x-1,y,STATE)) {
							_Moves[Move_Num]={x,y};
							Move_Num++;
						}
					}
				} else {
					if(!TAKE_Long_C_Type__BOX__TRUE(x+1,y)&&!TAKE_Long_C_Type__BOX__TRUE(x-1,y)) {
						Board_Box test(BoardSave);
						if(test.TAKE_Save_ChainEdgeBool(x+1,y,STATE)&&test.TAKE_Save_ChainEdgeBool(x-1,y,STATE)) {
							_Moves[Move_Num]={x,y};
							Move_Num++;
						}
					}
				}
				setBoard(BoardSave);
				STATE[x][y]=true;
			}
		}
	}
	for(int x=1;x<11;x+=2) {
		for(int y=0;y<11;y+=2) {
			if(_MAP_[x][y]==Vertical) {
				int BoardSave[LEN][LEN];
				COPY_BOARD(_MAP_,BoardSave);
				move(BLACK,{x,y});
				if(y==0) {
					if(!TAKE_Long_C_Type__BOX__TRUE(x,y+1)) {
						Board_Box test(BoardSave);
						if(test.TAKE_Save_ChainEdgeBool(x,y+1,STATE)&&test.TAKE_Save_AngleEdgeBool(x,y+1,STATE)) {
							_Moves[Move_Num]={x,y};
							Move_Num++;
						}
					}
				} else if(y==LEN-1) {
					if(!TAKE_Long_C_Type__BOX__TRUE(x,y-1)) {
						Board_Box test(BoardSave);
						if(test.TAKE_Save_ChainEdgeBool(x,y-1,STATE)&&test.TAKE_Save_AngleEdgeBool(x,y-1,STATE)) {
							_Moves[Move_Num]={x,y};
							Move_Num++;
						}
					}
				} else {
					if(!TAKE_Long_C_Type__BOX__TRUE(x,y+1)&&!TAKE_Long_C_Type__BOX__TRUE(x,y-1)) {
						Board_Box test(BoardSave);
						if(test.TAKE_Save_ChainEdgeBool(x,y+1,STATE)&&test.TAKE_Save_ChainEdgeBool(x,y-1,STATE)) {
							_Moves[Move_Num]={x,y};
							Move_Num++;
						}
					}
				}
				setBoard(BoardSave);
				STATE[x][y]=true;
			}
		}
	}
	return Move_Num;
}

/**
 * @brief 从一个格出发，注册他的所有派生链
 * @param BOX_POS 格子位置
 * @return 无返回值
 */
void Board_Box::Search_From_Box(POS BOX_POS) {
	for(auto n:WAY) {
		int bx=BOX_POS.first+n[0];
		int by=BOX_POS.second+n[1];
		int lx=(BOX_POS.first*2)-1+n[0];
		int ly=(BOX_POS.second*2)-1+n[1];
		if((_MAP_[lx][ly]==Horizon||_MAP_[lx][ly]==Vertical)&&_BOX__ID(bx,by)==CHAIN_BOX_) {
			if(Boxes_OF_Board_Box[bx][by].CHAIN_TYPE_ID==EMPTY) {
				POS New_Position={bx,by};
				REGISTER_Chain(BOX_POS,New_Position);
			}
		}
	}
}

/**
 * @brief 获取可用链空间的编号
 * @return int 可用链空间编号
 */
int Board_Box::First_Empty_Chain__Num() {
	for(int i=1;i<25;i++) {
		if(Chains_OF_Board_Box[i].Type==Not_Define) return i;
	}
	return 0;
}

/**
 * @brief 动态注册链
 * @param _Free_BOX_POS 自由格位置
 * @param F1st_Position 第一个位置
 * @return 无返回值
 */
void Board_Box::REGISTER_Chain(POS _Free_BOX_POS,POS F1st_Position) {
	POS _Position=_Free_BOX_POS;
	POS New_Position=F1st_Position;
	int Re=First_Empty_Chain__Num();
	Chains_OF_Board_Box[Re].CHAIN_Start_Position=_Position;
	Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN=0;
	Chains_OF_Board_Box[Re].Type=Not_Define;
	for(int i=0;i<_BOX_NUM;i++) {
		if(_BOX__ID(New_Position.first,New_Position.second)!=CHAIN_BOX_) {
			Chains_OF_Board_Box[Re].CHAIN_END_POS=New_Position;
			if(New_Position.first==_Free_BOX_POS.first&&New_Position.second==_Free_BOX_POS.second&&Boxes_OF_Board_Box[New_Position.first][New_Position.second].CHAIN_TYPE_ID==EMPTY) {
				Chains_OF_Board_Box[Re].Type=Pre_Circle;
				Boxes_OF_Board_Box[New_Position.first][New_Position.second].CHAIN_TYPE_ID=Re;
				Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN++;
			} else {
				Chains_OF_Board_Box[Re].Type=Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN<=2?Short_Chain:Long_Chain;
			}
			break;
		} else {
			POS m_Position=New_Position;
			Boxes_OF_Board_Box[New_Position.first][New_Position.second].CHAIN_TYPE_ID=Re;
			Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN++;
			New_Position=NEXT__BOX_(New_Position,_Position);
			_Position=m_Position;
		}
	}
}

/**
 * @brief 查找某个链格的下一个格子
 * @param Box_Founded 找到的格子
 * @param BOX_Lasted 上一个格子
 * @return POS 下一个格子位置
 */
POS Board_Box::NEXT__BOX_(POS Box_Founded,POS BOX_Lasted) {
	int fbx=Box_Founded.first*2-1;
	int fby=Box_Founded.second*2-1;
	POS AimBox;
	if(_MAP_[fbx+1][fby]!=OCCed_LINE&&((Box_Founded.first+1)!=BOX_Lasted.first||(Box_Founded.second)!=BOX_Lasted.second)) {
		return{Box_Founded.first+1,Box_Founded.second};
	} else if(_MAP_[fbx][fby+1]!=OCCed_LINE&&((Box_Founded.first)!=BOX_Lasted.first||(Box_Founded.second+1)!=BOX_Lasted.second)) {
		return{Box_Founded.first,Box_Founded.second+1};
	} else if(_MAP_[fbx-1][fby]!=OCCed_LINE&&((Box_Founded.first-1)!=BOX_Lasted.first||(Box_Founded.second)!=BOX_Lasted.second)) {
		return{Box_Founded.first-1,Box_Founded.second};
	} else if(_MAP_[fbx][fby-1]!=OCCed_LINE&&((Box_Founded.first)!=BOX_Lasted.first||(Box_Founded.second-1)!=BOX_Lasted.second)) {
		return{Box_Founded.first,Box_Founded.second-1};
	}
	return{0,0};
}

/**
 * @brief 从棋盘上获取格子类型
 * @param bx 横坐标
 * @param by 纵坐标
 * @return int 格子类型
 */
int Board_Box::_BOX__ID(int bx,int by) {
	if(bx>=1&&by>=1&&bx<=_BOX_LEN&&by<=_BOX_LEN) return Boxes_OF_Board_Box[bx][by].Type;
	return FREE_BOX_;
}

/**
 * @brief 定义所有的链
 * @param Chain_P 是否启用链处理
 * @return 无返回值
 */
void Board_Box::Chains_Define(bool Chain_P) {
	BoxesType_Define();
	Reset_Chains_Info();
	for(int i=1;i<=_BOX_LEN;i++) {
		for(int j=1;j<=_BOX_LEN;j++) {
			if(Boxes_OF_Board_Box[i][j].Type==3) {
				POS k={i,j};
				Search_From_Box(k);
			}
		}
	}
	for(int i=1;i<=_BOX_LEN;i++) {
		Search_From_Box({i,0});
		Search_From_Box({0,i});
		Search_From_Box({i,_BOX_LEN+1});
		Search_From_Box({_BOX_LEN+1,i});
	}
	for(int i=1;i<=_BOX_LEN;i++) {
		for(int j=1;j<=_BOX_LEN;j++) {
			if(Boxes_OF_Board_Box[i][j].Type==CHAIN_BOX_&&Boxes_OF_Board_Box[i][j].CHAIN_TYPE_ID==EMPTY) {
				Search_Circle({i,j});
			}
		}
	}
	if(Chain_P) {
		for(int i=1;i<25;i++) {
			if(Chains_OF_Board_Box[i].Type==Long_Chain) {
				for(int j=i+1;j<=25;j++) {
					if(Chains_OF_Board_Box[j].Type==Long_Chain&&Chains_OF_Board_Box[i].CHAIN_Start_Position==Chains_OF_Board_Box[j].CHAIN_Start_Position&&Chains_OF_Board_Box[i].CHAIN_END_POS==Chains_OF_Board_Box[j].CHAIN_END_POS) {
						if(Boxes_OF_Board_Box[Chains_OF_Board_Box[j].CHAIN_Start_Position.first][Chains_OF_Board_Box[j].CHAIN_Start_Position.second].CHAIN_TYPE_ID==EMPTY&&Boxes_OF_Board_Box[Chains_OF_Board_Box[j].CHAIN_END_POS.first][Chains_OF_Board_Box[j].CHAIN_END_POS.second].CHAIN_TYPE_ID==EMPTY) {
							INHERIT_CHAIN(i,j);
							Boxes_OF_Board_Box[Chains_OF_Board_Box[i].CHAIN_Start_Position.first][Chains_OF_Board_Box[i].CHAIN_Start_Position.second].CHAIN_TYPE_ID=i;
							Boxes_OF_Board_Box[Chains_OF_Board_Box[i].CHAIN_END_POS.first][Chains_OF_Board_Box[i].CHAIN_END_POS.second].CHAIN_TYPE_ID=i;
							Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN+=2;
							Chains_OF_Board_Box[i].CHAIN_Start_Position=Chains_OF_Board_Box[i].CHAIN_END_POS;
							Chains_OF_Board_Box[i].Type=Pre_Circle;
						}
					}
				}
			}
		}
		for(int y=1;y<=_BOX_LEN;y++) {
			for(int x=1;x<=_BOX_LEN;x++) {
				if(Boxes_OF_Board_Box[x][y].Type==3&&Chains_OF_Board_Box[Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID].Type==Pre_Circle) {
					int bx=x*2-1;
					int by=y*2-1;
					for(auto n:WAY) {
						int ex=bx+n[0];
						int ey=by+n[1];
						int nbx=x+n[0];
						int nby=y+n[1];
						if(_MAP_[ex][ey]!=OCCed_LINE&&Chains_OF_Board_Box[Boxes_OF_Board_Box[nbx][nby].CHAIN_TYPE_ID].Type==Long_Chain) {
							Chains_OF_Board_Box[Boxes_OF_Board_Box[nbx][nby].CHAIN_TYPE_ID].IF_Pre_Circle=true;
						}
					}
				}
			}
		}
		for(int y=1;y<=_BOX_LEN;y++) {
			for(int x=1;x<=_BOX_LEN;x++) {
				if(Boxes_OF_Board_Box[x][y].Type==FREE_BOX_&&Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID==EMPTY) {
					int ChainReg_Num[4];
					int Total=0;
					for(auto n:WAY) {
						int bx=x+n[0];
						int by=y+n[1];
						int lx=(x*2)-1+n[0];
						int ly=(y*2)-1+n[1];
						if(_MAP_[lx][ly]!=OCCed_LINE&&_BOX__ID(bx,by)==CHAIN_BOX_) {
							ChainReg_Num[Total]=Boxes_OF_Board_Box[bx][by].CHAIN_TYPE_ID;
							Total++;
						}
					}
					int FirstChain_Num=0;
					int FirstChainBoxes=0;
					for(int i=0;i<Total;i++) {
						if(Chains_OF_Board_Box[ChainReg_Num[i]]._BOX__NUM_OF_CHAIN>FirstChainBoxes) {
							FirstChain_Num=ChainReg_Num[i];
							FirstChainBoxes=Chains_OF_Board_Box[ChainReg_Num[i]]._BOX__NUM_OF_CHAIN;
						}
					}
					int SecChain_Num=0;
					int SecChainBoxes=0;
					for(int i=0;i<Total;i++) {
						if(Chains_OF_Board_Box[ChainReg_Num[i]]._BOX__NUM_OF_CHAIN>SecChainBoxes&&ChainReg_Num[i]!=FirstChain_Num) {
							SecChain_Num=ChainReg_Num[i];
							SecChainBoxes=Chains_OF_Board_Box[ChainReg_Num[i]]._BOX__NUM_OF_CHAIN;
						}
					}
					Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID=FirstChain_Num;
					Chains_OF_Board_Box[FirstChain_Num]._BOX__NUM_OF_CHAIN++;
					INHERIT_CHAIN(FirstChain_Num,SecChain_Num);
				}
			}
		}
	}
}

/**
 * @brief 搜索环
 * @param BOX_POS 格子位置
 * @return 无返回值
 */
void Board_Box::Search_Circle(POS BOX_POS) {
	for(auto n:WAY) {
		int bx=BOX_POS.first+n[0];
		int by=BOX_POS.second+n[1];
		int lx=(BOX_POS.first*2)-1+n[0];
		int ly=(BOX_POS.second*2)-1+n[1];
		if(_MAP_[lx][ly]!=OCCed_LINE&&_BOX__ID(bx,by)==CHAIN_BOX_) {
			if(Boxes_OF_Board_Box[bx][by].CHAIN_TYPE_ID==EMPTY) {
				POS New_Position={bx,by};
				REGISTER_Circle(BOX_POS,New_Position);
			}
		}
	}
}

/**
 * @brief 动态注册环
 * @param CHAIN_Start_Position 链起始位置
 * @param NXT_POS 下一个位置
 * @return 无返回值
 */
void Board_Box::REGISTER_Circle(POS CHAIN_Start_Position,POS NXT_POS) {
	POS _Position=CHAIN_Start_Position;
	POS New_Position=NXT_POS;
	int Re=First_Empty_Chain__Num();
	Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN=0;
	Chains_OF_Board_Box[Re].Type=Not_Define;
	for(int i=0;i<25;i++) {
		if(Boxes_OF_Board_Box[New_Position.first][New_Position.second].Type!=2) {
			INHERIT_CHAIN(EMPTY,Re);
			break;
		} else if(New_Position.first==CHAIN_Start_Position.first&&New_Position.second==CHAIN_Start_Position.second) {
			Chains_OF_Board_Box[Re].Type=Circle;
			Chains_OF_Board_Box[Re].CHAIN_Start_Position=New_Position;
			Chains_OF_Board_Box[Re].CHAIN_END_POS=New_Position;
			Boxes_OF_Board_Box[New_Position.first][New_Position.second].CHAIN_TYPE_ID=Re;
			Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN++;
			break;
		} else {
			POS m_Position=New_Position;
			Boxes_OF_Board_Box[New_Position.first][New_Position.second].CHAIN_TYPE_ID=Re;
			Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN++;
			New_Position=NEXT__BOX_(New_Position,_Position);
			_Position=m_Position;
		}
	}
}
/**
 * @brief 继承链操作，将祖先链的所有格子转移到继承者链中
 * @details 遍历所有格子，将属于祖先链的格子重新分配给继承者链，累加格子数量，销毁祖先链空间
 * @param Inheritor_Reg_ID 继承者链的注册ID
 * @param Ancester_Reg_ID 祖先链的注册ID
 * @return 无返回值
 */
void Board_Box::INHERIT_CHAIN(int Inheritor_Reg_ID,int Ancester_Reg_ID){
	for(int j=1;j<=_BOX_LEN;++j)
		for(int i=1;i<=_BOX_LEN;++i)
			if(Boxes_OF_Board_Box[i][j].CHAIN_TYPE_ID==Ancester_Reg_ID)
				Boxes_OF_Board_Box[i][j].CHAIN_TYPE_ID=Inheritor_Reg_ID;
	Chains_OF_Board_Box[Inheritor_Reg_ID]._BOX__NUM_OF_CHAIN+=Chains_OF_Board_Box[Ancester_Reg_ID]._BOX__NUM_OF_CHAIN;
	Chains_OF_Board_Box[Inheritor_Reg_ID].Type=Long_Chain;
	Chains_OF_Board_Box[Ancester_Reg_ID]._BOX__NUM_OF_CHAIN=0;
	Chains_OF_Board_Box[Ancester_Reg_ID].Type=Not_Define;
}

/**
 * @brief 定义死链结构，识别并注册所有死链和死环
 * @details 先定义格子类型，重置链信息，然后遍历所有死格，从每个死格开始搜索死链
 * @return 无返回值
 */
void Board_Box::DEFINE_Dead_Chain(){
	BoxesType_Define();
	Reset_Chains_Info();
	for(int i=1;i<=_BOX_LEN;++i)
		for(int j=1;j<=_BOX_LEN;++j)
			if(Boxes_OF_Board_Box[i][j].Type==DEAD_BOX_)
				SEARCH_Dead_Chain({i,j});
}

/**
 * @brief 从指定死格位置搜索死链
 * @details 遍历四个方向，检查相邻边是否为空且目标格子为链格，如果未归属则注册死链
 * @param BOX_POS 死格位置坐标
 * @return 无返回值
 */
void Board_Box::SEARCH_Dead_Chain(POS BOX_POS){
	for(auto n:WAY){
		int bx=BOX_POS.first+n[0],by=BOX_POS.second+n[1];
		int lx=(BOX_POS.first*2)-1+n[0],ly=(BOX_POS.second*2)-1+n[1];
		if((_MAP_[lx][ly]==Horizon||_MAP_[lx][ly]==Vertical)&&_BOX__ID(bx,by)==CHAIN_BOX_)
			if(Boxes_OF_Board_Box[bx][by].CHAIN_TYPE_ID==EMPTY)
				REGISTER_Dead_Chain(BOX_POS,{bx,by});
	}
}

/**
 * @brief 注册死链，从一个死格出发向链格方向进行链的注册
 * @details 设置起点坐标，获取空白链空间，遍历搜索链的每个格子，根据终点类型判断是死链还是死环
 * @param _Free_BOX_POS 自由死格位置
 * @param F1st_Position 第一个链格位置
 * @return 无返回值
 */
void Board_Box::REGISTER_Dead_Chain(POS _Free_BOX_POS,POS F1st_Position){
	POS _Position=_Free_BOX_POS,New_Position=F1st_Position;
	int Re=First_Empty_Chain__Num();
	Chains_OF_Board_Box[Re].CHAIN_Start_Position=_Position;
	Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN=1;
	Chains_OF_Board_Box[Re].Type=Not_Define;
	for(int i=0;i<_BOX_NUM;++i){
		if(_BOX__ID(New_Position.first,New_Position.second)!=CHAIN_BOX_){
			Chains_OF_Board_Box[Re].CHAIN_END_POS=New_Position;
			if(_BOX__ID(New_Position.first,New_Position.second)==DEAD_BOX_){
				Chains_OF_Board_Box[Re].Type=Dead_Circle;
				Boxes_OF_Board_Box[New_Position.first][New_Position.second].CHAIN_TYPE_ID=Re;
				++Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN;
			}else
				Chains_OF_Board_Box[Re].Type=Dead_Chain;
			break;
		}else{
			POS m_Position=New_Position;
			Boxes_OF_Board_Box[New_Position.first][New_Position.second].CHAIN_TYPE_ID=Re;
			++Chains_OF_Board_Box[Re]._BOX__NUM_OF_CHAIN;
			New_Position=NEXT__BOX_(New_Position,_Position);
			_Position=m_Position;
		}
	}
}

/**
 * @brief 捕获最短链，先手占据关键边，后手占据剩余边
 * @details 找到最短的长链或环，先手占据两个格子间的关键边，后手占据链中所有其他空余边
 * @param Player_L 当前玩家标识
 * @return bool 是否成功捕获链
 */
bool Board_Box::Captual_MIN_Chain(int Player_L){
	Chains_Define(false);
	int Least=0,LeastBox_Num=_BOX_NUM;
	for(int i=1;i<_BOX_NUM;++i)
		if((Chains_OF_Board_Box[i].Type==Long_Chain||Chains_OF_Board_Box[i].Type==Circle)&&
		   Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN<LeastBox_Num){
			LeastBox_Num=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			Least=i;
		}
	if(!Least)return false;
	bool Finish=false;
	for(int y=1;y<=_BOX_LEN;++y)
		for(int x=1;x<=_BOX_LEN;++x)
			if(Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID==Least&&!Finish){
				int bx=x*2-1,by=y*2-1;
				for(auto n:WAY){
					int ex=bx+n[0],ey=by+n[1],nbx=x+n[0],nby=y+n[1];
					if((_MAP_[ex][ey]==Horizon||_MAP_[ex][ey]==Vertical)&&
					   Boxes_OF_Board_Box[nbx][nby].CHAIN_TYPE_ID==Least){
						move(-Player_L,{ex,ey});
						Finish=true;
						break;
					}
				}
			}
	for(int y=1;y<=_BOX_LEN;++y)
		for(int x=1;x<=_BOX_LEN;++x)
			if(Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID==Least){
				int bx=x*2-1,by=y*2-1;
				for(auto n:WAY){
					int ex=bx+n[0],ey=by+n[1];
					if(_MAP_[ex][ey]==Horizon||_MAP_[ex][ey]==Vertical){
						move(Player_L,{ex,ey});
						break;
					}
				}
				Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID=0;
			}
	INHERIT_CHAIN(EMPTY,Least);
	BoxesType_Define();
	return true;
}

/**
 * @brief 获取打开最短链的位置坐标
 * @details 找到最短的长链，返回该链中两个相邻格子间的边坐标
 * @return POS 打开最短链的边坐标
 */
POS Board_Box::TAKE_OpenMIN_Chain_Position(){
	Chains_Define(false);
	int Least=0,LeastBox_Num=_BOX_NUM;
	for(int i=1;i<_BOX_NUM;++i)
		if(Chains_OF_Board_Box[i].Type==Long_Chain&&
		   Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN<LeastBox_Num){
			LeastBox_Num=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			Least=i;
		}
	for(int y=1;y<=_BOX_LEN;++y)
		for(int x=1;x<=_BOX_LEN;++x)
			if(Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID==Least){
				int bx=x*2-1,by=y*2-1;
				for(auto n:WAY){
					int ex=bx+n[0],ey=by+n[1],nbx=x+n[0],nby=y+n[1];
					if((_MAP_[ex][ey]==Horizon||_MAP_[ex][ey]==Vertical)&&
					   Boxes_OF_Board_Box[nbx][nby].CHAIN_TYPE_ID==Least)
						return{ex,ey};
				}
			}
	return{0,0};
}

/**
 * @brief 打开策略，根据当前链结构选择最优的打开位置
 * @details 统计长链、环、3格长链数量，根据策略选择打开最短环、最短链或3链
 * @return POS 策略选择的打开位置坐标
 */
POS Board_Box::openPolicy(){
	POS res={-1,-1};
	Chains_Define(false);
	int LCnum=0,Cnum=0,LC3num=0;
	for(int i=1;i<25;++i){
		if(Chains_OF_Board_Box[i].Type==Circle)++Cnum;
		if(Chains_OF_Board_Box[i].Type==Long_Chain){
			++LCnum;
			if(Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN==3)++LC3num;
		}
	}
	if(!LCnum&&Cnum)res=TAKE_OpenMIN_Circle_Position();
	else if(LCnum&&!Cnum)res=TAKE_OpenMIN_Chain_Position();
	else if(LCnum&&Cnum&&!LC3num)res=TAKE_OpenMIN_Circle_Position();
	else if(LCnum&&Cnum&&LC3num)res=TAKE_3_Open_Chain_Position();
	return res;
}

/**
 * @brief 获取打开最短环的位置坐标
 * @details 找到最短的环，返回该环中两个相邻格子间的边坐标
 * @return POS 打开最短环的边坐标
 */
POS Board_Box::TAKE_OpenMIN_Circle_Position(){
	Chains_Define(false);
	int Least=0,LeastBox_Num=_BOX_NUM;
	for(int i=1;i<_BOX_NUM;++i)
		if(Chains_OF_Board_Box[i].Type==Circle&&
		   Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN<LeastBox_Num){
			LeastBox_Num=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			Least=i;
		}
	for(int y=1;y<=_BOX_LEN;++y)
		for(int x=1;x<=_BOX_LEN;++x)
			if(Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID==Least){
				int bx=x*2-1,by=y*2-1;
				for(auto n:WAY){
					int ex=bx+n[0],ey=by+n[1],nbx=x+n[0],nby=y+n[1];
					if((_MAP_[ex][ey]==Horizon||_MAP_[ex][ey]==Vertical)&&
					   Boxes_OF_Board_Box[nbx][nby].CHAIN_TYPE_ID==Least)
						return{ex,ey};
				}
			}
	return{0,0};
}

/**
 * @brief 获取打开3链的位置坐标
 * @details 找到3格长链，返回该链中两个相邻格子间的边坐标
 * @return POS 打开3链的边坐标
 */
POS Board_Box::TAKE_3_Open_Chain_Position(){
	Chains_Define(false);
	int SuitChainidx=60;
	for(int i=1;i<_BOX_NUM;++i)
		if(Chains_OF_Board_Box[i].Type==Long_Chain&&
		   Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN==3)
			SuitChainidx=i;
	for(int y=1;y<=_BOX_LEN;++y)
		for(int x=1;x<=_BOX_LEN;++x)
			if(Boxes_OF_Board_Box[x][y].CHAIN_TYPE_ID==SuitChainidx){
				int bx=x*2-1,by=y*2-1;
				for(auto n:WAY){
					int ex=bx+n[0],ey=by+n[1],nbx=x+n[0],nby=y+n[1];
					if((_MAP_[ex][ey]==Horizon||_MAP_[ex][ey]==Vertical)&&
					   Boxes_OF_Board_Box[nbx][nby].CHAIN_TYPE_ID==SuitChainidx)
						return{ex,ey};
				}
			}
	return{0,0};
}

/**
 * @brief 判断状态是否理性，即剩余格子数是否合理分配
 * @details 检查剩余格子数是否大于等于牺牲格子数，判断是否达到理性状态
 * @param Box_Num 包含剩余格子数和牺牲格子数的位置结构
 * @return bool 是否为理性状态
 */
bool Board_Box::State_IS_Rational_(POS Box_Num){
	return Box_Num.first>=Box_Num.second;
}

/**
 * @brief 获取早期理性状态下的格子数量分配
 * @details 定义链结构，统计长链和环的数量及格子数，计算理性状态下的格子分配
 * @return POS 包含总格子数和牺牲格子数的位置结构
 */
POS Board_Box::TAKE_Early_Rational_Box_Num(){
	Chains_Define(false);
	int LC_Num=0,LCBox=0,C_Num=0,CBox=0,PCBox=0;
	for(int i=1;i<=25;++i)
		if(Chains_OF_Board_Box[i].Type!=Not_Define){
			if(Chains_OF_Board_Box[i].Type==Long_Chain){
				++LC_Num;
				LCBox+=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			}else if(Chains_OF_Board_Box[i].Type==Circle){
				++C_Num;
				CBox+=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			}
		}
	int Total=LCBox+PCBox+CBox,Sacrifice=0;
	if(!LC_Num&&C_Num)Sacrifice=(C_Num-1)*4;
	if(LC_Num)Sacrifice=(C_Num*4)+(LC_Num*2)-2;
	return{Total-Sacrifice,Sacrifice};
}

/**
 * @brief 获取理性状态下的格子数量分配
 * @details 定义完整链结构，统计各种链的数量，计算理性状态下的格子分配
 * @return POS 包含总格子数和牺牲格子数的位置结构
 */
POS Board_Box::TAKE_Rational_StateBox_Num(){
	Chains_Define(true);
	int LC_Num=0,LCBox=0,C_Num=0,CBox=0,PC_Num=0,PCBox=0;
	bool OnlyPreChain=true;
	for(int i=1;i<=_BOX_NUM;++i)
		if(Chains_OF_Board_Box[i].Type!=Not_Define){
			if(Chains_OF_Board_Box[i].Type==Long_Chain){
				if(!Chains_OF_Board_Box[i].IF_Pre_Circle)OnlyPreChain=false;
				++LC_Num;
				LCBox+=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			}else if(Chains_OF_Board_Box[i].Type==Circle){
				++C_Num;
				CBox+=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			}else if(Chains_OF_Board_Box[i].Type==Pre_Circle){
				++PC_Num;
				PCBox+=Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN;
			}
		}
	int Total=LCBox+PCBox+CBox,Sacrifice=0;
	if(OnlyPreChain){
		if(!LC_Num)Sacrifice=(C_Num-1)*4;
		else Sacrifice=(PC_Num*4)+(C_Num*4)+(LC_Num*2)-4;
	}else Sacrifice=(PC_Num*4)+(C_Num*4)+(LC_Num*2)-2;
	return{Total-Sacrifice,Sacrifice};
}

/**
 * @brief 获取棋盘获胜者，通过非理性吞并后判断胜负
 * @details 在非理性状态下吞并所有可能的链，直到达到理性状态，然后判断胜负
 * @param Player_L 当前玩家标识
 * @return int 获胜者标识
 */
int Board_Box::TAKE_Winner_OF_Board(int Player_L) {
    int player = Player_L;
    POS Box_Num;
    // 吞并直到理性或无链可吃
    while (true) {
        Box_Num = TAKE_Rational_StateBox_Num();
        if (State_IS_Rational_(Box_Num)) {
            break;
        }
        if (!Captual_MIN_Chain(player)) {
            break;
        }
        player = -player;
    }
    int w = Winner_TAKE_();
    if (w != 0) {
        return w;
    }
    int r, b;
    if (player == BLACK) {
        r = Box_Num.first + TAKE_Boxes_OF_Players(BLACK);
        b = Box_Num.second + TAKE_Boxes_OF_Players(WHITE);
    } else {
        r = Box_Num.second + TAKE_Boxes_OF_Players(BLACK);
        b = Box_Num.first + TAKE_Boxes_OF_Players(WHITE);
    }
    return (r > b ? BLACK : WHITE);
}

/**
 * @brief 获取棋盘获胜者（带评分），通过非理性吞并后判断胜负并设置评分
 * @details 逻辑同TAKE_Winner_OF_Board，额外根据Father_O设置Rating
 * @param Player_L 当前玩家标识
 * @param Father_O 父节点拥有者标识
 * @param Rating 引用，用于返回评分
 * @return int 获胜者标识
 */
int Board_Box::TAKE_Winner_OF_Board(int Player_L, int Father_O, int &Rating) {
    int player = Player_L;
    POS Box_Num;
    // 吞并直到理性或无链可吃
    while (true) {
        Box_Num = TAKE_Rational_StateBox_Num();
        if (State_IS_Rational_(Box_Num)) {
            break;
        }
        if (!Captual_MIN_Chain(player)) {
            break;
        }
        player = -player;
    }
    int w = Winner_TAKE_();
    if (w == 0) {
        int r, b;
        if (player == BLACK) {
            r = Box_Num.first + TAKE_Boxes_OF_Players(BLACK);
            b = Box_Num.second + TAKE_Boxes_OF_Players(WHITE);
        } else {
            r = Box_Num.second + TAKE_Boxes_OF_Players(BLACK);
            b = Box_Num.first + TAKE_Boxes_OF_Players(WHITE);
        }
        Rating = (Father_O == BLACK ? r : b);
        return (r > b ? BLACK : WHITE);
    } else {
        Rating = (Father_O == BLACK ? BOX_of_BLACK : BOX_of_WHITE);
        return w;
    }
}

/**
 * @brief 检查是否存在死链
 * @return bool 存在返回true，否则false
 */
bool Board_Box::TAKE_Dead_Chain_Exist() {
    DEFINE_Dead_Chain();
    for (int i = 0; i < _BOX_NUM; i++) {
        if (Chains_OF_Board_Box[i].Type == Dead_Chain) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 检查是否存在死圈（链长度>3）
 * @return bool 存在返回true，否则false
 */
bool Board_Box::TAKE_Dead_Circle_Exist() {
    DEFINE_Dead_Chain();
    for (int i = 0; i < _BOX_NUM; i++) {
        if (Chains_OF_Board_Box[i].Type == Dead_Circle && Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN > 3) {
            return true;
        }
    }
    return false;
}

/**
 * @brief 定义格子类型
 */
void Board_Box::BoxesType_Define() {
    for (int y = 1; y <= _BOX_LEN; y++) {
        for (int x = 1; x <= _BOX_LEN; x++) {
            int bx = (x << 1) - 1;
            int by = (y << 1) - 1;
            int bl = TAKE__Free_dom(bx, by);
            if (bl == 4) {
                Boxes_OF_Board_Box[x][y].Type = FREE_BOX_;
            } else {
                Boxes_OF_Board_Box[x][y].Type = bl;
            }
        }
    }
}

/**
 * @brief 判断是否保留短链等价边
 * @param x 坐标x
 * @param y 坐标y
 * @param STATE 状态数组
 * @return bool 保留返回true，否则false
 */
bool Board_Box::TAKE_Save_ChainEdgeBool(int x, int y, bool STATE[LEN][LEN]) {
    if (!Odd_TRUE(x) || !Odd_TRUE(y)) {
        return false;
    }
    Chains_Define(false);
    int idx = Boxes_OF_Board_Box[(x + 1) >> 1][(y + 1) >> 1].CHAIN_TYPE_ID;
    auto &c = Chains_OF_Board_Box[idx];
    if (c.Type == Short_Chain && c._BOX__NUM_OF_CHAIN == 1) {
        int cnt = 0;
        auto chk = [&](int xx, int yy) {
            if ((_MAP_[xx][yy] == Horizon || _MAP_[xx][yy] == Vertical) && !STATE[xx][yy]) {
                cnt++;
            }
        };
        chk(x, y - 1);
        chk(x + 1, y);
        chk(x, y + 1);
        chk(x - 1, y);
        if (cnt == 2) {
            return true;
        }
        if (cnt == 1) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 判断边角是否保留等价边
 * @param x 坐标x
 * @param y 坐标y
 * @param STATE 状态数组
 * @return bool 保留返回true，否则false
 */
bool Board_Box::TAKE_Save_AngleEdgeBool(int x, int y, bool STATE[LEN][LEN]) {
    // 四角：1,1  9,1  1,9  9,9
    if ((x == 1 || x == 9) && (y == 1 || y == 9)) {
        if (TAKE__Free_dom(x, y) == 4) {
            int cnt = 0;
            if (!STATE[(x)][(y == 1 ? y - 1 : y + 1)]) cnt++;
            if (!STATE[(x == 1 ? x - 1 : x + 1)][y]) cnt++;
            if (cnt == 2) {
                return true;
            }
            if (cnt == 1) {
                return false;
            }
        }
        return true;
    }
    return true;
}

/**
 * @brief 获取所有自由边
 * @param _Moves 存储自由边坐标数组
 * @return int 自由边数量
 */
int Board_Box::TAKE__Free__Moves(POS _Moves[60]) {
    bool STATE[LEN][LEN] = {false};
    int Move_Num = 0;
    for (int y = 1; y < LEN - 1; y += 2) {
        // 横向头部
        if (TAKE__Free___BOX__TRUE(1, y) && _MAP_[0][y] == Horizon) {
            if (TAKE_Save_AngleEdgeBool(1, y, STATE)) {
                _Moves[Move_Num++] = {0, y};
            }
            STATE[0][y] = true;
        }
        // 横向中间
        for (int x = 1; x < LEN - 3; x += 2) {
            if (TAKE__Free___BOX__TRUE(x, y) && TAKE__Free___BOX__TRUE(x + 2, y) && _MAP_[x + 1][y] == Horizon) {
                _Moves[Move_Num++] = {x + 1, y};
            }
            STATE[x + 1][y] = true;
        }
        // 横向尾部
        if (TAKE__Free___BOX__TRUE(LEN - 2, y) && _MAP_[LEN - 1][y] == Horizon) {
            if (TAKE_Save_AngleEdgeBool(LEN - 2, y, STATE)) {
                _Moves[Move_Num++] = {LEN - 1, y};
            }
            STATE[LEN - 1][y] = true;
        }
        // 纵向头部
        if (TAKE__Free___BOX__TRUE(y, 1) && _MAP_[y][0] == Vertical) {
            if (TAKE_Save_AngleEdgeBool(y, 1, STATE)) {
                _Moves[Move_Num++] = {y, 0};
            }
            STATE[y][0] = true;
        }
        // 纵向中间
        for (int x = 1; x < LEN - 3; x += 2) {
            if (TAKE__Free___BOX__TRUE(y, x) && TAKE__Free___BOX__TRUE(y, x + 2) && _MAP_[y][x + 1] == Vertical) {
                _Moves[Move_Num++] = {y, x + 1};
            }
            STATE[y][x + 1] = true;
        }
        // 纵向尾部
        if (TAKE__Free___BOX__TRUE(y, LEN - 2) && _MAP_[y][LEN - 1] == Vertical) {
            if (TAKE_Save_AngleEdgeBool(y, LEN - 2, STATE)) {
                _Moves[Move_Num++] = {y, LEN - 1};
            }
            STATE[y][LEN - 1] = true;
        }
    }
    return Move_Num;
}

/**
 * @brief 获取棋盘上未被占据的格子数
 * @return int 未被占据的格子数
 */
int Board_Box::get_board_size() {
    return _BOX_NUM - (this->BOX_of_BLACK + this->BOX_of_WHITE);
}

/**
 * @brief 计算长链的数量
 * @return int 长链数量
 */
int Board_Box::count_long_chains() {
    int count = 0;
    for (int i = 1; i < 25; ++i) {
        if (Chains_OF_Board_Box[i].Type == Long_Chain) {
            count++;
        }
    }
    return count;
}

/**
 * @brief 计算环的数量
 * @return int 环数量
 */
int Board_Box::count_loops() {
    int count = 0;
    for (int i = 1; i < 25; ++i) {
        if (Chains_OF_Board_Box[i].Type == Circle) {
            count++;
        }
    }
    return count;
}

/**
 * @brief 计算3格长链的数量
 * @return int 3格长链数量
 */
int Board_Box::count_3_chains() {
    int count = 0;
    for (int i = 1; i < 25; ++i) {
        if (Chains_OF_Board_Box[i].Type == Long_Chain && Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN == 3) {
            count++;
        }
    }
    return count;
}

/**
 * @brief 计算4格环的数量
 * @return int 4格环数量
 */
int Board_Box::count_4_loops() {
    int count = 0;
    for (int i = 1; i < 25; ++i) {
        if (Chains_OF_Board_Box[i].Type == Circle && Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN == 4) {
            count++;
        }
    }
    return count;
}
int Board_Box::count_all_chains() {
    int count = 0;
    for (int i = 1; i < 25; i++) {
        if (Chains_OF_Board_Box[i].Type == Long_Chain || Chains_OF_Board_Box[i].Type == Short_Chain) {
            count++;
        }
    }
    return count;
}
/**
 * @brief 计算tb(G)值
 * @return int tb(G)值
 */
int Board_Box::calculate_tb() {
    int num_loops = count_loops();
    int num_all_chains = count_all_chains();
    int num_3_chains = count_3_chains();

    if (num_loops > 0 && num_3_chains > 0) return 6;
    if (num_loops > 0) return 8;
    if (num_all_chains == 0 && num_loops == 0) return 0; // "otherwise" case
    return 4;
}

/**
 * @brief 计算控制值 c(G)
 * @return int 控制值
 */
int Board_Box::control_value() {
    int size_G = get_board_size();
    int long_chains = count_long_chains();
    int loops = count_loops();
    int tb_G = calculate_tb();

    return size_G - 4 * long_chains - 8 * loops + tb_G;
}

/**
 * @brief 检查链的构成是否符合 "4l + (anything except 3+3)"
 * @details "4l" 被解释为没有长度为1或2的链。"anything except 3+3" 被解释为3链的数量不等于2。
 * @return bool 符合条件返回true
 */
bool Board_Box::check_chain_composition() {
    int num_1_chains = 0;
    int num_2_chains = 0;
    int num_3_chains = 0;

    for (int i = 1; i < 25; ++i) {
        if (Chains_OF_Board_Box[i].Type == Short_Chain) {
            if (Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN == 1) num_1_chains++;
            if (Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN == 2) num_2_chains++;
        } else if (Chains_OF_Board_Box[i].Type == Long_Chain && Chains_OF_Board_Box[i]._BOX__NUM_OF_CHAIN == 3) {
            num_3_chains++;
        }
    }
    
    bool no_short_chains = (num_1_chains == 0 && num_2_chains == 0);
    bool not_two_3_chains = (num_3_chains != 2);
    
    return no_short_chains && not_two_3_chains;
}

/**
 * @brief 判断一个数是否为奇数
 * @param n 要检查的数
 * @return bool 是奇数返回true，否则false
 */
bool is_odd(int n) {
    return n % 2 != 0;
}

/**
 * @brief 对当前局面进行估值（完整实现版）
 * @details 严格遵循提供的伪代码逻辑，以覆盖方式计算value
 * @return int 估值分数
 */
int Board_Box::evaluate_v2() {
    Chains_Define(true);

    int cv = control_value();
    int num_3_chains = count_3_chains();
    int size_G = get_board_size();
    int num_4_loops = count_4_loops();

    // 伪代码块 1
    if (cv >= 2) {
        return cv;
    } else if (cv == 0 && check_chain_composition()) {
        return 0;
    }

    // 伪代码块 2
    if (num_3_chains == 0 || (num_3_chains == 1 && size_G % 4 == 3)) {
        if (cv + num_4_loops >= 2) {
            return 0;
        } else {
            int cv_mod_8 = cv % 8;
            if (cv_mod_8 < 0) cv_mod_8 += 8;

            switch (cv_mod_8) {
                case 0: return 1;
                case 1: case 7: return 2;
                case 2: case 6: return 3;
                case 3: case 5: return 4;
                case 4: return 4;
                default: break; 
            }
        }
    }

    // 伪代码块 3
    int f = num_4_loops;
    if (cv + 4 * f < 2) {
        if (is_odd(size_G)) {
            return is_odd(f) ? 1 : 3;
        }
    }

    // 伪代码块 4
    if (size_G % 4 == 2) {
        return 2;
    } else if (size_G % 4 == 0) {
        return is_odd(num_4_loops) ? 0 : 4;
    } else { // size_G % 4 is 1 or 3
        return is_odd(size_G) ? 1 : 2;
    }
    
    return 0; // Default return if no other condition is met
}