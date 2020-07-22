#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <cfloat>
#include <set>
#include <algorithm>

using namespace std;

typedef pair<int, int> Pair;

enum Dir {
    left, right
};

struct Cell {
    int parent = -1;
    double  g = FLT_MAX;
    bool fromElevator = false;
    bool placeElevator = false;
    Dir direction = Dir::right;

    bool canPlaceElevator(int maxElevator) {
        return fromElevator;
    }

};

class Level {
    int nbFloors, width,size, addElevators;
    vector<int> elevators;
public:
    Level(int nbFloors, int width,int addElevators) : nbFloors(nbFloors), width(width),size(nbFloors * width),addElevators(addElevators) {}

    int getFloor(int i) const {
        return (i%size) / width;
    }

    bool hasUpCell(int i) const {

        return find(elevators.begin(), elevators.end(), toBase(i)) != elevators.end();
    }

    bool hasNextCell(int i) const {
        if (hasUpCell(i)) {
            return false;
        }
        return (i % width) + 1 < width;
    }

    int oppositeCell(int i) const{
        int addElev = i / (size * 2);
        int base = i % (size * 2);
        int f= getFloor(base);
        int v = ((f* 2 + 1) * width - base - 1 + size) % (size * 2) + (addElev *size * 2);
        return v;
    }

    int toBase(int i) const{

        if ( i >=  size* 2){
            i = i % (size * 2);
        }

        if (i < size){
            return i;
        }
        else {
            return oppositeCell(i);
        }
    }

    bool hasOppositeCell(int i ) const{
        int opp = oppositeCell(i);
        return hasNextCell(opp);
    }

    void addElevator(int elevatorFloor, int elevatorPosition) {
        int e = elevatorFloor * width + elevatorPosition;
        elevators.push_back(e);
    }

    int getSize() const {
        return nbFloors * width;
    }

    int getWidth() const {
        return width;
    }

    int isDest(int src, int dest) const{
        src = toBase(src);
        return src == dest;
    }

    int dist(int src, int dest) {
        int s_floor = src / width;
        int d_floor = dest / width;

        int s_pos = src % width;
        int d_pos = dest % width;

        return abs(s_floor - d_floor) * width + abs(s_pos - d_pos);

    }


    bool hasAddElevator(int i ){
        int t = (i / (size * 2));
        return (i / (size * 2)) < addElevators;
    }

    int totalSize()const{
        return size * 2 * (addElevators +1);
    }

};

class Graph {
    list<int> *adj;
    int size;

    const int BLOCK_TIME = 4;
public:

    explicit Graph(int size) : size(size) {
        adj = new list<int>[size];
    }

    void addEdge(int u, int v) {
        adj[u].push_back(v);
    }

    void removeEdge(int u, int v) {
        adj[u].remove(v);
    }

    const list<int> getAdj(int index) const {
        return adj[index];
    }

    int getSize() const {
        return size;
    }

    void aStar(int src, int dest, Level *level, Cell *cells) {
        bool *closedList = new bool[level->totalSize()];
        for (int i = 0; i < level->getSize(); i++) {
            closedList[i] = false;
        }

        cells[src].g = 0.0;
        cells[src].parent = src;
        cells[src].direction = Dir::right;

        set<pair<double, int>> openList;
        openList.insert(make_pair(0.0, src));


        bool foundDest = false;

        while (!openList.empty()) {
            pair<double, int> p = *openList.begin();
            openList.erase(openList.begin());
            int i = p.second;

            closedList[i] = true;

            double gNew;

            //Next Cell
            if (level->hasNextCell(i)) {
                int d = i + 1;
                if (level->isDest(d,dest)) {
                    cells[level->toBase(d)].parent = i;
                    cells[level->toBase(d)].g = cells[i].g;
                    cells[level->toBase(d)].placeElevator = false;
                    foundDest = true;
                    return;
                } else if (!closedList[d]) {
                    gNew = cells[i].g + 1 ;

                    if (cells[d].g == FLT_MAX || cells[d].g > gNew) {

                        openList.insert(make_pair(gNew, d));

                        cells[d].g = gNew;

                        cells[d].parent = i;
                        cells[d].direction = Dir::right;
                        cells[d].placeElevator = false;
                    }
                }
            }
            //Opposite Cell
            if (level->hasOppositeCell(i)) {
                int d = level->oppositeCell(i) + 1;
                if (level->isDest(d,dest)) {
                    cells[level->toBase(d)].parent = i;
                    cells[level->toBase(d)].g = cells[i].g;
                    cells[level->toBase(d)].placeElevator = false;
                    foundDest = true;
                    return;
                } else if (!closedList[d]) {
                    gNew = cells[i].g +BLOCK_TIME;

                    if (cells[d].g == FLT_MAX || cells[d].g > gNew) {

                        openList.insert(make_pair(gNew, d));

                        cells[d].g = gNew;

                        cells[d].parent = i;
                        cells[d].direction = Dir::left;
                        cells[d].placeElevator = false;
                    }
                }
            }
            //Elevator
            if (level->hasUpCell(i)) {
                int d = i + level->getWidth();
                if (level->isDest(d,dest)) {
                    cells[level->toBase(d)].parent = i;
                    cells[level->toBase(d)].g = cells[i].g;
                    cells[level->toBase(d)].placeElevator = false;
                    foundDest = true;
                    return;
                } else if (!closedList[d]) {
                    gNew = cells[i].g + 1;

                    if (cells[d].g == FLT_MAX || cells[d].g > gNew) {

                        openList.insert(make_pair(gNew, d));

                        cells[d].g = gNew;
                        cells[d].parent = i;
                        cells[d].direction = cells[i].direction;
                        cells[d].placeElevator = false;
                        cells[d].fromElevator = true;
                    }
                }
            }
            //Build elevator
            if (cells[i].fromElevator && level->hasAddElevator(i) ) {
                int d = i + level->getSize() * 2 ;
                d += level->getWidth();
                if (level->isDest(d,dest)) {
                    cells[level->toBase(d)].parent = i;
                    cells[level->toBase(d)].placeElevator = true;
                    cells[level->toBase(d)].g = cells[i].g;
                    foundDest = true;
                    return;
                } else if (!closedList[d]) {
                    gNew = cells[i].g + BLOCK_TIME;

                    if (cells[d].g == FLT_MAX || cells[d].g > gNew) {
                        int ii= level->toBase(i);
                        int dd = level->toBase(d);
                        openList.insert(make_pair(gNew, d));

                        cells[d].g = gNew;

                        cells[d].parent = i;
                        cells[d].direction = cells[i].direction;
                        cells[d].placeElevator = true;
                        cells[d].fromElevator = true;
                    }
                }
            }
        }
        if (!foundDest) {
            cout << "Did not found exit" << endl;
        }
    }

};

int main() {
    int nbFloors; // number of floors
    int width; // width of the area
    int nbRounds; // maximum number of rounds
    int exitFloor; // floor on which the exit is found
    int exitPos; // position of the exit on its floor
    int nbTotalClones; // number of generated clones
    int nbAdditionalElevators; // number of additional elevators that you can build
    int nbElevators; // number of elevators
    cin >> nbFloors >> width >> nbRounds >> exitFloor >> exitPos >> nbTotalClones >> nbAdditionalElevators
        >> nbElevators;
    cin.ignore();

    Level level = Level(nbFloors, width,nbAdditionalElevators);
    cout << level.totalSize();
    for (int i = 0; i < nbElevators; i++) {
        int elevatorFloor; // floor on which this elevator is found
        int elevatorPos; // position of the elevator on its floor
        cin >> elevatorFloor >> elevatorPos;
        cin.ignore();
        level.addElevator(elevatorFloor, elevatorPos);

    }

    int cloneIndex = 6;

    Graph graph = Graph(level.getSize());

    Cell *cells = new Cell[level.totalSize()];

    int exitIndex = exitFloor * width + exitPos;
    graph.aStar(cloneIndex, exitIndex, &level, cells);

    vector<int> path;
    vector<int> addElevators;
    bool nextElevator = false;

    int cursor = exitIndex;
    path.push_back(exitIndex);

    while (cursor != cloneIndex){
        cursor = cells[cursor].parent;
        if (nextElevator){
            nextElevator = false;
            addElevators.push_back(level.toBase(cursor));
        }
        if (cells[cursor].placeElevator){
            nextElevator = true;
        }
        path.push_back(level.toBase(cursor));
    }



    for (auto p : path){
        cout << p;
        if (find(addElevators.begin(), addElevators.end(),p) != addElevators.end()){
            cout << " add elevator";
        }
        cout <<endl;
    }

}