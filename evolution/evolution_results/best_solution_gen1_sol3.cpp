#include <bits/stdc++.h>
using namespace std;

using ll = long long;
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
        g[v].push_back({u, w1, w2});          // graph is undirected
    }
    
    /* ---------- prime table up to N ---------- */
    vector<char> isPrime(N + 1, true);
    isPrime.assign(N + 1, true);
    if (N >= 0) isPrime[0] = false;
    if (N >= 1) isPrime[1] = false;
    for (int p = 2; p * p <= N; ++p) if (isPrime[p])
        for (int q = p * p; q <= N; q += p) isPrime[q] = false;
    
    /* ---------- Dijkstra on (node, step) ---------- */
    using State = tuple<ll, int, int>;          // (dist, node, step)
    priority_queue<State, vector<State>, greater<State>> pq;
    
    const ll INF = (1LL<<62);
    
    // dist[node][step]  – stored sparsely
    vector< unordered_map<int,ll> > dist(N);
    vector< unordered_map<int, pair<int,int>> > parent(N);
    
    dist[A][0] = 0;
    pq.emplace(0LL, A, 0);
    
    int finalStep = -1;                         // step of the optimal state
    ll finalDist = INF;
    
    while (!pq.empty()) {
        auto [d, u, k] = pq.top(); pq.pop();
        auto it = dist[u].find(k);
        if (it == dist[u].end() || d != it->second) continue; // outdated
        
        if (u == B) {               // first time we pop B -> optimal
            finalStep = k;
            finalDist = d;
            break;
        }
        if (k == N-1) continue;     // cannot go further (simple path limit)
        int nk = k + 1;
        for (const auto &e : g[u]) {
            ll nd = d + (isPrime[nk] ? 3LL * e.w2 : (ll)e.w1);
            auto &mp = dist[e.to];
            auto it2 = mp.find(nk);
            if (it2 == mp.end() || nd < it2->second) {
                mp[nk] = nd;
                parent[e.to][nk] = {u, k};
                pq.emplace(nd, e.to, nk);
            }
        }
    }
    
    if (finalStep == -1) {               // no path (should not happen for connected input)
        cout << -1 << "\n";
        return 0;
    }
    
    /* ---------- rebuild vertex sequence ---------- */
    vector<int> path;
    int curNode = B;
    int curStep = finalStep;
    while (true) {
        path.push_back(curNode);
        if (curNode == A && curStep == 0) break;
        auto pr = parent[curNode][curStep];
        curNode = pr.first;
        curStep = pr.second;
    }
    reverse(path.begin(), path.end());
    
    cout << path.size() << "\n";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) cout << ' ';
        cout << path[i];
    }
    cout << "\n";
    return 0;
}
