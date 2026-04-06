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
int DSATUR(int n, int **adj) {
    vector<int> color(n+1, 0);
    vector<int> degree(n+1, 0);
    vector<int> sat(n+1, 0);

    // tính degree
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            if (adj[i][j]) degree[i]++;

    // chọn đỉnh đầu tiên: degree lớn nhất
    int first = 1;
    for (int i = 2; i <= n; i++)
        if (degree[i] > degree[first])
            first = i;

    color[first] = 1;

    // update saturation
    for (int j = 1; j <= n; j++)
        if (adj[first][j])
            sat[j]++;

    // lặp
    for (int step = 2; step <= n; step++) {
        int u = -1;

        // chọn đỉnh: max sat → tie-break degree
        for (int i = 1; i <= n; i++) {
            if (color[i] != 0) continue;

            if (u == -1 || 
                sat[i] > sat[u] || 
                (sat[i] == sat[u] && degree[i] > degree[u])) {
                u = i;
            }
        }

        // tìm màu nhỏ nhất hợp lệ
        vector<bool> used(n+1, false);
        for (int j = 1; j <= n; j++)
            if (adj[u][j] && color[j])
                used[color[j]] = true;

        int c = 1;
        while (used[c]) c++;

        color[u] = c;

        // update saturation
        for (int v = 1; v <= n; v++) {
            if (adj[u][v] && color[v] == 0) {
                // kiểm tra xem màu c đã xuất hiện chưa
                bool found = false;
                for (int k = 1; k <= n; k++) {
                    if (adj[v][k] && color[k] == c) {
                        found = true;
                        break;
                    }
                }
                if (!found) sat[v]++;
            }
        }
    }

    return *max_element(color.begin(), color.end());
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
            return DSATUR(n, adj);
        });

        int dsaturColor = dsaturRes.first;
        long long dsaturTime = dsaturRes.second;

        out << tc.filename << ",DSATUR,"
            << dsaturColor << "," << dsaturTime << ","
            << tc.best_k << ","
            << (dsaturColor == tc.best_k) << "\n";


        // ===================== TABU =====================
        pair<int, long long> tabuRes = measure([&]() {
            return TabuSearch(n, adj, 1000, tc.best_k);
        });

        int tabuColor = tabuRes.first;
        long long tabuTime = tabuRes.second;

        out << tc.filename << ",Tabu,"
            << tabuColor << "," << tabuTime << ","
            << tc.best_k << ","
            << (tabuColor == tc.best_k) << "\n";


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
                << (bnbColor == tc.best_k) << "\n";
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