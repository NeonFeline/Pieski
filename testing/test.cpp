#include <bits/stdc++.h>
#include "../src/prime.hpp"
using namespace std;

const long long PENALTY = 1000000000000000000LL; // 1e18

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;

    int src, dest;
    cin >> src >> dest;

    vector<unordered_map<int, pair<int, int>>> graph(N + 1);

    for (int i = 0; i < M; i++) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        graph[u][v] = make_pair(w1, w2);
        graph[v][u] = make_pair(w1, w2);
    }

    int k;
    cin >> k;
    vector<int> path(k);
    for (int i = 0; i < k; i++) {
        cin >> path[i];
    }

    if (k == 0) {
        cout << PENALTY << endl;
        return 0;
    }

    if (path[0] != src || path[k-1] != dest) {
        cout << PENALTY << endl;
        return 0;
    }

    if (k == 1) {
        cout << 0 << endl;
        return 0;
    }

    unordered_set<int> seen;
    for (int i = 0; i < k; i++) {
        if (seen.find(path[i]) != seen.end()) {
            cout << PENALTY << endl;
            return 0;
        }
        seen.insert(path[i]);
    }

    long long total_cost = 0;
    for (int i = 0; i < k - 1; i++) {
        int u = path[i];
        int v = path[i+1];
        if (graph[u].find(v) == graph[u].end()) {
            cout << PENALTY << endl;
            return 0;
        }
        auto edge_weights = graph[u][v];
        int w1 = edge_weights.first;
        int w2 = edge_weights.second;
        int edge_index = i + 1;
        if (prime_table[edge_index]) {
            total_cost += 3LL * w2;
        } else {
            total_cost += w1;
        }
    }

    cout << total_cost << endl;
    return 0;
}
