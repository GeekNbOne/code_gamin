#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>

using namespace std;

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
    void removeEdge(int u, int v){
        adj[u].remove(v);
    }

    const list<int> getAdj(int index) const {
        return adj[index];
    }

    int getSize() const {
        return size;
    }

    bool shortest_path(int src, int dest,int* pred,int* dist){
        bool * visited = new bool[size];
        for (int i =0 ; i < size;i++){
            visited[i] = false;
        }

        list<int> queue;

        queue.push_back(src);
        visited[src] = true;
        dist[src] = 0;

        while(queue.size() >0){
            int s = queue.front();
            queue.pop_front();

            for (auto i: adj[s]){
                if(!visited[i]){
                    visited[i] = true;
                    dist[i] = dist[s] + 1;
                    pred[i] = s;
                    queue.push_back(i);
                    if (i == dest){
                        return true;
                    }
                }
            }

        }
        return false;
    }


};

int main()
{
    int N; // the total number of nodes in the level, including the gateways
    int L; // the number of links
    int E; // the number of exit gateways
    cin >> N >> L >> E; cin.ignore();
    cerr << N << " " <<L <<" "<<E  << endl;

    Graph graph = Graph(N);
    vector<int> gateways;

    for (int i = 0; i < L; i++) {
        int N1; // N1 and N2 defines a link between these nodes
        int N2;
        cin >> N1 >> N2; cin.ignore();
        cerr << N1 << " " << N2 << endl;
        graph.addEdge(N1, N2);
        graph.addEdge(N2, N1);
    }
    for (int i = 0; i < E; i++) {
        int EI; // the index of a gateway node
        cin >> EI; cin.ignore();
        cerr << EI << endl;
        gateways.push_back(EI);
    }

    // game loop
    while (1) {
        int SI; // The index of the node on which the Skynet agent is positioned this turn
        cin >> SI; cin.ignore();
        cerr << SI <<endl;
        int min_dist = INT32_MAX;
        int min_pred = 0;
        int min_gateway = 0;

        for (auto gateway : gateways){
            int * pred = new int[N];
            int * dist = new int[N];
            if (graph.shortest_path(SI, gateway, pred, dist)){
                if(dist[gateway] < min_dist){
                    min_dist = dist[gateway];
                    min_pred = pred[gateway];
                    min_gateway = gateway;
                }
            }
        }
        graph.removeEdge(min_pred,min_gateway);
        graph.removeEdge(min_gateway,min_pred);
        cout << min_pred << " " <<min_gateway << endl;
    }
}