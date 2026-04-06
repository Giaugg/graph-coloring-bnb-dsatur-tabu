#include <bits/stdc++.h>
using namespace std;

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

int main() {
    int n = 0, m = 0;
    int **adj = nullptr;

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

    // free memory
    for (int i = 0; i <= n; i++) {
        delete[] adj[i];
    }
    delete[] adj;

    return 0;
}