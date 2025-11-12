#include <bits/stdc++.h>
using namespace std;

const long long INF = 1e18;
const int MAX_STEP = 100000;
const int K = 100;

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    cin >> N >> M;
    int A, B;
    cin >> A >> B;

    vector<vector<tuple<int, int, int>>> graph(N);
    for (int i = 0; i < M; i++) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        graph[u].push_back({v, w1, w2});
        graph[v].push_back({u, w1, w2});
    }

    vector<bool> is_prime(MAX_STEP + 1, true);
    is_prime[0] = false;
    is_prime[1] = false;
    for (int i = 2; i * i <= MAX_STEP; i++) {
        if (is_prime[i]) {
            for (int j = i * i; j <= MAX_STEP; j += i) {
                is_prime[j] = false;
            }
        }
    }

    int total_primes = 0;
    for (int i = 1; i <= MAX_STEP; i++) {
        if (is_prime[i]) total_primes++;
    }
    int primes_up_to_K = 0;
    for (int i = 1; i <= K; i++) {
        if (is_prime[i]) primes_up_to_K++;
    }
    double density_beyond_K = (double)(total_primes - primes_up_to_K) / (MAX_STEP - K);

    vector<double> h2(N, 1e18);
    vector<int> parent_h2(N, -1);
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq_back;

    h2[B] = 0.0;
    pq_back.push({0.0, B});

    while (!pq_back.empty()) {
        auto [cost, u] = pq_back.top();
        pq_back.pop();
        if (abs(cost - h2[u]) > 1e-9) continue;
        for (auto &edge : graph[u]) {
            int v = get<0>(edge);
            int w1 = get<1>(edge);
            int w2 = get<2>(edge);
            double edge_cost = (1.0 - density_beyond_K) * w1 + density_beyond_K * (3.0 * w2);
            double new_cost = cost + edge_cost;
            if (new_cost < h2[v] - 1e-9) {
                h2[v] = new_cost;
                parent_h2[v] = u;
                pq_back.push({new_cost, v});
            }
        }
    }

    vector<vector<long long>> dist(N, vector<long long>(K + 1, INF));
    vector<vector<pair<int, int>>> parent_state(N, vector<pair<int, int>>(K + 1, {-1, -1}));
    priority_queue<tuple<long long, int, int>, vector<tuple<long long, int, int>>, greater<tuple<long long, int, int>>> pq_exact;

    dist[A][0] = 0;
    pq_exact.push({0, A, 0});

    double best_total = 1e18;
    int best_node = -1;
    int best_step = -1;

    while (!pq_exact.empty()) {
        auto [cost, u, s] = pq_exact.top();
        pq_exact.pop();
        if (cost != dist[u][s]) continue;

        double total_cost_candidate = (double)cost + h2[u];
        if (total_cost_candidate < best_total) {
            best_total = total_cost_candidate;
            best_node = u;
            best_step = s;
        }

        if (s == K) continue;

        for (auto &edge : graph[u]) {
            int v = get<0>(edge);
            int w1 = get<1>(edge);
            int w2 = get<2>(edge);
            int next_step = s + 1;
            long long edge_cost = is_prime[next_step] ? (3LL * w2) : (long long)w1;
            long long new_cost = cost + edge_cost;
            if (new_cost < dist[v][next_step]) {
                dist[v][next_step] = new_cost;
                parent_state[v][next_step] = {u, s};
                pq_exact.push({new_cost, v, next_step});
            }
        }
    }

    if (best_node == -1) {
        cout << "No path found" << endl;
        return 0;
    }

    vector<int> exact_path;
    stack<int> path_stack;
    int cur_node = best_node;
    int cur_step = best_step;
    while (cur_node != -1 && cur_step != -1) {
        path_stack.push(cur_node);
        auto [prev_node, prev_step] = parent_state[cur_node][cur_step];
        cur_node = prev_node;
        cur_step = prev_step;
    }
    while (!path_stack.empty()) {
        exact_path.push_back(path_stack.top());
        path_stack.pop();
    }

    vector<int> full_path = exact_path;
    if (best_node != B) {
        vector<int> approx_path;
        int cur = best_node;
        approx_path.push_back(cur);
        while (cur != B && parent_h2[cur] != -1) {
            cur = parent_h2[cur];
            approx_path.push_back(cur);
        }
        if (cur != B) {
        } else {
            for (int i = 1; i < approx_path.size(); i++) {
                full_path.push_back(approx_path[i]);
            }
        }
    }

    cout << full_path.size() << endl;
    for (int i = 0; i < full_path.size(); i++) {
        if (i > 0) cout << " ";
        cout << full_path[i];
    }
    cout << endl;

    return 0;
}
