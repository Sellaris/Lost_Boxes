#include "board.h"
#include <cstring>

/**
 * @brief 默认构造函数，初始化棋盘状态
 * @details 同为偶是点，同为奇是格子，先偶后奇是横线，先奇后偶是竖线
 * @return 无返回值
 */
Board::Board() {
	for(int i=0;i<11;++i) {
		for(int j=0;j<11;++j) {
			if(i%2==0&&j%2==0) {
				_MAP_[i][j]=DOT;
			} else if(i%2==0&&j%2==1) {
				_MAP_[i][j]=Horizon;
			} else if(i%2==1&&j%2==1) {
				_MAP_[i][j]=_BOX_;
			} else {
				_MAP_[i][j]=Vertical;
			}
		}
	}
	BOX_of_BLACK=BOX_of_WHITE=0;
}

/**
 * @brief 带参数构造函数，根据数组初始化棋盘
 * @param Array 输入的棋盘数组
 * @return 无返回值
 */
Board::Board(int Array[LEN][LEN]) {
	for(int i=0;i<LEN;i++) {
		for(int j=0;j<LEN;j++) {
			_MAP_[i][j]=Array[i][j];
		}
	}
	memset(_MOVE_NUMBERS_, 0, sizeof(_MOVE_NUMBERS_));
	BOX_of_BLACK=BOX_of_WHITE=0;
	for(int i=1;i<11;i+=2) {
		for(int j=1;j<11;j+=2) {
			if(_MAP_[i][j]==BLACK) {
				BOX_of_BLACK++;
			} else if(_MAP_[i][j]==WHITE) {
				BOX_of_WHITE++;
			}
		}
	}
}

/**
 * @brief 获得一个格子的自由度，自由度即格子周围未被占领的边的数量
 * @param x 横坐标
 * @param y 纵坐标
 * @return int 自由度
 */
int Board::TAKE__Free_dom(int x,int y) {
	int cnt=4;
	if(_MAP_[x-1][y]==OCCed_LINE) --cnt;
	if(_MAP_[x+1][y]==OCCed_LINE) --cnt;
	if(_MAP_[x][y-1]==OCCed_LINE) --cnt;
	if(_MAP_[x][y+1]==OCCed_LINE) --cnt;
	return cnt;
}

/**
 * @brief 通过POS返回自由度
 * @param l 坐标
 * @return int 自由度
 */
int Board::TAKE__Free_dom(POS l) {
	return TAKE__Free_dom(l.first,l.second);
}

/**
 * @brief 占线操作
 * @details 默认传进来的线都是没有被占过的，如果传进来的线是横线，要考虑它上下两个格子自由度是否为3；如果已经是3，那么再占一条线，这个格子就属于传进来下棋方的。同时要考虑边界情况，比如横线在最上面时，只需考虑线下面的格子，同理，如果是竖线，就要考虑左右两个格子
 * @param player 代表下棋方
 * @param l 代表要占的线的坐标
 * @return int 返回的是这条线产生的被占了的格子的数目
 */
int Board::move(int player,POS l, int move_num) {
	int res=0;
	if(!is_Free_Line(l)) return 0;
	if(move_num > 0) _MOVE_NUMBERS_[l.first][l.second] = move_num;
	if(_MAP_[l.first][l.second]==Horizon) {
		if(l.first==0) {
			if(TAKE__Free_dom(1,l.second)==1) {
				_MAP_[1][l.second]=player;
				++res;
			}
		} else if(l.first==10) {
			if(TAKE__Free_dom(9,l.second)==1) {
				_MAP_[9][l.second]=player;
				++res;
			}
		} else {
			if(TAKE__Free_dom(l.first-1,l.second)==1) {
				_MAP_[l.first-1][l.second]=player;
				++res;
			}
			if(TAKE__Free_dom(l.first+1,l.second)==1) {
				_MAP_[l.first+1][l.second]=player;
				++res;
			}
		}
	} else {
		if(l.second==0) {
			if(TAKE__Free_dom(l.first,1)==1) {
				_MAP_[l.first][1]=player;
				++res;
			}
		} else if(l.second==10) {
			if(TAKE__Free_dom(l.first,9)==1) {
				_MAP_[l.first][9]=player;
				++res;
			}
		} else {
			if(TAKE__Free_dom(l.first,l.second-1)==1) {
				_MAP_[l.first][l.second-1]=player;
				++res;
			}
			if(TAKE__Free_dom(l.first,l.second+1)==1) {
				_MAP_[l.first][l.second+1]=player;
				++res;
			}
		}
	}
	_MAP_[l.first][l.second]=OCCed_LINE;
	player==BLACK?BOX_of_BLACK+=res:BOX_of_WHITE+=res;
	return res;
}

/**
 * @brief 吃C型格
 * @param player 代表此时的下棋方
 * @return POS 返回一个C型格的坐标，找不到返回{-1，-1}
 */
POS Board::_TAKE_C_Box(int player, int move_num) {
	for(int i=1;i<11;i+=2) {
		for(int j=1;j<11;j+=2) {
			if(TAKE__Free_dom(i,j)==1) {
				if(_MAP_[i-1][j]!=OCCed_LINE) {
					move(player,{i-1,j}, move_num);
					return{i-1,j};
				} else if(_MAP_[i+1][j]!=OCCed_LINE) {
					move(player,{i+1,j}, move_num);
					return{i+1,j};
				} else if(_MAP_[i][j+1]!=OCCed_LINE) {
					move(player,{i,j+1}, move_num);
					return{i,j+1};
				} else {
					move(player,{i,j-1}, move_num);
					return{i,j-1};
				}
			}
		}
	}
	return{-1,-1};
}

/**
 * @brief 判断是否为自由线
 * @param l 坐标
 * @return bool 是否为自由线
 */
bool Board::is_Free_Line(POS l) const {
	return _MAP_[l.first][l.second]==Vertical || _MAP_[l.first][l.second]==Horizon;
}

/**
 * @brief 判断是否为自由线
 * @param i 横坐标
 * @param j 纵坐标
 * @return bool 是否为自由线
 */
bool Board::is_Free_Line(int i,int j) const {
	return _MAP_[i][j]==Vertical||_MAP_[i][j]==Horizon;
}

/**
 * @brief 判断一个坐标是否会造成C型格
 * @param l 坐标
 * @return bool true表示会造成C型格
 */
bool Board::MAKE_C_Box_TRUE(POS l) {
	if(_MAP_[l.first][l.second]==Horizon) {
		if(l.first==0) {
			return TAKE__Free_dom(l.first,l.second+1)==2;
		} else if(l.first==10) {
			return TAKE__Free_dom(l.first,l.second-1)==2;
		} else {
			return TAKE__Free_dom(l.first,l.second-1)==2||TAKE__Free_dom(l.first,l.second+1)==2;
		}
	} else {
		if(l.second==0) {
			return TAKE__Free_dom(l.first+1,l.second)==2;
		} else if(l.second==10) {
			return TAKE__Free_dom(l.first-1,l.second)==2;
		} else {
			return TAKE__Free_dom(l.first-1,l.second)==2||TAKE__Free_dom(l.first+1,l.second)==2;
		}
	}
}

/**
 * @brief 判断是否赢得格子
 * @param l 需要判断的边的坐标
 * @return bool 如果传进来的l占据了格子，返回true，否则返回false
 */
bool Board::EARN_BOX_TRUE(POS l) {
	if(_MAP_[l.first][l.second]==Horizon) {
		if(l.first==0) {
			return TAKE__Free_dom(1,l.second)==1;
		} else if(l.first==10) {
			return TAKE__Free_dom(9,l.second)==1;
		} else {
			return TAKE__Free_dom(l.first-1,l.second)==1||TAKE__Free_dom(l.first+1,l.second)==1;
		}
	} else {
		if(l.second==0) {
			return TAKE__Free_dom(l.first,1)==1;
		} else if(l.second==10) {
			return TAKE__Free_dom(l.first,9)==1;
		} else {
			return TAKE__Free_dom(l.first,l.second-1)==1||TAKE__Free_dom(l.first,l.second+1)==1;
		}
	}
}

/**
 * @brief 判断棋局是否结束
 * @return bool 如果游戏已经结束，返回true，否则返回false
 */
bool Board::END_TRUE() const {
	return BOX_of_BLACK+BOX_of_WHITE==25;
}

/**
 * @brief 判断赢家
 * @return int 如果黑方胜利，返回BLACK，白方胜利返回WHITE，目前无胜方返回EMPTY
 */
int Board::Winner_TAKE_() const {
	if(BOX_of_BLACK+BOX_of_WHITE!=25) return EMPTY;
	return BOX_of_BLACK>BOX_of_WHITE?BLACK:WHITE;
}

/**
 * @brief 占领C型格类型限制
 * @param Player 玩家
 * @param pace 步伐记录
 * @return bool 是否成功占领
 */
bool Board::TAKE__C_Box_Type_Limit(int Player,vector<POS>&pace, int total_moves) {
	for(int y=1;y<LEN-1;y+=2) {
		for(int x=1;x<LEN-1;x+=2) {
			if(TAKE__C_Type__BOX__TRUE(x,y)) {
				if((_MAP_[x+1][y]==Horizon||_MAP_[x+1][y]==Vertical)&&!TAKE__C_Type__BOX__TRUE(x+2,y)) {
					move(Player,{x+1,y}, total_moves + pace.size() + 1);
					pace.emplace_back(x+1,y);
					return true;
				} else if((_MAP_[x-1][y]==Horizon||_MAP_[x-1][y]==Vertical)&&!TAKE__C_Type__BOX__TRUE(x-2,y)) {
					move(Player,{x-1,y}, total_moves + pace.size() + 1);
					pace.emplace_back(x-1,y);
					return true;
				} else if((_MAP_[x][y+1]==Horizon||_MAP_[x][y+1]==Vertical)&&!TAKE__C_Type__BOX__TRUE(x,y+2)) {
					move(Player,{x,y+1}, total_moves + pace.size() + 1);
					pace.emplace_back(x,y+1);
					return true;
				} else if((_MAP_[x][y-1]==Vertical||_MAP_[x][y-1]==Vertical)&&!TAKE__C_Type__BOX__TRUE(x,y-2)) {
					move(Player,{x,y-1}, total_moves + pace.size() + 1);
					pace.emplace_back(x,y-1);
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * @brief 复制棋盘
 * @param SOUR 源棋盘
 * @param TarTAKE_ 目标棋盘
 * @return 无返回值
 */
void COPY_BOARD(int SOUR[LEN][LEN],int TarTAKE_[LEN][LEN]) {
	for(int y=0;y<LEN;y++) {
		for(int x=0;x<LEN;x++) {
			TarTAKE_[x][y]=SOUR[x][y];
		}
	}
}

/**
 * @brief 设置棋盘
 * @param Array 棋盘数组
 * @return 无返回值
 */
void Board::setBoard(int Array[LEN][LEN]) {
	for(int i=0;i<LEN;i++) {
		for(int j=0;j<LEN;j++) {
			_MAP_[i][j]=Array[i][j];
		}
	}
	memset(_MOVE_NUMBERS_, 0, sizeof(_MOVE_NUMBERS_));
	BOX_of_BLACK=BOX_of_WHITE=0;
	for(int i=1;i<11;i+=2) {
		for(int j=1;j<11;j+=2) {
			if(_MAP_[i][j]==BLACK) {
				BOX_of_BLACK++;
			} else if(_MAP_[i][j]==WHITE) {
				BOX_of_WHITE++;
			}
		}
	}
}

/**
 * @brief 获取过滤后的移动数量
 * @return int 移动数量
 */
int Board::TAKE_Num_of_FILTER_Move() {
	int Move_Num=0;
	for(int x=0;x<11;x+=2) {
		for(int y=1;y<11;y+=2) {
			if(_MAP_[x][y]==Horizon) {
				int BoardSave[LEN][LEN];
				COPY_BOARD(_MAP_,BoardSave);
				move(BLACK,{x,y});
				if(x==0) {
					if(!TAKE_Long_C_Type__BOX__TRUE(x+1,y)) Move_Num++;
				} else if(x==LEN-1) {
					if(!TAKE_Long_C_Type__BOX__TRUE(x-1,y)) Move_Num++;
				} else {
					if(!TAKE_Long_C_Type__BOX__TRUE(x+1,y)&&!TAKE_Long_C_Type__BOX__TRUE(x-1,y)) Move_Num++;
				}
				setBoard(BoardSave);
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
					if(!TAKE_Long_C_Type__BOX__TRUE(x,y+1)) Move_Num++;
				} else if(y==LEN-1) {
					if(!TAKE_Long_C_Type__BOX__TRUE(x,y-1)) Move_Num++;
				} else {
					if(!TAKE_Long_C_Type__BOX__TRUE(x,y+1)&&!TAKE_Long_C_Type__BOX__TRUE(x,y-1)) Move_Num++;
				}
				setBoard(BoardSave);
			}
		}
	}
	return Move_Num;
}

/**
 * @brief 判断是否为C型格
 * @param bx 横坐标
 * @param by 纵坐标
 * @return bool 是否为C型格
 */
bool Board::TAKE__C_Type__BOX__TRUE(int bx,int by) {
	if(bx>=1&&bx<=LEN-2&&by>=1&&by<=LEN-2&&Odd_TRUE(bx)&&Odd_TRUE(by)) {
		return TAKE__Free_dom(bx,by)==DEAD_BOX_;
	}
	return false;
}

/**
 * @brief 占领所有C型格
 * @param Player 玩家
 * @param pace 步伐记录
 * @return 无返回值
 */
void Board::_TAKE_All_C_TYPE_Boxes(int Player,vector<POS>&pace, int total_moves) {
	POS flag={-1,-1};
	while(true) {
		POS tmp=_TAKE_C_Box(Player, total_moves + pace.size() + 1);
		if(tmp==flag) break;
		pace.emplace_back(tmp);
	}
}

/**
 * @brief 占领所有C型格
 * @param Player 玩家
 * @return 无返回值
 */
void Board::_TAKE_All_C_Boxes(int Player, int& move_count, bool do_numbering) {
	POS flag={-1,-1};
	while(true) {
		int move_num = do_numbering ? move_count : 0;
		POS tmp=_TAKE_C_Box(Player, move_num);
		if(tmp==flag) break;
		if(do_numbering) move_count++;
	}
}

/**
 * @brief 判断是否存在长C型格
 * @return bool 是否存在长C型格
 */
bool Board::TAKE_Long_C_Box_Type_Exist() {
	for(int by=1;by<LEN-1;by+=2) {
		for(int bx=1;bx<LEN-1;bx+=2) {
			if(TAKE__Free_dom(bx,by)==DEAD_BOX_) {
				for(auto n:WAY) {
					int ex=bx+n[0];
					int ey=by+n[1];
					int nbx=bx+n[0]+n[0];
					int nby=by+n[1]+n[1];
					if((_MAP_[ex][ey]==Horizon||_MAP_[ex][ey]==Vertical)&&nbx>=1&&nbx<=LEN-2&&nby>=1&&nby<=LEN-2) {
						if(TAKE__Free_dom(nbx,nby)==CHAIN_BOX_) return true;
					}
				}
			}
		}
	}
	return false;
}

/**
 * @brief 获取自由边数量
 * @return int 自由边数量
 */
int Board::TAKE__Free_Edge_Num() {
	int Edge_Num=0;
	for(int y=1;y<LEN-1;y+=2) {
		if(TAKE__Free___BOX__TRUE(1,y)&&_MAP_[0][y]==Horizon) Edge_Num++;
		for(int x=1;x<LEN-3;x+=2) {
			if(TAKE__Free___BOX__TRUE(x,y)&&TAKE__Free___BOX__TRUE(x+2,y)&&_MAP_[x+1][y]==Horizon) Edge_Num++;
		}
		if(TAKE__Free___BOX__TRUE(LEN-2,y)&&_MAP_[LEN-1][y]==Horizon) Edge_Num++;
		if(TAKE__Free___BOX__TRUE(y,1)&&_MAP_[y][0]==Vertical) Edge_Num++;
		for(int x=1;x<LEN-3;x+=2) {
			if(TAKE__Free___BOX__TRUE(y,x)&&TAKE__Free___BOX__TRUE(y,x+2)&&_MAP_[y][x+1]==Vertical) Edge_Num++;
		}
		if(TAKE__Free___BOX__TRUE(y,LEN-2)&&_MAP_[y][LEN-1]==Vertical) Edge_Num++;
	}
	return Edge_Num;
}

/**
 * @brief 判断是否为自由格
 * @param bx 横坐标
 * @param by 纵坐标
 * @return bool 是否为自由格
 */
bool Board::TAKE__Free___BOX__TRUE(int bx,int by) {
	if(1<=bx&&LEN-2>=bx&&1<=by&&LEN-2>=by&&Odd_TRUE(bx)&&Odd_TRUE(by)) {
		return TAKE__Free_dom(bx,by)>=3;
	}
	return false;
}

/**
 * @brief 撤销移动模拟
 * @param l 坐标
 * @return 无返回值
 */
void Board::undo_move_sim(POS l) {
	_MOVE_NUMBERS_[l.first][l.second] = 0;
	if(l.first%2==0&&l.second%2==1) {
		if(l.first==0) {
			switch(_MAP_[l.first+1][l.second]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first+1][l.second]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first+1][l.second]=EMPTY;break;
				default:break;
			}
		} else if(l.first==10) {
			switch(_MAP_[l.first-1][l.second]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first-1][l.second]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first-1][l.second]=EMPTY;break;
				default:break;
			}
		} else {
			switch(_MAP_[l.first+1][l.second]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first+1][l.second]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first+1][l.second]=EMPTY;break;
				default:break;
			}
			switch(_MAP_[l.first-1][l.second]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first-1][l.second]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first-1][l.second]=EMPTY;break;
				default:break;
			}
		}
		_MAP_[l.first][l.second]=Horizon;
	} else if(l.first%2==1&&l.second%2==0) {
		if(l.second==0) {
			switch(_MAP_[l.first][l.second+1]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first][l.second+1]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first][l.second+1]=EMPTY;break;
				default:break;
			}
		} else if(l.second==10) {
			switch(_MAP_[l.first][l.second-1]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first][l.second-1]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first][l.second-1]=EMPTY;break;
				default:break;
			}
		} else {
			switch(_MAP_[l.first][l.second+1]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first][l.second+1]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first][l.second+1]=EMPTY;break;
				default:break;
			}
			switch(_MAP_[l.first][l.second-1]) {
				case BLACK:BOX_of_BLACK--;_MAP_[l.first][l.second-1]=EMPTY;break;
				case WHITE:BOX_of_WHITE--;_MAP_[l.first][l.second-1]=EMPTY;break;
				default:break;
			}
		}
		_MAP_[l.first][l.second]=Vertical;
	}
}

/**
 * @brief 判断是否存在安全边
 * @return bool 是否存在安全边
 */
bool Board::Safe_Edge_TRUE() {
	for(int i=1;i<11;i+=2) {
		for(int j=0;j<11;j+=2) {
			if(_MAP_[i][j]!=OCCed_LINE&&!MAKE_C_Box_TRUE(POS{i,j})) return true;
		}
	}
	for(int i=0;i<11;i+=2) {
		for(int j=1;j<11;j+=2) {
			if(_MAP_[i][j]!=OCCed_LINE&&!MAKE_C_Box_TRUE(POS{i,j})) return true;
		}
	}
	return false;
}

/**
 * @brief 获取公共边
 * @param a 坐标a
 * @param b 坐标b
 * @return POS 公共边坐标
 */
POS Board::Public_Side_GET(POS a,POS b) {
	POS res={-1,-1};
	POS A,B;
	A.first=a.first*2-1,A.second=a.second*2-1;
	B.first=b.first*2-1,B.second=b.second*2-1;
	if(A.first==B.first&&A.second==(B.second-2)) res={B.first,B.second-1};
	if(A.first==B.first&&A.second==(B.second+2)) res={B.first,B.second+1};
	if(A.second==B.second&&A.first==(B.first-2)) res={B.first-1,B.second};
	if(A.second==B.second&&A.first==(B.first+2)) res={B.first+1,B.second};
	return res;
}

/**
 * @brief 获取玩家的格子数量
 * @param player 玩家
 * @return int 格子数量
 */
int Board::TAKE_Boxes_OF_Players(int player) const {
	switch(player) {
		case BLACK:return BOX_of_BLACK;
		case WHITE:return BOX_of_WHITE;
		default:return 0;
	}
}

/**
 * @brief 判断数字是否为奇数
 * @param num 数字
 * @return bool 是否为奇数
 */
bool Odd_TRUE(int num) {
	return num%2!=0;
}

/**
 * @brief 判断数字是否为偶数
 * @param num 数字
 * @return bool 是否为偶数
 */
bool Even_TRUE(int num) {
	return num%2==0;
}

/**
 * @brief 判断一个格子是否是一条长死格的起点
 * @param bx 横坐标
 * @param by 纵坐标
 * @return bool 是否为长死格起点
 */
bool Board::TAKE_Long_C_Type__BOX__TRUE(int bx,int by) {
	if(TAKE__Free_dom(bx,by)==DEAD_BOX_) {
		for(auto n:WAY) {
			int ex=bx+n[0];
			int ey=by+n[1];
			int nbx=bx+n[0]+n[0];
			int nby=by+n[1]+n[1];
			if((_MAP_[ex][ey]==Horizon||_MAP_[ex][ey]==Vertical)&&nbx>=1&&nbx<=LEN-2&&nby>=1&&nby<=LEN-2) {
				if(TAKE__Free_dom(nbx,nby)==CHAIN_BOX_) return true;
			}
		}
	}
	return false;
}

/**
 * @brief 获取双连接位置
 * @param Player 玩家
 * @return POS 双连接位置
 */
POS Board::TAKE_Double_LINK_Position(int Player) {
	for(int by=1;by<LEN-1;by+=2) {
		for(int bx=1;bx<LEN-1;bx+=2) {
			if(TAKE_Long_C_Type__BOX__TRUE(bx,by)) {
				for(auto n:WAY) {
					int ex=bx+n[0];
					int ey=by+n[1];
					int nbx=bx+n[0]+n[0];
					int nby=by+n[1]+n[1];
					if((_MAP_[ex][ey]==Vertical|_MAP_[ex][ey]==Horizon)&&nbx>=1&&nbx<=LEN-2&&nby>=1&&nby<=LEN-2) {
						for(auto dir:WAY) {
							int nex=nbx+dir[0];
							int ney=nby+dir[1];
							if((_MAP_[nex][ney]==Horizon||_MAP_[nex][ney]==Vertical)&&(nex!=ex||ney!=ey)) {
								return{nex,ney};
							}
						}
					}
				}
			}
		}
	}
	return{0,0};
}