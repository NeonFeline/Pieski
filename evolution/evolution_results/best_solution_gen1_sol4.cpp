#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int to;
    long long w1;   // original w1
    long long w2;   // original w2
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int N, M;
    if (!(cin >> N >> M)) return 0;
    int A, B;
    cin >> A >> B;

    vector<vector<Edge>> adj(N);
    for (int i = 0; i < M; ++i) {
        int u, v;
        long long w1, w2;
        cin >> u >> v >> w1 >> w2;
        adj[u].push_back({v, w1, w2});
        adj[v].push_back({u, w1, w2});          // undirected graph
    }

    const long long INF = (1LL << 62);
    vector<long long> dist(N, INF);
    vector<int> pred(N, -1);
    vector<char> visited(N, 0);

    using State = pair<long long,int>;          // (dist, vertex)
    priority_queue<State, vector<State>, greater<State>> pq;

    dist[A] = 0;
    pq.emplace(0, A);

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (visited[u]) continue;
        visited[u] = 1;
        if (u == B) break;                      // we already have the best distance to B

        for (const auto &e : adj[u]) {
            int v = e.to;
            long long w = e.w1 + 3LL * e.w2;    // auxiliary weight
            if (dist[v] > d + w) {
                dist[v] = d + w;
                pred[v] = u;
                pq.emplace(dist[v], v);
            }
        }
    }

    // Reconstruct path (if unreachable we output 0)
    if (dist[B] == INF) {
        cout << 0 << "\n";
        return 0;
    }

    vector<int> path;
    for (int cur = B; cur != -1; cur = pred[cur])
        path.push_back(cur);
    reverse(path.begin(), path.end());

    cout << path.size() << "\n";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) cout << ' ';
        cout << path[i];
    }
    cout << "\n";

    // -----------------------------------------------------------------
    // (Optional) compute the *real* Blackie‑length of the returned path
    // -----------------------------------------------------------------
    long long realA = 0, realB = 0;
    // prime test up to path length (≤ N)
    vector<char> isPrime(N + 1, true);
    isPrime.assign(N + 1, true);
    if (N >= 0) isPrime[0] = false;
    if (N >= 1) isPrime[1] = false;
    for (int p = 2; p * p <= N; ++p)
        if (isPrime[p])
            for (int q = p * p; q <= N; q += p) isPrime[q] = false;

    for (size_t i = 1; i < path.size(); ++i) {          // i = edge index (1‑based)
        int u = path[i-1];
        int v = path[i];
        // find the edge (u,v) – there may be several, we pick the first one
        const Edge* chosen = nullptr;
        for (const auto &e : adj[u]) if (e.to == v) { chosen = &e; break; }
        if (!chosen) continue; // should never happen
        if (isPrime[i])
            realB += 3LL * chosen->w2;
        else
            realA += chosen->w1;
    }
    // Uncomment the following line to see the exact Blackie‑length (debug):
    // cerr << "Exact Blackie‑length = " << realA + realB << "\n";

    return 0;
}
