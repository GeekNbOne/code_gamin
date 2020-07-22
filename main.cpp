#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;
class Collision{

};


class Point{

public:
    float x,y;
    Point(float x, float y) : x(x), y(y) {}

    float distance2(const Point& other) const{
        return (x-other.x)*(x-other.x)+(y-other.y)+(y-other.y);
    }

    float distance(const Point& other) const{
        return sqrt(distance2(other));
    }

};

class Unit:public Point{
    int id;

public:
    float r,vx,vy;

    Unit(float x, float y, int id, float r, float vx, float vy) : Point(x, y), id(id), r(r), vx(vx), vy(vy) {}

    Collision collision(Unit* u) const{}
    virtual void bounce(Unit* u)=0;

};

class Checkpoint: public Unit{
public:
    void bounce(Unit *u) override {

    }
};

class Pod:public Unit{
    float angle;
    int nextCheckpointId;
    int checked,timeout;
    Pod* partner;
    bool shield;

    double truncate(double val){
        if(val>=0) return floor(val);
        else return ceil(val);
    }
public:
    float getAngle(const Point& p) const{
        float d = this->distance(p);
        float dx = (p.x - this->x) / d;
        float dy = (p.y - this->y) / d;

        // Simple trigonometry. We multiply by 180.0 / PI to convert radiants to degrees.
        float a = acos(dx) * 180.0 / M_PI;

        // If the point I want is below me, I have to shift the angle for it to be correct
        if (dy < 0) {
            a = 360.0 - a;
        }

        return a;
    }

    float diffAngle(Point p) {
        float a = this->getAngle(p);

        // To know whether we should turn clockwise or not we look at the two ways and keep the smallest
        // The ternary operators replace the use of a modulo operator which would be slower
        float right = angle <= a ? a - angle : 360.0 - angle + a;
        float left = angle >= a ? angle - a : angle + 360.0 - a;

        if (right < left) {
            return right;
        } else {
            // We return a negative angle if we must rotate to left
            return -left;
        }
    }

    void rotate(Point p) {
        float a = diffAngle(p);

        // Can't turn by more than 18° in one turn
        if (a > 18.0) {
            a = 18.0;
        } else if (a < -18.0) {
            a = -18.0;
        }

        angle += a;

        // The % operator is slow. If we can avoid it, it's better.
        if (angle >= 360.0) {
            angle = angle - 360.0;
        } else if (angle < 0.0) {
            angle += 360.0;
        }
    }

    void boost(int thrust) {
        // Don't forget that a pod which has activated its shield cannot accelerate for 3 turns
        if (shield) {
            return;
        }

        // Conversion of the angle to radiants
        float ra = angle * M_PI / 180.0;

        // Trigonometry
        this->vx += cos(ra) * thrust;
        this->vy += sin(ra) * thrust;
    }

    void move(float t) {
        this->x += this->vx * t;
        this->y += this->vy * t;
    }

    void end() {
        x = round(x);
        y = round(y);
        vx = truncate(vx * 0.85);
        vy = truncate(vy * 0.85);

        // Don't forget that the timeout goes down by 1 each turn. It is reset to 100 when you pass a checkpoint
        timeout -= 1;
    }

    void play(Point p, int thrust) {
        rotate(p);
        boost(thrust);
        move(1.0);
        end();
    }
};



int main()
{
    bool first = true;
    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        int opponentX;
        int opponentY;
        cin >> opponentX >> opponentY; cin.ignore();

        auto checkPoint= Unit(nextCheckpointX,nextCheckpointY,600);

        int thrust;

        if (nextCheckpointAngle > 90||nextCheckpointAngle < -90){
            thrust =0;
        } else{
            thrust=100;
        }
        if (first){
            first = false;
            cout << nextCheckpointX << " " << nextCheckpointY << " " << "BOOST" <<endl;
        }else{
            cout << nextCheckpointX << " " << nextCheckpointY << " " << thrust <<endl;
        }

    }
}