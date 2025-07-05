// Board_Box_Plus.cpp – **全面重写**：按“自由度=2”定义重新检测 2‑链、3‑链，以及环。
// 兼容原 Node_Plus 等价裁剪接口：
//    • chains   → 含所有开放边（POS）的一维向量；
//    • cycles   → 同上。
// 逻辑要点
//    1. 链/环组件 = 由 **所有自由度=2 的格子** 经其自由边互联构成的连通分量。
//    2. 若组件中每个格子在组件内的邻接度 = 2 ➜ 环；否则 ➜ 链（含单格 & 二格短链、三格链等）。
//    3. 单格短链（自由边成对边缘 Opposite）仍直接作为一条 2 边链写入 chains。
//    4. 四角等价（自由度=4）保持在 Node_Plus 中处理。

#include "Board_Box.h"
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <chrono>

static const int dr[4] = { -1, 0, 1,  0 };
static const int dc[4] = {  0, 1, 0, -1 };

// ----------------------------- POS 哈希 -----------------------------
struct custom_hash {
    static uint64_t splitmix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    size_t operator()(uint64_t x) const {
        static const uint64_t FIXED_RANDOM =
            std::chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + FIXED_RANDOM);
    }
    size_t operator()(const POS &p) const {
        static const uint64_t FIXED_RANDOM =
            std::chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(static_cast<uint64_t>(p.first) + FIXED_RANDOM) ^
               (splitmix64(static_cast<uint64_t>(p.second) + FIXED_RANDOM) << 1);
    }
};

// --------------------------- ctor & helpers -------------------------
Board_Box_Plus::Board_Box_Plus(const Board &b) : _board(b) {}

inline int Board_Box_Plus::CountDrawnEdges(int bx, int by) const {
    return 4 - _board.TAKE__Free_dom(bx, by);
}

// 返回格子 (bx,by) 所有自由边 POS 向量
static inline void collect_free_edges(const Board &board, int bx, int by,
                                      std::vector<POS> &out) {
    for (int d = 0; d < 4; ++d) {
        int nr = bx + dr[d], nc = by + dc[d];
        POS e = board.Public_Side_GET({bx, by}, {nr, nc});
        if (e.first >= 0 && board.is_Free_Line(e)) out.push_back(e);
    }
}

// ----------------------------- 主函数 -------------------------------
void Board_Box_Plus::GetChainsAndCycles(std::vector<std::vector<POS>> &chains,
                                        std::vector<std::vector<POS>> &cycles) {
    const int N = LEN;

    // ---------- 0) 单格短链（对边缺口，规则 1） ----------
    // 这部分保持与旧实现一致，避免漏判
    std::unordered_set<POS, custom_hash> global_used_edges;
    for (int bx = 1; bx < N; bx += 2) {
        for (int by = 1; by < N; by += 2) {
            if (CountDrawnEdges(bx, by) != 2) continue;
            std::vector<POS> edges;
            collect_free_edges(_board, bx, by, edges);
            if (edges.size() != 2) continue; // 理论安全
            // 若两条自由边方向相反 -> 单格短链
            int d0 = (edges[0].first == bx ? (edges[0].second > by ? 1 : 3)
                                           : (edges[0].first > bx ? 2 : 0));
            int d1 = (edges[1].first == bx ? (edges[1].second > by ? 1 : 3)
                                           : (edges[1].first > bx ? 2 : 0));
            if ((d0 + 2) % 4 == d1) {
                for (auto &e : edges) global_used_edges.insert(e);
                chains.push_back(edges); // 2 条边的链
            }
        }
    }

    // ---------- 1) 以自由度=2 格子为节点的连通分量 ----------
    std::unordered_set<POS, custom_hash> visited_box;
    auto box_key = [](int x, int y) { return std::make_pair(x, y); };

    for (int sx = 1; sx < N; sx += 2) {
        for (int sy = 1; sy < N; sy += 2) {
            if (CountDrawnEdges(sx, sy) != 2) continue;
            POS box_s = box_key(sx, sy);
            if (visited_box.count(box_s)) continue;

            // BFS 收集该组件所有格子 & 边
            std::queue<POS> q;
            std::vector<POS> component_boxes;
            std::unordered_set<POS, custom_hash> component_edges_set;

            q.push(box_s);
            visited_box.insert(box_s);
            while (!q.empty()) {
                POS cur = q.front();
                q.pop();
                int cx = cur.first, cy = cur.second;
                component_boxes.push_back(cur);

                std::vector<POS> f_edges;
                collect_free_edges(_board, cx, cy, f_edges);
                for (auto &edge : f_edges) {
                    component_edges_set.insert(edge);
                    // 找到通过此自由边相邻的格子中心
                    int dx = (edge.first == cx) ? 0 : (edge.first > cx ? 1 : -1);
                    int dy = (edge.second == cy) ? 0 : (edge.second > cy ? 1 : -1);
                    int nx = cx + dx, ny = cy + dy;
                    if (nx < 1 || nx >= N || ny < 1 || ny >= N) continue;
                    if (CountDrawnEdges(nx, ny) != 2) continue;
                    POS nb = box_key(nx, ny);
                    if (!visited_box.count(nb)) {
                        visited_box.insert(nb);
                        q.push(nb);
                    }
                }
            }

            // 把 set 转成有序 vector，供 Node_Plus 使用
            std::vector<POS> comp_edges(component_edges_set.begin(),
                                        component_edges_set.end());
            if (comp_edges.empty()) continue;

            // ---------- 1.a) 判定环 or 链 ----------
            // 统计组件内邻接度（仅计与组件内格子共享的自由边）
            std::unordered_map<POS, int, custom_hash> deg_in;
            for (auto &b : component_boxes) deg_in[b] = 0;
            for (auto &b : component_boxes) {
                int bx = b.first, by = b.second;
                std::vector<POS> f_edges;
                collect_free_edges(_board, bx, by, f_edges);
                for (auto &e : f_edges) {
                    int dx = (e.first == bx) ? 0 : (e.first > bx ? 1 : -1);
                    int dy = (e.second == by) ? 0 : (e.second > by ? 1 : -1);
                    int nx = bx + dx, ny = by + dy;
                    if (nx < 1 || nx >= N || ny < 1 || ny >= N) continue;
                    if (CountDrawnEdges(nx, ny) != 2) continue;
                    POS nb = box_key(nx, ny);
                    if (deg_in.count(nb)) {
                        deg_in[b]++;
                    }
                }
            }

            bool is_cycle = true;
            for (auto &[p, d] : deg_in) {
                if (d != 2) {
                    is_cycle = false;
                    break;
                }
            }
            if (is_cycle && component_boxes.size() >= 4) {
                cycles.push_back(std::move(comp_edges));
            } else {
                chains.push_back(std::move(comp_edges));
            }

            // 标记其边已被使用，避免后续再次插入其它集合
            for (auto &e : comp_edges) global_used_edges.insert(e);
        }
    }

    // ---------- done ----------
}
