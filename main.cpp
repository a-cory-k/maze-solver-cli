#include <iostream>
#include <sstream>
#include <cassert>
struct Point {
    int x;
    int y;

    bool operator==(const Point &other) const {
        return x == other.x && y == other.y;
    }
};

Point operator+(const Point& a, const Point& b) {
    return Point{a.x + b.x, a.y + b.y};
}

namespace std {
    template<>
    struct hash<Point> {
        size_t operator()(const Point& p) const {
            size_t h1 = std::hash<int>()(p.x);
            size_t h2 = std::hash<int>()(p.y);
            return h1 ^ (h2 << 1);
        }
    };
}
struct allPathInfo {
    int color;
    Point point;
};
struct Result {
    std::unordered_map<Point, Point> parent;
    int distance;
    std::vector<allPathInfo> allPath;
};
struct DFSResult {
    
};
const std::vector<Point> directions = {
        {1, 0},
        {-1, 0},
        {0, 1},
        {0, -1}
};

struct InputData {
    std::vector<std::string> grid;
    Point start;
    Point end;
};

InputData read_input() {
    InputData data;
    std::string line;

    while (std::getline(std::cin, line)) {
        if (line.empty())
            break;
        data.grid.push_back(line);
    }

    if (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        std::string tmp;
        ss >> tmp >> data.start.x >> data.start.y;
    }

    if (std::getline(std::cin, line)) {
        std::stringstream ss(line);
        std::string tmp;
        ss >> tmp >> data.end.x >> data.end.y;
    }

    return data;
}

std::vector<Point> reconstruct_path(
        const std::unordered_map<Point, Point>& parent,
        Point start,
        Point end
) {
    std::vector<Point> path;

    if (start == end) {
        path.push_back(start);
        return path;
    }

    auto it = parent.find(end);
    if (it == parent.end()) {
        return {};
    }

    Point cur = end;
    path.push_back(cur);

    while (!(cur == start)) {
        auto p = parent.find(cur);
        if (p == parent.end()) return {};
        cur = p->second;
        path.push_back(cur);
    }

    std::reverse(path.begin(), path.end());
    return path;
}

void print_maze_with_bfs(
        std::vector<std::string> grid,
        const Result& res,
        Point start,
        Point end
) {

    for (const auto& step : res.allPath) {
        Point p = step.point;
        if (p.x < 0 || p.y < 0 || p.y >= (int)grid.size() || p.x >= (int)grid[0].size())
            continue;

        if (grid[p.y][p.x] == ' ') {
            grid[p.y][p.x] = '.';
        }
    }

    auto path = reconstruct_path(res.parent, start, end);
    for (const auto& p : path) {
        if (grid[p.y][p.x] == '.' || grid[p.y][p.x] == ' ') {
            grid[p.y][p.x] = '*';
        }
    }

    grid[start.y][start.x] = 'S';
    grid[end.y][end.x] = 'E';

    for (const auto& row : grid) {
        std::cout << row << "\n";
    }
}

Result bfs (const std::vector<std::string>& map, const Point & start, const Point & end) {
    std::queue<Point> q;
    q.push(start);
    unsigned rows = map.size();
    unsigned cols = map[0].size();

    std::unordered_map<Point, Point> parent;
    std::unordered_map<Point, int> distance;
    std::vector<allPathInfo> allPath;

    distance[start] = 0;
    allPath.push_back({0,start});

    while (!q.empty()) {
        Point current_pos = q.front();
        q.pop();
        if (current_pos == end) {
            break;
        }
        for (const auto  & direction:directions) {
            Point new_pos = current_pos + direction;

            if (new_pos.y < 0 || new_pos.x < 0
                || new_pos.x >= cols || new_pos.y >= rows
                || map[new_pos.y][new_pos.x] == 'X') {
                continue;
            }
            if (distance.find(new_pos) != distance.end()) {
                allPath.push_back({1, new_pos});
                continue;
            }

            allPath.push_back({0, new_pos});
            parent[new_pos] = current_pos;
            distance[new_pos] = distance[current_pos] + 1;
            q.push(new_pos);
        }
    }
    auto it = distance.find(end);
    int answer = (it == distance.end()) ? -1 : it->second;
    return {parent, answer, allPath};
}

Result dfs(const std::vector<std::string>& map,
           const Point& start,
           const Point& end)
{
    unsigned rows = map.size();
    unsigned cols = map[0].size();

    std::unordered_map<Point, Point> parent;
    std::unordered_map<Point, int> distance;
    std::vector<allPathInfo> allPath;

    std::function<bool(Point)> visit = [&](Point current_pos) {
        if (current_pos == end)
            return true;

        for (const auto& direction : directions) {
            Point new_pos = current_pos + direction;

            if (new_pos.y < 0 || new_pos.x < 0
                || new_pos.x >= cols || new_pos.y >= rows
                || map[new_pos.y][new_pos.x] == 'X')
                continue;

            if (distance.find(new_pos) != distance.end()) {
                allPath.push_back({1, new_pos});
                continue;
            }

            parent[new_pos] = current_pos;
            distance[new_pos] = distance[current_pos] + 1;
            allPath.push_back({0, new_pos});

            if (visit(new_pos))
                return true;
        }
        return false;
    };

    distance[start] = 0;
    allPath.push_back({0, start});
    visit(start);

    auto it = distance.find(end);
    int answer = (it == distance.end()) ? -1 : it->second;

    return {parent, answer, allPath};
}
std::vector<Point> random_search (std::string map) {

}
std::vector<Point> greedy_search (std::string map) {

}
std::vector<Point> heuristic (std::string map) {

}
int main() {
    const std::vector<std::string> first_input = {
            "XXXXXXXXXXXXX",
            "X           X",
            "X  XXXXXXX  X",
            "X     X     X",
            "X X X X XXX X",
            "X X         X",
            "XX  X XXXXX X",
            "X   X       X",
            "X X X XXX XXX",
            "X           X",
            "X XXX X   X X",
            "X           X",
            "XXXXXXXXXXXXX"
    };

    Point s{1, 7};
    Point e{5, 3};

    Result resBFS = bfs(first_input, s, e);
    assert(resBFS.distance == 8);

    print_maze_with_bfs(first_input, resBFS, s, e);

    std::cout << "dist = " << resBFS.distance << "\n";
    assert(resBFS.distance == 8);
    std::vector<std::string> first = first_input;

    //InputData input = read_input();


    printf("%d", resBFS.distance);

    return 0;
}
