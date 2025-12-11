/**
 * Validator for the Good Path Problem (C++ Port)
 *
 * Checks if a given solution represents a valid good path:
 * 1. All nodes in the path are distinct
 * 2. Consecutive nodes in the path are connected by edges
 * 3. Non-consecutive nodes in the path have NO edges between them
 *
 * Compilation:
 * g++ -O3 -o validator validator.cpp
 *
 * Usage:
 * ./validator <input_file> <output_file> [time_file]
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <iomanip>
#include <cmath>

struct Graph {
    int n, m;
    std::vector<std::unordered_set<int>> adj;
};

struct PathResult {
    int k;
    std::vector<int> path;
};

// Read graph from input file
Graph read_input(const std::string& input_file) {
    std::ifstream infile(input_file);
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open input file: " + input_file);
    }

    int n, m;
    if (!(infile >> n >> m)) {
        throw std::runtime_error("Error reading N and M from input file");
    }

    std::vector<std::unordered_set<int>> adj(n);
    for (int i = 0; i < m; ++i) {
        int u, v;
        infile >> u >> v;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            adj[u].insert(v);
            adj[v].insert(u);
        }
    }

    return {n, m, adj};
}

// Read path from output file
PathResult read_output(const std::string& output_file) {
    std::ifstream infile(output_file);
    if (!infile.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_file);
    }

    int k;
    if (!(infile >> k)) {
        throw std::runtime_error("Error reading K from output file");
    }

    std::vector<int> path;
    if (k > 0) {
        path.resize(k);
        for (int i = 0; i < k; ++i) {
            infile >> path[i];
        }
    }

    return {k, path};
}

struct ValidationResult {
    bool is_valid;
    std::string message;
    int score;
};

ValidationResult validate_path(const Graph& graph, int k, const std::vector<int>& path) {
    // Check if k matches path length
    if (k != (int)path.size()) {
        return {false, "Path length mismatch: k=" + std::to_string(k) +
            " but path has " + std::to_string(path.size()) + " nodes", 0};
    }

    // Empty path is valid
    if (k == 0) {
        return {true, "Valid empty path", 0};
    }

    // 1. Check valid range and prepare for uniqueness check
    // We use a vector to store the position of each node in the path.
    // -1 indicates the node is not in the path.
    std::vector<int> pos_in_path(graph.n, -1);

    for (int i = 0; i < k; ++i) {
        int node = path[i];

        // Check range
        if (node < 0 || node >= graph.n) {
            return {false, "Invalid node " + std::to_string(node) +
                " at position " + std::to_string(i) +
                " (must be in range [0, " + std::to_string(graph.n - 1) + "])", 0};
        }

        // Check duplicates
        if (pos_in_path[node] != -1) {
            return {false, "Path contains duplicate node: " + std::to_string(node), 0};
        }
        pos_in_path[node] = i;
    }

    // 2. Check consecutive nodes are connected
    for (int i = 0; i < k - 1; ++i) {
        int u = path[i];
        int v = path[i + 1];
        if (graph.adj[u].find(v) == graph.adj[u].end()) {
            return {false, "No edge between consecutive nodes " + std::to_string(u) +
                " and " + std::to_string(v) +
                " at positions " + std::to_string(i) + " and " + std::to_string(i + 1), 0};
        }
    }

    // 3. Check non-consecutive nodes have NO edges between them
    // We iterate through every node in the path and check its neighbors.
    // If a neighbor is also in the path, it MUST be the immediate previous or next node.
    for (int i = 0; i < k; ++i) {
        int u = path[i];

        for (int neighbor : graph.adj[u]) {
            // Check if neighbor is part of the solution path
            int neighbor_pos = pos_in_path[neighbor];

            if (neighbor_pos != -1) {
                // If neighbor is in path, the position difference must be exactly 1
                if (std::abs(i - neighbor_pos) > 1) {
                    return {false, "Edge exists between non-consecutive nodes " + std::to_string(u) +
                        " (pos " + std::to_string(i) + ") and " + std::to_string(neighbor) +
                        " (pos " + std::to_string(neighbor_pos) + ")", 0};
                }
            }
        }
    }

    return {true, "Valid good path with " + std::to_string(k) + " nodes", k};
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> [time_file]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    std::string time_file = (argc == 4) ? argv[3] : "";

    try {
        Graph graph = read_input(input_file);
        PathResult res = read_output(output_file);

        ValidationResult val = validate_path(graph, res.k, res.path);

        // Read execution time if provided
        double elapsed_time = -1.0;
        if (!time_file.empty()) {
            std::ifstream tf(time_file);
            if (tf.is_open()) {
                tf >> elapsed_time;
            }
        }

        // Print result
        std::cout << "Graph: N=" << graph.n << ", M=" << graph.m << std::endl;
        std::cout << "Score: " << val.score << std::endl;
        if (elapsed_time >= 0) {
            std::cout << std::fixed << std::setprecision(4) << "Time: " << elapsed_time << "s" << std::endl;
        }

        if (val.is_valid) {
            std::cout << "Status: " << "\u2713" << " VALID" << std::endl; // Checkmark
            return 0;
        } else {
            std::cerr << "Status: " << "\u2717" << " INVALID - " << val.message << std::endl; // Cross
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
