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

    vector<vector<Edge>> g(N);
    for (int i = 0; i < M; ++i) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        g[u].push_back({v, w1, w2});
        g[v].push_back({u, w1, w2});            // undirected
    }

    /* ----------  prime table up to N (max possible edge index) ---------- */
    vector<char> isPrime(N + 1, true);
    isPrime[0] = isPrime[1] = false;
    vector<int> primes;
    for (int i = 2; i <= N; ++i) {
        if (isPrime[i]) primes.push_back(i);
        for (int p : primes) {
            long long v = 1LL * i * p;
            if (v > N) break;
            isPrime[v] = false;
            if (i % p == 0) break;
        }
    }

    const ll INF = (1LL << 60);
    // key = vertex * (N+1) + len   (len ≤ N)
    auto key = [&](int v, int len) -> long long {
        return 1LL * v * (N + 1) + len;
    };

    unordered_map<long long, ll> dist;          // distance for each state
    unordered_map<long long, pair<int,int>> parent; // predecessor state

    priority_queue<State, vector<State>, greater<State>> pq;

    dist[key(A,0)] = 0;
    pq.emplace(0LL, A, 0);

    ll answerDist = INF;
    int answerLen = -1;

    while (!pq.empty()) {
        auto [d, v, len] = pq.top(); pq.pop();
        long long curKey = key(v, len);
        auto it = dist.find(curKey);
        if (it == dist.end() || d != it->second) continue; // outdated

        if (v == B) {               // first time we pop B -> optimal
            answerDist = d;
            answerLen = len;
            break;
        }

        for (const Edge &e : g[v]) {
            int to = e.to;
            int nlen = len + 1;
            if (nlen > N) continue;                     // cannot be longer than N
            ll add = isPrime[nlen] ? 3LL * e.w2 : (ll)e.w1;
            ll nd = d + add;
            long long nk = key(to, nlen);
            auto it2 = dist.find(nk);
            if (it2 == dist.end() || nd < it2->second) {
                dist[nk] = nd;
                parent[nk] = {v, len};
                pq.emplace(nd, to, nlen);
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
        long long curKey = key(curV, curLen);
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
