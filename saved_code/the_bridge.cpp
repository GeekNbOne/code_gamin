#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

enum Command {
    UP, DOWN, JUMP, WAIT, SPEED, SLOW
};

class Level {
    string l[4];
public:
    Level(string l0, string l1, string l2, string l3) {
        l[0] = l0;
        l[1] = l1;
        l[2] = l2;
        l[3] = l3;
    }

    bool move(int x, int lane, int speed) {
        for (int i = 0; i < speed; i++) {
            if (l[lane][x + i + 1] == '0') {
                return false;
            }
        }
        return true;
    }

    bool jump(int x, int lane, int speed) {
        if (l[lane][x + speed] == '0') {
            return false;
        }
        return true;
    }

    bool up(int x, int lane, int speed) {
        if (speed == 0) {
            return true;
        }
        for (int i = 0; i < speed - 1; i++) {
            if (l[lane][x + i + 1] == '0') {
                return false;
            }
            if (l[lane - 1][x + i + 1] == '0') {
                return false;
            }
        }

        if (l[lane - 1][x + speed] == '0') {
            return false;
        }
        return true;
    }

    bool down(int x, int lane, int speed) {
        if (speed == 0) {
            return true;
        }
        for (int i = 0; i < speed - 1; i++) {
            if (l[lane][x + i + 1] == '0') {
                return false;
            }
            if (l[lane + 1][x + i + 1] == '0') {
                return false;
            }
        }

        if (l[lane + 1][x + speed] == '0') {
            return false;
        }
        return true;
    }

    int length() const {
        return l[0].length();
    }
};

struct GameState {
    int speed;
    int x;
    bool y[4];
    int turn;
    bool noCommand = true;
    Command currentCommand = Command::WAIT;

    bool nextCommand() {

        if (noCommand) {
            noCommand = false;
            if (speed == 0) {
                currentCommand = SPEED;
                return true;
            } else {
                currentCommand = WAIT;
                return true;
            }
        } else {
            switch (currentCommand) {
                case WAIT:
                    currentCommand = SPEED;
                    return true;
                case DOWN:
                    if (canGoUp()) {
                        currentCommand = UP;
                        return true;
                    } else {
                        return false;
                    }
                case JUMP:
                    if (canGoDown()) {
                        currentCommand = DOWN;
                        return true;
                    } else if (canGoUp()) {
                        currentCommand = UP;
                        return true;
                    } else {
                        return false;
                    }
                case SLOW:
                    currentCommand = JUMP;
                    return true;
                case SPEED:
                    if (speed <= 1) {
                        currentCommand = JUMP;
                    } else {
                        currentCommand = SLOW;
                    }
                    return true;
                case UP:
                    return false;

            }
        }
    }

    bool alive(int V) {
        return moto() >= V;
    }

    int moto() const {
        int v = 0;
        for (int i = 0; i < 4; i++) {
            v += y[i];
        }
        return v;
    }

    bool canGoUp() const {
        return y[0] == false;
    }

    bool canGoDown() const {
        return y[3] == false;
    }

    GameState *next(Level &level) {
        GameState *newState = new GameState();
        newState->speed = speed;
        newState->turn = turn + 1;
        switch (currentCommand) {
            case Command::UP:
                for (int i = 0; i < 3; i++) {
                    if (y[i + 1]) {
                        newState->y[i] = level.up(x, i + 1, speed);
                    } else {
                        newState->y[i] = false;
                    }
                    newState->y[3] = false;
                }
                break;
            case Command::DOWN:
                for (int i = 1; i < 4; i++) {
                    if (y[i - 1]) {
                        newState->y[i] = level.down(x, i - 1, speed);
                    } else {
                        newState->y[i] = false;
                    }
                    newState->y[0] = false;
                }
                break;
            case Command::JUMP:
                for (int i = 0; i < 4; i++) {
                    if (y[i]) {
                        newState->y[i] = level.jump(x, i, newState->speed);
                    } else {
                        newState->y[i] = false;
                    }
                }
                break;
            case Command::WAIT:
                for (int i = 0; i < 4; i++) {
                    if (y[i]) {
                        newState->y[i] = level.move(x, i, newState->speed);
                    } else {
                        newState->y[i] = false;
                    }
                }
                break;
            case Command::SPEED:
                newState->speed++;
                for (int i = 0; i < 4; i++) {
                    if (y[i]) {
                        newState->y[i] = level.move(x, i, newState->speed);
                    } else {
                        newState->y[i] = false;
                    }
                }
                break;
            case Command::SLOW:
                newState->speed--;
                for (int i = 0; i < 4; i++) {
                    if (y[i]) {
                        newState->y[i] = level.move(x, i, newState->speed);
                    } else {
                        newState->y[i] = false;
                    }
                }
                break;
        }
        newState->x = x + newState->speed;

        return newState;
    }

};


vector<GameState*> run(Level& level,int V,bool start[]){
    vector<GameState *> turns;
    GameState* gameState = new GameState();
    gameState->turn = 1;
    gameState->speed = 0;
    gameState->x = 0;
    gameState->y[0] = start[0];
    gameState->y[1] = start[1];
    gameState->y[2] = start[2];
    gameState->y[3] = start[3];

    turns.push_back(gameState);

    while (turns.back()->x < level.length()) {

        GameState *gameState = turns.back();

        while (gameState->nextCommand() && gameState->turn < 50  ) {
            GameState *nextState = gameState->next(level);
            if (nextState->alive(V)) {
                turns.push_back(nextState);
                gameState = nextState;
            } else {
                delete nextState;
            }

            if ( gameState->x > level.length()){
                return turns;
            }
        }

        turns.pop_back();
        delete gameState;

    }
}

int main() {
    int M; // the amount of motorbikes to control
    cin >> M;
    cin.ignore();
    int V; // the minimum amount of motorbikes that must survive
    cin >> V;
    cin.ignore();
    string L0; // L0 to L3 are lanes of the road. A dot character . represents a safe space, a zero 0 represents a hole in the road.
    cin >> L0;
    cin.ignore();
    string L1;
    cin >> L1;
    cin.ignore();
    string L2;
    cin >> L2;
    cin.ignore();
    string L3;
    cin >> L3;
    cin.ignore();


    Level level = Level(L0, L1, L2, L3);

    bool start[4]= {true,true,true,true};

    vector<GameState*> turns = run(level,V,start);

    for (auto turn : turns) {

        switch (turn->currentCommand) {
            case UP:
                cout << "UP";
                break;
            case SPEED:
                cout << "SPEED";
                break;
            case SLOW:
                cout << "SLOW";
                break;
            case JUMP:
                cout << "JUMP";
                break;
            case DOWN:
                cout << "DOWN";
                break;
            case WAIT:
                cout << "WAIT";
                break;
        }
        cout << endl;
    }
    cout << turns.size() << endl;


}