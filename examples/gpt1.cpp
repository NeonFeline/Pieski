#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF64 = (int64)4e18;
static inline int64 mul3(int64 x){ return x >= (INF64/3) ? (INF64/2) : 3LL * x; }

struct Edge {
    int to;
    int id;   // input edge id (same for both directions)
    int64 w1; // weight for composite indices
    int64 w3; // 3*w2 weight for prime indices
};

struct Timer {
    chrono::steady_clock::time_point start;
    Timer() { start = chrono::steady_clock::now(); }
    inline double elapsed() const {
        using namespace chrono;
        return duration_cast<duration<double>>(steady_clock::now() - start).count();
    }
};

// Prime sieve + prefix pi up to maxN
struct PrimeSvc {
    vector<char> isPrime;
    vector<int> pi; // pi[i] = #primes <= i
    void build(int maxN) {
        isPrime.assign(maxN + 1, 1);
        if (maxN >= 0) isPrime[0] = 0;
        if (maxN >= 1) isPrime[1] = 0;
        for (int i = 2; (int64)i * i <= maxN; ++i) {
            if (isPrime[i]) {
                for (int j = i * i; j <= maxN; j += i) isPrime[j] = 0;
            }
        }
        pi.assign(maxN + 1, 0);
        int cnt = 0;
        for (int i = 0; i <= maxN; ++i) {
            if (isPrime[i]) ++cnt;
            pi[i] = cnt;
        }
    }
    inline bool prime(int x) const {
        return (x >= 0 && x < (int)isPrime.size() && isPrime[x]);
    }
    inline int primesInRange(int l_inclusive, int r_inclusive) const {
        if (r_inclusive < l_inclusive) return 0;
        l_inclusive = max(l_inclusive, 0);
        if (r_inclusive >= (int)pi.size()) r_inclusive = (int)pi.size() - 1;
        if (l_inclusive <= 0) return pi[r_inclusive];
        return pi[r_inclusive] - pi[l_inclusive - 1];
    }
};

// Dijkstra (int64 weights)
template <class WeightGetter>
void dijkstra_ll(const vector<vector<Edge>>& adj, int N, int src,
                 WeightGetter weight_of_edge,
                 vector<int64>& dist, vector<int>& parent, vector<int>& parentAdjIdx,
                 const vector<char>* allowed = nullptr, int bannedEdgeId = -1)
{
    dist.assign(N, INF64);
    parent.assign(N, -1);
    parentAdjIdx.assign(N, -1);
    if (src < 0 || src >= N) return;

    using P = pair<int64,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[src] = 0;
    pq.push({0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        for (int ei = 0; ei < (int)adj[u].size(); ++ei) {
            const Edge& e = adj[u][ei];
            if (bannedEdgeId >= 0 && e.id == bannedEdgeId) continue;
            int v = e.to;
            if (allowed && !(*allowed)[v]) continue;
            int64 w = weight_of_edge(e);
            if (w >= INF64/4) continue;
            int64 nd = d + w;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                parentAdjIdx[v] = ei;
                pq.push({nd, v});
            }
        }
    }
}

// Dijkstra (double weights) for parametric c_lambda
template <class WeightGetter>
void dijkstra_double(const vector<vector<Edge>>& adj, int N, int src,
                     WeightGetter weight_of_edge,
                     vector<double>& dist, vector<int>& parent, vector<int>& parentAdjIdx,
                     const vector<char>* allowed = nullptr, int bannedEdgeId = -1)
{
    dist.assign(N, numeric_limits<double>::infinity());
    parent.assign(N, -1);
    parentAdjIdx.assign(N, -1);
    if (src < 0 || src >= N) return;

    using P = pair<double,int>;
    priority_queue<P, vector<P>, greater<P>> pq;
    dist[src] = 0.0;
    pq.push({0.0, src});

    while (!pq.empty()) {
        auto [d, u] = pq.top(); pq.pop();
        if (d != dist[u]) continue;
        for (int ei = 0; ei < (int)adj[u].size(); ++ei) {
            const Edge& e = adj[u][ei];
            if (bannedEdgeId >= 0 && e.id == bannedEdgeId) continue;
            int v = e.to;
            if (allowed && !(*allowed)[v]) continue;
            double w = weight_of_edge(e);
            if (!isfinite(w) || w >= 1e300) continue;
            double nd = d + w;
            if (nd + 1e-18 < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                parentAdjIdx[v] = ei;
                pq.push({nd, v});
            }
        }
    }
}

// BFS from B for hop-lower-bound and reachability
void bfs_unweighted(const vector<vector<Edge>>& adj, int N, int B, vector<int>& hop, vector<char>& canReach) {
    hop.assign(N, INT_MAX);
    canReach.assign(N, 0);
    if (B < 0 || B >= N) return;
    deque<int> dq;
    hop[B] = 0; canReach[B] = 1; dq.push_back(B);
    while (!dq.empty()) {
        int u = dq.front(); dq.pop_front();
        for (const Edge& e : adj[u]) {
            int v = e.to;
            if (!canReach[v]) {
                canReach[v] = 1;
                hop[v] = hop[u] + 1;
                dq.push_back(v);
            }
        }
    }
}

// Reconstruct path nodes and exact arcs (u, adjIdx)
void reconstruct_path(const vector<vector<Edge>>& adj, int src, int dst,
                      const vector<int>& parent, const vector<int>& parentAdjIdx,
                      vector<int>& outNodes, vector<pair<int,int>>& outArcs) {
    outNodes.clear(); outArcs.clear();
    if (src == dst) { outNodes.push_back(src); return; }
    if (dst < 0 || dst >= (int)parent.size() || parent[dst] == -1) return;
    vector<int> revNodes;
    vector<pair<int,int>> revArcs;
    int v = dst;
    revNodes.push_back(v);
    while (v != src) {
        int u = parent[v];
        if (u < 0) { outNodes.clear(); outArcs.clear(); return; }
        int ei = parentAdjIdx[v];
        revArcs.emplace_back(u, ei);
        v = u;
        revNodes.push_back(v);
    }
    reverse(revNodes.begin(), revNodes.end());
    reverse(revArcs.begin(), revArcs.end());
    outNodes = move(revNodes);
    outArcs = move(revArcs);
}

// Evaluate blackie cost using exact arcs (to disambiguate multiedges)
int64 evaluate_blackie_cost_arcs(const vector<vector<Edge>>& adj, const vector<pair<int,int>>& arcs, const PrimeSvc& ps) {
    int64 g = 0;
    for (int i = 0; i < (int)arcs.size(); ++i) {
        int u = arcs[i].first;
        int ei = arcs[i].second;
        if (u < 0 || u >= (int)adj.size()) return INF64;
        if (ei < 0 || ei >= (int)adj[u].size()) return INF64;
        const Edge& e = adj[u][ei];
        int step = i + 1;
        int64 add = ps.prime(step) ? e.w3 : e.w1;
        if (add >= INF64/4 || g >= INF64/4) return INF64;
        g += add;
    }
    return g;
}

// IDA* with stronger heuristic; exact and acyclic
struct IDAStar {
    const vector<vector<Edge>>& adj;
    const PrimeSvc& ps;
    const vector<int64>& dMinToB;   // Dijkstra(B, min(w1, w3))
    const vector<int64>& d1_toB;    // Dijkstra(B, w1)
    const vector<int>& hopLB;       // BFS hop LB
    const vector<char>& allowed;    // corridor mask
    int N, A, B;
    int64 minDeltaPos;              // min over edges of max(0, w3 - w1)

    vector<char> visited;
    vector<int> stackNode;
    vector<int> stackNextEdgeIdx;
    vector<int64> stackStepCost;

    IDAStar(const vector<vector<Edge>>& g, const PrimeSvc& ps_,
            const vector<int64>& dMin, const vector<int64>& d1toB_,
            const vector<int>& hop_, const vector<char>& allow,
            int n, int a, int b, int64 minDeltaPos_)
        : adj(g), ps(ps_), dMinToB(dMin), d1_toB(d1toB_), hopLB(hop_), allowed(allow),
          N(n), A(a), B(b), minDeltaPos(minDeltaPos_) {
        visited.assign(N, 0);
        stackNode.assign(N + 5, -1);
        stackNextEdgeIdx.assign(N + 5, 0);
        stackStepCost.assign(N + 5, 0);
    }

    inline int64 primesLB_k_to_kplusH(int k, int H) const {
        int l = k + 1;
        int r = k + H;
        if (l > r) return 0;
        if (r >= (int)ps.pi.size()) r = (int)ps.pi.size() - 1;
        if (l < 0) l = 0;
        return (int64)ps.primesInRange(l, r);
    }

    inline int64 heuristic(int u, int k) const {
        int64 h1 = dMinToB[u];
        int64 h2 = d1_toB[u];
        if (h2 >= INF64/4) return h1; // unreachable fallback
        int H = hopLB[u];
        if (H == INT_MAX) return h1;
        int64 primesNeed = primesLB_k_to_kplusH(k, H);
        int64 add = (minDeltaPos > 0) ? (primesNeed * minDeltaPos) : 0;
        int64 bound2 = (h2 >= INF64/4 - add) ? INF64/2 : (h2 + add);
        return max(h1, bound2);
    }

    int search(double timeBudgetSec, const Timer& timer, int64 UB, vector<int>& outPath, int64& outCost) {
        outPath.clear();
        if (!allowed[A] || !allowed[B]) return 0;

        int64 threshold = heuristic(A, 0);
        if (threshold >= UB) threshold = UB - 1;
        if (threshold < 0) threshold = 0;

        int64 bestCost = UB;
        int64 nextThreshold = INF64;

        while (true) {
            fill(visited.begin(), visited.end(), 0);
            int depth = 0;
            int64 g = 0;
            visited[A] = 1;
            stackNode[0] = A;
            stackNextEdgeIdx[0] = 0;
            stackStepCost[0] = 0;
            nextThreshold = INF64;
            vector<int> bestPathLocal;
            bool found = false;

            while (true) {
                if ((depth & 0x3FF) == 0) {
                    if (timer.elapsed() > timeBudgetSec) return -1; // timeout
                }
                int u = stackNode[depth];
                int64 f = g + heuristic(u, depth);
                if (f > threshold || f >= bestCost) {
                    // backtrack
                backtrack:
                    if (depth == 0) break;
                    {
                        int pu = stackNode[depth];
                        visited[pu] = 0;
                        g -= stackStepCost[depth];
                        --depth;
                    }
                    continue;
                }
                if (u == B) {
                    bestCost = g;
                    bestPathLocal.assign(stackNode.begin(), stackNode.begin() + depth + 1);
                    found = true;
                    break;
                }

                int &ei = stackNextEdgeIdx[depth];
                if (ei >= (int)adj[u].size()) {
                    goto backtrack;
                }
                const Edge& e = adj[u][ei++];
                int v = e.to;
                if (!allowed[v] || visited[v]) continue;

                int nextStep = depth + 1;
                int64 stepCost = ps.prime(nextStep) ? e.w3 : e.w1;
                if (stepCost >= INF64/4 || g >= INF64/4) continue;
                int64 ng = g + stepCost;
                if (ng >= bestCost) continue;
                int64 fchild = ng + heuristic(v, nextStep);
                if (fchild > threshold || fchild >= bestCost) {
                    nextThreshold = min(nextThreshold, fchild);
                    continue;
                }
                visited[v] = 1;
                ++depth;
                stackNode[depth] = v;
                stackNextEdgeIdx[depth] = 0;
                stackStepCost[depth] = stepCost;
                g = ng;
            }
            if (found) {
                outCost = bestCost;
                outPath = move(bestPathLocal);
                return 1;
            }
            if (nextThreshold >= bestCost) return 0;
            if (nextThreshold >= INF64/4) return 0;
            threshold = nextThreshold;
            if (threshold >= UB) threshold = UB - 1;
            if (threshold < 0) threshold = 0;
        }
    }
};

// Utility: compute corridor by w1 distances
void build_w1_corridor(const vector<int64>& d1_fromA, const vector<int64>& d1_toB, int64 D1, double eps, vector<char>& allowed) {
    int N = (int)d1_fromA.size();
    if (D1 >= INF64/4) {
        fill(allowed.begin(), allowed.end(), 1);
        return;
    }
    long double rhs = (long double)D1 * (1.0L + (long double)eps) + 1e-9L;
    for (int u = 0; u < N; ++u) {
        long double lhs = (long double)d1_fromA[u] + (long double)d1_toB[u];
        allowed[u] = (lhs <= rhs);
    }
}

// Single-edge detour (Yen-like 1-deviation). Bans one edge id and recomputes, optionally inside corridor.
bool single_edge_detour_w1(const vector<vector<Edge>>& adj, int N, int A, int B, int bannedEdgeId,
                           const vector<char>* allowed, vector<int>& nodes, vector<pair<int,int>>& arcs,
                           int64& blackieCost, const PrimeSvc& ps)
{
    auto w1Getter = [](const Edge& e)->int64 { return e.w1; };
    vector<int64> dist; vector<int> parent, pEdge;
    dijkstra_ll(adj, N, A, w1Getter, dist, parent, pEdge, allowed, bannedEdgeId);
    if (dist[B] >= INF64/4) return false;
    reconstruct_path(adj, A, B, parent, pEdge, nodes, arcs);
    blackieCost = evaluate_blackie_cost_arcs(adj, arcs, ps);
    return blackieCost < INF64/4;
}

// Generate lambda candidates near predicted prime fractions
static inline void generate_lambda_candidates(int Hmin, int Hmax, const PrimeSvc& ps, vector<double>& lambdas) {
    lambdas.clear();
    auto push_unique = [&](double x){
        x = max(0.0, min(1.0, x));
        for (double z : lambdas) if (fabs(z - x) < 1e-3) return;
        lambdas.push_back(x);
    };
    // core seeds
    push_unique(0.0); push_unique(0.15); push_unique(0.25); push_unique(0.35);
    push_unique(0.5); push_unique(0.65); push_unique(0.85);
    // primes fraction around H in [Hmin..Hmax], sample a few points
    int samples = 6;
    for (int i = 0; i < samples; ++i) {
        int L = Hmin + (int)((long long)(Hmax - Hmin) * i / max(1, samples - 1));
        L = max(L, 1);
        double f = (double)ps.primesInRange(1, L) / (double)L;
        push_unique(f);
        push_unique(min(1.0, f + 0.1));
        push_unique(max(0.0, f - 0.1));
    }
    sort(lambdas.begin(), lambdas.end());
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Timer timer;
    const double TIME_BUDGET_SEC = 19.5; // keep margin under 20s

    int N, M;
    if (!(cin >> N >> M)) {
        cout << -1 << "\n";
        return 0;
    }

    // Try to read A,B; if not present here, fallback to A=0, B=N-1 (compat with earlier format)
    int A, B;
    {
        streampos sp = cin.tellg();
        int maybeA, maybeB;
        if (cin >> maybeA >> maybeB) {
            A = maybeA; B = maybeB;
        } else {
            cin.clear();
            cin.seekg(sp);
            A = 0; B = N - 1;
        }
    }

    if (A < 0 || A >= N || B < 0 || B >= N) {
        cout << -1 << "\n";
        return 0;
    }

    vector<vector<Edge>> adj(N);
    adj.reserve(N);

    int64 minW1 = INF64, minW3 = INF64;
    int64 minDeltaPos = INF64; // min max(0, w3 - w1)
    minDeltaPos = INF64;

    vector<pair<int,int>> edgesRaw;
    edgesRaw.reserve(M);

    for (int i = 0; i < M; ++i) {
        int u,v; int64 w1, w2;
        if (!(cin >> u >> v >> w1 >> w2)) {
            cout << -1 << "\n";
            return 0;
        }
        if (u < 0 || u >= N || v < 0 || v >= N) continue;
        int64 w3 = mul3(w2);
        Edge e1{v, i, w1, w3};
        Edge e2{u, i, w1, w3};
        adj[u].push_back(e1);
        adj[v].push_back(e2);
        minW1 = min(minW1, w1);
        minW3 = min(minW3, w3);
        int64 deltaPos = w3 > w1 ? (w3 - w1) : 0;
        if (deltaPos < minDeltaPos) minDeltaPos = deltaPos;
    }
    if (A == B) {
        cout << 1 << "\n" << A << "\n";
        return 0;
    }
    if (minDeltaPos == INF64) minDeltaPos = 0;

    // Precompute primes up to 2N+10 for safe k+hop
    PrimeSvc ps; ps.build(max(4, 2*N + 10));

    // Reachability and hop distance
    vector<int> hop; vector<char> canReach;
    bfs_unweighted(adj, N, B, hop, canReach);
    if (!canReach[A]) {
        cout << -1 << "\n";
        return 0;
    }
    int H0 = hop[A];

    // Heuristic distances: dMinToB = Dijkstra from B with weight min(w1, w3)
    vector<int64> dMinToB;
    {
        auto wMin = [](const Edge& e)->int64 { return min(e.w1, e.w3); };
        vector<int> parent, pEdge;
        dijkstra_ll(adj, N, B, wMin, dMinToB, parent, pEdge);
    }

    // w1 distances from A and to B
    vector<int64> d1_fromA, d1_toB;
    {
        auto w1Getter = [](const Edge& e)->int64 { return e.w1; };
        vector<int> parent, pEdge;
        dijkstra_ll(adj, N, A, w1Getter, d1_fromA, parent, pEdge);
        dijkstra_ll(adj, N, B, w1Getter, d1_toB, parent, pEdge);
    }
    int64 D1 = d1_fromA[B];

    // Initial UBs: multiple Dijkstras
    int64 UB = INF64;
    vector<int> bestNodes;
    vector<pair<int,int>> bestArcs;

    auto consider_path = [&](const vector<int>& nodes, const vector<pair<int,int>>& arcs, int64 explicitCost = -1) {
        if (nodes.empty()) return;
        int64 cost = (explicitCost >= 0) ? explicitCost : evaluate_blackie_cost_arcs(adj, arcs, ps);
        if (cost < UB) { UB = cost; bestNodes = nodes; bestArcs = arcs; }
    };

    auto try_dij_ll = [&](auto weightGetter) {
        vector<int64> dist; vector<int> parent, pEdge;
        dijkstra_ll(adj, N, A, weightGetter, dist, parent, pEdge);
        if (dist[B] >= INF64/4) return;
        vector<int> nodes; vector<pair<int,int>> arcs;
        reconstruct_path(adj, A, B, parent, pEdge, nodes, arcs);
        consider_path(nodes, arcs, -1);
    };

    try_dij_ll([](const Edge& e){ return e.w1; });
    if (timer.elapsed() < TIME_BUDGET_SEC * 0.25) try_dij_ll([](const Edge& e){ return e.w3; });
    if (timer.elapsed() < TIME_BUDGET_SEC * 0.30) try_dij_ll([](const Edge& e){ return min(e.w1, e.w3); });

    // Parametric reweighting c_lambda, in a w1-corridor when available
    vector<char> allowed(N, 1);
    if (D1 < INF64/4) build_w1_corridor(d1_fromA, d1_toB, D1, 0.30, allowed); // roomy corridor for fast runs

    auto parametric_runs = [&](const vector<double>& lambdas, double timeFracLimit){
        for (double lam : lambdas) {
            if (timer.elapsed() > timeFracLimit) break;
            auto wLam = [lam](const Edge& e)->double {
                double w1d = (double)e.w1, w3d = (double)e.w3;
                return w1d + lam * (w3d - w1d);
            };
            vector<double> dist; vector<int> parent, pEdge;
            dijkstra_double(adj, N, A, wLam, dist, parent, pEdge,
                            (D1 < INF64/4 ? &allowed : nullptr), -1);
            if (!isfinite(dist[B])) continue;
            vector<int> nodes; vector<pair<int,int>> arcs;
            reconstruct_path(adj, A, B, parent, pEdge, nodes, arcs);
            consider_path(nodes, arcs, -1);
        }
    };

    {
        int Hmax = min(N - 1, max(H0 + 20, H0 * 2));
        vector<double> lambdas;
        generate_lambda_candidates(max(1, H0), max(2, Hmax), ps, lambdas);
        double limit = TIME_BUDGET_SEC * 0.65;
        parametric_runs(lambdas, limit);
    }

    // Strategy selection based on size/density
    double avgDeg = (N > 0 ? (2.0 * (double)M / (double)N) : 0.0);
    bool smallGraph = (N <= 20000 && (long long)M <= 300000) || (N <= 50000 && avgDeg <= 8.0);
    bool timeLeftForExact = (timer.elapsed() < TIME_BUDGET_SEC * 0.80);

    // Try exact IDA* on corridor (progressively widening)
    auto run_exact_in_corridor = [&](double maxTimeShare){
        if (UB >= INF64/4) return; // need at least one UB to bound search well
        vector<double> epsList = {0.0, 0.05, 0.10, 0.20, 0.35, 0.50, 1.00};
        vector<char> allow(N, 1);
        for (double eps : epsList) {
            if (timer.elapsed() > maxTimeShare) break;
            if (D1 < INF64/4) build_w1_corridor(d1_fromA, d1_toB, D1, eps, allow);
            else fill(allow.begin(), allow.end(), 1);
            allow[A] = 1; allow[B] = 1;

            IDAStar ida(adj, ps, dMinToB, d1_toB, hop, allow, N, A, B, minDeltaPos);
            vector<int> idaNodes;
            int64 idaCost = INF64;
            double perRunBudget = max(0.4, (TIME_BUDGET_SEC - timer.elapsed()) / 2.5);
            if (perRunBudget <= 0.05) break;
            int rc = ida.search(perRunBudget, timer, UB, idaNodes, idaCost);
            if (rc == 1) {
                // exact solution within corridor
                if (idaCost < UB) {
                    UB = idaCost;
                    bestNodes = idaNodes;
                    bestArcs.clear(); // arcs not needed for output
                }
                // Found exact; we can stop if eps==1
                if (eps >= 0.50) break;
            } else if (rc == -1) {
                // timeout on this corridor size
                break;
            }
        }
    };

    // Large graph: more approximate iterations and local 1-edge detours
    auto local_single_edge_detours = [&](int maxTrials){
        if (bestNodes.empty()) return;
        // Build edge positions list from bestArcs to score by contribution
        vector<int> nodes = bestNodes;
        vector<pair<int,int>> arcs = bestArcs;
        if (arcs.empty()) {
            // We don't have arcs if best came from IDA*. Recreate arcs by mapping any edge u->v (first seen).
            // But we need edge ids to ban. Build a quick id list by scanning adj[u] to find the lightest (w1+w3) edge to v.
            arcs.reserve(nodes.size() > 0 ? nodes.size()-1 : 0);
            for (size_t i = 0; i + 1 < nodes.size(); ++i) {
                int u = nodes[i], v = nodes[i+1];
                int bestEi = -1; int64 bestKey = INF64;
                for (int ei = 0; ei < (int)adj[u].size(); ++ei) if (adj[u][ei].to == v) {
                    int64 key = adj[u][ei].w1 + adj[u][ei].w3;
                    if (key < bestKey) { bestKey = key; bestEi = ei; }
                }
                if (bestEi < 0) { arcs.clear(); break; }
                arcs.emplace_back(u, bestEi);
            }
        }
        if (arcs.empty()) return;

        // Score positions by actual blackie step cost
        struct Item { int pos; int id; int64 cost; bool prime; };
        vector<Item> items;
        items.reserve(arcs.size());
        int64 curCost = 0;
        for (int i = 0; i < (int)arcs.size(); ++i) {
            int u = arcs[i].first, ei = arcs[i].second;
            const Edge& e = adj[u][ei];
            bool p = ps.prime(i+1);
            int64 c = p ? e.w3 : e.w1;
            items.push_back({i, e.id, c, p});
            curCost += c;
        }
        // Choose top positions by cost (prefer prime positions if delta positive)
        sort(items.begin(), items.end(), [&](const Item& a, const Item& b){
            if (a.cost != b.cost) return a.cost > b.cost;
            if (a.prime != b.prime) return a.prime > b.prime;
            return a.pos < b.pos;
        });
        int trials = min((int)items.size(), maxTrials);
        vector<char> allowCorr(N, 1);
        if (D1 < INF64/4) build_w1_corridor(d1_fromA, d1_toB, D1, 0.50, allowCorr);

        for (int t = 0; t < trials; ++t) {
            if (timer.elapsed() > TIME_BUDGET_SEC * 0.95) break;
            int bannedId = items[t].id;
            vector<int> candNodes; vector<pair<int,int>> candArcs;
            int64 candCost;
            if (single_edge_detour_w1(adj, N, A, B, bannedId, &allowCorr, candNodes, candArcs, candCost, ps)) {
                if (candCost < UB) {
                    UB = candCost;
                    bestNodes = move(candNodes);
                    bestArcs = move(candArcs);
                }
            }
        }
    };

    if (smallGraph && timeLeftForExact) {
        run_exact_in_corridor(TIME_BUDGET_SEC * 0.88);
    } else {
        // Large/dense: do more param runs and local improvements
        if (timer.elapsed() < TIME_BUDGET_SEC * 0.75) {
            vector<double> lambdas2 = {0.08, 0.18, 0.28, 0.42, 0.58, 0.72, 0.88};
            double limit = TIME_BUDGET_SEC * 0.82;
            parametric_runs(lambdas2, limit);
        }
        if (timer.elapsed() < TIME_BUDGET_SEC * 0.90) {
            local_single_edge_detours(24); // try up to 24 heaviest edges
        }
        // Quick exact attempt in a tight corridor if time remains
        if (timer.elapsed() < TIME_BUDGET_SEC * 0.93) {
            run_exact_in_corridor(TIME_BUDGET_SEC * 0.98);
        }
    }

    if (bestNodes.empty()) {
        // Fallback: try plain w1 path one more time
        vector<int64> dist; vector<int> parent, pEdge;
        auto w1Getter = [](const Edge& e)->int64 { return e.w1; };
        dijkstra_ll(adj, N, A, w1Getter, dist, parent, pEdge);
        if (dist[B] >= INF64/4) { cout << -1 << "\n"; return 0; }
        reconstruct_path(adj, A, B, parent, pEdge, bestNodes, bestArcs);
    }

    // Output solution path (0-based node IDs)
    cout << (int)bestNodes.size() << "\n";
    for (size_t i = 0; i < bestNodes.size(); ++i) {
        if (i) cout << ' ';
        cout << bestNodes[i];
    }
    cout << "\n";
    return 0;
}
