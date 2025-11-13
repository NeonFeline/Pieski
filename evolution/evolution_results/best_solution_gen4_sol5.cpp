#include <bits/stdc++.h>
using namespace std;

/*** ---------- 1. linear sieve for primes ---------- ***/
vector<char> linearSieve(int n)                 // isPrime[i] = true ⇔ i is prime
{
    vector<char> isPrime(n + 1, true);
    vector<int>  primes;
    isPrime[0] = isPrime[1] = false;

    for (int i = 2; i <= n; ++i) {
        if (isPrime[i]) primes.push_back(i);
        for (int p : primes) {
            long long v = 1LL * i * p;
            if (v > n) break;
            isPrime[(int)v] = false;
            if (i % p == 0) break;
        }
    }
    return isPrime;
}

/*** ---------- 2. graph definition ---------- ***/
struct Edge {
    int to;
    int w1;
    int w2;
};

/*** ---------- 3. Dijkstra on (vertex,length) ---------- ***/
struct State {
    long long dist;
    int       v;
    int       len;               // number of edges already taken

    bool operator<(State const& other) const { return dist > other.dist; }
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
        g[v].push_back({u, w1, w2});          // undirected graph
    }

    /* ----- prime table up to N (max possible path length) ----- */
    const int MAXLEN = N;                     // a simple path can contain at most N‑1 edges
    vector<char> isPrime = linearSieve(MAXLEN);

    /* ----- Dijkstra containers ----- */
    const long long INF = (1LL << 60);

    // distance[v][len] is stored sparsely: only visited pairs are kept
    // we use a vector of unordered_map because len ≤ N (≈1e5) and
    // the number of stored pairs is moderate.
    vector< unordered_map<int,long long> > dist(N);
    vector< unordered_map<int,int> >        predV(N);   // predecessor vertex
    vector< unordered_map<int,int> >        predL(N);   // predecessor length

    priority_queue<State> pq;

    dist[A][0] = 0;
    pq.push({0LL, A, 0});

    int finalLen = -1;                         // length of the optimal path
    while (!pq.empty()) {
        State cur = pq.top(); pq.pop();

        // the entry may be outdated
        auto it = dist[cur.v].find(cur.len);
        if (it == dist[cur.v].end() || it->second != cur.dist) continue;

        if (cur.v == B) {                      // first time we pop B → optimal
            finalLen = cur.len;
            break;
        }

        for (const Edge& e : g[cur.v]) {
            int nxt = e.to;
            int nlen = cur.len + 1;
            if (nlen > MAXLEN) continue;       // safety, should never happen

            long long add = isPrime[nlen] ? 3LL * e.w2 : (long long)e.w1;
            long long ndist = cur.dist + add;

            auto itPrev = dist[nxt].find(nlen);
            if (itPrev == dist[nxt].end() || ndist < itPrev->second) {
                dist[nxt][nlen] = ndist;
                predV[nxt][nlen] = cur.v;
                predL[nxt][nlen] = cur.len;
                pq.push({ndist, nxt, nlen});
            }
        }
    }

    if (finalLen == -1) {                      // no path (should not happen)
        cout << -1 << '\n';
        return 0;
    }

    /* ----- 4. reconstruct the path ----- */
    vector<int> path;
    int curV = B, curL = finalLen;
    while (true) {
        path.push_back(curV);
        if (curV == A && curL == 0) break;
        int pv = predV[curV][curL];
        int pl = predL[curV][curL];
        curV = pv; curL = pl;
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
