#include <bits/stdc++.h>
using namespace std;

/*---------------------------  fast scanner  ---------------------------*/
struct FastScanner {
    static const int BUFSIZE = 1 << 20;
    char buf[BUFSIZE];
    int idx, size;
    FastScanner() : idx(0), size(0) {}
    inline char getcharFast() {
        if (idx >= size) {
            size = (int)fread(buf, 1, BUFSIZE, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }
    template <class T>
    bool nextInt(T &out) {
        char c; T sign = 1; T val = 0;
        do { c = getcharFast(); if (!c) return false; } while (c!='-' && (c<'0' || c>'9'));
        if (c == '-') { sign = -1; c = getcharFast(); }
        for (; c >= '0' && c <= '9'; c = getcharFast())
            val = val * 10 + (c - '0');
        out = val * sign;
        return true;
    }
} scanner;

/*---------------------------  edge struct  ---------------------------*/
struct Edge {
    int to;
    long long w1, w2;
};

/*---------------------------  prime sieve  ---------------------------*/
vector<char> sievePrimes(int n) {
    vector<char> isPrime(n + 1, true);
    if (n >= 0) isPrime[0] = false;
    if (n >= 1) isPrime[1] = false;
    for (int p = 2; 1LL * p * p <= n; ++p)
        if (isPrime[p])
            for (int q = p * p; q <= n; q += p)
                isPrime[q] = false;
    return isPrime;
}

/*---------------------------  main  ---------------------------*/
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, M;
    if (!scanner.nextInt(N)) return 0;
    scanner.nextInt(M);
    int A, B;
    scanner.nextInt(A);
    scanner.nextInt(B);

    vector<vector<Edge>> adj(N);
    adj.reserve(N);
    for (int i = 0; i < M; ++i) {
        int u, v; long long w1, w2;
        scanner.nextInt(u); scanner.nextInt(v);
        scanner.nextInt(w1); scanner.nextInt(w2);
        adj[u].push_back({v, w1, w2});
        adj[v].push_back({u, w1, w2});               // undirected
    }

    /*--- prime table (max possible length = N-1) ---*/
    vector<char> isPrime = sievePrimes(N);

    /*--- Dijkstra on layered graph ---------------------------------*/
    using State = tuple<long long,int,int>;           // (dist, vertex, length)
    priority_queue<State, vector<State>, greater<State>> pq;

    const long long INF = (1LL<<62);

    // sparse distance table: for each vertex keep a map length->dist
    vector< unordered_map<int,long long> > dist(N);
    vector< unordered_map<int,pair<int,int>> > pred(N); // (prevVertex, prevLength)

    dist[A][0] = 0;
    pq.emplace(0LL, A, 0);

    while (!pq.empty()) {
        auto [d, v, len] = pq.top(); pq.pop();

        auto it = dist[v].find(len);
        if (it == dist[v].end() || d != it->second) continue; // stale entry

        if (v == B) {
            // we cannot stop here because a shorter distance may exist
            // with a different length; we let the algorithm finish.
        }

        if (len == N-1) continue; // cannot extend beyond N-1 edges

        for (const Edge &e : adj[v]) {
            int u = e.to;
            int nlen = len + 1;
            long long w = isPrime[nlen] ? 3LL * e.w2 : e.w1;
            long long nd = d + w;

            auto &du = dist[u];
            auto it2 = du.find(nlen);
            if (it2 == du.end() || nd < it2->second) {
                du[nlen] = nd;
                pred[u][nlen] = {v, len};
                pq.emplace(nd, u, nlen);
            }
        }
    }

    /*--- find the best length that reached B -----------------------*/
    if (dist[B].empty()) {
        cout << 0 << "\n";
        return 0;
    }
    long long bestDist = INF;
    int bestLen = -1;
    for (auto &pr : dist[B]) {
        if (pr.second < bestDist) {
            bestDist = pr.second;
            bestLen = pr.first;
        }
    }

    /*--- reconstruct the path --------------------------------------*/
    vector<int> path;
    int cur = B;
    int curLen = bestLen;
    while (true) {
        path.push_back(cur);
        if (cur == A && curLen == 0) break;
        auto p = pred[cur][curLen];
        cur = p.first;
        curLen = p.second;
    }
    reverse(path.begin(), path.end());

    cout << path.size() << "\n";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) cout << ' ';
        cout << path[i];
    }
    cout << "\n";

    /*--- (optional) exact Blackie‑length of the printed path --------*/
    long long exact = 0;
    for (size_t i = 1; i < path.size(); ++i) {
        int u = path[i-1];
        int v = path[i];
        // find the edge that was actually used (the one stored in pred)
        // we can retrieve it by scanning the adjacency list of u;
        // the graph can contain parallel edges, but any of them yields
        // the same cost because the algorithm already chose the cheapest
        // one for this length.
        const Edge *chosen = nullptr;
        for (const Edge &e : adj[u])
            if (e.to == v) { chosen = &e; break; }
        // safety check – should never happen
        if (!chosen) continue;
        if (isPrime[i]) exact += 3LL * chosen->w2;
        else           exact += chosen->w1;
    }
    // Uncomment for debugging:
    // cerr << "Exact Blackie‑length = " << exact << "\n";

    return 0;
}
