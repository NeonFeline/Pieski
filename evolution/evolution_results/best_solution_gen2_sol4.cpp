#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int to;
    int w1;
    int w2;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    if (!(cin >> N >> M)) return 0;
    int A, B;
    cin >> A >> B;

    vector<vector<Edge>> g(N);
    for (int i = 0; i < M; ++i) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        g[u].push_back({v, w1, w2});
        g[v].push_back({u, w1, w2});          // undirected
    }

    /* ---------- prime table up to N (max possible path length) ---------- */
    vector<char> isPrime(N + 1, true);
    if (N >= 0) isPrime[0] = false;
    if (N >= 1) isPrime[1] = false;
    for (int p = 2; p * p <= N; ++p)
        if (isPrime[p])
            for (int x = p * p; x <= N; x += p) isPrime[x] = false;

    const long long INF = (1LL << 60);
    vector<long long> dist(N, INF);          // best distance known for the vertex
    vector<int>       prev(N, -1);           // predecessor on that best path
    vector<int>       plen(N, 0);            // length of that best path

    using State = tuple<long long, int, int>; // (dist, vertex, length)
    priority_queue<State, vector<State>, greater<State>> pq;

    dist[A] = 0;
    plen[A] = 0;
    pq.emplace(0LL, A, 0);

    while (!pq.empty()) {
        auto [d, u, len] = pq.top(); pq.pop();
        if (d != dist[u]) continue;               // outdated entry

        if (u == B) break;                        // optimal distance for B found

        for (const auto &e : g[u]) {
            int v = e.to;
            int nlen = len + 1;
            long long add = isPrime[nlen] ? 3LL * e.w2 : (long long) e.w1;
            long long nd = d + add;
            if (nd < dist[v]) {
                dist[v] = nd;
                prev[v] = u;
                plen[v] = nlen;
                pq.emplace(nd, v, nlen);
            }
        }
    }

    /* ---------- output ---------- */
    if (dist[B] == INF) {          // no path (should not happen with positive edges)
        cout << -1 << '\n';
        return 0;
    }

    vector<int> path;
    for (int cur = B; cur != -1; cur = prev[cur])
        path.push_back(cur);
    reverse(path.begin(), path.end());

    cout << path.size() << '\n';
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) cout << ' ';
        cout << path[i];
    }
    cout << '\n';

    /* ---------- (optional) exact Blackie‑length verification ---------- */
    /*
    long long realA = 0, realB = 0;
    for (size_t i = 1; i < path.size(); ++i) {          // edge index = i (1‑based)
        int u = path[i-1];
        int v = path[i];
        // find the edge (u,v) – there may be several, we take the first one
        const Edge* chosen = nullptr;
        for (const auto &e : g[u])
            if (e.to == v) { chosen = &e; break; }
        if (!chosen) continue; // should never happen
        if (isPrime[i])
            realB += 3LL * chosen->w2;
        else
            realA += chosen->w1;
    }
    // cerr << "Exact Blackie‑length = " << realA + realB << '\n';
    */
    return 0;
}
