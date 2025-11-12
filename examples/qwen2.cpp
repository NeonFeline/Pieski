#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <climits>
#include <cstdio>
#include <tuple>
#include <map>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <cstring>
using namespace std;
using namespace chrono;

typedef long long ll;
const ll INF = 1e18;
const int MAX_PRIME = 300;
vector<bool> is_prime(MAX_PRIME + 1, true);

void precompute_primes() {
    is_prime[0] = is_prime[1] = false;
    for (int i = 2; i * i <= MAX_PRIME; i++) {
        if (is_prime[i]) {
            for (int j = i * i; j <= MAX_PRIME; j += i) {
                is_prime[j] = false;
            }
        }
    }
}

int main() {
    precompute_primes();
    int N, M;
    scanf("%d %d", &N, &M);
    int A, B;
    scanf("%d %d", &A, &B);

    if (A == B) {
        printf("1\n%d\n", A);
        return 0;
    }

    map<pair<int, int>, pair<ll, ll>> aggMap;
    for (int i = 0; i < M; i++) {
        int u, v, w1, w2;
        scanf("%d %d %d %d", &u, &v, &w1, &w2);
        if (u == v) continue;

        int a = min(u, v);
        int b = max(u, v);
        ll non_prime_cost = w1;
        ll prime_cost = 3LL * w2;

        auto key = make_pair(a, b);
        if (aggMap.find(key) == aggMap.end()) {
            aggMap[key] = make_pair(non_prime_cost, prime_cost);
        } else {
            pair<ll, ll>& current = aggMap[key];
            if (non_prime_cost < current.first)
                current.first = non_prime_cost;
            if (prime_cost < current.second)
                current.second = prime_cost;
        }
    }

    vector<vector<tuple<int, ll, ll>>> adj(N);
    for (auto& entry : aggMap) {
        int u = entry.first.first;
        int v = entry.first.second;
        ll np = entry.second.first;
        ll p = entry.second.second;

        adj[u].push_back(make_tuple(v, np, p));
        adj[v].push_back(make_tuple(u, np, p));
    }

    int L_max = min(N - 1, 200);
    vector<vector<ll>> dist(N, vector<ll>(L_max + 1, INF));
    vector<vector<int>> prev_node(N, vector<int>(L_max + 1, -1));

    using State = tuple<ll, int, int>;
    priority_queue<State, vector<State>, greater<State>> pq;

    dist[A][0] = 0;
    pq.push(make_tuple(0, A, 0));

    ll best_cost = INF;
    int best_k = -1;
    bool timed_out = false;
    int processed_count = 0;
    auto start_time = steady_clock::now();

    while (!pq.empty()) {
        auto [cost, u, k] = pq.top();
        pq.pop();
        processed_count++;

        if (processed_count % 50000 == 0) {
            auto current_time = steady_clock::now();
            auto elapsed = duration_cast<seconds>(current_time - start_time).count();
            if (elapsed >= 18) {
                timed_out = true;
                break;
            }
        }

        if (cost != dist[u][k])
            continue;

        if (u == B && cost < best_cost) {
            best_cost = cost;
            best_k = k;
        }

        if (best_cost != INF && cost >= best_cost)
            continue;

        if (k == L_max)
            continue;

        for (auto& edge : adj[u]) {
            int v = get<0>(edge);
            ll non_prime_cost = get<1>(edge);
            ll prime_cost = get<2>(edge);

            int next_k = k + 1;
            ll edge_cost = is_prime[next_k] ? prime_cost : non_prime_cost;
            ll new_cost = cost + edge_cost;

            if (new_cost < dist[v][next_k]) {
                dist[v][next_k] = new_cost;
                prev_node[v][next_k] = u;
                pq.push(make_tuple(new_cost, v, next_k));
            }
        }
    }

    vector<int> path;
    if (best_k != -1) {
        int cur = B;
        int cur_k = best_k;
        while (cur_k >= 0) {
            path.push_back(cur);
            if (cur_k == 0) break;
            int prev = prev_node[cur][cur_k];
            if (prev == -1) break;
            cur = prev;
            cur_k--;
        }
        reverse(path.begin(), path.end());
    }

    if (path.empty() || path.front() != A || path.back() != B) {
        vector<int> parent(N, -1);
        vector<bool> visited(N, false);
        queue<int> q;

        q.push(A);
        visited[A] = true;
        parent[A] = -1;

        while (!q.empty()) {
            int u = q.front(); q.pop();
            if (u == B) break;

            for (auto& edge : adj[u]) {
                int v = get<0>(edge);
                if (!visited[v]) {
                    visited[v] = true;
                    parent[v] = u;
                    q.push(v);
                }
            }
        }

        if (visited[B]) {
            int cur = B;
            while (cur != -1) {
                path.push_back(cur);
                cur = parent[cur];
            }
            reverse(path.begin(), path.end());
        } else {
            path = {A};
        }
    }

    printf("%d\n", (int)path.size());
    for (int i = 0; i < path.size(); i++) {
        if (i > 0) printf(" ");
        printf("%d", path[i]);
    }
    printf("\n");

    return 0;
}
