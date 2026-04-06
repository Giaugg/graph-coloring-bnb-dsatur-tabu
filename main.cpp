#include <bits/stdc++.h>
#include <chrono>
#include <curl/curl.h>

using namespace std;
using namespace chrono;

int n = 0, m = 0;
int **adj = nullptr;

// read graph from DIMACS format
void readDIMACS(const string &filename, int &n, int &m, int ***adj) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open file!\n";
        return;
    }

    string line;

    while (getline(file, line)) {
        if (line.empty()) continue;

        if (line[0] == 'c') continue;

        if (line[0] == 'p') {
            string tmp1, tmp2;
            stringstream ss(line);
            ss >> tmp1 >> tmp2 >> n >> m;

            // cấp phát động
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

// DSATUR heuristic
int DSATUR(int n, int **adj) {
    vector<int> color(n+1, 0);
    vector<int> degree(n+1, 0);

    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++)
            if (adj[i][j]) degree[i]++;

    for (int step = 1; step <= n; step++) {
        int u = -1, maxSat = -1;

        for (int i = 1; i <= n; i++) {
            if (color[i] != 0) continue;

            set<int> used;
            for (int j = 1; j <= n; j++)
                if (adj[i][j] && color[j])
                    used.insert(color[j]);

            if ((int)used.size() > maxSat) {
                maxSat = used.size();
                u = i;
            }
        }

        set<int> used;
        for (int j = 1; j <= n; j++)
            if (adj[u][j] && color[j])
                used.insert(color[j]);

        int c = 1;
        while (used.count(c)) c++;

        color[u] = c;
    }

    int maxColor = 0;
    for (int i = 1; i <= n; i++)
        maxColor = max(maxColor, color[i]);

    return maxColor;
}


// Branch & Bound
int best = INT_MAX;

bool isSafe(int u, int c, int **adj, int color[]) {
    for (int v = 1; v <= n; v++)
        if (adj[u][v] && color[v] == c)
            return false;
    return true;
}

void BnB(int u, int usedColors, int **adj, int color[]) {
    if (u > n) {
        best = min(best, usedColors);
        return;
    }

    if (usedColors >= best) return;

    for (int c = 1; c <= usedColors + 1; c++) {
        if (isSafe(u, c, adj, color)) {
            color[u] = c;
            BnB(u + 1, max(usedColors, c), adj, color);
            color[u] = 0;
        }
    }
}

int BranchAndBound(int n, int **adj) {
    int color[1000] = {0};
    best = INT_MAX;
    BnB(1, 0, adj, color);
    return best;
}


// Tabu Search heuristic
int TabuSearch(int n, int **adj, int maxIter = 1000) {
    vector<int> color(n+1);
    srand(time(0));

    int k = DSATUR(n, adj); // bắt đầu từ nghiệm tốt

    for (int i = 1; i <= n; i++)
        color[i] = rand() % k + 1;

    int bestConflict = INT_MAX;

    for (int iter = 0; iter < maxIter; iter++) {
        int conflict = 0;

        for (int i = 1; i <= n; i++)
            for (int j = i+1; j <= n; j++)
                if (adj[i][j] && color[i] == color[j])
                    conflict++;

        bestConflict = min(bestConflict, conflict);

        if (conflict == 0) return k;

        // random move
        int u = rand() % n + 1;
        int newColor = rand() % k + 1;
        color[u] = newColor;
    }

    return k; // heuristic nên không đảm bảo tối ưu
}

int main() {

    const string filename = "testcase1.txt";

    readDIMACS(filename, n, m, &adj);

    cout << "Number of vertices: " << n << endl;
    cout << "Number of edges: " << m << endl;

    // In ma trận
    for (int i = 1; i <= n; i++) {
        cout << "Vertex " << i << ": ";
        for (int j = 1; j <= n; j++) {
            if (adj[i][j]) {
                cout << j << " ";
            }
        }
        cout << endl;
    }

    // DSATUR
    auto t1 = chrono::high_resolution_clock::now();
    int res1 = DSATUR(n, adj);
    auto t2 = chrono::high_resolution_clock::now();
    cout << "DSATUR: " << res1 
        << " | Time: " 
        << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() 
        << " ms\n";

    // Branch & Bound
    auto t3 = chrono::high_resolution_clock::now();
    int res2 = BranchAndBound(n, adj);
    auto t4 = chrono::high_resolution_clock::now();
    cout << "BnB: " << res2 
        << " | Time: " 
        << chrono::duration_cast<chrono::milliseconds>(t4 - t3).count() 
        << " ms\n";

    // Tabu
    auto t5 = chrono::high_resolution_clock::now();
    int res3 = TabuSearch(n, adj);
    auto t6 = chrono::high_resolution_clock::now();
    cout << "Tabu: " << res3 
        << " | Time: " 
        << chrono::duration_cast<chrono::milliseconds>(t6 - t5).count() 
        << " ms\n";

    // free memory
    for (int i = 0; i <= n; i++) {
        delete[] adj[i];
    }
    delete[] adj;

    return 0;
}