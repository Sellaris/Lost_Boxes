#include "../src/Component/cJSON.h"
#include "../src/Component/Recorder.h"
#include "AISearch/UCT.h"
#include "AISearch/board.h"
#include "AISearch/Params.h"
#include "Component/Time.h"
#include <SFML/Graphics.hpp>
#include <future>
#include <iostream>
#include <windows.h>
#include <thread>
#include <fstream>
#include "AISearch/UCT_TT_ANN.h"
#include "AISearch/UCT_TT.h"
#include "AISearch/FeatureExtractor.h"
#include "AISearch/ANN.h"
#include <iomanip>
extern "C" {
  #include "fann.h"
}
std::string BlackName="先手";
std::string WhiteName="后手";

sf::RenderWindow mainWindow;
sf::Font font;
sf::Texture texture;
sf::Sprite sprite;

Board *gameBoard;
int nowPlayer=BLACK;
POS nowMove={-1,-1};

Time black_time;
Time white_time;

// 替换原有模式变量，使用枚举管理
enum PlayerMode{HUMAN_MODE=0,AI_OLD_MODE=1,AI_NEW_MODE=2,AI_ANN_MODE=3};
PlayerMode black_mode=HUMAN_MODE;
PlayerMode white_mode=HUMAN_MODE;

int game_begin=0;

STEPs steps[60];
int top=-1;
vector<POS> ai_steps;

sf::CircleShape now_player(10);
sf::RectangleShape first_button(sf::Vector2f(100,80));
sf::RectangleShape second_button(sf::Vector2f(100,80));
sf::RectangleShape GameButton(sf::Vector2f(300.f,100.f));
sf::RectangleShape undo_button(sf::Vector2f(150.f,100.f));
sf::RectangleShape redo_button(sf::Vector2f(150.f,100.f));
sf::RectangleShape print_button(sf::Vector2f(300,100));
sf::RectangleShape load_button(sf::Vector2f(300,100));

void showGameBoard();
void showVisualLine();
void showLine();
void showBox();
void showDots();
void initSidebar();
void showSidebar();
void showNowPlayer();
void show_Information();
void showGameBegin();
void showUndoAndRedo();
void showPrintBoard();
void showSetPlayer();
void showLoadBoard();
void handleButtons(int x,int y);
void handleBoard(int x,int y);
bool contains(sf::RectangleShape &box,int x,int y);
std::future<void> work;
int status=0;
void AIMove();

// 主函数：程序入口，初始化窗口与资源，进入主事件循环
int main(){
    SetConsoleOutputCP(65001); // 设置控制台输出为UTF-8
    gameBoard=new Board;
    //TEST
    for(int i=1;i<=10;i++){
        std::cout<<"TEST"<<i<<"\n";
        std::vector<float> input = extract_features(*gameBoard, 1);
        //input[6]=0;
        for(fann_type i:input) std::cout<<std::fixed<<std::setprecision(4)<<i<<" ";
        std::cout<<processANN(input)<<"\n";
    }

    mainWindow.create(sf::VideoMode(1500,1000),L"Sela 点格棋",sf::Style::Default);
    mainWindow.setVerticalSyncEnabled(true);
    font.loadFromFile("res/LXGWWenKai-Bold.ttf");
    texture.loadFromFile("res/board.jpg",sf::IntRect(0,0,1000,1000));
    sprite.setTexture(texture);
    std::ifstream config("config.ini");
    if(config.is_open()){
        BlackName.clear();
        WhiteName.clear();
        config>>BlackName>>WhiteName;
        config.close();
    }
    initSidebar();
    while(mainWindow.isOpen()){
        sf::Event event;
        while(mainWindow.pollEvent(event)){
            if(event.type==sf::Event::Closed){
                mainWindow.close();
            }else if(event.type==sf::Event::MouseButtonPressed&&!work.valid()){
                if(event.mouseButton.x<1000){
                    handleBoard(event.mouseButton.x,event.mouseButton.y);
                }else{
                    handleButtons(event.mouseButton.x,event.mouseButton.y);
                }
            }
        }
        if(game_begin==1) AIMove();
        mainWindow.clear(sf::Color(66,66,66));
        showGameBoard();
        showSidebar();
        mainWindow.display();
    }
    
    // 获取最终比分
    int black_score = gameBoard->BOX_of_BLACK;
    int white_score = gameBoard->BOX_of_WHITE;
    // 判断胜负
    int winner = BLACK;
    if (white_score > black_score) {winner = WHITE;}
    // 输出结果信息
    std::cout << "胜方：" 
            << (winner == BLACK ? "先手（BLACK）": "后手（WHITE）")
            << "，比分：" 
            << black_score << "-" << white_score 
            << std::endl;

    return 0;
}

// 显示游戏棋盘（背景+所有图元）
void showGameBoard(){
    mainWindow.draw(sprite);
    showDots();
    showVisualLine();
    showBox();
    showLine();
}

// 显示棋盘上的点位
void showDots(){
    for(int i=0;i<6;++i){
        for(int j=0;j<6;++j){
            sf::CircleShape c(15);
            c.setFillColor(sf::Color::Black);
            c.setPosition(110+i*150,110+j*150);
            mainWindow.draw(c);
        }
    }
}

// 显示当前鼠标位置的预览线
void showVisualLine(){
    sf::RectangleShape line(sf::Vector2f(120.f,10.f));
    sf::Vector2i pos=sf::Mouse::getPosition(mainWindow);
    int x=pos.x;
    int y=pos.y;
    line.setFillColor(sf::Color(97,97,97));
    if(x<890&&x>110&&y<890&&y>110){
        if((y-110)%150<30&&(x-140)%150<120){
            line.setPosition((x-110)/150*150+140,(y-110)/150*150+120);
            mainWindow.draw(line);
        }else if((x-110)%150<30&&(y-140)%150<120){
            line.rotate(90.f);
            line.setPosition((x-110)/150*150+130,(y-110)/150*150+140);
            mainWindow.draw(line);
        }
    }
}

// 显示游戏中的落子线段（黑线和红线）
void showLine(){
    for(int i=0;i<11;i++){
        for(int j=0;j<11;j++){
            if(gameBoard->_MOVE_NUMBERS_[i][j] > 0){
                int move_num = gameBoard->_MOVE_NUMBERS_[i][j];
                sf::Text num_text(std::to_string(move_num), font, 60);
                num_text.setFillColor(sf::Color::White);

                if(i % 2 == 0){ // horizontal line
                    sf::RectangleShape line(sf::Vector2f(120.f,10.f));
                    line.setFillColor(sf::Color::Black);
                    line.setPosition(j/2*150+140,i/2*150+120);
                    mainWindow.draw(line);

                    sf::FloatRect textRect = num_text.getLocalBounds();
                    num_text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
                    num_text.setPosition(j/2*150+140 + 60, i/2*150+120 + 5);
                    mainWindow.draw(num_text);
                }else{ // vertical line
                    sf::RectangleShape line(sf::Vector2f(10.f,120.f));
                    line.setFillColor(sf::Color::Black);
                    line.setPosition(j/2*150+120,i/2*150+140);
                    mainWindow.draw(line);

                    sf::FloatRect textRect = num_text.getLocalBounds();
                    num_text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top  + textRect.height/2.0f);
                    num_text.setPosition(j/2*150+120 + 5, i/2*150+140 + 60);
                    mainWindow.draw(num_text);
                }
            }
        }
    }

    if(nowMove.first!=-1){
        if(nowMove.first%2==0){
            sf::RectangleShape line(sf::Vector2f(120.f,2.f));
            line.setFillColor(sf::Color::Red);
            line.setPosition(nowMove.second/2*150+140,nowMove.first/2*150+124);
            mainWindow.draw(line);
        }else{
            sf::RectangleShape line(sf::Vector2f(2.f,120.f));
            line.setFillColor(sf::Color::Red);
            line.setPosition(nowMove.second/2*150+124,nowMove.first/2*150+140);
            mainWindow.draw(line);
        }
    }
}

// 显示棋盘中的占领方色块（黑/白方）
void showBox(){
    for(int i=1;i<11;i+=2){
        for(int j=1;j<11;j+=2){
            if(gameBoard->_MAP_[i][j]!=EMPTY){
                sf::RectangleShape box(sf::Vector2f(120.f,120.f));
                if(gameBoard->_MAP_[i][j]==BLACK){
                    box.setFillColor(sf::Color(244,67,54));
                }else{
                    box.setFillColor(sf::Color(33,150,243));
                }
                box.setPosition(j/2*150+140,i/2*150+140);
                mainWindow.draw(box);
            }
        }
    }
}

// 初始化右侧边栏组件样式与布局
void initSidebar(){
    now_player.setFillColor(sf::Color(158,158,158));
    now_player.setOutlineColor(sf::Color(183,28,28));
    now_player.setOutlineThickness(10);
    first_button.setPosition(1100,250);
    second_button.setPosition(1300,250);
    GameButton.setPosition(1100,350);
    undo_button.setFillColor(sf::Color(41,121,255));
    redo_button.setFillColor(sf::Color(29,233,182));
    undo_button.setPosition(1050,500);
    redo_button.setPosition(1300,500);
    print_button.setFillColor(sf::Color(121,85,72));
    print_button.setPosition(1100,650);
    load_button.setFillColor(sf::Color(69,90,100));
    load_button.setPosition(1100,800);
}

// 显示右侧边栏全部内容
void showSidebar(){
    show_Information();
    showNowPlayer();
    showSetPlayer();
    showGameBegin();
    showUndoAndRedo();
    showPrintBoard();
    showLoadBoard();
}

// 显示当前回合玩家指示器
void showNowPlayer(){
    if(nowPlayer==BLACK){
        now_player.setPosition(1050,50);
    }else{
        now_player.setPosition(1430,50);
    }
    mainWindow.draw(now_player);
}

// 显示先后手信息、分数与剩余时间
void show_Information(){
    sf::Text x_title(L"先手",font,50);
    sf::Text h_title(L"后手",font,50);
    x_title.setPosition(1100,25);
    h_title.setPosition(1300,25);
    mainWindow.draw(x_title);
    mainWindow.draw(h_title);

    sf::Text x_Rating(std::to_string(gameBoard->BOX_of_BLACK),font,40);
    sf::Text h_Rating(std::to_string(gameBoard->BOX_of_WHITE),font,40);
    x_Rating.setPosition(1130,80);
    h_Rating.setPosition(1330,80);
    mainWindow.draw(x_Rating);
    mainWindow.draw(h_Rating);

    std::string REG;
    time_t b_time=black_time.get();
    if(b_time>60){
        REG=std::to_string(b_time/60)+"m "+std::to_string(b_time%60)+"s";
    }else{
        REG=std::to_string(b_time)+"s";
    }
    sf::Text x_time(REG,font,30);

    REG.clear();
    time_t w_time=white_time.get();
    if(w_time>60){
        REG=std::to_string(w_time/60)+"m "+std::to_string(w_time%60)+"s";
    }else{
        REG=std::to_string(w_time)+"s";
    }
    sf::Text h_time(REG,font,30);
    x_time.setPosition(1130,150);
    h_time.setPosition(1330,150);
    mainWindow.draw(x_time);
    mainWindow.draw(h_time);
}

// 显示玩家类型选择按钮
void showSetPlayer(){
    sf::Text first_human_text(L"玩家",font,40);
    sf::Text first_ai_old_text(L"旧机",font,40);
    sf::Text first_ai_new_text(L"新机",font,40);
    sf::Text first_ai_ann_text(L"ANN",font,40);
    sf::Text second_human_text(L"玩家",font,40);
    sf::Text second_ai_old_text(L"旧机",font,40);
    sf::Text second_ai_new_text(L"新机",font,40);
    sf::Text second_ai_ann_text(L"ANN",font,40);
    first_human_text.setPosition(1110,260);
    first_ai_old_text.setPosition(1110,260);
    first_ai_new_text.setPosition(1110,260);
    first_ai_ann_text.setPosition(1110,260);
    second_human_text.setPosition(1310,260);
    second_ai_old_text.setPosition(1310,260);
    second_ai_new_text.setPosition(1310,260);
    second_ai_ann_text.setPosition(1310,260);

    // 先手按钮
    switch(black_mode){
        case HUMAN_MODE:
            first_button.setFillColor(sf::Color(255,196,0));
            mainWindow.draw(first_button);
            mainWindow.draw(first_human_text);
            break;
        case AI_OLD_MODE:
            first_button.setFillColor(sf::Color(1,87,155));
            mainWindow.draw(first_button);
            mainWindow.draw(first_ai_old_text);
            break;
        case AI_NEW_MODE:
            first_button.setFillColor(sf::Color(46,125,50));
            mainWindow.draw(first_button);
            mainWindow.draw(first_ai_new_text);
            break;        
        case AI_ANN_MODE:
            first_button.setFillColor(sf::Color(46,125,50));
            mainWindow.draw(first_button);
            mainWindow.draw(first_ai_ann_text);
            break;
    }
    // 后手按钮
    switch(white_mode){
        case HUMAN_MODE:
            second_button.setFillColor(sf::Color(255,196,0));
            mainWindow.draw(second_button);
            mainWindow.draw(second_human_text);
            break;
        case AI_OLD_MODE:
            second_button.setFillColor(sf::Color(1,87,155));
            mainWindow.draw(second_button);
            mainWindow.draw(second_ai_old_text);
            break;
        case AI_NEW_MODE:
            second_button.setFillColor(sf::Color(46,125,50));
            mainWindow.draw(second_button);
            mainWindow.draw(second_ai_new_text);
            break;        
        case AI_ANN_MODE:
            second_button.setFillColor(sf::Color(46,125,50));
            mainWindow.draw(second_button);
            mainWindow.draw(second_ai_ann_text);
            break;
    }
}

// 显示"开始/继续/结束游戏"按钮
void showGameBegin(){
    sf::Text BeginText("",font,50);
    BeginText.setPosition(1150,370);
    if(game_begin==0){
        BeginText.setString(L"开始游戏");
        GameButton.setFillColor(sf::Color(76,175,80));
    }else if(game_begin==1){
        BeginText.setString(L"结束游戏");
        GameButton.setFillColor(sf::Color(183,28,28));
    }else{
        BeginText.setString(L"继续游戏");
        GameButton.setFillColor(sf::Color(255,152,0));
    }
    mainWindow.draw(GameButton);
    mainWindow.draw(BeginText);
}
// 显示 Undo 与 Redo 按钮及标签
void showUndoAndRedo(){
    sf::Text undo_text("undo",font,50);
    sf::Text redo_text("redo",font,50);
    undo_text.setPosition(1070,510);
    redo_text.setPosition(1325,510);
    mainWindow.draw(undo_button);
    mainWindow.draw(redo_button);
    mainWindow.draw(undo_text);
    mainWindow.draw(redo_text);
}

// 显示"打印棋盘"按钮
void showPrintBoard(){
    sf::Text print_text(L"打印棋盘",font,50);
    print_text.setPosition(1150,675);
    mainWindow.draw(print_button);
    mainWindow.draw(print_text);
}

// 显示"加载棋局"按钮
void showLoadBoard(){
    sf::Text load_text(L"加载棋局",font,50);
    load_text.setPosition(1150,825);
    mainWindow.draw(load_button);
    mainWindow.draw(load_text);
}

// 判断鼠标点击是否在矩形框内
bool contains(sf::RectangleShape &box,int x,int y){
    x-=box.getPosition().x;
    y-=box.getPosition().y;
    return box.getLocalBounds().contains(x,y);
}

// 执行 AISearch 落子逻辑（异步搜索）
void AIMove(){
    if(!work.valid()){
        if(nowPlayer==BLACK && black_mode!=HUMAN_MODE){
            ai_steps.clear();
            if(black_mode==AI_OLD_MODE){
                work = std::async(std::launch::async, Turn_Move_Game, std::ref(*gameBoard), std::ref(nowPlayer), &status, std::ref(ai_steps), top + 1);
            }else if (black_mode==AI_NEW_MODE){
                work = std::async(std::launch::async, Turn_Move_Game_TT, std::ref(*gameBoard), std::ref(nowPlayer), &status, std::ref(ai_steps), top + 1);
            }else if (black_mode==AI_ANN_MODE){
                work = std::async(std::launch::async, Turn_Move_Game_TT_ANN, std::ref(*gameBoard), std::ref(nowPlayer), &status, std::ref(ai_steps), top + 1);
            }
        }else if(nowPlayer==WHITE && white_mode!=HUMAN_MODE){
            ai_steps.clear();
            if(white_mode==AI_OLD_MODE){
                work = std::async(std::launch::async, Turn_Move_Game, std::ref(*gameBoard), std::ref(nowPlayer), &status, std::ref(ai_steps), top + 1);
            }else if (white_mode==AI_NEW_MODE){
                work = std::async(std::launch::async, Turn_Move_Game_TT, std::ref(*gameBoard), std::ref(nowPlayer), &status, std::ref(ai_steps), top + 1);
            }else if (white_mode==AI_ANN_MODE){
                work = std::async(std::launch::async, Turn_Move_Game_TT_ANN, std::ref(*gameBoard), std::ref(nowPlayer), &status, std::ref(ai_steps), top + 1);
            }
        }
    }
    if(status){
        work.wait();
        work.get();
        if(nowPlayer==BLACK){
            black_time.stop();
            white_time.start();
        }else{
            white_time.stop();
            black_time.start();
        }
        for(const auto &i: ai_steps){
            steps[++top] = {nowPlayer, i};
            std::string REG;
            REG += ((nowPlayer==-1) ? "b(" : "r(");
            REG += static_cast<char>('a'+i.second/2);
            REG += static_cast<char>('0'+6-((i.first+1)/2));
            REG += (i.first%2==0) ? ",h)" : ",v)";
            std::cout << "AISearch Move: " << REG << "\n";
        }
        std::cout << "\n";
        if(!ai_steps.empty())
            nowMove = ai_steps.back();
        nowPlayer = -nowPlayer;
        status = 0;
    }
}
// 处理点击右侧按钮的事件逻辑
void handleButtons(int x,int y){
    if(contains(first_button,x,y)){
        black_mode = static_cast<PlayerMode>((black_mode + 1) % 4);
    }else if(contains(second_button,x,y)){
        white_mode = static_cast<PlayerMode>((white_mode + 1) % 4);
    }else if(contains(GameButton,x,y)){
        if(game_begin==1){
            delete gameBoard;
            gameBoard=new Board;
            game_begin=0;
            nowPlayer=BLACK;
            black_time.reset();
            white_time.reset();
            top=-1;
            nowMove={-1,-1};
        }else if(game_begin==0){
            game_begin=1;
            if(nowPlayer==BLACK){
                black_time.begin();
                white_time.begin();
                white_time.stop();
            }else{
                white_time.begin();
                black_time.begin();
                black_time.stop();
            }
        }else{
            game_begin=1;
            if(nowPlayer==BLACK){
                black_time.start();
            }else{
                white_time.start();
            }
        }
    }else if(contains(redo_button,x,y)){
        black_time.stop();
        white_time.stop();
        if(top!=60&&steps[top+1].player!=EMPTY){
            int temp=steps[top+1].player;
            while(top!=60&&steps[top+1].player!=EMPTY&&steps[top+1].player==temp){
                top++;
                gameBoard->move(steps[top].player,steps[top].ACTs, top + 1);
                nowMove=steps[top].ACTs;
            }
            nowPlayer=-temp;
        }
        game_begin=-1;
    }else if(contains(undo_button,x,y)){
        black_time.stop();
        white_time.stop();
        if(top!=-1){
            int temp=steps[top].player;
            while(top!=-1&&steps[top].player==temp){
                gameBoard->undo_move_sim(steps[top].ACTs);
                top--;
            }
            nowMove=steps[top].ACTs;
            nowPlayer=temp;
        }
        game_begin=-1;
    }else if(contains(print_button,x,y)){
        Game_Date ginfo=Game_Date(gameBoard->BOX_of_BLACK,gameBoard->BOX_of_WHITE,BlackName.c_str(),WhiteName.c_str());
        ginfo.STEPsRecord(steps);
        ginfo.RecordEnd();
        ginfo.DataPrint();
        ginfo.deleterecord();
    }else if(contains(load_button,x,y)){
        // 预留加载棋盘操作
        //TODO 加载棋盘
    }
}

// 处理点击棋盘区域进行落子的逻辑
void handleBoard(int x,int y){
    if(x>890||x<110||y>890||y<110) return;
    if((y-110)%150<30&&(x-140)%150<120){
        int bx=(y-110)/150*2;
        int by=(x-140)/150*2+1;
        if(!gameBoard->is_Free_Line(bx,by)) return;
        steps[++top]={nowPlayer,POS{bx,by}};
        nowMove={bx,by};
        if(gameBoard->move(nowPlayer,{bx,by}, top + 1)==0){
            if(game_begin==1){
                if(nowPlayer==BLACK){
                    black_time.stop();
                    white_time.start();
                }else{
                    white_time.stop();
                    black_time.start();
                }
            }
            nowPlayer=-nowPlayer;
        }
    }
    if((x-110)%150<30&&(y-140)%150<120){
        int bx=(y-140)/150*2+1;
        int by=(x-110)/150*2;
        if(!gameBoard->is_Free_Line(bx,by)) return;
        steps[++top]={nowPlayer,POS{bx,by}};
        nowMove={bx,by};
        if(gameBoard->move(nowPlayer,{bx,by}, top + 1)==0){
            if(game_begin==1){
                if(nowPlayer==BLACK){
                    black_time.stop();
                    white_time.start();
                }else{
                    white_time.stop();
                    black_time.start();
                }
            }
            nowPlayer=-nowPlayer;
        }
    }
}