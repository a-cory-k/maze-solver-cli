#include <iostream>
#include <sstream>
#include <cassert>
#include <random>
#include <math.h>
#include <unordered_set>
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

static int rankMark(char ch) {
    switch (ch) {
        case ' ': return 0;
        case 'o': return 1;
        case 'r': return 2;
        case 'c': return 3;
        case '*': return 4;
        case 'S':
        case 'E': return 5;
        case 'X': return 6;
        default:  return 0;
    }
}

static char markFromColor(int color) {
    if (color == 0) return 'o'; // open
    if (color == 1) return 'c'; // closed
    return 'r';                 // rejected
}

void print_maze(
        std::vector<std::string> grid,
        const Result& res,
        Point start,
        Point end
) {
    for (const auto& step : res.allPath) {
        Point p = step.point;

        if (p.x < 0 || p.y < 0 ||
            p.y >= (int)grid.size() ||
            p.x >= (int)grid[0].size())
            continue;

        if (grid[p.y][p.x] == 'X') continue;
        if (p == start || p == end) continue;

        char newMark = markFromColor(step.color);
        char &cell = grid[p.y][p.x];

        if (rankMark(newMark) > rankMark(cell))
            cell = newMark;
    }

    auto path = reconstruct_path(res.parent, start, end);
    for (const auto& p : path) {
        if (p == start || p == end) continue;
        grid[p.y][p.x] = '*';
    }

    grid[start.y][start.x] = 'S';
    grid[end.y][end.x] = 'E';

    for (const auto& row : grid)
        std::cout << row << "\n";
}

Result bfs (const std::vector<std::string>& map,
            const Point & start,
            const Point & end) {
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

Result dfs (const std::vector<std::string>& map,
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

Result randomSearch (const std::vector<std::string>& map,
                    const Point& start,
                    const Point& end)
{
    const int rows = static_cast<int>(map.size());
    const int cols = rows ? static_cast<int>(map[0].size()) : 0;

    std::unordered_map<Point, Point> parent;
    std::unordered_map<Point, int> distance;
    std::vector<allPathInfo> allPath;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<Point> open;
    open.push_back(start);

    distance[start] = 0;
    allPath.push_back({0, start});

    while (!open.empty()) {
        std::uniform_int_distribution<int> pick(0, static_cast<int>(open.size()) - 1);
        int idx = pick(gen);

        Point current = open[idx];
        open[idx] = open.back();
        open.pop_back();

        if (current == end) break;

        for (const auto& direction : directions) {
            Point next = current + direction;

            if (next.y < 0 || next.x < 0
                || next.x >= cols || next.y >= rows
                || map[next.y][next.x] == 'X')
                continue;

            if (distance.find(next) != distance.end()) {
                allPath.push_back({1, next});
                continue;
            }

            parent[next] = current;
            distance[next] = distance[current] + 1;
            allPath.push_back({0, next});

            open.push_back(next);
        }
    }

    auto it = distance.find(end);
    int answer = (it == distance.end()) ? -1 : it->second;
    return {parent, answer, allPath};
}

Result greedySearch (const std::vector<std::string>& map,
                    const Point& start,
                    const Point& end)
{
    unsigned rows = map.size();
    unsigned cols = map[0].size();

    auto heuristic = [&](const Point& p) {
        return sqrt(pow(p.x - end.x,2) + pow(p.y - end.y,2));
    };

    auto cmp = [&](const Point& a, const Point& b) {
        return heuristic(a) > heuristic(b);
    };

    std::priority_queue<Point, std::vector<Point>, decltype(cmp)> pq(cmp);

    std::unordered_map<Point, Point> parent;
    std::unordered_map<Point, int> distance;
    std::vector<allPathInfo> allPath;

    pq.push(start);
    distance[start] = 0;
    allPath.push_back({0, start});

    while (!pq.empty()) {
        Point current = pq.top();
        pq.pop();

        if (current == end)
            break;

        for (const auto& direction : directions) {
            Point next = current + direction;

            if (next.y < 0 || next.x < 0
                || next.x >= cols || next.y >= rows
                || map[next.y][next.x] == 'X')
                continue;

            if (distance.find(next) != distance.end()) {
                allPath.push_back({1, next});
                continue;
            }

            parent[next] = current;
            distance[next] = distance[current] + 1;
            allPath.push_back({0, next});
            pq.push(next);
        }
    }

    auto it = distance.find(end);
    int answer = (it == distance.end()) ? -1 : it->second;

    return {parent, answer, allPath};
}

Result searchA (const std::vector<std::string>& map,
               const Point& start,
               const Point& end)
{
    const int rows = (int)map.size();
    const int cols = rows ? (int)map[0].size() : 0;

    auto heuristic = [&](const Point& p) {
        return sqrt(pow(p.x - end.x,2) + pow(p.y - end.y,2));
    };

    struct Node {
        Point p;
        double f;
    };

    struct Cmp {
        bool operator()(const Node& a, const Node& b) const {
            return a.f > b.f;
        }
    };

    std::priority_queue<Node, std::vector<Node>, Cmp> open;

    std::unordered_map<Point, Point> parent;
    std::unordered_map<Point, int> gScore;
    std::unordered_set<Point> closed;

    std::vector<allPathInfo> allPath;

    gScore[start] = 0;
    open.push({start, heuristic(start)});
    allPath.push_back({0, start}); // discovered

    while (!open.empty()) {
        Node node = open.top();
        open.pop();

        Point current = node.p;

        if (closed.count(current)) continue;

        closed.insert(current);
        allPath.push_back({1, current}); // closed

        if (current == end)
            break;

        for (const auto& d : directions) {
            Point next = current + d;

            if (next.y < 0 || next.x < 0 ||
                next.x >= cols || next.y >= rows ||
                map[next.y][next.x] == 'X')
                continue;

            if (closed.count(next)) continue;

            int tentative = gScore[current] + 1;

            auto it = gScore.find(next);

            if (it == gScore.end() || tentative < it->second) {
                parent[next] = current;
                gScore[next] = tentative;

                open.push({next, tentative + heuristic(next)});
                allPath.push_back({0, next}); // added to open
            } else {
                allPath.push_back({2, next}); // rejected
            }
        }
    }

    int answer = gScore.count(end) ? gScore[end] : -1;
    return {parent, answer, allPath};
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

    Result resBFS = searchA (first_input, s, e);
    //assert(resBFS.distance == 8);

    print_maze(first_input, resBFS, s, e);

    std::cout << "dist = " << resBFS.distance << "\n";
    std::vector<std::string> first = first_input;

    //InputData input = read_input();


    printf("%d", resBFS.distance);

    return 0;
}
