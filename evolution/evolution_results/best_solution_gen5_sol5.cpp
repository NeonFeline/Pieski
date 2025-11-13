#include <bits/stdc++.h>
using namespace std;

using ll = long long;

/***  edge definition  ***/
struct Edge {
    int to;
    int w1;
    int w2;
};

/***  key for a (vertex , length) pair  ***/
static inline long long make_key(int v, int len, int stride) {
    return 1LL * v * stride + len;          // stride = N+1
}

/***  prime sieve up to limit (inclusive)  ***/
vector<char> build_prime_table(int limit) {
    vector<char> isPrime(limit + 1, true);
    if (limit >= 0) isPrime[0] = false;
    if (limit >= 1) isPrime[1] = false;
    for (int p = 2; 1LL * p * p <= limit; ++p)
        if (isPrime[p])
            for (int x = p * p; x <= limit; x += p)
                isPrime[x] = false;
    return isPrime;
}

/***  main  ***/
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
        g[v].push_back({u, w1, w2});          // undirected graph
    }

    /* ---------- prime table ---------- */
    const vector<char> isPrime = build_prime_table(N);   // path length never exceeds N

    const ll INF = (1LL << 60);
    const int stride = N + 1;                // for key generation

    /* ---------- Dijkstra on (vertex,length) states ---------- */
    using State = tuple<ll, int, int>;       // (dist , vertex , length)

    unordered_map<long long, ll> dist;               // distance for each state
    unordered_map<long long, pair<int,int>> parent; // predecessor state

    priority_queue<State, vector<State>, greater<State>> pq;

    long long startKey = make_key(A, 0, stride);
    dist[startKey] = 0;
    pq.emplace(0LL, A, 0);

    ll answerDist = INF;
    int answerLen = -1;

    while (!pq.empty()) {
        auto [d, v, len] = pq.top(); pq.pop();
        long long curKey = make_key(v, len, stride);
        auto it = dist.find(curKey);
        if (it == dist.end() || d != it->second) continue;   // outdated entry

        if (v == B) {                     // first time we pop B → optimal
            answerDist = d;
            answerLen = len;
            break;
        }

        for (const Edge &e : g[v]) {
            int nl = len + 1;
            if (nl > N) continue;         // a simple path cannot be longer than N
            ll add = isPrime[nl] ? 3LL * e.w2 : (ll) e.w1;
            ll nd = d + add;
            long long nk = make_key(e.to, nl, stride);
            auto it2 = dist.find(nk);
            if (it2 == dist.end() || nd < it2->second) {
                dist[nk] = nd;
                parent[nk] = {v, len};
                pq.emplace(nd, e.to, nl);
            }
        }
    }

    if (answerDist == INF) {
        cout << -1 << '\n';
        return 0;
    }

    /* ---------- reconstruct the path ---------- */
    vector<int> path;
    int curV = B, curLen = answerLen;
    while (!(curV == A && curLen == 0)) {
        path.push_back(curV);
        long long curKey = make_key(curV, curLen, stride);
        auto pr = parent[curKey];
        curV = pr.first;
        curLen = pr.second;
    }
    path.push_back(A);
    reverse(path.begin(), path.end());

    cout << path.size() << '\n';
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) cout << ' ';
        cout << path[i];
    }
    cout << '\n';
    return 0;
}
