struct State {
    long long dist;   // total Blackie‑length up to this state
    int       node;   // current vertex
    int       len;    // number of edges already taken
    bool operator<(State const& o) const { return dist > o.dist; } // for min‑heap
};
