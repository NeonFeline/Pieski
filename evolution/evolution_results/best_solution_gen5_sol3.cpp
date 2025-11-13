#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int to;
    int w1;
    int w2;
};

using ll = long long;
using State = tuple<ll, int, int>;          // (dist , vertex , path length)

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    if (!(cin >> N >> M)) return 0;
    int A, B;
    cin >> A >> B;

    /* ---------- read graph (undirected) ---------- */
    vector<vector<Edge>> g(N);
    for (int i = 0; i < M; ++i) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        g[u].push_back({v, w1, w2});
        g[v].push_back({u, w1, w2});          // change to directed if required
    }

    /* ---------- prime table up to N (max possible length) ---------- */
    vector<char> isPrime(N + 1, true);
    if (N >= 0) isPrime[0] = false;
    if (N >= 1) isPrime[1] = false;
    for (int p = 2; p * p <= N; ++p) if (isPrime[p])
        for (int x = p * p; x <= N; x += p) isPrime[x] = false;

    /* ---------- helpers for encoding a state ---------- */
    const ll INF = (1LL << 60);
    const ll MULT = static_cast<ll>(N) + 1;          // > max length
    auto key = [&](int v, int len) -> ll {
        return static_cast<ll>(v) * MULT + len;
    };

    unordered_map<ll, ll> dist;                     // state -> best distance
    unordered_map<ll, pair<int,int>> parent;        // state -> (prev vertex, prev len)
    dist.reserve(N * 4);    // rough hint, will grow as needed
    parent.reserve(N * 4);

    priority_queue<State, vector<State>, greater<State>> pq;

    dist[key(A,0)] = 0;
    pq.emplace(0LL, A, 0);

    ll answerDist = INF;
    int answerLen = -1;

    while (!pq.empty()) {
        auto [d, v, len] = pq.top(); pq.pop();
        ll curKey = key(v, len);
        auto it = dist.find(curKey);
        if (it == dist.end() || d != it->second) continue;   // outdated entry

        if (v == B) {                     // first pop of B ⇒ optimal
            answerDist = d;
            answerLen  = len;
            break;
        }

        for (const Edge &e : g[v]) {
            int to = e.to;
            int nlen = len + 1;
            if (nlen > N) continue;       // cannot be longer than N (acyclic)
            ll add = isPrime[nlen] ? 3LL * e.w2 : static_cast<ll>(e.w1);
            ll nd = d + add;
            ll nKey = key(to, nlen);
            auto it2 = dist.find(nKey);
            if (it2 == dist.end() || nd < it2->second) {
                dist[nKey] = nd;
                parent[nKey] = {v, len};
                pq.emplace(nd, to, nlen);
            }
        }
    }

    if (answerDist == INF) {
        cout << -1 << '\n';
        return 0;
    }

    /* ---------- reconstruct vertex sequence ---------- */
    vector<int> path;
    int curV = B;
    int curLen = answerLen;
    while (!(curV == A && curLen == 0)) {
        path.push_back(curV);
        ll curKey = key(curV, curLen);
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
