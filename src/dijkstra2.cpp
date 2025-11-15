#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <limits>
#include <atomic>
#include <algorithm>
#include <cmath>

#include "prime.hpp"
#include "readInput.hpp"
#include "boolGen.hpp"
#include "bitmask.hpp"

std::atomic<bool> timeout(false);

void timer(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    timeout = true;
}


void dijkstra(
        const adjList_t& node_connections, const std::vector<char>& edge_bitmask,
        const size_t source_node, const size_t target_node, 
        size_t& best_blackie_len, std::vector<size_t>& best_path) {

    const size_t nodes_n = node_connections.size();
    
    struct conn_t {
        size_t current_blackie_len;
        size_t next_node;
        size_t cur_edges_n;

        bool operator<(const conn_t& other) const {
            return current_blackie_len > other.current_blackie_len; 
        }
    };

    std::priority_queue<conn_t> to_visit;
    to_visit.push( {0, source_node, 0} );

    std::vector<size_t> parents (nodes_n, 0);
    std::vector<size_t> distances (nodes_n, std::numeric_limits<size_t>::max());
    distances[source_node] = 0;

    size_t iterations = 0;
    while (not to_visit.empty()) {
        const auto [cur_blackie_len, cur_node, cur_edges_n] = to_visit.top();
        to_visit.pop();

        if (cur_node == target_node) break;
        if (distances[cur_node] != cur_blackie_len) continue;

        const bool nextEdgePrime = prime_table[cur_edges_n + 1];

        if (++iterations % 1000 == 0 and timeout) break;

        for (size_t i = 0; i < node_connections[cur_node].size(); i++) {
            auto [weight, other_node, edge_id] = node_connections[cur_node][i];

            if (not edge_bitmask[edge_id]) continue;

            if (nextEdgePrime) weight *= 3;
            // size_t new_weight = nextEdgePrime ? weight * 3 : weight;
            const size_t new_blackie_len = cur_blackie_len + weight;
            if (new_blackie_len >= best_blackie_len) continue;
            if (new_blackie_len > distances[other_node]) continue;

            distances[other_node] = new_blackie_len;
            parents[other_node] = cur_node;

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
    auto& [node_connections, source_node, target_node, nodes_n, edges_n] = task;

    std::thread t(timer, 19);

    size_t best_blackie_len = std::numeric_limits<size_t>::max();
    std::vector<size_t> best_path;

    // BooleanGenerator gen(1.0, 0.7);
    // Bitmask edge_bitmask(edges_n, gen);
    double cur_prob = 1;
    // std::uniform_real_distribution<double> prob_dist(0.5, 1);
    std::mt19937 rng {std::random_device{}()};
    std::vector<char> edge_bitmask(edges_n, 1);

    size_t counter = 0;

    while (not timeout) {
        dijkstra(node_connections, edge_bitmask, source_node, target_node, best_blackie_len, best_path);
        
        
        // cur_prob = prob_dist(rng);
        cur_prob = std::max(cur_prob - 0.001, 0.5);
        std::bernoulli_distribution dist(cur_prob);
        for (size_t i = 0; i < edges_n; i++) {
            edge_bitmask[i] = dist(rng);
        }

        // gen.change_probability();
        // edge_bitmask.reshuffle_bits(gen);
        counter++;
    }

    std::cout << counter << " iterations of dijkstra ran\n";
    std::cout << "best blackie len found is " << best_blackie_len << "\n";

    
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