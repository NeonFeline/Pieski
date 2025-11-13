#include <bits/stdc++.h>
using namespace std;

using ll   = long long;
using u64  = uint64_t;

/* ----------  helpers for packing (vertex , length) into one 64‑bit key ---------- */
inline u64 make_key(int v, int len) noexcept {
    return ( (u64)(uint32_t)v << 32 ) | (uint32_t)len;
}
inline pair<int,int> decode_key(u64 key) noexcept {
    int v   = (int)(key >> 32);
    int len = (int)(key & 0xffffffffULL);
    return {v, len};
}

/* ----------  edge structure ---------- */
struct Edge {
    int to;
    int w1;
    int w2;
};

/* ----------  state for the priority queue ---------- */
struct State {
    ll dist;   // accumulated Blackie‑length
    int v;     // current vertex
    int len;   // number of edges already taken

    bool operator>(State const& other) const noexcept {
        return dist > other.dist;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    /* ----------  input ---------- */
    int N, M;
    if(!(cin >> N >> M)) return 0;
    int A, B;
    cin >> A >> B;

    vector<vector<Edge>> g(N);
    for (int i = 0; i < M; ++i) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        g[u].push_back({v, w1, w2});
        g[v].push_back({u, w1, w2});          // undirected
    }

    /* ----------  prime table up to N (max possible length) ---------- */
    vector<char> isPrime(N + 1, true);
    isPrime[0] = isPrime[1] = false;
    vector<int> primes;
    for (int i = 2; i <= N; ++i) {
        if (isPrime[i]) primes.push_back(i);
        for (int p : primes) {
            long long v = 1LL * i * p;
            if (v > N) break;
            isPrime[(int)v] = false;
            if (i % p == 0) break;
        }
    }

    /* ----------  Dijkstra on (vertex, length) ---------- */
    const ll INF = (1LL << 60);

    // hash tables: state → distance / predecessor
    unordered_map<u64, ll>    dist;
    unordered_map<u64, u64>   parent;   // predecessor packed key
    dist.reserve(N * 4);
    parent.reserve(N * 4);

    priority_queue<State, vector<State>, greater<State>> pq;

    u64 startKey = make_key(A, 0);
    dist[startKey] = 0;
    pq.push({0, A, 0});

    ll   answerDist = INF;
    int  answerLen  = -1;
    u64  answerKey  = 0;          // will hold the key of (B, answerLen)

    while (!pq.empty()) {
        State cur = pq.top(); pq.pop();
        u64 curKey = make_key(cur.v, cur.len);

        // outdated entry ?
        auto itDist = dist.find(curKey);
        if (itDist == dist.end() || cur.dist != itDist->second) continue;

        // reached destination – first pop is optimal
        if (cur.v == B) {
            answerDist = cur.dist;
            answerLen  = cur.len;
            answerKey  = curKey;
            break;
        }

        // expand neighbours
        for (const Edge& e : g[cur.v]) {
            int nlen = cur.len + 1;
            if (nlen > N) continue;               // cannot be longer than N
            ll add = isPrime[nlen] ? 3LL * e.w2 : (ll)e.w1;
            ll ndist = cur.dist + add;
            u64 nKey = make_key(e.to, nlen);

            auto it = dist.find(nKey);
            if (it == dist.end() || ndist < it->second) {
                dist[nKey] = ndist;
                parent[nKey] = curKey;
                pq.push({ndist, e.to, nlen});
            }
        }
    }

    if (answerDist == INF) {
        cout << -1 << '\n';
        return 0;
    }

    /* ----------  reconstruct original vertex sequence ---------- */
    vector<int> path;
    u64 cur = answerKey;
    while (true) {
        auto [v, len] = decode_key(cur);
        path.push_back(v);
        if (v == A && len == 0) break;
        cur = parent.at(cur);
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
