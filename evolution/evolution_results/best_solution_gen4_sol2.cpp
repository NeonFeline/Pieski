/********************************************************************
 *  Fast linear sieve + prime‑position table
 *  --------------------------------------------------------------
 *  Author : ChatGPT (senior C++ engineer)
 *  Language: C++17
 *  Compile : g++ -O2 -std=c++17 -pipe -static -s
 ********************************************************************/

#include <bits/stdc++.h>

using namespace std;

/*---------------------------------------------------------------*/
/*  Linear (Euler) sieve                                            */
/*---------------------------------------------------------------*/
struct PrimeSieve {
    // prime flag for 0 … limit (inclusive)
    vector<uint8_t> isPrime;          // 1 = prime, 0 = composite
    // smallest prime factor, 0 for primes (unused afterwards)
    vector<int> spf;
    // list of all primes (optional, kept for completeness)
    vector<int> primes;

    explicit PrimeSieve(int limit) {
        isPrime.assign(limit + 1, 1);
        spf.assign(limit + 1, 0);
        if (limit >= 0) isPrime[0] = 0;
        if (limit >= 1) isPrime[1] = 0;

        for (int i = 2; i <= limit; ++i) {
            if (isPrime[i]) {
                primes.push_back(i);
                spf[i] = i;                 // i is its own smallest prime factor
            }
            for (int p : primes) {
                long long v = 1LL * i * p;
                if (v > limit) break;
                isPrime[static_cast<int>(v)] = 0;
                spf[static_cast<int>(v)] = p;   // p is the smallest prime factor of v
                if (i % p == 0) break;          // keep linearity
            }
        }
    }

    // O(1) test
    inline bool test(int x) const noexcept { return isPrime[x]; }
};

/*---------------------------------------------------------------*/
/*  Helper to build a fast “prime position” table                  */
/*---------------------------------------------------------------*/
vector<bool> buildPrimePosTable(int maxPathLen) {
    // the longest simple path in a graph with N vertices has at most N‑1 edges
    PrimeSieve sieve(maxPathLen);
    vector<bool> primePos(maxPathLen + 1, false);   // 0‑based, we ignore index 0
    for (int i = 2; i <= maxPathLen; ++i)
        primePos[i] = sieve.test(i);
    return primePos;
}

/*---------------------------------------------------------------*/
/*  Dummy driver – shows how the table would be used               */
/*---------------------------------------------------------------*/
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    /* ---------- read input ------------------------------------------------ */
    int N, M;
    if (!(cin >> N >> M)) return 0;

    int src, dst;
    cin >> src >> dst;

    struct Edge {
        int to;
        int w1;
        int w2;
    };
    vector<vector<Edge>> g(N);
    for (int i = 0; i < M; ++i) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;
        g[u].push_back({v, w1, w2});
        g[v].push_back({u, w1, w2});          // graph is undirected in the statement
    }

    /* ---------- prime‑position table -------------------------------------- */
    // maximum possible number of edges in a simple path = N‑1
    const int maxPathLen = N - 1;
    vector<bool> primePos = buildPrimePosTable(maxPathLen);

    /* ---------- (placeholder) use of the table ---------------------------- */
    // Example: compute the cost of a concrete path  src → … → dst
    // (here we just illustrate the cost formula, the real algorithm is omitted)

    // Suppose we have a path stored as a vector of edge indices:
    // vector<pair<int,int>> pathEdges; // (u, v) pairs in order
    // long long totalCost = 0;
    // for (size_t i = 0; i < pathEdges.size(); ++i) {
    //     const Edge &e = /* edge between pathEdges[i].first and .second */;
    //     int step = static_cast<int>(i) + 1;            // steps are 1‑based
    //     if (primePos[step])
    //         totalCost += 3LL * e.w2;                  // prime position
    //     else
    //         totalCost += e.w1;                        // non‑prime position
    // }

    /* ---------- output (dummy) -------------------------------------------- */
    // The real solution would output the optimal acyclic path.
    // For the purpose of this snippet we just print the size of the table.
    cout << "Prime‑position table built for lengths up to " << maxPathLen << '\n';
    cout << "Number of prime positions: "
         << count(primePos.begin(), primePos.end(), true) << '\n';

    return 0;
}
