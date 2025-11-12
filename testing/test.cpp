#include <bits/stdc++.h>
using namespace std;

const int MAX_N = 100000;
bool is_prime[100001];

void precompute_primes() {
    memset(is_prime, true, sizeof(is_prime));
    is_prime[0] = false;
    is_prime[1] = false;
    for (int i = 2; i * i <= MAX_N; i++) {
        if (is_prime[i]) {
            for (int j = i * i; j <= MAX_N; j += i) {
                is_prime[j] = false;
            }
        }
    }
}

int main() {
    precompute_primes();

    int N, M;
    cin >> N >> M;

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
        if (is_prime[edge_index]) {
            total_cost += 3 * w2;
        } else {
            total_cost += w1;
        }
    }

    cout << total_cost << endl;
    return 0;
}
