// key = (node << 20) | len   – 20 bits are enough because len ≤ 1e5 (< 2^17)
unordered_map<uint64_t, pair<int,int>> parent;   // (prevNode , prevLen)
unordered_map<uint64_t, long long>      bestDist; // minimal distance for the state
