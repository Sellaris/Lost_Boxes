#include "FeatureExtractor.h"
#include <vector>
#include <iostream>

namespace {

// 辅助结构体，用于模拟Go代码中的节点
struct Node {
    int x, y;
    int degree;
    bool ground;
    std::vector<int> edges; // 存储边的索引
};

// 辅助结构体，用于模拟Go代码中的边
struct Edge {
    int idx;
    int node[2]; // 连接的两个节点索引
    int code;
    bool exist;
};

// 棋盘转换为图结构
void buildGraph(const Board& board, std::vector<Node>& nodes, std::vector<Edge>& edges) {
    // 初始化节点 (7x7网格，包括边界)
    nodes.resize(49);
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 7; ++j) {
            int idx = i * 7 + j;
            nodes[idx].x = i;
            nodes[idx].y = j;
            nodes[idx].degree = 0;
            nodes[idx].ground = (i == 0 || i == 6 || j == 0 || j == 6);
            nodes[idx].edges.clear();
        }
    }
    
    // 初始化边 (60条边)
    edges.resize(60);
    for (int i = 0; i < 60; ++i) {
        edges[i].idx = i;
        edges[i].exist = false;
        edges[i].code = 1 << (i % 30);
        
        int x = (i % 30) / 6 + 1;
        int y = (i % 30) % 6 + 1;
        
        if (i < 30) { // 水平边
            int node1 = x * 7 + (y - 1);
            int node2 = x * 7 + y;
            edges[i].node[0] = node1;
            edges[i].node[1] = node2;
            
            // 检查边是否存在 - 水平边在Board中的坐标是(2*x, y)
            int map_x = 2 * x;
            int map_y = y;
            if (map_x < 11 && map_y < 11) {
                edges[i].exist = (board._MAP_[map_x][map_y] == Horizon);
            }
        } else { // 垂直边
            int node1 = (y - 1) * 7 + x;
            int node2 = y * 7 + x;
            edges[i].node[0] = node1;
            edges[i].node[1] = node2;
            
            // 检查边是否存在 - 垂直边在Board中的坐标是(x, 2*y)
            int map_x = x;
            int map_y = 2 * y;
            if (map_x < 11 && map_y < 11) {
                edges[i].exist = (board._MAP_[map_x][map_y] == Vertical);
            }
        }
        
        // 如果边存在，更新节点度数
        if (edges[i].exist) {
            nodes[edges[i].node[0]].degree++;
            nodes[edges[i].node[1]].degree++;
            nodes[edges[i].node[0]].edges.push_back(i);
            nodes[edges[i].node[1]].edges.push_back(i);
        }
    }
    
    // 边界节点的度数调整（模拟Go代码中的边界处理）
    for (int idx : {0, 6}) {
        for (int j = 1; j <= 5; ++j) {
            nodes[idx * 7 + j].degree += 3;
            nodes[j * 7 + idx].degree += 3;
        }
    }
}

// 模拟Go代码的walkToEnd函数
std::pair<std::vector<std::vector<int>>, std::vector<int>> walkToEnd(
    const std::vector<Node>& nodes, const std::vector<Edge>& edges, int start_node) {
    
    std::vector<std::vector<int>> buf;
    std::vector<int> end_nodes;
    
    if (nodes[start_node].degree == 1) {
        // 从度为1的节点开始遍历
        std::vector<int> path;
        std::vector<bool> visited(49, false);
        
        int current = start_node;
        path.push_back(current);
        visited[current] = true;
        
        while (nodes[current].degree < 3) {
            bool found_next = false;
            for (int edge_idx : nodes[current].edges) {
                const Edge& edge = edges[edge_idx];
                int next = (edge.node[0] == current) ? edge.node[1] : edge.node[0];
                
                if (!visited[next]) {
                    if (nodes[next].degree == 1) {
                        // 找到终点
                        path.push_back(next);
                        buf.push_back(path);
                        end_nodes.push_back(next);
                        found_next = true;
                        break;
                    } else if (nodes[next].degree == 2) {
                        // 继续遍历
                        current = next;
                        path.push_back(current);
                        visited[current] = true;
                        found_next = true;
                        break;
                    }
                }
            }
            if (!found_next) break;
        }
    }
    
    return {buf, end_nodes};
}

// walkLoop a new version, returns the path of the loop. Returns an empty vector if no loop is found.
std::vector<int> walkLoop(const std::vector<Node>& nodes, const std::vector<Edge>& edges, int start_node) {
    if (nodes[start_node].degree != 2) return {};

    std::vector<int> path;
    std::vector<bool> visited_in_path(49, false); // Visited for this specific walk

    int current = start_node;
    
    while (true) {
        path.push_back(current);
        visited_in_path[current] = true;

        bool found_next = false;
        for (int edge_idx : nodes[current].edges) {
            const Edge& edge = edges[edge_idx];
            int next = (edge.node[0] == current) ? edge.node[1] : edge.node[0];
            
            // If we found the start node and it's not the immediate previous node, it's a loop.
            if (next == start_node) {
                if (path.size() > 2) {
                    return path; // Loop found
                }
                continue; // This is just back and forth edge, not a real loop, continue search
            }
            
            // Continue traversal if the next node is degree 2 and not already in our current path.
            if (nodes[next].degree == 2 && !visited_in_path[next]) {
                current = next;
                found_next = true;
                break;
            }
        }
        if (!found_next) {
            // Dead end, not a loop that started from here.
            break;
        }
    }
    
    return {}; // No loop found
}

// 模拟Go代码的nodeComp函数
int nodeComp(const Node& a, const Node& b) {
    if (a.x < b.x) return -1;
    if (a.x > b.x) return 1;
    if (a.y < b.y) return -1;
    if (a.y > b.y) return 1;
    return 0;
}

} // anonymous namespace

Info GetInfo(const Board& board, int currentPlayer) {
    Info info = {};
    
    // 构建图结构
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    buildGraph(board, nodes, edges);
    
    // 1. 统计点的度数 (Point[0~4])
    // The logic is corrected to match the Go implementation: iterate all 45 non-corner nodes.
    for (int i = 0; i < 49; ++i) {
        int x = i / 7;
        int y = i % 7;
        // Skip 4 corner nodes, same as Go version
        if ((x == 0 || x == 6) && (y == 0 || y == 6)) {
            continue;
        }
        int deg = nodes[i].degree;
        if (deg > 4) {
            info.Point[0]++;
        } else if (deg >= 1 && deg <= 4) {
            info.Point[deg]++;
        }
    }
    
    // Point[0] 特殊处理边界点 (This part is removed as it's incorrect and replaced by the logic above)
    
    // 2. 遍历度为3和4的节点，统计链和halfheart
    for (int degree = 4; degree >= 3; --degree) {
        for (int i = 0; i < 49; ++i) {
            if (nodes[i].degree == degree) {
                if (degree == 4 || (degree == 3 && nodes[i].x >= 1 && nodes[i].x <= 5 && 
                                   nodes[i].y >= 1 && nodes[i].y <= 5)) {
                    
                    auto buf_end = walkToEnd(nodes, edges, i);
                    for (size_t k = 0; k < buf_end.second.size(); ++k) {
                        int end_node = buf_end.second[k];
                        if (nodes[end_node].degree == 1) {
                            info.Halfheart++;
                        } else {
                            int ll = buf_end.first[k].size() - 1;
                            if (ll > 10) ll = 10;
                            if (nodeComp(nodes[i], nodes[end_node]) == 1) {
                                info.Link[ll]++;
                            } else if (buf_end.first[k].size() > 1) {
                                info.Link[ll]++;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 3. 遍历度为1的节点，统计fourlink
    for (int i = 0; i < 49; ++i) {
        if (nodes[i].degree == 1) {
            auto buf_end = walkToEnd(nodes, edges, i);
            if (buf_end.first.size() == 1) {
                if (buf_end.second[0] >= 3) {
                    info.Halfheart++;
                } else {
                    info.Fourlink++;
                }
            }
        }
    }
    
    // 4. 遍历度为2的节点，统计环
    std::vector<bool> visited(49, false);
    for (int i = 0; i < 49; ++i) {
        if (nodes[i].degree == 2 && !visited[i]) {
            auto path = walkLoop(nodes, edges, i);
            if (!path.empty()) {
                int l = path.size();
                int length = (l - 4) >> 1;
                if (length > 4) length = 4;
                if (length >= 0) {
                    info.Loop[length]++;
                }
                
                // 标记环上的所有节点为已访问
                for (int node_idx : path) {
                    visited[node_idx] = true;
                }
            }
        }
    }
    
    return info;
}

std::vector<fann_type> extract_features(const Board& board, int currentPlayer) {
    Info info = GetInfo(board, currentPlayer);
    std::vector<fann_type> features(25, 0.0f);
    
    // Point features (5) - 按照Go代码的顺序，从Point[0]开始
    for (int i = 0; i < 5; ++i) {
        features[i] = static_cast<fann_type>(info.Point[i]) / 25.0f;
    }
    
    // Link features (11)
    for (int i = 0; i < 11; ++i) {
        features[5 + i] = static_cast<fann_type>(info.Link[i]) / 40.0f;
    }
    
    // Loop features (5)
    for (int i = 0; i < 5; ++i) {
        features[16 + i] = static_cast<fann_type>(info.Loop[i]) / 5.0f;
    }
    
    // Halfheart feature (1)
    features[21] = static_cast<fann_type>(info.Halfheart) / 10.0f;
    
    // Fourlink feature (1)
    features[22] = static_cast<fann_type>(info.Fourlink) / 4.0f;
    
    // Score features (2) - 当前玩家和对手的得分
    int player_scores[2];
    player_scores[0] = board.TAKE_Boxes_OF_Players(currentPlayer);
    player_scores[1] = board.TAKE_Boxes_OF_Players(-currentPlayer);
    features[23] = static_cast<fann_type>(player_scores[0]) / 16.0f;
    features[24] = static_cast<fann_type>(player_scores[1]) / 16.0f;
    
    return features;
}