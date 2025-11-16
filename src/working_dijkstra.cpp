#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <limits>
#include <atomic>
#include <algorithm>
#include <cmath>
#include <random>
#include <bitset>

constexpr size_t MAX_PRIME = 100000;

constexpr std::bitset<MAX_PRIME + 1> generate_prime_table() noexcept {
    std::bitset<MAX_PRIME + 1> table;
    table.set();
    table[0] = false;
    table[1] = false;

    for (size_t i = 2; i * i <= MAX_PRIME; ++i) {
        if (table[i]) {
            for (size_t j = i * i; j <= MAX_PRIME; j += i)
                table[j] = false;
        }
    }

    return table;
}

constexpr auto prime_table = generate_prime_table();

struct edge_t {
    size_t weight, prime_weight, neighbor;
    uint64_t id;
};

using adjList_t = std::vector<std::vector<edge_t>>;

struct task_t {
    adjList_t node_connections;
    size_t source_node, target_node;
    size_t nodes_n, edges_n;
};

task_t read_input() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::istream& input = std::cin;
    
    size_t nodes_n, edges_n;
    input >> nodes_n >> edges_n;
    
    size_t source_node, target_node;
    input >> source_node >> target_node;
    
    adjList_t node_connections(
        nodes_n, std::vector<edge_t> {} // (weight, neighbour)
    );
    
    for (size_t i = 0; i < edges_n; i++) {
        size_t u, v, w1, w2;
        input >> u >> v >> w1 >> w2;
        node_connections[u].emplace_back(w1, w2, v, i);
        node_connections[v].emplace_back(w1, w2, u, i);
    }
    return {node_connections, source_node, target_node, nodes_n, edges_n};
}

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

    std::vector<size_t> parents (nodes_n, std::numeric_limits<size_t>::max());
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
            auto [weight, prime_weight, other_node, edge_id] = node_connections[cur_node][i];

            if (not edge_bitmask[edge_id]) continue;

            const size_t new_blackie_len = nextEdgePrime ? cur_blackie_len + prime_weight * 3 : cur_blackie_len + weight;
            if (new_blackie_len >= best_blackie_len) continue;
            if (new_blackie_len >= distances[other_node]) continue;

            distances[other_node] = new_blackie_len;
            parents[other_node] = cur_node;

            to_visit.emplace(new_blackie_len, other_node, cur_edges_n + 1);
        }
    }

    if (distances[target_node] < best_blackie_len) {
        best_blackie_len = distances[target_node];

        best_path.clear();
        for (size_t node = target_node; node != source_node; node = parents[node]) {
            best_path.push_back(node);
        }
        best_path.push_back(source_node);
        std::reverse(best_path.begin(), best_path.end()); // path is currently reversed
    }
}

int main() {

    task_t task = read_input();
    auto& [node_connections, source_node, target_node, nodes_n, edges_n] = task;

    std::thread t(timer, 19);

    size_t best_blackie_len = std::numeric_limits<size_t>::max();
    std::vector<size_t> best_path;
    
    double cur_prob = 0.9;
    std::mt19937 rng {std::random_device{}()};
    std::vector<char> edge_bitmask(edges_n, 1);

    size_t counter = 0;

    while (not timeout) {
        dijkstra(node_connections, edge_bitmask, source_node, target_node, best_blackie_len, best_path);
        
        cur_prob = std::max(cur_prob - 0.003, 0.5);
        std::bernoulli_distribution dist(cur_prob);
        for (size_t i = 0; i < edges_n; i++) {
            edge_bitmask[i] = dist(rng);
        }

        counter++;
    }

    
    t.detach();

    std::cout << best_path.size() << "\n";

    for (size_t i = 0; i < best_path.size(); i++) {
        if (i > 0) std::cout << " ";
        std::cout << best_path[i];
    }
    std::cout << "\n";
    
    return 0;
}