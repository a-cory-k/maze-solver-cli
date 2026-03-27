#include <iostream>
#include <sstream>
#include <random>
#include <cmath>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <memory>
#include <string>

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
    int color;   // 0=open/discovered, 1=closed/expanded OR "already visited" in BFS/DFS , 2=rejected (A*)
    Point point;
};

struct Result {
    std::unordered_map<Point, Point> parent;
    int distance;
    std::vector<allPathInfo> allPath;
};

static const std::vector<Point> directions = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
};

struct InputData {
    std::vector<std::string> grid;
    Point start{};
    Point end{};
};

InputData read_input() {
    InputData data;
    std::string line;

    bool gotStart = false, gotEnd = false;

    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string s = line;
        std::replace(s.begin(), s.end(), ',', ' ');
        std::stringstream ss(s);
        std::string tag;
        ss >> tag;

        if (tag == "start") {
            if (!(ss >> data.start.x >> data.start.y))
                throw std::runtime_error("Bad start line: " + line);
            gotStart = true;
            continue;
        }
        if (tag == "end") {
            if (!(ss >> data.end.x >> data.end.y))
                throw std::runtime_error("Bad end line: " + line);
            gotEnd = true;
            continue;
        }

        data.grid.push_back(line);
    }

    if (!gotStart) throw std::runtime_error("Missing start line");
    if (!gotEnd)   throw std::runtime_error("Missing end line");
    if (data.grid.empty()) throw std::runtime_error("Empty grid");

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
    if (it == parent.end()) return {};

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

class Maze {
public:
    explicit Maze(std::vector<std::string> grid)
            : m_grid(std::move(grid))
    {
        if (m_grid.empty()) throw std::runtime_error("Maze: empty grid");
        m_rows = (int)m_grid.size();
        m_cols = (int)m_grid[0].size();
        for (const auto& row : m_grid) {
            if ((int)row.size() != m_cols)
                throw std::runtime_error("Maze: non-rectangular grid");
        }
    }

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }

    bool inBounds(Point p) const {
        return p.x >= 0 && p.y >= 0 && p.x < m_cols && p.y < m_rows;
    }

    bool passable(Point p) const {
        return inBounds(p) && m_grid[p.y][p.x] != 'X';
    }

    const std::vector<std::string>& grid() const { return m_grid; }

private:
    std::vector<std::string> m_grid;
    int m_rows = 0;
    int m_cols = 0;
};

struct ISearch {
    virtual ~ISearch() = default;
    virtual Result run(const Maze& maze, Point start, Point end) = 0;
    virtual const char* name() const = 0;
};

class BfsSearch : public ISearch {
public:
    const char* name() const override { return "bfs"; }

    Result run(const Maze& maze, Point start, Point end) override {
        std::queue<Point> q;
        q.push(start);

        std::unordered_map<Point, Point> parent;
        std::unordered_map<Point, int> distance;
        std::vector<allPathInfo> allPath;

        distance[start] = 0;
        allPath.push_back({0, start});

        while (!q.empty()) {
            Point current_pos = q.front();
            q.pop();

            if (current_pos == end) break;

            for (const auto & direction : directions) {
                Point new_pos = current_pos + direction;

                if (!maze.passable(new_pos)) continue;

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
};

class DfsSearch : public ISearch {
public:
    const char* name() const override { return "dfs"; }

    Result run(const Maze& maze, Point start, Point end) override {
        std::unordered_map<Point, Point> parent;
        std::unordered_map<Point, int> distance;
        std::vector<allPathInfo> allPath;

        std::function<bool(Point)> visit = [&](Point current_pos) {
            if (current_pos == end) return true;

            for (const auto& direction : directions) {
                Point new_pos = current_pos + direction;

                if (!maze.passable(new_pos)) continue;

                if (distance.find(new_pos) != distance.end()) {
                    allPath.push_back({1, new_pos});
                    continue;
                }

                parent[new_pos] = current_pos;
                distance[new_pos] = distance[current_pos] + 1;
                allPath.push_back({0, new_pos});

                if (visit(new_pos)) return true;
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
};

class RandomSearchAlgo : public ISearch {
public:
    const char* name() const override { return "random"; }

    Result run(const Maze& maze, Point start, Point end) override {
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
            std::uniform_int_distribution<int> pick(0, (int)open.size() - 1);
            int idx = pick(gen);

            Point current = open[idx];
            open[idx] = open.back();
            open.pop_back();

            if (current == end) break;

            for (const auto& direction : directions) {
                Point next = current + direction;

                if (!maze.passable(next)) continue;

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
};

class GreedySearch : public ISearch {
public:
    const char* name() const override { return "greedy"; }

    Result run(const Maze& maze, Point start, Point end) override {
        auto heuristic = [&](const Point& p) -> double {
            double dx = double(p.x - end.x);
            double dy = double(p.y - end.y);
            return std::sqrt(dx*dx + dy*dy);
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

            if (current == end) break;

            for (const auto& direction : directions) {
                Point next = current + direction;

                if (!maze.passable(next)) continue;

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
};

class AStarSearch : public ISearch {
public:
    const char* name() const override { return "astar"; }

    Result run(const Maze& maze, Point start, Point end) override {
        auto heuristic = [&](const Point& p) -> double {
            double dx = double(p.x - end.x);
            double dy = double(p.y - end.y);
            return std::sqrt(dx*dx + dy*dy);
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
        allPath.push_back({0, start}); // discovered/open

        while (!open.empty()) {
            Node node = open.top();
            open.pop();

            Point current = node.p;

            if (closed.count(current)) continue;

            closed.insert(current);
            allPath.push_back({1, current}); // closed/expanded

            if (current == end) break;

            for (const auto& d : directions) {
                Point next = current + d;

                if (!maze.passable(next)) continue;
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
};

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

static void clear_screen() {
    std::cout << "\x1b[2J\x1b[H";
}

static void sleep_ms(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

static void print_viewport(
        const std::vector<std::string>& grid,
        Point focus,
        int viewW,
        int viewH
) {
    int rows = (int)grid.size();
    int cols = rows ? (int)grid[0].size() : 0;

    int x0 = focus.x - viewW / 2;
    int y0 = focus.y - viewH / 2;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x0 + viewW > cols) x0 = std::max(0, cols - viewW);
    if (y0 + viewH > rows) y0 = std::max(0, rows - viewH);

    int x1 = std::min(cols, x0 + viewW);
    int y1 = std::min(rows, y0 + viewH);

    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) std::cout << grid[y][x];
        std::cout << "\n";
    }
    std::cout << "viewport x:[" << x0 << "," << (x1-1) << "]  y:[" << y0 << "," << (y1-1) << "]\n";
}

class Animator {
public:
    void printAnimated(
            const Maze& maze,
            const Result& res,
            Point start,
            Point end,
            int delayMs = 150,
            bool show_final_path_each_frame = false
    ) const {
        const auto& original_grid = maze.grid();
        std::vector<std::string> grid;

        auto finalPath = reconstruct_path(res.parent, start, end);

        for (size_t k = 0; k <= res.allPath.size(); ++k) {
            grid = original_grid;

            for (size_t i = 0; i < k; ++i) {
                const auto& step = res.allPath[i];
                Point p = step.point;

                if (p.y < 0 || p.y >= (int)grid.size()) continue;
                if (p.x < 0 || p.x >= (int)grid[0].size()) continue;

                if (grid[p.y][p.x] == 'X') continue;
                if (p == start || p == end) continue;

                char newMark = markFromColor(step.color);
                char &cell = grid[p.y][p.x];
                if (rankMark(newMark) > rankMark(cell))
                    cell = newMark;
            }

            if (show_final_path_each_frame) {
                for (const auto& p : finalPath) {
                    if (p == start || p == end) continue;
                    grid[p.y][p.x] = '*';
                }
            }

            grid[start.y][start.x] = 'S';
            grid[end.y][end.x] = 'E';

            clear_screen();
            Point focus = start;
            if (k > 0 && k-1 < res.allPath.size()) focus = res.allPath[k-1].point;

            print_viewport(grid, focus, 80, 22);
            std::cout << "step " << k << "/" << res.allPath.size()
                      << "   dist=" << res.distance << "\n";
            std::cout.flush();

            sleep_ms(delayMs);
        }

        if (!finalPath.empty()) {
            grid = original_grid;

            for (const auto& step : res.allPath) {
                Point p = step.point;
                if (grid[p.y][p.x] == 'X') continue;
                if (p == start || p == end) continue;

                char newMark = markFromColor(step.color);
                char &cell = grid[p.y][p.x];
                if (rankMark(newMark) > rankMark(cell))
                    cell = newMark;
            }

            for (const auto& p : finalPath) {
                if (p == start || p == end) continue;
                grid[p.y][p.x] = '*';
            }

            grid[start.y][start.x] = 'S';
            grid[end.y][end.x] = 'E';

            clear_screen();
            for (const auto& row : grid) std::cout << row << "\n";
            int openCount = 0;
            for (const auto& step : res.allPath) {
                if (step.color == 0) openCount++;
            }

            std::cout << "DONE   dist=" << res.distance
                      << "   opened_nodes=" << openCount << "\n";
            print_viewport(grid, end, 80, 22);
        }
    }
};

static std::unique_ptr<ISearch> makeSearch(const std::string& name) {
    if (name == "bfs")    return std::make_unique<BfsSearch>();
    if (name == "dfs")    return std::make_unique<DfsSearch>();
    if (name == "random") return std::make_unique<RandomSearchAlgo>();
    if (name == "greedy") return std::make_unique<GreedySearch>();
    if (name == "astar")  return std::make_unique<AStarSearch>();

    throw std::runtime_error("Unknown algorithm: " + name +
                             " (use: bfs, dfs, random, greedy, astar)");
}

int main(int argc, char** argv) {
    try {
        std::string algoName = (argc >= 2) ? std::string(argv[1]) : "astar";

        InputData input = read_input();
        Maze maze(input.grid);

        auto algo = makeSearch(algoName);
        Result res = algo->run(maze, input.start, input.end);

        Animator anim;
        anim.printAnimated(maze, res, input.start, input.end);

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}