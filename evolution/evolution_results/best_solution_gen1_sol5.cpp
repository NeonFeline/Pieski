#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int N, M;
    if (!(cin >> N >> M)) return 0;
    int A, B;
    cin >> A >> B;
    
    vector<vector<int>> adj(N);
    for (int i = 0; i < M; ++i) {
        int u, v, w1, w2;
        cin >> u >> v >> w1 >> w2;          // w1, w2 are ignored
        adj[u].push_back(v);
        adj[v].push_back(u);                // treat as undirected
    }
    
    vector<int> parent(N, -1);
    vector<char> visited(N, 0);
    queue<int> q;
    
    q.push(A);
    visited[A] = 1;
    
    while (!q.empty()) {
        int v = q.front(); q.pop();
        if (v == B) break;                  // destination reached
        for (int to : adj[v]) {
            if (!visited[to]) {
                visited[to] = 1;
                parent[to] = v;
                q.push(to);
            }
        }
    }
    
    // reconstruct path
    vector<int> path;
    for (int cur = B; cur != -1; cur = parent[cur])
        path.push_back(cur);
    reverse(path.begin(), path.end());
    
    cout << path.size() << '\n';
    for (size_t i = 0; i < path.size(); ++i) {
        if (i) cout << ' ';
        cout << path[i];
    }
    cout << '\n';
    return 0;
}
