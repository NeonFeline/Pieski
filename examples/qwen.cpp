#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <cstdio>
#include <tuple>
#include <cmath>

using namespace std;

typedef long long ll;
const ll INF = LLONG_MAX;
const int MAX_L_SIEVE = 300;
vector<bool> is_prime_vector(MAX_L_SIEVE + 1, false);

void sieve() {
    if (MAX_L_SIEVE < 1)
        return;
    is_prime_vector[0] = false;
    if (MAX_L_SIEVE >= 1)
        is_prime_vector[1] = false;
    if (MAX_L_SIEVE >= 2) {
        for (int i = 2; i <= MAX_L_SIEVE; i++) {
            is_prime_vector[i] = true;
        }
        for (int i = 2; i * i <= MAX_L_SIEVE; i++) {
            if (is_prime_vector[i]) {
                for (int j = i * i; j <= MAX_L_SIEVE; j += i) {
                    is_prime_vector[j] = false;
                }
            }
        }
    }
}

int main() {
    sieve();
    int N, M;
    scanf("%d %d", &N, &M);
    int A, B;
    scanf("%d %d", &A, &B);

    vector<unordered_map<int, pair<ll, ll>>> bestEdge(N);

    for (int i = 0; i < M; i++) {
        int u, v, w1, w2;
        scanf("%d %d %d %d", &u, &v, &w1, &w2);
        ll cost_non_prime = (ll)w1;
        ll cost_prime = 3LL * (ll)w2;

        if (bestEdge[u].find(v) == bestEdge[u].end()) {
            bestEdge[u][v] = make_pair(cost_non_prime, cost_prime);
        } else {
            pair<ll, ll>& p = bestEdge[u][v];
            if (cost_non_prime < p.first)
                p.first = cost_non_prime;
            if (cost_prime < p.second)
                p.second = cost_prime;
        }

        if (bestEdge[v].find(u) == bestEdge[v].end()) {
            bestEdge[v][u] = make_pair(cost_non_prime, cost_prime);
        } else {
            pair<ll, ll>& p = bestEdge[v][u];
            if (cost_non_prime < p.first)
                p.first = cost_non_prime;
            if (cost_prime < p.second)
                p.second = cost_prime;
        }
    }

    int L_max = min(N - 1, 200);
    if (L_max < 0)
        L_max = 0;

    vector<vector<ll>> dist(N, vector<ll>(L_max + 1, INF));
    vector<vector<int>> prev_node(N, vector<int>(L_max + 1, -1));

    priority_queue<tuple<ll, int, int>, vector<tuple<ll, int, int>>, greater<tuple<ll, int, int>>> pq;

    dist[A][0] = 0;
    pq.push(make_tuple(0, A, 0));

    ll best_cost_B = INF;
    int best_k_B = -1;

    while (!pq.empty()) {
        auto [cost, u, k] = pq.top();
        pq.pop();
        if (cost != dist[u][k])
            continue;

        if (u == B) {
            if (cost < best_cost_B) {
                best_cost_B = cost;
                best_k_B = k;
            }
        }

        if (best_cost_B != INF && cost >= best_cost_B) {
            continue;
        }

        if (k == L_max) {
            continue;
        }

        for (const auto& edge : bestEdge[u]) {
            int v = edge.first;
            const pair<ll, ll>& costs = edge.second;
            ll non_prime_cost = costs.first;
            ll prime_cost = costs.second;
            int next_k = k + 1;
            ll edge_cost;
            if (next_k <= MAX_L_SIEVE) {
                edge_cost = is_prime_vector[next_k] ? prime_cost : non_prime_cost;
            } else {
                bool is_prime = true;
                if (next_k <= 1)
                    is_prime = false;
                else {
                    int limit = sqrt(next_k);
                    for (int j = 2; j <= limit; j++) {
                        if (next_k % j == 0) {
                            is_prime = false;
                            break;
                        }
                    }
                }
                edge_cost = is_prime ? prime_cost : non_prime_cost;
            }

            ll new_cost = cost + edge_cost;
            if (new_cost < dist[v][next_k]) {
                dist[v][next_k] = new_cost;
                prev_node[v][next_k] = u;
                pq.push(make_tuple(new_cost, v, next_k));
            }
        }
    }

    vector<int> path;
    if (best_k_B != -1) {
        int cur_node = B;
        int cur_k = best_k_B;
        while (cur_k >= 0) {
            path.push_back(cur_node);
            if (cur_k == 0)
                break;
            int prev = prev_node[cur_node][cur_k];
            if (prev == -1) {
                break;
            }
            cur_node = prev;
            cur_k--;
        }
        reverse(path.begin(), path.end());
    } else {
        vector<int> parent_bfs(N, -1);
        vector<bool> visited(N, false);
        queue<int> q;
        q.push(A);
        visited[A] = true;
        parent_bfs[A] = -1;

        while (!q.empty()) {
            int u = q.front();
            q.pop();
            if (u == B)
                break;
            for (const auto& edge : bestEdge[u]) {
                int v = edge.first;
                if (!visited[v]) {
                    visited[v] = true;
                    parent_bfs[v] = u;
                    q.push(v);
                }
            }
        }

        if (visited[B]) {
            int cur = B;
            while (cur != -1) {
                path.push_back(cur);
                cur = parent_bfs[cur];
            }
            reverse(path.begin(), path.end());
        } else {
            path.push_back(A);
        }
    }

    printf("%d\n", (int)path.size());
    for (int i = 0; i < path.size(); i++) {
        if (i > 0)
            printf(" ");
        printf("%d", path[i]);
    }
    printf("\n");

    return 0;
}
