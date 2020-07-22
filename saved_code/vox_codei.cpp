#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <list>

using namespace std;

enum CellType {
    Block, Free, Node
};

struct CellState {
    bool hasActiveBomb;
    int bombCooldown;
    bool exploded;
    int bombCounter = 0;
    bool isTarget = false;

    CellState(bool hasActiveBomb, int bombCooldown, bool exploded) : hasActiveBomb(hasActiveBomb),
                                                                     bombCooldown(bombCooldown), exploded(exploded) {}


    void explode() {
        hasActiveBomb = false;
        bombCooldown = 0;
        exploded = true;
    }

    void placeBomb() {
        hasActiveBomb = true;
        bombCooldown = 3;
        bombCounter++;
    }

    void age() {
        if (hasActiveBomb) bombCooldown--;
    }

};

class Cell {
    CellType cellType;

    vector<CellState> states;

public:

    Cell(CellType cellType) : cellType(cellType) {
        states.emplace_back(CellState(false, 0, false));
    }

    [[nodiscard]] CellType getCellType() const {
        return cellType;
    }

    void placeBomb() {
        states.back().placeBomb();
    }

    void next() {
        auto newState = CellState(states.back());
        newState.age();
        states.emplace_back(newState);
    }

    void redoState() {
        states.pop_back();
    }

    bool checkExplosion() const {
        return states.back().hasActiveBomb && states.back().bombCooldown == 0;
    }

    bool hasActiveBomb() const {
        return states.back().hasActiveBomb;
    }

    bool isBlock() const {
        return cellType == Block;
    }

    void explode() {
        states.back().explode();
    }

    int bombCounter() const {
        return states.back().bombCounter;
    }

    bool hasActiveNode() const {
        return !states.back().exploded && cellType == Node && !states.back().isTarget;
    }

    bool hasDeactivateNode() const {
        return !states.back().exploded && cellType == Node && states.back().isTarget;
    }

    bool hasExplodedNode() const {
        return states.back().exploded && cellType == Node;
    }

    void setIsTarget() {
        states.back().isTarget = true;
    }

    friend ostream &operator<<(ostream &os, const Cell &cell) {
        switch (cell.cellType) {

            case Block:
                os << "#";
                break;
            case Free:
                if (cell.states.back().hasActiveBomb) {
                    os << cell.states.back().bombCooldown;
                } else {
                    os << ".";
                }
                break;
            case Node:
                if (cell.states.back().exploded) {
                    os << "X";
                } else {
                    os << "@";
                }
                break;
        }
        return os;
    }


};

class Terrain {
    int height, width;
    vector<string> &rows;

    vector<vector<Cell *> *> cells;

    bool coordInBorder(int x, int y) const {
        return x >= 0 && x < height && y >= 0 && y < width;
    }

    bool explodeCell(int x, int y, vector<pair<int, int>> &explosions) {
        if (coordInBorder(x, y) && !(*cells[x])[y]->isBlock()) {
            if ((*cells[x])[y]->hasActiveBomb()) {
                explosions.emplace_back(make_pair(x, y));
            }
            (*cells[x])[y]->explode();
            return true;
        } else {
            return false;
        }
    }


public:
    Terrain(int height, int width, vector<string> &rows) : height(height), width(width), rows(rows) {
        for (auto row :rows) {
            auto *rowCells = new vector<Cell *>;
            cells.emplace_back(rowCells);
            for (auto c:row) {
                switch (c) {
                    case '.':
                        rowCells->emplace_back(new Cell(Free));
                        break;
                    case '@':
                        rowCells->emplace_back(new Cell(Node));
                        break;
                    case '#':
                        rowCells->emplace_back(new Cell(Block));
                        break;
                }
            }
        }
    }

    friend ostream &operator<<(ostream &os, const Terrain &terrain) {
        for (auto row : (terrain.cells)) {
            for (auto cell : *row) {
                os << *cell;
            }
            os << endl;
        }

        os << "Bombs: " << terrain.bombCounter() << " ActiveNodes: " << terrain.activeNodes() << endl;
        return os;
    }

    void placeBomb(int x, int y) {
        (*cells[x])[y]->placeBomb();
        //up
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x - i, y) && !(*cells[x - i])[y]->isBlock()) {
                (*cells[x - i])[y]->setIsTarget();
            } else {
                break;
            }
        }
        //down
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x + i, y) && !(*cells[x + i])[y]->isBlock()) {
                (*cells[x + i])[y]->setIsTarget();
            } else {
                break;
            }
        }
        //left
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y - i) && !(*cells[x])[y - i]->isBlock()) {
                (*cells[x])[y - i]->setIsTarget();
            } else {
                break;
            }
        }
        //right
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y + i) && !(*cells[x])[y + i]->isBlock()) {
                (*cells[x])[y + i]->setIsTarget();
            } else {
                break;
            }
        }
    }

    void nextState() {
        vector<pair<int, int>> explosions;
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                Cell *cell = (*cells[x])[y];
                cell->next();
                if (cell->checkExplosion()) {
                    explosions.emplace_back(make_pair(x, y));
                }
            }
        }

        //Check for explosion
        while (!explosions.empty()) {
            auto cell_coord = explosions.back();
            explosions.pop_back();
            int base_x = cell_coord.first, base_y = cell_coord.second;
            Cell *cell = (*cells[base_x])[base_y];
            cell->explode();

            //up
            for (int i = 1; i < 4; i++) {
                if (!explodeCell(base_x - i, base_y, explosions)) break;
            }
            //down
            for (int i = 1; i < 4; i++) {
                if (!explodeCell(base_x + i, base_y, explosions)) break;
            }
            //left
            for (int i = 1; i < 4; i++) {
                if (!explodeCell(base_x, base_y - i, explosions)) break;
            }
            //right
            for (int i = 1; i < 4; i++) {
                if (!explodeCell(base_x, base_y + i, explosions)) break;
            }
        }
    }

    void previousState() {
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                (*cells[x])[y]->redoState();
            }
        }
    }

    int bombCounter() const {
        int tot = 0;
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                tot += (*cells[x])[y]->bombCounter();
            }
        }
        return tot;
    }

    int activeNodes() const {
        int tot = 0;
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                if ((*cells[x])[y]->hasActiveNode()) tot++;
            }
        }
        return tot;
    }

    int deactivatedNodes() const{
        int tot = 0;
        for (int x = 0; x < height; x++) {
            for (int y = 0; y < width; y++) {
                if ((*cells[x])[y]->hasDeactivateNode()) tot++;
            }
        }
        return tot;
    }

    int nodeCount(int x, int y) const {

        int tot = 0;

        //up
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x - i, y) && !(*cells[x - i])[y]->isBlock()) {
                if ((*cells[x - i])[y]->hasActiveNode()) tot++;
            } else {
                break;
            }
        }
        //down
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x + i, y) && !(*cells[x + i])[y]->isBlock()) {
                if ((*cells[x + i])[y]->hasActiveNode()) tot++;
            } else {
                break;
            }
        }
        //left
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y - i) && !(*cells[x])[y - i]->isBlock()) {
                if ((*cells[x])[y - i]->hasActiveNode()) tot++;
            } else {
                break;
            }
        }
        //right
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y + i) && !(*cells[x])[y + i]->isBlock()) {
                if ((*cells[x])[y + i]->hasActiveNode()) tot++;
            } else {
                break;
            }
        }

        return tot;
    }

    int deactivatedNodeCount(int x, int y) const {

        int tot = 0;

        //up
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x - i, y) && !(*cells[x - i])[y]->isBlock()) {
                if ((*cells[x - i])[y]->hasDeactivateNode()) tot++;
            } else {
                break;
            }
        }
        //down
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x + i, y) && !(*cells[x + i])[y]->isBlock()) {
                if ((*cells[x + i])[y]->hasDeactivateNode()) tot++;
            } else {
                break;
            }
        }
        //left
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y - i) && !(*cells[x])[y - i]->isBlock()) {
                if ((*cells[x])[y - i]->hasDeactivateNode()) tot++;
            } else {
                break;
            }
        }
        //right
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y + i) && !(*cells[x])[y + i]->isBlock()) {
                if ((*cells[x])[y + i]->hasDeactivateNode()) tot++;
            } else {
                break;
            }
        }

        return tot;
    }

    int explodedNodeCount(int x, int y) const {

        int tot = 0;

        //up
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x - i, y) && !(*cells[x - i])[y]->isBlock()) {
                if ((*cells[x - i])[y]->hasExplodedNode()) tot++;
            } else {
                break;
            }
        }
        //down
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x + i, y) && !(*cells[x + i])[y]->isBlock()) {
                if ((*cells[x + i])[y]->hasExplodedNode()) tot++;
            } else {
                break;
            }
        }
        //left
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y - i) && !(*cells[x])[y - i]->isBlock()) {
                if ((*cells[x])[y - i]->hasExplodedNode()) tot++;
            } else {
                break;
            }
        }
        //right
        for (int i = 1; i < 4; i++) {
            if (coordInBorder(x, y + i) && !(*cells[x])[y + i]->isBlock()) {
                if ((*cells[x])[y + i]->hasExplodedNode()) tot++;
            } else {
                break;
            }
        }

        return tot;
    }

    bool canPlaceBomb(int x, int y) const{
        Cell* cell = (*cells[x])[y];
        return cell->getCellType() == Free || cell->hasExplodedNode();
    }

    pair<int,int> next(int turn,vector<pair<int,int>>& visited,vector<vector<pair<int,int>>>& turnVisited){

        int max_count=0;
        auto max_pair = make_pair(-1,-1);
        for (int x=0; x < height;x++){
            for (int y = 0; y < width;y++){
                auto p = make_pair(x,y);
                if(find(visited.begin(), visited.end(),p) == visited.end()){
                    if (find(turnVisited[turn].begin(), turnVisited[turn].end(),p) == turnVisited[turn].end()){
                        if (nodeCount(x,y) > max_count && canPlaceBomb(x,y)){
                            max_count = nodeCount(x,y);
                            max_pair = p;
                        }
                    }
                }
            }
        }
        return max_pair;
    }
};

class Solver {
    Terrain &terrain;

public:
    Solver(Terrain &terrain) : terrain(terrain) {}

    list<string> commands(int bombs, int turns) {
        vector<vector<pair<int,int>>> turnVisited;
        for (int x=0; x < turns;x++){
            turnVisited.emplace_back(vector<pair<int,int>>());
        }
        vector<pair<int,int>> visited;

        list<string> result;

        int currentTurn = 0;
        int currentBombs = 0;

        while(terrain.activeNodes() > 0 ){
            //Find a cell with activeNode > 0 and not in visited < turn;
//            cout << "Turn: " << currentTurn << endl << terrain <<endl;

            if (currentTurn < turns && currentBombs < bombs){
                terrain.nextState();
                auto p = terrain.next(currentTurn,visited,turnVisited);
                if (p == make_pair(-1,-1)){

                    visited.emplace_back(p);
                    result.emplace_back("WAIT");

                }else{
                    terrain.placeBomb(p.first,p.second);
                    visited.emplace_back(p);
                    turnVisited[currentTurn].emplace_back(p);
                    result.emplace_back(to_string(p.second) + " " + to_string(p.first));
                    currentBombs++;
                }
                currentTurn++;
            } else{


                if (terrain.deactivatedNodes() > 0 ){
                    while(visited.back() == make_pair(-1,-1)) {
                        result.pop_back();
                        visited.pop_back();
                        turnVisited[currentTurn].clear();
                        currentTurn--;
                        terrain.previousState();
                    }
                }else{
                    while(visited.size() > 1){
                        if(visited.back() != make_pair(-1,-1)) currentBombs--;
                        result.pop_back();
                        visited.pop_back();
                        turnVisited[currentTurn].clear();
                        currentTurn--;
                        terrain.previousState();
                    }
                }

                result.pop_back();
                visited.pop_back();
                currentTurn--;
                currentBombs--;
                terrain.previousState();
            }


        }

        return result;



    }

};

int main() {
    int width; // width of the firewall grid
    int height; // height of the firewall grid
    cin >> width >> height;
    cin.ignore();


    vector<string> rows;

    for (int i = 0; i < height; i++) {
        string mapRow;
        getline(cin, mapRow); // one line of the firewall grid
        rows.emplace_back(mapRow);
    }

    auto terrain = Terrain(height, width, rows);


    auto solver = Solver(terrain);

    list<string> result = solver.commands(2,10);

    for (auto s : result){
        cout << s << endl;
    }

//    while (1) {
//        int rounds; // number of rounds left before the end of the game
//        int bombs; // number of bombs left
//        cin >> rounds >> bombs; cin.ignore();
//        terrain.startTurn();
//        auto p = terrain.next(bombs);
//        if (p.first != -1){
//            cout << p.second <<" " <<p.first <<endl;
//            terrain.placeBomb(p.first,p.second);
//        }else{
//            cout <<"WAIT" << endl;
//        }
//
//
//    }
}