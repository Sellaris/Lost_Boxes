// Board_Box_Plus.cpp
#include "Board_Box.h"
#include <unordered_set>
#include <algorithm>
#include <chrono>
static const int dr[4] = {-1, 0, 1,  0};
static const int dc[4] = { 0, 1, 0, -1};

struct custom_hash {
    // 一个高质量的 64 位混合函数
    static uint64_t splitmix64(uint64_t x) {
        // 参考量子随机数发生器的位混合算法
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }
    // 用于种子化，保证每次运行哈希结果不同（防撞库攻击）
    size_t operator()(uint64_t x) const {
        static const uint64_t FIXED_RANDOM =
            std::chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + FIXED_RANDOM);
    }
    // 针对 pair<int,int> 的哈希函数
    size_t operator()(const POS& p) const {
        static const uint64_t FIXED_RANDOM =
            std::chrono::steady_clock::now().time_since_epoch().count();
        // 将 first 和 second 分别哈希后再合并
        uint64_t h1 = splitmix64(static_cast<uint64_t>(p.first)  + FIXED_RANDOM);
        uint64_t h2 = splitmix64(static_cast<uint64_t>(p.second) + FIXED_RANDOM);
        // 简单地异或或相加都可以，这里选用异或以减少碰撞
        return h1 ^ (h2 << 1);
    }
};
Board_Box_Plus::Board_Box_Plus(const Board& b)
  : _board(b)
{ }

int Board_Box_Plus::CountDrawnEdges(int bx, int by) const {
    // 总共 4 条边，减去自由边数即可
    return 4 - _board.TAKE__Free_dom(bx, by);
}

POS Board_Box_Plus::MissingEdge(int bx, int by) const {
    // 找那条还没被占的边
    for (int d = 0; d < 4; ++d) {
        int nr = bx + dr[d], nc = by + dc[d];
        POS e = _board.Public_Side_GET({bx,by}, {nr, nc});
        if (e.first>=0 && _board.is_Free_Line(e)) {
            return e;
        }
    }
    return {-1,-1}; // 不应到这里
}

std::tuple<int,int,POS> 
Board_Box_Plus::TraceNext(int bx, int by, const POS& e0) const {
    // 已知 e0 是 (bx,by) 的一个缺口边，找到另一个缺口
    POS found = {-1,-1};
    for (int d = 0; d < 4; ++d) {
        int nr = bx + dr[d], nc = by + dc[d];
        POS e = _board.Public_Side_GET({bx,by}, {nr,nc});
        if (e == e0) continue;
        if (e.first>=0 && _board.is_Free_Line(e)) {
            found = e;
            break;
        }
    }
    // 根据 found 推断下一个中心格
    int nbx = bx + ((found.first==bx) ? 0 : (found.first>bx ? 1 : -1));
    int nby = by + ((found.second==by) ? 0 : (found.second>by ? 1 : -1));
    return {nbx, nby, found};
}

void Board_Box_Plus::GetChainsAndCycles(
    std::vector<std::vector<POS>>& chains,
    std::vector<std::vector<POS>>& cycles
) {
    // —— 1. 链检测 ——  
    std::unordered_set<POS,custom_hash> used;
    for (int bx = 1; bx < LEN; bx += 2) {
        for (int by = 1; by < LEN; by += 2) {
            if (CountDrawnEdges(bx,by) != 2) continue;
            // 找到所有缺口边
            std::vector<POS> free_edges;
            for (int d = 0; d < 4; ++d) {
                int nr = bx + dr[d], nc = by + dc[d];
                POS e = _board.Public_Side_GET({bx,by}, {nr, nc});
                if (e.first >= 0 && _board.is_Free_Line(e)) {
                    free_edges.push_back(e);
                }
            }
            // 对每条缺口边，若未访问，则从该边出发追踪整条链
            for (const POS& start_edge : free_edges) {
                if (used.count(start_edge)) continue;
                std::vector<POS> chain;
                // 向两个方向追踪
                for (int dir = 0; dir < 2; ++dir) {
                    int cx = bx, cy = by;
                    POS cur_edge = start_edge;
                    int prev_bx = -1, prev_by = -1;
                    while (true) {
                        if (dir == 0) chain.push_back(cur_edge); // 正向加入
                        else chain.insert(chain.begin(), cur_edge); // 反向插入头部
                        used.insert(cur_edge);
                        // 找到下一个格子
                        int next_bx = cx, next_by = cy;
                        if (cur_edge.first == cx) {
                            next_bx = cx;
                            next_by = (cur_edge.second == cy) ? cy : cur_edge.second + (cur_edge.second > cy ? -1 : 1);
                        } else if (cur_edge.second == cy) {
                            next_by = cy;
                            next_bx = (cur_edge.first == cx) ? cx : cur_edge.first + (cur_edge.first > cx ? -1 : 1);
                        }
                        if (next_bx < 1 || next_bx >= LEN || next_by < 1 || next_by >= LEN) break;
                        if (next_bx == prev_bx && next_by == prev_by) break;
                        int deg = CountDrawnEdges(next_bx, next_by);
                        if (deg != 2 && deg != 3) break;
                        // 找下一个缺口边
                        POS next_edge = {-1,-1};
                        for (int nd = 0; nd < 4; ++nd) {
                            int nnr = next_bx + dr[nd], nnc = next_by + dc[nd];
                            POS e = _board.Public_Side_GET({next_bx, next_by}, {nnr, nnc});
                            if (e.first < 0 || ! _board.is_Free_Line(e)) continue;
                            if (e == cur_edge) continue;
                            next_edge = e;
                            break;
                        }
                        if (next_edge.first < 0) break;
                        prev_bx = cx; prev_by = cy;
                        cx = next_bx; cy = next_by;
                        cur_edge = next_edge;
                        if (deg == 2) break; // 到达端点
                    }
                }
                if (!chain.empty()) chains.push_back(chain);
            }
        }
    }

    // —— 2. 环检测 ——  
    // 收集中间箱的两条缺口边
    std::unordered_set<POS,custom_hash> midEdges;
    for (int bx = 1; bx < LEN; bx += 2) {
        for (int by = 1; by < LEN; by += 2) {
            if (CountDrawnEdges(bx,by) == 2) {
                POS a = MissingEdge(bx,by);
                auto [nx,ny,b] = TraceNext(bx,by,a);
                midEdges.insert(a);
                if (_board.is_Free_Line(b)) midEdges.insert(b);
            }
        }
    }
    // 在 midEdges 上按上下左右(步长2)做 DFS 找简单环
    auto neighbors = [&](const POS& u){
        std::vector<POS> res;
        for (auto d : std::initializer_list<std::pair<int,int>>{{2,0},{-2,0},{0,2},{0,-2}}) {
            POS v = {u.first + d.first, u.second + d.second};
            if (midEdges.count(v) && _board.is_Free_Line(v)) res.push_back(v);
        }
        return res;
    };
    std::unordered_set<POS,custom_hash> vis;
    std::vector<POS> stk;
    std::function<void(POS,POS)> dfs = [&](POS u, POS p){
        vis.insert(u);
        stk.push_back(u);
        for (auto& v : neighbors(u)) {
            if (v == p) continue;
            auto it = std::find(stk.begin(), stk.end(), v);
            if (it != stk.end()) {
                std::vector<POS> cycle(it, stk.end());
                if (cycle.size() >= 4) cycles.push_back(cycle);
            } else if (!vis.count(v)) {
                dfs(v, u);
            }
        }
        stk.pop_back();
    };
    for (auto& e : midEdges) {
        if (!vis.count(e)) dfs(e, {-1,-1});
    }
}
