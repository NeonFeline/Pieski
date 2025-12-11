/**
 * Good Path Validator
 * Reads Graph + Solution from STDIN
 * Outputs Score to STDOUT (0 if invalid)
 */
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

int main() {
    // Optimize I/O operations for speed
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    // --- 1. Read Graph ---
    int N, M;
    if (!(cin >> N >> M)) return 0;

    // Adjacency list to store the graph
    vector<vector<int>> adj(N);
    for (int i = 0; i < M; i++) {
        int u, v;
        cin >> u >> v;
        // Ensure nodes are within bounds before accessing vector
        if (u >= 0 && u < N && v >= 0 && v < N) {
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
    }

    // --- 2. Read Solution Path ---
    int K;
    if (!(cin >> K)) return 0;

    vector<int> path(K);
    for (int i = 0; i < K; i++) {
        cin >> path[i];
    }

    // Handle empty path case (valid, score 0)
    if (K == 0) {
        cout << 0 << endl;
        return 0;
    }

    // --- 3. Validate Path ---

    // Position map: pos[node] = index in path (0 to K-1), or -1 if not in path
    vector<int> pos(N, -1);

    // Step A: Check Range and Uniqueness
    for (int i = 0; i < K; i++) {
        int u = path[i];

        // Check 1: Node index bounds
        if (u < 0 || u >= N) {
            cout << 0 << endl; // Invalid node index
            return 0;
        }

        // Check 2: Duplicates
        if (pos[u] != -1) {
            cout << 0 << endl; // Duplicate node found
            return 0;
        }

        pos[u] = i;
    }

    // Step B: Check Connectivity and "Good Path" Property (No Chords)
    // We iterate every node in the path and check its neighbors in the graph.
    for (int i = 0; i < K; i++) {
        int u = path[i];

        // Track if we found connections to previous and next nodes
        bool connected_to_prev = (i == 0);      // First node has no prev
        bool connected_to_next = (i == K - 1);  // Last node has no next

        for (int v : adj[u]) {
            int neighbor_idx = pos[v];

            // If neighbor is NOT in the path, we don't care about it
            if (neighbor_idx == -1) continue;

            int diff = abs(i - neighbor_idx);

            // Check 3: No Chords (Non-consecutive nodes must NOT be connected)
            if (diff > 1) {
                cout << 0 << endl; // Invalid: Chord detected
                return 0;
            }

            // If neighbor is adjacent in path (diff == 1), mark as connected
            if (neighbor_idx == i - 1) connected_to_prev = true;
            if (neighbor_idx == i + 1) connected_to_next = true;
        }

        // Check 4: Consecutive nodes MUST be connected
        if (!connected_to_prev || !connected_to_next) {
            cout << 0 << endl; // Invalid: Missing edge between consecutive nodes
            return 0;
        }
    }

    // --- 4. Success ---
    // If we reach here, the path is valid. Output K.
    cout << K << endl;

    return 0;
}
