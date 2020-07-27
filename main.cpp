#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
using namespace std;

struct Point{
    float x,  y;
    Point() :x(0),y(0){}
    Point(float x, float y): x(x),y(y){}

    friend Point operator+(const Point& p1, const Point& p2){
        return Point(p1.x + p2.x,p1.y + p2.y);
    }

    friend Point operator-(const Point& p1, const Point& p2){
        return Point(p1.x - p2.x,p1.y - p2.y);
    }

    friend Point operator*(const Point&p1, float val){
        return Point(p1.x * val, p1.y * val);
    }

    float magnitude() const{
        return sqrt(x*x + y* y);
    }

    float angle() const{
        return atan2(y,x);
    }

    Point rotate(float angle){
        return Point(x * cos(angle) - y * sin(angle),y * cos(angle) + x * sin(angle));
    }

    friend ostream &operator<<(ostream &os, const Point &point) {
        os << "(" << point.x << "," << point.y <<")";
        return os;
    }

};

int main()
{

    Point last;
    bool  first = true;
    bool boost = true;
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

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        auto myPod= Point(x,y);
        auto enemyPod = Point(opponentX,opponentY);
        auto checkpoint = Point(nextCheckpointX,nextCheckpointY);

        Point speed;

        if (first){
            first = false;
        }else{
            speed = (myPod - last) * 0.85;
        }
        last = myPod;

        float tolerance = 360 * min(1.0,nextCheckpointDist / 6000.0) ;
        cerr << speed << endl;
        cerr << "Tolerance: " << tolerance <<endl;
        cerr  << "Speed angle: " << speed.angle() * 180 / M_PI << " Target Angle:" << (checkpoint - myPod).angle() * 180 / M_PI <<endl;
        float speedAngle = abs(speed.angle() - (checkpoint - myPod).angle()) * 180 / M_PI;

        int thrust =abs(nextCheckpointAngle) >90? 0:(int)(75 * min(1.0,nextCheckpointDist / 5000.0) +25);


//        if (speedAngle > tolerance){
//            thrust = 25;
//        }

        Point target;

        if ((myPod - enemyPod).magnitude() < 1500){
            auto myPodAngle = (myPod - checkpoint).angle();
            auto enemy = (enemyPod - checkpoint).rotate(-myPodAngle);
            target = Point(0,enemy.y>0?400:-400);
            target = target.rotate(myPodAngle);
            target = target + checkpoint;
        }else{
            target = checkpoint;
        }

        if (boost && nextCheckpointDist > 3000 && speedAngle < 5){
            boost= false;
            cout << round(target.x) << " " << round(target.y) << " BOOST" << endl;
        }else{
            cout << round(target.x) << " " << round(target.y) << " " << thrust << endl;
        }

    }
}