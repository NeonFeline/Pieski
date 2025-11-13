#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int to;
    int w1;
    int w2;
};

/* 64‑bit key = (vertex << 32) | length   – both fit into 32 bits */
inline long long make_key(int v, int len) {
    return (static_cast<long long>(v) << 32) | static_cast<unsigned>(len);
}

/* --------------------------------------------------------------- */
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
        g[v].push_back({u, w1, w2});               // undirected
    }

    /* ----------  prime table up to N ---------- */
    vector<char> isPrime(N + 1, true);
    if (N >= 0) isPrime[0] = false;
    if (N >= 1) isPrime[1] = false;
    for (int p = 2; p * 1LL * p <= N; ++p)
        if (isPrime[p])
            for (int x = p * p; x <= N; x += p) isPrime[x] = false;

    const long long INF = (1LL << 62);

    /* ----------  Dijkstra on (vertex, length) ---------- */
    using State = tuple<long long, int, int>;          // (dist, vertex, length)
    priority_queue<State, vector<State>, greater<State>> pq;

    vector< unordered_map<int,long long> > dist(N);
    unordered_map< long long, pair<int,int> > parent; // key -> (prevV, prevLen)

    dist[A][0] = 0;
    pq.emplace(0LL, A, 0);

    int bestLen = -1;          // length of the optimal destination state
    long long bestDist = INF;

    while (!pq.empty()) {
        auto [d, v, len] = pq.top(); pq.pop();

        auto it = dist[v].find(len);
        if (it == dist[v].end() || d != it->second) continue; // outdated

        if (v == B) {                 // first time we pop B → optimal
            bestDist = d;
            bestLen  = len;
            break;
        }

        for (const auto &e : g[v]) {
            int to = e.to;
            int nlen = len + 1;
            long long add = isPrime[nlen] ? 3LL * e.w2 : static_cast<long long>(e.w1);
            long long nd = d + add;

            auto &mp = dist[to];
            auto it2 = mp.find(nlen);
            if (it2 == mp.end() || nd < it2->second) {
                mp[nlen] = nd;
                parent[ make_key(to, nlen) ] = {v, len};
                pq.emplace(nd, to, nlen);
            }
        }
    }

    if (bestLen == -1) {                 // no path
        cout << 0 << '\n';
        return 0;
    }

    /* ----------  rebuild the path ---------- */
    vector<int> path;
    int curV = B, curL = bestLen;
    while (true) {
        path.push_back(curV);
        if (curV == A && curL == 0) break;
        auto p = parent[ make_key(curV, curL) ];
        curV = p.first;
        curL = p.second;
    }
    reverse(path.begin(), path.end());

    cout << path.size() << '\n';
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) cout << ' ';
        cout << path[i];
    }
    cout << '\n';
    return 0;
}
