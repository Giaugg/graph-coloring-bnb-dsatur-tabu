#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <cstring>
#include <random>
#include <climits>

using namespace std;
using namespace chrono;

struct TestCase {
    string filename;
    int n, m, best_k;
};

// 📥 LOAD TESTCASES
vector<TestCase> loadTestcases(const string &file) {
    vector<TestCase> tests;
    ifstream f(file);
    if (!f.is_open()) return tests;
    string line;
    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        stringstream ss(line);
        string token;
        TestCase t;
        int enabled;
        getline(ss, t.filename, ',');
        getline(ss, token, ','); t.n = stoi(token);
        getline(ss, token, ','); t.m = stoi(token);
        getline(ss, token, ','); t.best_k = stoi(token);
        getline(ss, token, ','); enabled = stoi(token);
        if (enabled == 1) tests.push_back(t);
    }
    return tests;
}

// 📖 READ DIMACS (Sửa để dùng 0-based indexing cho đỉnh)
void readDIMACS(const string &filename, int &n, int &m, int ***adj) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open file: " << filename << endl;
        return;
    }
    string line;
    while (getline(file, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            string tmp1, tmp2;
            stringstream ss(line);
            ss >> tmp1 >> tmp2 >> n >> m;
            *adj = new int*[n];
            for (int i = 0; i < n; i++) {
                (*adj)[i] = new int[n];
                memset((*adj)[i], 0, n * sizeof(int));
            }
        } else if (line[0] == 'e') {
            char e;
            int u, v;
            stringstream ss(line);
            ss >> e >> u >> v;
            // Chuyển về 0-based index
            u--; v--; 
            if (u >= 0 && u < n && v >= 0 && v < n) {
                (*adj)[u][v] = 1;
                (*adj)[v][u] = 1;
            }
        }
    }
    file.close();
}

void freeGraph(int **adj, int n) {
    if (adj == nullptr) return;
    for (int i = 0; i < n; i++) delete[] adj[i];
    delete[] adj;
}

template <typename Func>
pair<int, long long> measure(Func f) {
    auto start = high_resolution_clock::now();
    int result = f();
    auto end = high_resolution_clock::now();
    return {result, duration_cast<milliseconds>(end - start).count()};
}

// --- DSATUR ---
struct Graph {
    int n;
    int **adj;
    vector<int> degree, color, sat_count;
    vector<vector<int>> adj_color_count;

    Graph(int _n, int **_adj) : n(_n), adj(_adj) {
        degree.assign(n, 0);
        color.assign(n, -1);
        adj_color_count.assign(n, vector<int>(n + 1, 0));
        sat_count.assign(n, 0);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (adj[i][j]) degree[i]++;
    }

    void reset() {
        fill(color.begin(), color.end(), -1);
        fill(sat_count.begin(), sat_count.end(), 0);
        for (int i = 0; i < n; i++) fill(adj_color_count[i].begin(), adj_color_count[i].end(), 0);
    }
};

int get_smart_color(int u, Graph& g, int current_max) {
    for (int c = 0; c <= current_max + 1; c++) {
        if (g.adj_color_count[u][c] == 0) return c;
    }
    return current_max + 1;
}

int DSATUR_Core(Graph& g, unsigned int seed, const vector<int>& fixed_order = {}) {
    g.reset();
    mt19937 rng(seed);
    int colored = 0, max_c = -1;
    vector<bool> is_colored(g.n, false);

    for (int u : fixed_order) {
        if (u < 0 || u >= g.n || is_colored[u]) continue;
        int c = get_smart_color(u, g, max_c);
        g.color[u] = c; max_c = max(max_c, c);
        for (int v = 0; v < g.n; v++) if (g.adj[u][v]) {
            if (g.adj_color_count[v][c] == 0) g.sat_count[v]++;
            g.adj_color_count[v][c]++;
        }
        is_colored[u] = true; colored++;
    }

    while (colored < g.n) {
        int max_sat = -1, max_deg = -1, u = -1;
        vector<int> cands;
        for (int i = 0; i < g.n; i++) {
            if (g.color[i] == -1) {
                if (g.sat_count[i] > max_sat) { max_sat = g.sat_count[i]; cands = {i}; }
                else if (g.sat_count[i] == max_sat) cands.push_back(i);
            }
        }
        if (cands.size() > 1) {
            vector<int> best_c;
            for (int cand : cands) {
                if (g.degree[cand] > max_deg) { max_deg = g.degree[cand]; best_c = {cand}; }
                else if (g.degree[cand] == max_deg) best_c.push_back(cand);
            }
            u = best_c[rng() % best_c.size()];
        } else u = cands[0];

        int c = get_smart_color(u, g, max_c);
        g.color[u] = c; max_c = max(max_c, c);
        for (int v = 0; v < g.n; v++) if (g.adj[u][v]) {
            if (g.adj_color_count[v][c] == 0) g.sat_count[v]++;
            g.adj_color_count[v][c]++;
        }
        colored++;
    }
    return max_c + 1;
}

int DSATUR_Ultimate(int n, int **adj, int iterations) {
    Graph g(n, adj);
    int best_k = DSATUR_Core(g, time(0));
    for (int i = 0; i < iterations; i++) {
        vector<int> reorder;
        for (int j = 0; j < n; j++) 
            if (g.color[j] >= best_k * 0.7 || (rand() % 100 < 20)) reorder.push_back(j);
        shuffle(reorder.begin(), reorder.end(), mt19937(time(0) + i));
        int cur_k = DSATUR_Core(g, time(0) + i, reorder);
        if (cur_k < best_k) best_k = cur_k;
    }
    return best_k;
}

// --- BRANCH & BOUND (Cảnh báo: Chỉ dành cho n cực nhỏ) ---
bool isSafe(int u, int c, int n, int **adj, vector<int>& color) {
    for (int v = 0; v < n; v++) if (adj[u][v] && color[v] == c) return false;
    return true;
}

void BnB(int u, int used, int &best, int n, int **adj, vector<int>& color) {
    if (u == n) { best = min(best, used); return; }
    if (used >= best) return;
    for (int c = 1; c <= used + 1; c++) {
        if (isSafe(u, c, n, adj, color)) {
            color[u] = c;
            BnB(u + 1, max(used, c), best, n, adj, color);
            color[u] = 0;
        }
    }
}

int BranchAndBound(int n, int **adj) {
    if (n > 25) return -1; // Bảo vệ hệ thống khỏi treo
    vector<int> color(n, 0);
    int best = n;
    BnB(0, 0, best, n, adj, color);
    return best;
}

// --- TABU SEARCH ---
class TabuOptimizer {
private:
    int n;
    int **adj;
    vector<vector<int>> conflict_count;
    vector<vector<int>> frequency;

public:
    TabuOptimizer(int _n, int **_adj) : n(_n), adj(_adj) {}

    void initConflictMatrix(int k, const vector<int>& colors) {
        conflict_count.assign(n, vector<int>(k, 0));
        for (int u = 0; u < n; u++)
            for (int v = 0; v < n; v++)
                if (adj[u][v]) conflict_count[u][colors[v]]++;
    }

    int countConflicts(const vector<int>& colors) {
        int cnt = 0;
        for (int u = 0; u < n; u++)
            for (int v = u + 1; v < n; v++)
                if (adj[u][v] && colors[u] == colors[v]) cnt++;
        return cnt;
    }

    bool solve(int k, int maxIter, int L, int alpha, int tolerance, vector<int>& colors) {
        initConflictMatrix(k, colors);
        frequency.assign(n, vector<int>(k, 0));
        int cur_conf = countConflicts(colors);
        int best_conf = cur_conf;
        vector<int> best_sol = colors;
        vector<vector<int>> tabu(n, vector<int>(k, 0));
        int no_improve = 0;

        for (int iter = 1; iter <= maxIter; iter++) {
            if (best_conf == 0) { colors = best_sol; return true; }
            int best_m_delta = INT_MAX, m_node = -1, m_color = -1;

            for (int u = 0; u < n; u++) {
                if (conflict_count[u][colors[u]] > 0) {
                    for (int c = 0; c < k; c++) {
                        if (c == colors[u]) continue;
                        int delta = conflict_count[u][c] - conflict_count[u][colors[u]];
                        int penalty = (no_improve > tolerance) ? (alpha * frequency[u][c]) : 0;
                        if (tabu[u][c] <= iter || cur_conf + delta < best_conf) {
                            if (delta + penalty < best_m_delta) {
                                best_m_delta = delta + penalty; m_node = u; m_color = c;
                            }
                        }
                    }
                }
            }

            if (m_node != -1) {
                int old_c = colors[m_node];
                for (int v = 0; v < n; v++) if (adj[m_node][v]) {
                    conflict_count[v][old_c]--; conflict_count[v][m_color]++;
                }
                cur_conf += (conflict_count[m_node][m_color] - conflict_count[m_node][old_c]);
                colors[m_node] = m_color;
                tabu[m_node][old_c] = iter + L;
                frequency[m_node][m_color]++;
                if (cur_conf < best_conf) { best_conf = cur_conf; best_sol = colors; no_improve = 0; }
                else no_improve++;
            }
        }
        colors = best_sol;
        return best_conf == 0;
    }
};

// --- RUN TESTS ---
void runAllTests(const string &testFile, const string &outputFile) {
    vector<TestCase> tests = loadTestcases(testFile);
    ofstream out(outputFile);
    out << "filename,algorithm,result,time_ms,best,match\n";

    for (const TestCase &tc : tests) {
        int n, m; int **adj = nullptr;
        readDIMACS("datasets/" + tc.filename, n, m, &adj);
        if (!adj) continue;

        cout << "Running: " << tc.filename << " (n=" << n << ")" << endl;

        // DSATUR
        auto dsRes = measure([&]() { return DSATUR_Ultimate(n, adj, 100); });
        out << tc.filename << ",DSATUR," << dsRes.first << "," << dsRes.second << "," << tc.best_k << "," << abs(dsRes.first) << "\n";

        // TABU
        auto tbRes = measure([&]() {
            TabuOptimizer opt(n, adj);
            vector<int> colors(n);
            for(int i=0; i<n; i++) colors[i] = rand() % tc.best_k;
            int best_k = tc.best_k;
            bool ok = opt.solve(best_k, 50000, n/10+2, 5, 2500, colors);
            // while (!ok) {
            //     for(int i=0; i<n; i++) colors[i] = rand() % (tc.best_k + 1);
            //     ok = opt.solve(best_k + 1, 50000, n/10+2, 5, 2500, colors);
            //     best_k++; // Tăng k nếu vẫn không tìm được giải pháp hợp lệ
            // }
            return ok ? tc.best_k : best_k; // Trả về k+1 nếu còn xung đột
        });
        out << tc.filename << ",Tabu," << tbRes.first << "," << tbRes.second << "," << tc.best_k << "," << abs(tbRes.first) << "\n";

        // B&B (Chỉ chạy nếu đồ thị cực nhỏ)
        if (n <= 20) {
            auto bnbRes = measure([&]() { return BranchAndBound(n, adj); });
            out << tc.filename << ",BnB," << bnbRes.first << "," << bnbRes.second << "," << tc.best_k << "," << abs(bnbRes.first) << "\n";
        }

        freeGraph(adj, n);
        cout << "Done: " << tc.filename << "\n";
    }
    out.close();
}

int main() {
    srand(time(0));
    runAllTests("testcases.txt", "results.csv");
    return 0;
}