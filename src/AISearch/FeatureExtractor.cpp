#include "FeatureExtractor.h"
#include <vector>
#include <iostream>

namespace {

// 辅助结构体，用于模拟Go代码中的节点
struct Node {
    int x, y;                  // 坐标
    int degree;                // 节点的度数
    bool ground;               // 是否是边界节点
    std::vector<int> edges;    // 存储边的索引
};

// 辅助结构体，用于模拟Go代码中的边
struct Edge {
    int idx;                   // 边的索引
    int node[2];               // 连接的两个节点索引
    bool exist;                // 边是否存在
};

// 节点比较函数，比较两个节点的坐标顺序
int nodeComp(const Node& a, const Node& b) {
    if (a.x < b.x) return -1;  // 如果 a.x 小于 b.x，返回 -1
    if (a.x > b.x) return 1;   // 如果 a.x 大于 b.x，返回 1
    if (a.y < b.y) return -1;  // 如果 a.x 相等，比较 y 坐标
    if (a.y > b.y) return 1;
    return 0;  // 如果两个节点的坐标都相等，返回 0
}

// 重新构建图，简化节点和边的映射
void buildGraph(const Board& board, std::vector<Node>& nodes, std::vector<Edge>& edges) {
    const int grid_size = 6;  // 6x6网格
    const int edge_count = (grid_size - 1) * grid_size * 2; // 水平和垂直边的数量
    nodes.resize(grid_size * grid_size);
    edges.resize(edge_count);

    // 初始化节点
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            int idx = i * grid_size + j;
            nodes[idx].x = i;
            nodes[idx].y = j;
            nodes[idx].degree = 0;
            nodes[idx].ground = (i == 0 || i == grid_size - 1 || j == 0 || j == grid_size - 1);
            nodes[idx].edges.clear();
        }
    }

    int edge_idx = 0;

    // 连接水平边
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size - 1; ++j) {
            int node1 = i * grid_size + j;
            int node2 = i * grid_size + (j + 1);
            edges[edge_idx] = {edge_idx, {node1, node2}, false};

            // 检查水平边是否存在
            int map_x = 2 * i;
            int map_y = j + 1;
            if (map_x < 11 && map_y < 11) {
                edges[edge_idx].exist = (board._MAP_[map_x][map_y] == Horizon);
            }
            
            // 更新节点度数
            if (edges[edge_idx].exist) {
                nodes[node1].degree++;
                nodes[node2].degree++;
            }

            edge_idx++;
        }
    }

    // 连接垂直边
    for (int i = 0; i < grid_size - 1; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            int node1 = i * grid_size + j;
            int node2 = (i + 1) * grid_size + j;
            edges[edge_idx] = {edge_idx, {node1, node2}, false};

            // 检查垂直边是否存在
            int map_x = j;
            int map_y = 2 * (i + 1);
            if (map_x < 11 && map_y < 11) {
                edges[edge_idx].exist = (board._MAP_[map_x][map_y] == Vertical);
            }
            
            // 更新节点度数
            if (edges[edge_idx].exist) {
                nodes[node1].degree++;
                nodes[node2].degree++;
            }

            edge_idx++;
        }
    }

    //std::cout << "Graph built successfully with " << nodes.size() << " nodes and " << edges.size() << " edges." << std::endl;
}

// 模拟Go代码的walkToEnd函数
std::pair<std::vector<std::vector<int>>, std::vector<int>> walkToEnd(
    const std::vector<Node>& nodes, const std::vector<Edge>& edges, int start_node) {
    
    std::vector<std::vector<int>> buf;
    std::vector<int> end_nodes;
    
    if (nodes[start_node].degree == 1) {
        // 从度为1的节点开始遍历
        std::vector<int> path;
        std::vector<bool> visited(36, false);  // 6x6网格的节点数

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
    std::vector<bool> visited_in_path(36, false);  // 6x6网格的节点数

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
                    std::cout << "Loop found with path size: " << path.size() << std::endl;
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

} // anonymous namespace

Info GetInfo(const Board& board, int currentPlayer) {
    Info info = {};
    
    // 构建图结构
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    buildGraph(board, nodes, edges);
    
    // 1. 统计点的度数 (Point[0~4])
    for (int i = 0; i < 36; ++i) {  // 更新为6x6网格的节点数
        int x = i / 6;
        int y = i % 6;
        // Skip 4 corner nodes
        if ((x == 0 || x == 5) && (y == 0 || y == 5)) {
            continue;
        }
        int deg = nodes[i].degree;
        if (deg > 4) {
            info.Point[0]++;
        } else if (deg >= 1 && deg <= 4) {
            info.Point[deg]++;
        }
    }
    
    // 2. 遍历度为3和4的节点，统计链和halfheart
    for (int degree = 4; degree >= 3; --degree) {
        for (int i = 0; i < 36; ++i) {
            if (nodes[i].degree == degree) {
                if (degree == 4 || (degree == 3 && nodes[i].x >= 1 && nodes[i].x <= 4 && 
                                   nodes[i].y >= 1 && nodes[i].y <= 4)) {
                    
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
    for (int i = 0; i < 36; ++i) {  // 更新为6x6网格的节点数
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
    std::vector<bool> visited(36, false);  // 更新为6x6网格的节点数
    for (int i = 0; i < 36; ++i) {  // 更新为6x6网格的节点数
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
