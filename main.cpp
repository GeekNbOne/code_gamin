#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;


long Fibonnaci[] = {
        1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 46368,
        75025, 121393, 196418, 317811, 514229, 832040, 1346269, 2178309, 3524578, 5702887, 9227465, 14930352, 24157817,
        39088169, 63245986, 102334155, 165580141, 267914296, 433494437, 701408733, 1134903170, 1836311903, 2971215073,
        4807526976, 7778742049, 12586269025, 20365011074, 32951280099, 53316291173, 86267571272, 139583862445,
        225851433717, 365435296162, 591286729879, 956722026041, 1548008755920, 2504730781961, 4052739537881,
        6557470319842, 10610209857723, 17167680177565, 27777890035288, 44945570212853, 72723460248141, 117669030460994,
        190392490709135, 308061521170129, 498454011879264, 806515533049393, 1304969544928657, 2111485077978050,
        3416454622906707, 5527939700884757, 8944394323791464, 14472334024676221, 23416728348467685, 37889062373143906,
        61305790721611591, 99194853094755497, 160500643816367088, 259695496911122585, 420196140727489673,
        679891637638612258, 1100087778366101931, 1779979416004714189, 2880067194370816120, 4660046610375530309,
        7540113804746346429
};

struct Point {
    int x, y;

    Point(int x, int y) : x(x), y(y) {}

    float distance(Point &unit) const {
        return sqrt((x - unit.x) * (x - unit.x) + (y - unit.y) * (y - unit.y));
    }

    Point &operator+=(const Point &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    friend Point operator-(Point lhs, const Point &rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y};
    }

    friend Point operator+(Point lhs, const Point &rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    string toString() const {
        return to_string(x) + " " + to_string(y);
    }

    float magnitude() const {
        return sqrt(x * x + y * y);
    }

    Point clamp(float distance) {
        float m = magnitude();
        if (m > distance) {
            return {(int) (((float) x) * distance / m), (int) (((float) y) * distance / m)};
        }

        return *this;
    }


};

class Unit {

    Point position;

public:
    Unit(int x, int y) : position(Point(x, y)) {}

    explicit Unit(Point point) : position(point) {}

    Unit(const Unit &u) : Unit(u.position) {}

    int x() const { return position.x; }

    int y() const { return position.y; }

    float distance(Unit &unit) const {
        return position.distance(unit.position);
    }

    float distance(Unit *unit) const {
        return position.distance(unit->position);
    }

    void setPosition(Point dest) {
        position = dest;
    }

    Point getPosition() const {
        return position;
    }

    void move(Point dest, float moveDistance) {
        position += (dest - position).clamp(moveDistance);
    }

    string posString() const {
        return position.toString();
    }
};

class Ash : public Unit {

    const int moveDistance = 1000;
public:
    Ash(int x, int y) : Unit(x, y) {}

    Ash(Point point) : Unit(point) {}

    Ash(const Ash &ash) : Ash(ash.getPosition()) {}

    void simMove(Point dest) {
        move(dest, moveDistance);
    }

    friend ostream &operator<<(ostream &os, const Ash &ash) {
        os << "Ash: " << ash.posString();
        return os;
    }

};

class UnitID : public Unit {
    int id;

public:
    UnitID(int id, int x, int y) : id(id), Unit(x, y) {}

    UnitID(int id, Point position) : id(id), Unit(position) {}

    UnitID(const UnitID &unitId) : UnitID(unitId.id, unitId.getPosition()) {}

    int getId() const {
        return id;
    }

    string toString() const {
        return "(" + to_string(id) + "): " + posString();
    }

    friend ostream &operator<<(ostream &os, const UnitID &id) {
        os << "(" << to_string(id.id) << "): " << id.posString();
        return os;
    }
};

class Human : public UnitID {

public:
    Human(int id, int x, int y) : UnitID(id, x, y) {}

    Human(int id, Point position) : UnitID(id, position) {}

    Human(const Human &h) : Human(h.getId(), h.getPosition()) {}

    friend ostream &operator<<(ostream &os, const Human &human) {
        os << "Human " << human.toString();
        return os;
    }

};

class Zombie : public UnitID {
    Point next;
public:
    Zombie(int id, int x, int y, int nextX, int nextY) : UnitID(id, x, y), next(nextX, nextY) {}

    Zombie(int id, Point position, Point next) : UnitID(id, position), next(next) {}

    Zombie(const Zombie &zombie) : Zombie(zombie.getId(), zombie.getPosition(), zombie.next) {}

    void setNext(Point nextPosition) {
        next = nextPosition;
    }

    void simMove() {
        move(next, 400);
    }

    friend ostream &operator<<(ostream &os, const Zombie &zombie) {
        os << "Zombie " << zombie.toString();
        return os;
    }
};

class Round {

    Ash *ash;
    vector<Human *> humans;
    vector<Zombie *> zombies;

    void zombiesMove() {
        for (auto z: zombies) {
            z->simMove();
        }
    }

    void zombiesFindNext() {

        for (auto z: zombies) {
            z->setNext(findZombieNext(z));
        }
    }

    Point findZombieNext(Zombie *z) {
        float minDistance = z->distance(ash);
        Unit *target = ash;

        for (auto h :humans) {
            float hz = z->distance(h);
            if (hz < minDistance) {
                minDistance = hz;
                target = h;
            }
        }
        return target->getPosition();
    }

public:

    Round() : humans(vector<Human *>()), zombies(vector<Zombie *>()) {}

    Round(const Round &r) : humans(vector<Human *>()), zombies(vector<Zombie *>()) {
        ash = new Ash(*(r.ash));

        for (auto h: r.humans) {
            humans.emplace_back(new Human(*h));
        }

        for (auto z: r.zombies) {
            zombies.emplace_back(new Zombie(*z));
        }
    }

    long sim(const vector<const Point> &ashDest) {
        long tot = 0;
        for (auto a:ashDest) {
            tot += simRound(a);
        }
        return tot;
    }

    long simRound(const Point ashDest) {

        long point = 0;

        //Find Zombie next
        zombiesFindNext();
        //Zombies move
        zombiesMove();
        //Ash moves
        ash->simMove(ashDest);
        //Ash destroy zombies

        vector<Zombie *> zombiesToDestroy = vector<Zombie *>();
        long humanMult = humans.size();
        int combo = 0;
        for (auto z:zombies) {
            if (z->distance(ash) <= 2000) {
                point += (humanMult * 10) * Fibonnaci[combo++];
                zombiesToDestroy.emplace_back(z);
            }
        }

        for (auto z:zombiesToDestroy) {
            zombies.erase(remove(zombies.begin(), zombies.end(), z), zombies.end());
        }

        //Zombies eats
        vector<Human *> humanAlive = vector<Human *>();

        for (auto h:humans) {
            bool toEat = false;
            for (auto z: zombies) {
                if (z->distance(h) < 400) {
                    toEat = true;
                    break;
                }
            }
            if(!toEat){
                humanAlive.emplace_back(h);
            }
        }
        humans= humanAlive;

        return point;
    }

    Zombie *closestZombie() {
        return *(min_element(zombies.begin(), zombies.end(),
                             [&](Zombie *z1, Zombie *z2) { return ash->distance(z1) < ash->distance(z2); }));
    }

    Point ZombieCentroid() const {

        float zx = 0, zy = 0;
        for (auto z : zombies) {
            zx += z->x();
            zy += z->y();
        }

        return Point(zx / zombies.size(), zy / zombies.size());
    }

    void addAsh(int x, int y) {
        ash = new Ash(x, y);
    }

    void addHuman(int id, int x, int y) {
        humans.emplace_back(new Human(id, x, y));
    }

    void addZombie(int id, int x, int y, int nextX, int nextY) {
        zombies.emplace_back(new Zombie(id, x, y, nextX, nextY));
    }

    friend ostream &operator<<(ostream &os, const Round &round) {

        os << (*round.ash) << endl;

        for (auto h: round.humans)
            os << (*h) << endl;

        for (auto z: round.zombies)
            os << (*z) << endl;
        return os;
    }


};

class AI {
public:
    virtual string next(Round &round) = 0;
};


class SimpleAI : public AI {

    /// Follow the zombies centroid

public:
    string next(Round &round) {

        Round rSim = round;

        cerr << "Pre sim r2" << endl << rSim << endl;

        for(int i =0 ; i < 10; i++){
            rSim.simRound(Point(8250, 4500));
            cerr << "Sim " << i + 1 << endl << rSim << endl;
        }





        return round.ZombieCentroid().toString();
    }
};


int main() {

    AI *ai = new SimpleAI;
    // game loop
//    while (1)
    {

        Round round = Round();
        int x;
        int y;
        cin >> x >> y;
        cin.ignore();

        cerr << x << " " << y << endl;

        round.addAsh(x, y);

        int humanCount;
        cin >> humanCount;
        cin.ignore();

        cerr << humanCount << endl;
        for (int i = 0; i < humanCount; i++) {
            int humanId;
            int humanX;
            int humanY;
            cin >> humanId >> humanX >> humanY;
            cin.ignore();
            round.addHuman(humanId, humanX, humanY);

            cerr << humanId << " " << humanX << " " << humanY << endl;

        }
        int zombieCount;
        cin >> zombieCount;
        cin.ignore();
        cerr << zombieCount << endl;
        for (int i = 0; i < zombieCount; i++) {
            int zombieId;
            int zombieX;
            int zombieY;
            int zombieXNext;
            int zombieYNext;
            cin >> zombieId >> zombieX >> zombieY >> zombieXNext >> zombieYNext;
            cin.ignore();

            round.addZombie(zombieId, zombieX, zombieY, zombieXNext, zombieYNext);


            cerr << zombieId << " " << zombieX << " " << zombieY << " " <<
                 zombieXNext << " " << zombieYNext << " " << endl;


        }

        cout << ai->next(round) << endl; // Your destination coordinates
    }
}