#include "FeatureExtractor.h"
#include "board.h"
#include <vector>
#include <numeric>
#include <functional>
#include <algorithm>
#include <iostream>

namespace {

// Corresponds to the Node struct in qboard.go
struct Node {
    int x, y, degree;
    bool ground;
    int edges[4]; // 0: up, 1: right, 2: down, 3: left
};

// Corresponds to the Edge struct in qboard.go
struct Edge {
    int idx;
    bool cut;
    int node_indices[2];
};

// Corresponds to the LinkState struct in qboard.go
struct LinkState {
    int ty = -1, length = 0;
};

// Helper for DSU data structure
int find_set(int v, std::vector<int>& parent) {
    if (v == parent[v])
        return v;
    return parent[v] = find_set(parent[v], parent);
}

void union_sets(int a, int b, std::vector<int>& parent) {
    a = find_set(a, parent);
    b = find_set(b, parent);
    if (a != b)
        parent[b] = a;
}

// walkToEnd from qboard.go
auto walkToEnd(int start_node_idx, const std::vector<Node>& nodes, const std::vector<Edge>& edges, std::vector<bool>& visited_nodes) -> std::pair<std::vector<std::vector<int>>, std::vector<int>> {
    std::vector<std::vector<int>> buf;
    std::vector<int> end_node_indices;

    if (start_node_idx < 0 || start_node_idx >= 49) return {buf, end_node_indices};

    const auto& s_node = nodes[start_node_idx];
    for (int edge_idx : s_node.edges) {
        if (edge_idx == -1 || edges[edge_idx].cut) continue;

        const auto& edge = edges[edge_idx];
        int other_node_idx = (edge.node_indices[0] == start_node_idx) ? edge.node_indices[1] : edge.node_indices[0];

        if (other_node_idx >= 0 && other_node_idx < 49 && !visited_nodes[other_node_idx]) {
            std::vector<int> current_path;
            current_path.push_back(edge_idx);
            
            int prev_edge_idx = edge_idx;
            int current_node_idx = other_node_idx;

            while (nodes[current_node_idx].degree == 2) {
                visited_nodes[current_node_idx] = true;
                bool found_next = false;
                for (int next_edge_idx : nodes[current_node_idx].edges) {
                    if (next_edge_idx != -1 && next_edge_idx != prev_edge_idx && !edges[next_edge_idx].cut) {
                        current_path.push_back(next_edge_idx);
                        prev_edge_idx = next_edge_idx;
                        int n0 = edges[next_edge_idx].node_indices[0];
                        int n1 = edges[next_edge_idx].node_indices[1];
                        current_node_idx = (n0 == current_node_idx) ? n1 : n0;
                        found_next = true;
                        break;
                    }
                }
                if (!found_next) break; 
            }
            buf.push_back(current_path);
            end_node_indices.push_back(current_node_idx);
        }
    }
    return {buf, end_node_indices};
}

// walkLoop from qboard.go
auto walkLoop(int start_node_idx, const std::vector<Node>& nodes, const std::vector<Edge>& edges, std::vector<bool>& visited_nodes) -> std::vector<int> {
    std::vector<int> es;
    if (start_node_idx < 0 || start_node_idx >= 49) return es;

    const auto& s_node = nodes[start_node_idx];
    visited_nodes[start_node_idx] = true;

    int first_edge_idx = -1;
    for (int edge_idx : s_node.edges) {
        if (edge_idx != -1 && !edges[edge_idx].cut) {
            first_edge_idx = edge_idx;
            break;
        }
    }
    if (first_edge_idx == -1) return es;

    es.push_back(first_edge_idx);
    int prev_edge_idx = first_edge_idx;
    int current_node_idx = (edges[first_edge_idx].node_indices[0] == start_node_idx) ? edges[first_edge_idx].node_indices[1] : edges[first_edge_idx].node_indices[0];

    while(current_node_idx != start_node_idx && current_node_idx >= 0 && current_node_idx < 49) {
        visited_nodes[current_node_idx] = true;
        bool found_next = false;
        for (int next_edge_idx : nodes[current_node_idx].edges) {
            if (next_edge_idx != -1 && next_edge_idx != prev_edge_idx && !edges[next_edge_idx].cut) {
                es.push_back(next_edge_idx);
                prev_edge_idx = next_edge_idx;
                int n0 = edges[next_edge_idx].node_indices[0];
                int n1 = edges[next_edge_idx].node_indices[1];
                current_node_idx = (n0 == current_node_idx) ? n1 : n0;
                found_next = true;
                break;
            }
        }
        if (!found_next) break;
    }
    return es;
}

} // anonymous namespace

Info GetInfo(const Board& board, int currentPlayer) {
    Info info;
    std::vector<Node> nodes(49);
    std::vector<Edge> edges(60);
    
    // 1. Initialize a 7x7 node grid
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 7; ++j) {
            int node_idx = i * 7 + j;
            nodes[node_idx] = {i, j, 0, (i == 0 || i == 6 || j == 0 || j == 6), {-1, -1, -1, -1}};
        }
    }

    int free_edges = 0;
    // 2. Build graph, mapping qboard.go edge indices to C++ Board coordinates
    for (int q_idx = 0; q_idx < 60; ++q_idx) {
        edges[q_idx].idx = q_idx;
        
        int r1, c1, r2, c2; // Node coordinates in the 7x7 grid (0-6)

        if (q_idx < 30) { // Horizontal edge
            r1 = r2 = (q_idx / 6) + 1;
            c1 = q_idx % 6;
            c2 = c1 + 1;
        } else { // Vertical edge
            c1 = c2 = ((q_idx - 30) / 6) + 1;
            r1 = (q_idx - 30) % 6;
            r2 = r1 + 1;
        }

        // Connect nodes and edges
        int n1_idx = r1 * 7 + c1;
        int n2_idx = r2 * 7 + c2;
        edges[q_idx].node_indices[0] = n1_idx;
        edges[q_idx].node_indices[1] = n2_idx;
        nodes[n1_idx].edges[r1 == r2 ? 1 : 2] = q_idx; // Right or Down
        nodes[n2_idx].edges[r1 == r2 ? 3 : 0] = q_idx; // Left or Up
        
        // Determine if the edge is cut
        bool is_boundary_edge = (r1 == 0 || r2 == 6 || c1 == 0 || c2 == 6);
        if (0) {
        } else {
            // Internal edges: map to Board _MAP_ and check
            int map_r, map_c;
            if (q_idx < 30) { // Horizontal
                map_r = 2 * r1 - 1;
                map_c = 2 * c1 ;
            } else { // Vertical
                map_r = 2 * r1 ;
                map_c = 2 * c1 - 1;
            }        
            if (board.is_Free_Line(map_r, map_c)) {
                free_edges++;
            }    

            //std::cout<<"map_r="<<map_r<<" map_c="<<map_c<<"\n";
            edges[q_idx].cut = !board.is_Free_Line(map_r, map_c);
        }
    }
    info.Link[0] = free_edges;

    // 3. Calculate node degrees
    for (int i = 0; i < 60; ++i) {
        if (!edges[i].cut) {
            nodes[edges[i].node_indices[0]].degree++;
            nodes[edges[i].node_indices[1]].degree++;
        }
    }
    // Note: Boundary (ground) nodes are intentionally not given extra degrees.
    // Their degree is naturally 0 or 1 from their single link to the interior,
    // correctly identifying them as chain terminators rather than junctions.
    // This restricts chain and loop analysis to the actual 5x5 game grid.
    
    // 4. Calculate Feature 0-4: Point features (from 5x5 inner grid of nodes)
    for (int i = 1; i <= 5; ++i) {
        for (int j = 1; j <= 5; ++j) {
            if(nodes[i*7+j].degree < 5)
                info.Point[nodes[i*7+j].degree]++;
        }
    }info.Point[0]+=20;

    // 5. Chain and Loop analysis
    std::vector<int> parent(60);
    std::iota(parent.begin(), parent.end(), 0);
    std::vector<LinkState> link_states(60);
    std::vector<bool> visited_nodes(49, false);

    auto o = [&](const std::vector<int>& buf) {
        if (buf.empty()) return -1;
        int p = buf[0];
        for (size_t i = 1; i < buf.size(); ++i) {
            union_sets(p, buf[i], parent);
            p = find_set(p, parent);
        }
        return p;
    };
    
    // Find chains from all non-visited nodes
    for (int i = 0; i < 49; ++i) {
        if (nodes[i].degree == 1 && !visited_nodes[i]) {
            visited_nodes[i] = true;
            auto [paths, ends] = walkToEnd(i, nodes, edges, visited_nodes);
            if (!paths.empty()) {
                auto& path = paths[0];
                int end_node_idx = ends[0];
                int root = o(path);
                if (root != -1) {
                    LinkState ls;
                    ls.length = path.size();
                    if (nodes[end_node_idx].ground || nodes[end_node_idx].degree == 1) { // O-...-O
                         if (path.size() == 1) ls.ty = 4;
                         else if (path.size() == 2) ls.ty = 5;
                         else if (path.size() == 3) ls.ty = 7;
                         else ls.ty = 6;
                    } else { // O-...-X
                         if (path.size() == 1) ls.ty = 1;
                         else if (path.size() == 2) ls.ty = 3;
                         else ls.ty = 2;
                    }
                    link_states[root] = ls;
                }
            }
        } else if (nodes[i].degree >= 3 && !visited_nodes[i]) {
            visited_nodes[i] = true;
            auto [paths, ends] = walkToEnd(i, nodes, edges, visited_nodes);
            for (size_t k = 0; k < paths.size(); ++k) {
                auto& path = paths[k];
                int end_node_idx = ends[k];
                int root = o(path);
                if (root != -1) {
                     LinkState ls;
                     ls.length = path.size();
                    if (nodes[end_node_idx].ground || nodes[end_node_idx].degree == 1) { // X-...-O
                        if (path.size() == 1) continue;
                        else if (path.size() == 2) ls.ty = 3;
                        else ls.ty = 2;
                    } else { // X-...-X
                        if (path.size() == 1) ls.ty = 8;
                        else if (path.size() == 2) ls.ty = 9;
                        else if (path.size() == 3) ls.ty = 10;
                        else ls.ty = 11;
                    }
                    link_states[root] = ls;
                }
            }
        }
    }
    
    // Find loops
    for (int i = 1; i <= 5; ++i) {
        for (int j = 1; j <= 5; ++j) {
            int node_idx = i * 7 + j;
            if (nodes[node_idx].degree == 2 && !visited_nodes[node_idx]) {
                auto path = walkLoop(node_idx, nodes, edges, visited_nodes);
                if (!path.empty()) {
                    int root = o(path);
                    if (root != -1) {
                        LinkState ls;
                        ls.ty = 12;
                        ls.length = path.size();
                        link_states[root] = ls;
                    }
                }
            }
        }
    }

    // 6. Aggregate features
    for (int i = 0; i < 60; ++i) {
        if (parent[i] == i) { // It's a root
            const auto& ls = link_states[i];
            if (ls.ty == -1) continue;

            if (ls.ty >= 8 && ls.ty <= 11) {
                int len = ls.length;
                if (len > 11)len=11;
                if(len > 1)
                     info.Link[len-1]++;
            } 
            else if (ls.ty == 3) {
                info.Halfheart++;
            } 
            else if (ls.ty == 7) {
                info.Fourlink++;
            } 
            else if (ls.ty == 12) {
                if (ls.length >= 4) {
                    int len_idx = (ls.length - 4) >> 1;
                    if (len_idx >= 0 && len_idx < 5) {
                        info.Loop[len_idx]++;
                    }
                }
            }
        }
    }

    return info;
}

std::vector<fann_type> extract_features(const Board& board, int currentPlayer) {
    Info info = GetInfo(board, currentPlayer);
    std::vector<fann_type> features(25);

    // Feat 0-4: Points
    for (int i = 0; i < 5; ++i) {
        features[i] = static_cast<fann_type>(info.Point[i]) / 25.0f;
    }
    // Feat 5-15: Hard Links by length
    for (int i = 0; i < 11; ++i) {
        features[i + 5] = static_cast<fann_type>(info.Link[i]) / 40.0f;
    }
    // Feat 16-20: Loops
    for (int i = 0; i < 5; ++i) {
        features[16 + i] = static_cast<fann_type>(info.Loop[i]) / 5.0f;
    }
    // Feat 21: Halfheart
    features[21] = static_cast<fann_type>(info.Halfheart) / 10.0f;
    // Feat 22: Fourlink
    features[22] = static_cast<fann_type>(info.Fourlink) / 4.0f;
    
    // Feat 23-24: Scores
    int s0 = (currentPlayer == BLACK) ? board.BOX_of_BLACK : board.BOX_of_WHITE;
    int s1 = (currentPlayer == BLACK) ? board.BOX_of_WHITE : board.BOX_of_BLACK;

    features[23] = static_cast<fann_type>(s0) / 16.0f;
    features[24] = static_cast<fann_type>(s1) / 16.0f;

    return features;
}
