#pragma once
#include <iostream>
#include <vector>
#include <utility>

struct edge_t {
    size_t weight, neighbor, id;
};

using adjList_t = std::vector<std::vector<edge_t>>;

struct task_t {
    adjList_t node_connections;
    size_t source_node, target_node;
};

task_t read_input() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    size_t nodes_n, edges_n;
    std::cin >> nodes_n >> edges_n;
    
    size_t source_node, target_node;
    std::cin >> source_node >> target_node;
    
    adjList_t node_connections(
        nodes_n, std::vector<edge_t> {} // (weight, neighbour)
    );
    
    for (size_t i = 0; i < edges_n; i++) {
        size_t u, v, w1, w2;
        std::cin >> u >> v >> w1 >> w2;
        node_connections[u].emplace_back(w1, v, i);
        node_connections[v].emplace_back(w2, u, i);
    }
    return {node_connections, source_node, target_node};
}


void printAdjList(const adjList_t& node_connections) {
    for (size_t i = 0; i < node_connections.size(); i++) {
        std::cout << i << ": ";
        for (auto [weight, other_node, edge_id]: node_connections[i]) {
            std::cout << other_node << " for " << weight << ", ";
        }
        std::cout << "\n";
    }
}