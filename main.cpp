#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

struct Point{
    int x,y;

    Point(int x, int y): x(x),y(y){}

    float distance(Point& unit) const{
        return sqrt((x - unit.x) * ( x - unit.x) + (y - unit.y) * ( y - unit.y) );
    }

    Point& operator+=(const Point& other){
        x += other.x;
        y += other.y;
        return *this;
    }

    friend Point operator-(Point lhs, const Point& rhs){
        return {lhs.x - rhs.x, lhs.y - rhs.y};
    }
    friend Point operator+(Point lhs, const Point& rhs){
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    string toString() const {
        return to_string(x) + " " + to_string(y);
    }

    float magnitude() const{
        return sqrt(x*x + y*y);
    }

    Point clamp(float distance){
        float m = magnitude();
        if(m > distance){
            return {(int)(((float) x ) * distance /m), (int)(((float) y ) * distance /m)};
        }

        return *this;
    }


};

class Unit{

    Point position;

public:
    Unit(int x, int y): position(Point(x, y)){}

    explicit Unit(Point point): position(point){}

    Unit( const  Unit& u):Unit(u.position){}

    int x()const{return position.x;}
    int y() const {return position.y;}

    float distance(Unit& unit) const{
        return position.distance(unit.position);
    }

    float distance(Unit* unit)const{
        return position.distance(unit->position);
    }

    void setPosition(Point dest){
        position = dest;
    }

    Point getPosition()const{
        return position;
    }

    void move(Point dest,float moveDistance){
        position += (dest - position).clamp(moveDistance);
    }

    string posString()const{
        return position.toString();
    }
};

class Ash : public Unit{

    const int moveDistance = 1000;
public:
    Ash(int x, int y) : Unit(x, y) {}

    Ash(Point point):Unit(point){}

    Ash(const Ash& ash): Ash(ash.getPosition()){}

    void simMove(Point dest){
        move(dest,moveDistance);
    }

    friend ostream &operator<<(ostream &os, const Ash &ash) {
        os << "Ash: " << ash.posString();
        return os;
    }

};

class UnitID: public Unit{
    int id;

public:
    UnitID(int id,int x, int y):id(id),Unit(x,y){}

    UnitID(int id, Point position):id(id),Unit(position){}

    UnitID(const UnitID& unitId): UnitID(unitId.id,unitId.getPosition()){}

    int getId()const{
        return id;
    }
    string toString() const{
        return "(" + to_string(id) + "): " + posString();
    }

    friend ostream &operator<<(ostream &os, const UnitID &id) {
        os << "(" << to_string(id.id) << "): " << id.posString();
        return os;
    }
};

class Human: public UnitID{

public:
    Human(int id,int x, int y) : UnitID(id,x, y) {}

    Human(int id,Point position ) : UnitID(id,position){}

    Human(const Human& h) :Human(h.getId(),h.getPosition()){}

    friend ostream &operator<<(ostream &os, const Human &human) {
        os << "Human " << human.toString() ;
        return os;
    }

};

class Zombie : public UnitID{
    Point next;
public:
    Zombie(int id, int x, int y,int nextX, int nextY) : UnitID(id,x, y),next(nextX,nextY) {}

    Zombie(int id, Point position, Point next):UnitID(id,position),next(next){}

    Zombie(const Zombie& zombie): Zombie(zombie.getId(), zombie.getPosition(), zombie.next){}

    void setNext(Point nextPosition){
        next = nextPosition;
    }

    void simMove(){
        move(next,400);
    }

    friend ostream &operator<<(ostream &os, const Zombie &zombie) {
        os << "Zombie " << zombie.toString();
        return os;
    }
};

class Round{

    Ash* ash;
    vector<Human*> humans = vector<Human*>();
    vector<Zombie*> zombies =  vector<Zombie*>();

    void zombiesMove(){
        for(auto z: zombies){
            z->simMove();
        }
    }
    void zombiesFindNext(){

        for(auto z: zombies){
            z->setNext(findZombieNext(z));
        }
    }

    Point findZombieNext(Zombie* z){
        float minDistance = z->distance(ash);
        Unit* target = ash;

        for(auto h :humans){
            float hz = z->distance(h);
            if(hz < minDistance){
                minDistance = hz;
                target = h;
            }
        }

        cout << target->posString() << endl;

        return target->getPosition();
    }

public:

    Round() = default;

    Round(const Round& r) {
        ash = new Ash(*(r.ash));
        for(auto h: r.humans){
            humans.emplace_back(new Human(*h));
        }

        for(auto z: r.zombies){
            zombies.emplace_back(new Zombie(*z));
        }
    }

    int simRound(Point ashDest){
        //Find Zombie next
        zombiesFindNext();
        //Zombies move
        zombiesMove();
        //Ash moves
        ash->simMove(ashDest);
        //Ash destroy zombies
        //Remove zombies from vector and accum points
        //Zombies eats
        //Remove human from vector if share coord
        return 0;
    }
    Zombie* closestZombie(){
        return *(min_element(zombies.begin(), zombies.end(),
                             [&](Zombie* z1, Zombie* z2){return ash->distance(z1) < ash->distance(z2);}));
    }

    Point ZombieCentroid() const{

        float zx=0, zy=0;
        for(auto z : zombies){
            zx += z->x();
            zy+= z->y();
        }

        return Point(zx / zombies.size(), zy / zombies.size());
    }

    void addAsh(int x, int y){
        ash = new Ash(x,y);
    }

    void addHuman(int id, int x, int y){
        humans.emplace_back(new Human(id,x,y));
    }

    void addZombie(int id, int x, int y, int nextX, int nextY){


        zombies.emplace_back(new Zombie(id,x,y,nextX,nextY));
        zombies.back()->setPosition(Point(8250,4500));
        cerr << zombies.back()->posString() << endl;
    }

    friend ostream &operator<<(ostream &os, const Round &round) {

        os << (*round.ash) <<endl;

        for(auto h: round.humans)
            os << (*h) << endl;

        for(auto z: round.zombies)
            os << (*z )<< endl;
        return os;
    }


};

class AI{
public:
    virtual string next(Round round) =0;
};


class SimpleAI :public AI{

    /// Follow the zombies centroid

public:
    string next(Round round){
        return round.ZombieCentroid().toString();
    }
};
int main()
{

    AI* ai = new SimpleAI;
    // game loop
//    while (1)
    {

        Round round = Round();
        int x;
        int y;
        cin >> x >> y; cin.ignore();

        cerr << x << " " << y <<endl;

        round.addAsh(x, y);

        int humanCount;
        cin >> humanCount; cin.ignore();

        cerr << humanCount <<endl;
        for (int i = 0; i < humanCount; i++) {
            int humanId;
            int humanX;
            int humanY;
            cin >> humanId >> humanX >> humanY; cin.ignore();
            round.addHuman(humanId, humanX, humanY);

            cerr << humanId << " " << humanX << " " << humanY <<endl;

        }
        int zombieCount;
        cin >> zombieCount; cin.ignore();
        cerr <<zombieCount << endl;
        for (int i = 0; i < zombieCount; i++) {
            int zombieId;
            int zombieX;
            int zombieY;
            int zombieXNext;
            int zombieYNext;
            cin >> zombieId >> zombieX >> zombieY >> zombieXNext >> zombieYNext; cin.ignore();

            round.addZombie(zombieId, zombieX, zombieY, zombieXNext, zombieYNext);


            cerr << zombieId << " " << zombieX << " " <<zombieY << " " <<
                 zombieXNext <<" "<< zombieYNext << " "<<endl;


        }

//        Round r2 = round;
//
//        cout << "Pre sim r2"<<endl << r2 <<endl;
//
//        r2.simRound(Point(5000,5000));
//
//        cout << "Post sim r2 "<<endl << r2 <<endl;

        cout << "Original r" <<endl << round << endl;


        cout << ai->next(round) << endl; // Your destination coordinates
    }
}