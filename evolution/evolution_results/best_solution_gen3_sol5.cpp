priority_queue<State> pq;
pq.push({0, A, 0});
bestDist[makeKey(A,0)] = 0;

while (!pq.empty()) {
    State cur = pq.top(); pq.pop();
    uint64_t curKey = makeKey(cur.node, cur.len);

    // stale entry ?
    if (cur.dist != bestDist[curKey]) continue;

    if (cur.node == B) {                     // first time we pop B → optimal
        finishState = cur;
        break;
    }

    for (const Edge &e : g[cur.node]) {
        int nxt = e.to;
        int nlen = cur.len + 1;
        long long add = isPrime[nlen] ? 3LL * e.w2 : (long long)e.w1;
        long long ndist = cur.dist + add;
        uint64_t nxtKey = makeKey(nxt, nlen);

        auto it = bestDist.find(nxtKey);
        if (it == bestDist.end() || ndist < it->second) {
            bestDist[nxtKey] = ndist;
            parent[nxtKey] = {cur.node, cur.len};
            pq.push({ndist, nxt, nlen});
        }
    }
}
