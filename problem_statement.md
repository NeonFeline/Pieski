# Problem Statement

A **good path** \( P \) in an undirected graph \( G \) is a sequence of distinct nodes  
\( (v_0, v_1, \ldots, v_k) \) such that for any \( i, j \) with \( |i - j| \neq 1 \), there is **no** edge \( v_i \to v_j \) in graph \( G \).

Given an undirected graph \( G \), find a good path \( P \) that contains as many nodes as possible.

If the found path is correct, your program will obtain points equal to the number of nodes on the found path.

---

# Input Format

- First line contains two integers **N**, **M**, denoting number of nodes and edges in the graph.  
- Each of the next **M** lines contains two integers **u, v**, denoting the endpoints of an edge.

---

# Output Format

- First line: integer **k** — the number of nodes on the path.  
- Second line: **k** integers — the IDs of nodes on the found path.

---

# Constraints

- \( 1 \leq N \leq 10^6 \)  
- \( 1 \leq M \leq 10^6 \)  
- Time limit: **20 seconds per test case**  
- Submission frequency: **3 minutes**

---

# Example Input
```
5 7
0 1
0 4
1 2
1 3
1 4
2 3
3 4
```
---

# Example Output
```
4
2 3 4 0
```

---

# Example Score
```
4
```
