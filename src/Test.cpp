#include "AISearch/board.h"
#include "AISearch/UCT.h"
#include "AISearch/UCT+.h"
#include <vector>
#include <iostream>
#include <Windows.h>


int main() {    
    SetConsoleOutputCP(65001); // 设置控制台输出为UTF-8

    int uct_win = 0, uct_plus_win = 0, draw = 0;
    int total_games = 25;

    for (int round = 0; round < total_games * 2; ++round) {
        Board board;
        int nowPlayer = (round %2) ? BLACK : WHITE; // 前10局UCT+先手，后10局UCT先手
        int status = 0;
        std::vector<POS> ai_steps;
        std::vector<STEPs> steps; // 用vector防止越界
        int move_count = 0;

        while (true) {
            ai_steps.clear();
            if (nowPlayer == BLACK) {
                if (round %2) {
                    // UCT+ 先手
                    Turn_Move_Game_PLUS(board, nowPlayer, &status, ai_steps, steps.size() + 1);
                } else {
                    // UCT 先手
                    Turn_Move_Game(board, nowPlayer, &status, ai_steps, steps.size() + 1);
                }
            } else {
                if (round %2) {
                    // UCT 后手
                    Turn_Move_Game(board, nowPlayer, &status, ai_steps, steps.size() + 1);
                } else {
                    // UCT+ 后手
                    Turn_Move_Game_PLUS(board, nowPlayer, &status, ai_steps, steps.size() + 1);
                }
            }
            if (ai_steps.empty()) {
                std::cout << "AI无可走步，强制结束。" << std::endl;
                break;
            }
            for (const auto& pos : ai_steps) {
                steps.push_back({nowPlayer, pos});
                board.move(nowPlayer, pos, steps.size());
                move_count++;
            }
            if (board.is_game_over()) break;
            nowPlayer = -nowPlayer;
        }

        int black_score = board.BOX_of_BLACK;
        int white_score = board.BOX_of_WHITE;
        if (black_score > white_score) {
            if (round %2) uct_plus_win++;
            else uct_win++;
        } else if (white_score > black_score) {
            if (round %2) uct_win++;
            else uct_plus_win++;
        } else {
            draw++;
        }
        std::cout << "第" << (round+1) << "局结束，BLACK: " << black_score << " WHITE: " << white_score << " 总步数:" << move_count << std::endl;
        if (round %2 && black_score > white_score) {
            std::cout << "UCT+胜" << std::endl;
        }else if (round %2 && white_score > black_score) {
            std::cout << "UCT胜" << std::endl;
        }else if (!(round %2) && black_score > white_score) {
            std::cout << "UCT胜" << std::endl;
        } else if (!(round %2) && white_score > black_score) {
            std::cout << "UCT+胜" << std::endl;
        }
    }

    std::cout << "UCT+胜局: " << uct_plus_win << std::endl;
    std::cout << "UCT胜局: " << uct_win << std::endl;
    std::cout << "平局: " << draw << std::endl;
    std::cout << "UCT+胜率: " << (uct_plus_win * 100.0 / (total_games * 2)) << "%" << std::endl;
    std::cout << "UCT胜率: " << (uct_win * 100.0 / (total_games * 2)) << "%" << std::endl;
    system("pause");
    return 0;
}
