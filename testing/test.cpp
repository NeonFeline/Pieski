#include <bits/stdc++.h>
#include "../prime.hpp"
using namespace std;

int main() {

    int N, M;
    cin >> N >> M;

    // Skip the next line (0 3) which appears to be source and destination
    int src, dest;
    cin >> src >> dest;

    vector<unordered_map<int, pair<int, int>>> graph(N + 1);

    for (int i = 0; i < M; i++) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        graph[u][v] = {w1, w2};
        graph[v][u] = {w1, w2};
    }

    int k;
    cin >> k;
    vector<int> path(k);
    for (int i = 0; i < k; i++) {
        cin >> path[i];
    }

    if (k < 2) {
        cout << 0 << endl;
        return 0;
    }

    long long total_cost = 0;
    for (int i = 0; i < k - 1; i++) {
        int u = path[i];
        int v = path[i + 1];
        if (graph[u].find(v) == graph[u].end()) {
            cerr << "Error: no edge between " << u << " and " << v << endl;
            return 1;
        }
        auto edge_weights = graph[u][v];
        int w1 = edge_weights.first;
        int w2 = edge_weights.second;
        int edge_index = i + 1;
        if (prime_table[edge_index]) {
            total_cost += 3 * w2;
        } else {
            total_cost += w1;
        }
    }

    cout << total_cost << endl;
    return 0;
}
