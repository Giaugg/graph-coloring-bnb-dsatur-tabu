#include <bits/stdc++.h>
#include <chrono>

using namespace std;
using namespace chrono;

struct TestCase {
    string filename;
    int n, m, best_k;
};

//////////////////////////////////////////////////
// LOAD TESTCASES
//////////////////////////////////////////////////
vector<TestCase> loadTestcases(const string &file) {
    vector<TestCase> tests;
    ifstream f(file);
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

        if (enabled == 1)
            tests.push_back(t);
    }

    return tests;
}

//////////////////////////////////////////////////
// READ DIMACS
//////////////////////////////////////////////////
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

            *adj = new int*[n + 1];
            for (int i = 0; i <= n; i++) {
                (*adj)[i] = new int[n + 1];
                memset((*adj)[i], 0, (n + 1) * sizeof(int));
            }
        }

        else if (line[0] == 'e') {
            char e;
            int u, v;
            stringstream ss(line);
            ss >> e >> u >> v;

            (*adj)[u][v] = 1;
            (*adj)[v][u] = 1;
        }
    }

    file.close();
}

//////////////////////////////////////////////////
// FREE MEMORY
//////////////////////////////////////////////////
void freeGraph(int **adj, int n) {
    for (int i = 0; i <= n; i++) delete[] adj[i];
    delete[] adj;
}

//////////////////////////////////////////////////
// TIMER
//////////////////////////////////////////////////
template <typename Func>
pair<int, long long> measure(Func f) {
    auto start = high_resolution_clock::now();
    int result = f();
    auto end = high_resolution_clock::now();
    long long time = duration_cast<milliseconds>(end - start).count();
    return {result, time};
}

//////////////////////////////////////////////////
// DSATUR
//////////////////////////////////////////////////
// Hàm chọn màu thông minh hơn: Least Constraining Color
int find_smart_color(int u, int n, int** adj, const vector<int>& color, const vector<vector<bool>>& saturation_colors) {
    int best_color = -1;
    double max_freedom = -1.0;
    
    // Giới hạn số màu thử nghiệm để đảm bảo tốc độ (thường không quá số màu hiện tại + 1)
    int max_used_color = 0;
    for (int c : color) if (c > max_used_color) max_used_color = c;
    int search_limit = max_used_color + 2;

    for (int c = 0; c < search_limit; c++) {
        // Nếu màu c không bị xung đột với láng giềng của u
        if (!saturation_colors[u][c]) {
            double current_freedom = 0;
            
            // Đánh giá "không gian thở" cho láng giềng
            for (int v = 0; v < n; v++) {
                if (adj[u][v] && color[v] == -1) {
                    // Nếu v chưa có màu c, việc chọn c cho u sẽ làm v mất đi 1 lựa chọn
                    // Ta ưu tiên màu nào mà láng giềng của nó đã có màu đó rồi (không làm mất thêm lựa chọn)
                    if (saturation_colors[v][c]) {
                        current_freedom += 1.0; 
                    } else {
                        // Tính toán trọng số dựa trên bậc: láng giềng bậc càng cao càng cần được ưu tiên lựa chọn
                        // current_freedom += 0.0; // Không cộng gì nếu lấy đi 1 lựa chọn của v
                    }
                }
            }

            if (current_freedom > max_freedom) {
                max_freedom = current_freedom;
                best_color = c;
            }
        }
    }
    return (best_color == -1) ? max_used_color + 1 : best_color;
}
int DSATUR_SmartColor(int n, int **adj, unsigned int seed) {
    vector<int> color(n, -1);
    vector<vector<bool>> saturation_colors(n, vector<bool>(n, false));
    vector<int> sat_count(n, 0);
    vector<int> degree(n, 0);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (adj[i][j]) degree[i]++;
        }
    }

    mt19937 g(seed);
    int colored_count = 0;

    while (colored_count < n) {
        // --- BƯỚC 1 & 2: CHỌN ĐỈNH (GIỮ NGUYÊN NHƯ CODE CŨ) ---
        int max_sat = -1;
        vector<int> candidates;
        for (int i = 0; i < n; i++) {
            if (color[i] == -1) {
                if (sat_count[i] > max_sat) {
                    max_sat = sat_count[i];
                    candidates.clear();
                    candidates.push_back(i);
                } else if (sat_count[i] == max_sat) {
                    candidates.push_back(i);
                }
            }
        }

        int u = -1;
        if (candidates.size() == 1) {
            u = candidates[0];
        } else {
            int max_deg = -1;
            vector<int> best_candidates;
            for (int cand : candidates) {
                if (degree[cand] > max_deg) {
                    max_deg = degree[cand];
                    best_candidates.clear();
                    best_candidates.push_back(cand);
                } else if (degree[cand] == max_deg) {
                    best_candidates.push_back(cand);
                }
            }
            uniform_int_distribution<int> dist(0, (int)best_candidates.size() - 1);
            u = best_candidates[dist(g)];
        }

        // --- BƯỚC 3: CHỌN MÀU (THAY ĐỔI Ở ĐÂY) ---
        // Thay find_first_absent bằng hàm chọn màu thông minh
        int c = find_smart_color(u, n, adj, color, saturation_colors);
        
        color[u] = c;
        colored_count++;

        // --- BƯỚC 4: CẬP NHẬT LÁNG GIỀNG (GIỮ NGUYÊN) ---
        for (int v = 0; v < n; v++) {
            if (adj[u][v] && color[v] == -1) {
                if (!saturation_colors[v][c]) {
                    saturation_colors[v][c] = true;
                    sat_count[v]++;
                }
            }
        }
    }

    int max_c = 0;
    for (int i = 0; i < n; i++) max_c = max(max_c, color[i]);
    return max_c + 1;
}

//////////////////////////////////////////////////
// BRANCH & BOUND
//////////////////////////////////////////////////
bool isSafe(int u, int c, int n, int **adj, int color[]) {
    for (int v = 1; v <= n; v++)
        if (adj[u][v] && color[v] == c)
            return false;
    return true;
}

void BnB(int u, int usedColors, int &best, int n, int **adj, int color[]) {
    if (u > n) {
        best = min(best, usedColors);
        return;
    }

    if (usedColors >= best) return;

    for (int c = 1; c <= usedColors + 1; c++) {
        if (isSafe(u, c, n, adj, color)) {
            color[u] = c;
            BnB(u + 1, max(usedColors, c), best, n, adj, color);
            color[u] = 0;
        }
    }
}

int BranchAndBound(int n, int **adj) {
    int color[1000] = {0};
    int best = INT_MAX;
    BnB(1, 0, best, n, adj, color);
    return best;
}

//////////////////////////////////////////////////
// TABU SEARCH (simple)
//////////////////////////////////////////////////
int TabuSearch(int n, int **adj, int maxIter, int k) {
    vector<int> color(n+1);
    srand(time(0));

    for (int i = 1; i <= n; i++)
        color[i] = rand() % k + 1;

    for (int iter = 0; iter < maxIter; iter++) {
        int conflict = 0;

        for (int i = 1; i <= n; i++)
            for (int j = i+1; j <= n; j++)
                if (adj[i][j] && color[i] == color[j])
                    conflict++;

        if (conflict == 0) return k;

        int u = rand() % n + 1;
        color[u] = rand() % k + 1;
    }

    return k;
}

//////////////////////////////////////////////////
// RUN ALL TESTS
//////////////////////////////////////////////////
void runAllTests(const string &testFile, const string &outputFile) {
    vector<TestCase> tests = loadTestcases(testFile);

    ofstream out(outputFile);
    out << "filename,algorithm,result,time_ms,best,match\n";

    for (const TestCase &tc : tests) {
        int n, m;
        int **adj = nullptr;

        // 📥 Load graph
        string path = "datasets/" + tc.filename;
        readDIMACS(path, n, m, &adj);

        cout << "Running: " << tc.filename << endl;

        // ===================== DSATUR =====================
        pair<int, long long> dsaturRes = measure([&]() {
            return DSATUR_SmartColor(n, adj, 100);
        });

        int dsaturColor = dsaturRes.first;
        long long dsaturTime = dsaturRes.second;

        out << tc.filename << ",DSATUR,"
            << dsaturColor << "," << dsaturTime << ","
            << tc.best_k << ","
            << ((dsaturColor - tc.best_k) > 0 ? dsaturColor - tc.best_k : tc.best_k - dsaturColor) << "\n";


        // ===================== TABU =====================
        pair<int, long long> tabuRes = measure([&]() {
            return TabuSearch(n, adj, 1000, tc.best_k);
        });

        int tabuColor = tabuRes.first;
        long long tabuTime = tabuRes.second;

        out << tc.filename << ",Tabu,"
            << tabuColor << "," << tabuTime << ","
            << tc.best_k << ","
            << ((tabuColor - tc.best_k) > 0 ? tabuColor - tc.best_k : tc.best_k - tabuColor) << "\n";


        // ===================== BRANCH & BOUND =====================
        if (n <= 100) {
            pair<int, long long> bnbRes = measure([&]() {
                return BranchAndBound(n, adj);
            });

            int bnbColor = bnbRes.first;
            long long bnbTime = bnbRes.second;

            out << tc.filename << ",BnB,"
                << bnbColor << "," << bnbTime << ","
                << tc.best_k << ","
                << ((bnbColor - tc.best_k) > 0 ? bnbColor - tc.best_k : tc.best_k - bnbColor) << "\n";
        }

        // 🧹 Free memory
        freeGraph(adj, n);

        cout << "Done: " << tc.filename << "\n\n";
    }

    out.close();
}

//////////////////////////////////////////////////
// MAIN
//////////////////////////////////////////////////
int main() {
    // 👉 chạy benchmark toàn bộ
    runAllTests("testcases.txt", "results.csv");

    // 👉 test riêng 1 file (debug)
    /*
    int n, m;
    int **adj = nullptr;

    readDIMACS("datasets/testcase1.txt", n, m, &adj);

    cout << "DSATUR: " << DSATUR(n, adj) << endl;
    cout << "BnB: " << BranchAndBound(n, adj) << endl;
    cout << "Tabu: " << TabuSearch(n, adj) << endl;

    freeGraph(adj, n);
    */

    return 0;
}