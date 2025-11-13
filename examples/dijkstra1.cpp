#include <iostream>
#include <vector>
#include <utility>
#include <thread>
#include <queue>
#include <limits>
#include <atomic>
#include <algorithm>
#include <stack>

#include "../prime.hpp"
#include "../readInput.hpp"
#include "../boolGen.hpp"

std::atomic<bool> timeout(false);

void timer(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    timeout = true;
}


void dijkstra(
        const adjList_t& node_connections, const size_t source_node, const size_t target_node, 
        size_t& best_blackie_len, std::vector<size_t>& best_path) {

    const size_t nodes_n = node_connections.size();
    
    using conn_t = std::tuple<size_t, size_t, size_t>; // current blackie len, next node, current amount of edges

    std::stack<conn_t> to_visit ({ {0, source_node, 0} });
    std::vector<size_t> parents (nodes_n, 0);
    std::vector<size_t> distances (nodes_n, std::numeric_limits<size_t>::max());
    std::vector<int> times_visited_left (nodes_n, 1);
    distances[source_node] = 0;

    size_t iterations = 0;
    while (not to_visit.empty()) {
        const auto [cur_blackie_len, cur_node, cur_edges_n] = std::move(to_visit.top());
        to_visit.pop();

        times_visited_left[cur_node]--;
        const bool nextEdgePrime = prime_table[cur_edges_n + 1];

        if (++iterations % 1000 == 0 and timeout) break;

        for (auto [weight, other_node]: node_connections[cur_node]) {
            if (not times_visited_left[other_node]) continue;

            if (nextEdgePrime) weight *= 3;
            const size_t new_blackie_len = cur_blackie_len + weight;
            if (new_blackie_len >= best_blackie_len) continue;
            if (new_blackie_len > distances[other_node]) continue;

            distances[other_node] = new_blackie_len;
            parents[other_node] = cur_node;

            if (other_node == target_node) break;
            to_visit.emplace(new_blackie_len, other_node, cur_edges_n + 1);
        }
    }

    if (distances[target_node] < best_blackie_len) {
        best_blackie_len = distances[target_node];
        // std::cout << "new best blackie len is " << best_blackie_len << "\n";
        best_path.clear();
        for (size_t node = target_node; node != source_node; node = parents[node]) {
            best_path.push_back(node);
        }
        best_path.push_back(source_node); // include the source node
        std::reverse(best_path.begin(), best_path.end()); // path is currently reversed
    }
}

int main() {

    task_t task = read_input();
    auto& [node_connections, source_node, target_node] = task;

    const size_t nodes_n = node_connections.size();
    std::thread t(timer, 19);

    size_t best_blackie_len = std::numeric_limits<size_t>::max();
    std::vector<size_t> best_path;

    dijkstra(node_connections, source_node, target_node, best_blackie_len, best_path);

    BooleanGenerator gen(0.88);

    size_t counter = 0;
    while (not timeout) {
        adjList_t changed_node_connections (nodes_n, std::vector<std::pair<size_t, size_t>>());
 

        for (size_t i = 0; i < nodes_n; i++) {
            auto& other_nodes = node_connections[i];
            for (auto& other_node: other_nodes) {
                if (gen.next()) {
                    changed_node_connections[i].push_back(other_node);
                }
            }
        } 

        dijkstra(changed_node_connections, source_node, target_node, best_blackie_len, best_path);
        gen.change_probability();
    }

    
    t.detach();

    if (best_blackie_len == std::numeric_limits<size_t>::max()) {
        std::cout << "NO PATH" << "\n";
    } else {
        
        std::cout << best_path.size() << "\n";

        for (size_t i = 0; i < best_path.size(); i++) {
            if (i > 0) std::cout << " ";
            std::cout << best_path[i];
        }
        std::cout << "\n";
    }
    
    return 0;
}