# Maze Solver CLI
---
A C++ tool for finding the shortest paths in a maze using various search algorithms and step-by-step visualization in the terminal.

### Implemented Algorithms
BFS (Breadth-First Search) - guarantees the shortest path in an unweighted graph.

DFS (Depth-First Search) - depth-first search (can find suboptimal paths).

*A (A-Star)** - efficient search using heuristics (Chebyshev/Euclidean distance).

Greedy Best-First Search - uses only the distance to the goal.

Random Search - an experimental algorithm that randomly selects the next node.

---

### Map Legend
The following notations are used during the animation:

S - Starting point.

E - Exit point (End).

X - Wall (impassable area).

o - Open node (queued for examination).

c - Closed node (already explored).

r - Rejected node (in A* — a longer path was found).

\* - Final path.

---

### Input data format
```
XXXXXXXXXXXXX
X           X
X  XXXXXXX  X
X     X     X
X X X X XXX X
X X         X
XX  X XXXXX X
X   X       X
X X X XXX XXX
X           X
X XXX X   X X
X           X
XXXXXXXXXXXXX
start 1, 7
end 5, 3
```

---

### Building and Running

https://github.com/user-attachments/assets/b0722d4e-6fe1-465f-8e2b-dc81c9166bad


Compile the project with any modern C++ compiler (supporting C++17 or higher):

```
g++ -std=c++17 main.cpp -o maze_solver
Running with algorithm selection (astar is used by default):
```

```
./maze_solver astar < map.txt

./maze_solver bfs < map.txt

```

