#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <map>
#include <numeric>

using namespace std;
using namespace std::chrono;

int SIM_STEPS = 6;

class Graph {
    list<int> *adj;
    int size;

public:

    explicit Graph(int size) : size(size) {
        adj = new list<int>[size];
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
    }

    const list<int> getAdj(int index) const {
        return adj[index];
    }

    int getSize() const {
        return size;
    }


};

class Simulation {
    Graph *graph;
    int steps;

    void
    doStep(vector<int> &myPacs, int total, vector<int> &bestSimulation, int &bestTotal, int *pellets, bool *visited,
           int step) {


        if (step >= steps) {
            if (total > bestTotal) {
                bestSimulation = myPacs;
                bestTotal = total;
            }
        } else {

            //Get next moves by pac
            vector<vector<int>> pacsNext;
            getAdj(myPacs, pacsNext, visited);

            //Get all possible combinations for that step
            vector<vector<int>> simulations;
            combination(pacsNext, simulations);

            //For each simulation
            for (auto simulation: simulations) {

                //Check for collision
                if (!hasCollision(myPacs, simulation)) {
                    vector<int> pacPellets;
                    for (int i = 0; i < simulation.size(); i++) {
                        //Add pellets to total and remove pellet from vector
                        pacPellets.push_back(pellets[simulation[i]]);
                        pellets[simulation[i]] = 0;
                        //update visited
                        visited[simulation[i] + i * graph->getSize()] = true;
                    }
                    //do sim


                    doStep(simulation, accumulate(pacPellets.begin(), pacPellets.end(), total), bestSimulation,
                           bestTotal,
                           pellets, visited, step + 1);
                    for (int i = 0; i < simulation.size(); i++) {
                        //Remove pellets from total and replace pellet into vector
                        pellets[simulation[i]] = pacPellets[i];
                        //replace visited
                        visited[simulation[i] + i * graph->getSize()] = false;
                    }

                }
            }
        }


    }

    void getEnemyWeight(vector<int> &enemyPacs, double *cumulativeWeights) {
        int pathSize = graph->getSize();

        double *weights = new double[pathSize * (steps + 1)];
//        double * cumulativeWeights = new double[pathSize * (steps+1)];

        for (auto pac : enemyPacs) {
            weights[pac] = 1.0;
            cumulativeWeights[pac] = 1.0;
        }
        for (int i = 1; i <= steps; i++) {
            for (int j = 0; j < pathSize; j++) {
                int prevIndex = (i - 1) * pathSize + j;
                if (weights[prevIndex] > 0) {
                    list<int> nexts = graph->getAdj(j);
                    double weight = 1 / (double) nexts.size();
                    for (auto next:nexts) {
                        weights[i * pathSize + next] = weights[prevIndex] * weight;
                    }
                }
            }
        }
        for (int i = 0; i < pathSize; i++) {
            for (int j = 1; j <= steps; j++) {
                cumulativeWeights[j * pathSize + i] = max(cumulativeWeights[(j - 1) * pathSize + i],
                                                          weights[j * pathSize + i]);
            }
        }

        delete weights;
    }

    void combination(vector<vector<int>> &source, vector<vector<int>> &results) {

        int n = source.size();

        int *indices = new int[n];

        for (int i = 0; i < n; i++)
            indices[i] = 0;

        while (1) {

            vector<int> single;
            for (int i = 0; i < n; i++) {
                vector<int> s = source[i];
                int j = indices[i];
                int k = s[j];
                single.push_back(source[i][indices[i]]);
            }

            results.emplace_back(single);

            int next = n - 1;
            while (next >= 0 &&
                   (indices[next] + 1 >= source[next].size()))
                next--;

            if (next < 0)
                return;

            indices[next]++;

            for (int i = next + 1; i < n; i++)
                indices[i] = 0;
        }
    }

    void getAdj(vector<int> &myPacs, vector<vector<int>> &pacsNext, bool *visited) {
        for (int i = 0; i < myPacs.size(); i++) {
            vector<int> pacNext;
            for (auto next : graph->getAdj(myPacs[i])) {
                pacNext.push_back(next);
//                if (!visited[i * graph->getSize() + next]) {
//                    pacNext.push_back(next);
//                }
            }
            pacsNext.push_back(pacNext);
        }
    }

    bool hasCollision(vector<int> &source, vector<int> &dest) {

        vector<bool> collided = vector<bool>(dest.size(), false);

        for (int i = 0; i < dest.size() - 1; i++) {
            for (int j = i + 1; j < dest.size(); j++) {
                //Going on the same cell
                if (dest[i] == dest[j]) {
                    return true;
                }
                    //Crossing path
                else if (dest[i] == source[j] && dest[j] == source[i]) {
                    return true;
                }
            }
        }

        return false;
    }

public:
    Simulation(Graph *graph, int steps) : graph(graph), steps(steps) {}

    vector<int> run(vector<int> myPacs, vector<int> enemyPacs, int *pellets) {

        bool *visited = new bool[graph->getSize() * myPacs.size()];
        for (int i = 0; i < graph->getSize() * myPacs.size(); i++) {
            visited[i] = false;
        }
        for (int i = 0; i < myPacs.size(); i++) {
            visited[i * graph->getSize() + myPacs[i]] = true;
        }
        vector<int> bestSimulation;
        int bestTotal = -1;
        doStep(myPacs, 0, bestSimulation, bestTotal, pellets, visited, 0);

        return bestSimulation;
    }


};

class Edge {
    int src, dest;
public:
    Edge(int src, int dest) : src(src), dest(dest) {}

    int getSrc() const {
        return src;
    }

    int getDest() const {
        return dest;
    }
};

class PointRegister {
    int currentIndex = 0;
    map<int, pair<int, int>> toPoint;
    map<pair<int, int>, int> toIndex;
public:
    pair<int, int> getPoint(int index) {
        return toPoint[index];
    }

    int getIndex(pair<int, int> point) {
        if (toIndex.find(point) == toIndex.end()) {
            toPoint[currentIndex] = point;
            toIndex[point] = currentIndex++;

        }
        return toIndex[point];
    }

    int size() {
        return currentIndex;
    }


};


class FirstPelletSimulation {
    Graph &graph;

    int findTarget(int pac, vector<int> &myPacs, vector<int> &enemyPacs, int *pellets) {
        bool *visited = new bool[graph.getSize()];
        for (int i = 0; i < graph.getSize(); i++) {
            if (find(myPacs.begin(), myPacs.end(), i) != myPacs.end()) {
                visited[i] = true;
            } else if (find(enemyPacs.begin(), enemyPacs.end(), i) != enemyPacs.end()) {
                visited[i] = true;
            } else {
                visited[i] = false;
            }
        }

        list<int> queue;
        queue.push_back(pac);

        while (queue.size() > 0) {

            int s = queue.front();
            queue.pop_front();

            for (auto a: graph.getAdj(s)) {
                if (pellets[a] > 0) {
                    return a;
                }
                if (!visited[a]) {
                    queue.push_back(a);
                    visited[a] = true;
                }

            }

        }
        return pac;

    }

public:
    FirstPelletSimulation(Graph &graph) : graph(graph) {}

    vector<int> run(vector<int> &myPacs, vector<int> &enemyPacs, int *pellets) {

        vector<int> result;

        for (auto pac:myPacs) {
            result.push_back(findTarget(pac, myPacs, enemyPacs, pellets));
        }

        return result;
    }
};

class Map {
    int width, height;
    vector<string> rows;
    PointRegister paths = PointRegister();

public:
    Map(int width, int height) : width(width), height(height) {}

    void addRow(string row) {
        rows.emplace_back(row);
    }

    Graph buildGraph() {
        vector<Edge> edges;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (rows[y][x] == ' ') {

                    int index = paths.getIndex(make_pair(x, y));

                    for (pair<int, int> d_point: {make_pair(x, (y - 1) % height), make_pair((x - 1) % width, y)}) {
                        if (rows[d_point.second][d_point.first] == ' ') {
                            int d_index = paths.getIndex(d_point);
                            edges.emplace_back(Edge(index, d_index));
                        }
                    }
                }
            }
        }

        Graph graph = Graph(paths.size());
        for (Edge edge: edges) {
            int x = edge.getSrc();
            int y = edge.getDest();
            graph.addEdge(x, y);
            graph.addEdge(y, x);
        }
        return graph;
    }

    pair<int, int> getPoint(int index) {
        return paths.getPoint(index);
    }

    int getIndex(pair<int, int> point) {
        return paths.getIndex(point);
    }

    int getIndex(int x, int y) {
        return paths.getIndex(make_pair(x, y));
    }

    void printIndex() {
        for (int i = 0; i < paths.size(); i++) {
            pair<int, int> point = paths.getPoint(i);

            cout << i << "\t" << point.first << "\t" << point.second << endl;
        }
    }

    void toIndex(vector<pair<int,int>>& src, vector<int>& dest){
        for_each(src.begin(), src.end(), [&](pair<int, int> &p) { dest.emplace_back(paths.getIndex(p)); });
    }

    vector<pair<int, int>> next(Simulation &sim, vector<pair<int, int>> &pacs, vector<pair<int, int>>  &enemy, int *pellets) {

        vector<int> pacIndex;
        vector<int> enemyIndex;

        toIndex(pacs,pacIndex);
        toIndex(enemy,enemyIndex);

        vector<int> nextValues = sim.run(pacIndex, enemyIndex, pellets);
        vector<pair<int, int>> results;
        for (auto val:nextValues) {
            results.push_back(paths.getPoint(val));
        }
        return results;
    }

    vector<pair<int,int>> next(FirstPelletSimulation &sim,vector<pair<int, int>> &pacs, vector<pair<int, int>>  &enemy, int *pellets){
        vector<int> pacIndex;
        vector<int> enemyIndex;

        toIndex(pacs,pacIndex);
        toIndex(enemy,enemyIndex);

        vector<int> nextValues = sim.run(pacIndex, enemyIndex, pellets);
        vector<pair<int, int>> results;
        for (auto val:nextValues) {
            results.push_back(paths.getPoint(val));
        }
        return results;
    }


};


int main() {

    auto start = std::chrono::steady_clock::now();

    int width;
    int height;
    cin >> width >> height;
    cin.ignore();

    Map pacMap = Map(width, height);
    for (int i = 0; i < height; i++) {
        string row;
        getline(cin, row);
        cout << row << endl;
        pacMap.addRow(row);
    }


    Graph graph = pacMap.buildGraph();

    FirstPelletSimulation sim = FirstPelletSimulation(graph);

    vector<pair<int, int>> pacs = {make_pair(0, 1), make_pair(5, 3), make_pair(9, 5), make_pair(5, 9)};
    vector<pair<int, int>> enemy = {make_pair(15, 7), make_pair(15, 3)};
    vector<int> pellets10 = {1};
    int *pellets = new int[graph.getSize()];
    for (int i = 0; i < graph.getSize(); i++) {
        if (find(pacs.begin(), pacs.end(), pacMap.getPoint(i)) != pacs.end()) {
            pellets[i] = 0;
        } else if (find(pellets10.begin(), pellets10.end(), i) != pellets10.end()) {
            pellets[i] = 10;
        } else {
            pellets[i] = 1;
        }

    }


    for (auto pac:pacs) {
        cout << "(" << pac.first << "," << pac.second << ") ";
    }
    cout << endl;

    vector<pair<int, int>> s = pacMap.next(sim, pacs, enemy, pellets);
    for (auto i:s) {

        cout << "(" << i.first << "," << i.second << ") ";
    }
    cout << endl;


    auto end = std::chrono::steady_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "elapsed time: " << duration.count() << "ms\n";


}