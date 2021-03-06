#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

using namespace std;

const int CHECKPOINT_RADIUS = 600;
const int POD_RADIUS = 400;

class Unit;

struct Collision {
    const Unit *a;
    const Unit *b;
    double t{};

    Collision(const Unit *a, const Unit *b, double t) : a(a), b(b), t(t) {}
};

class Point {

public:
    double x, y;

    Point(double x, double y) : x(x), y(y) {}

    double distance2(const Point &other) const {
        return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
    }

    double distance(const Point &other) const {
        return sqrt(distance2(other));
    }

    Point closest(Point a, Point b) const {
        double da = b.y - a.y;
        double db = a.x - b.x;
        double c1 = da * a.x + db * a.y;
        double c2 = -db * this->x + da * this->y;
        double det = da * da + db * db;
        double cx = 0;
        double cy = 0;

        if (det != 0) {
            cx = (da * c1 - db * c2) / det;
            cy = (da * c2 + db * c1) / det;
        } else {
            // The point is already on the line
            cx = this->x;
            cy = this->y;
        }

        return {cx, cy};
    }

    friend Point operator+(const Point &p1, const Point &p2) {
        return Point(p1.x + p2.x, p1.y + p2.y);
    }

    friend Point operator-(const Point &p1, const Point &p2) {
        return Point(p1.x - p2.x, p1.y - p2.y);
    }

};

class Unit : public Point {
    const int id;
    static int _id;


public:
    double vx, vy, r;

    Unit(double x, double y, double r, double vx, double vy) : Point(x, y), id(_id++), r(r), vx(vx), vy(vy) {}

    Collision collision(Unit *u) const {
        // Square of the distance
        double dist = this->distance2(*u);

        // Sum of the radii squared
        double sr = (this->r + u->r) * (this->r + u->r);

        // We take everything squared to avoid calling sqrt uselessly. It is better for performances

        if (dist < sr) {
            // Objects are already touching each other. We have an immediate collision.
            return {this, u, 0.0};
        }

        // Optimisation. Objects with the same speed will never collide
        if (this->vx == u->vx && this->vy == u->vy) {
            return Collision(this, u, numeric_limits<double>::infinity());
        }

        // We place ourselves in the reference frame of u. u is therefore stationary and is at (0,0)
        double x = this->x - u->x;
        double y = this->y - u->y;
        Point myp = Point(x, y);
        double vx = this->vx - u->vx;
        double vy = this->vy - u->vy;
        Point up = Point(0, 0);

        // We look for the closest point to u (which is in (0,0)) on the line described by our speed vector
        Point p = up.closest(myp, Point(x + vx, y + vy));

        // Square of the distance between u and the closest point to u on the line described by our speed vector
        double pdist = up.distance2(p);

        // Square of the distance between us and that point
        double mypdist = myp.distance2(p);

        // If the distance between u and this line is less than the sum of the radii, there might be a collision
        if (pdist < sr) {
            // Our speed on the line
            double length = sqrt(vx * vx + vy * vy);

            // We move along the line to find the point of impact
            double backdist = sqrt(sr - pdist);
            p.x = p.x - backdist * (vx / length);
            p.y = p.y - backdist * (vy / length);

            // If the point is now further away it means we are not going the right way, therefore the collision won't happen
            if (myp.distance2(p) > mypdist) {
                return Collision(this, u, numeric_limits<double>::infinity());
            }

            pdist = p.distance(myp);

            // The point of impact is further than what we can travel in one turn
            if (pdist > length) {
                return Collision(this, u, numeric_limits<double>::infinity());
            }

            // Time needed to reach the impact point
            double t = pdist / length;

            return Collision(this, u, t);
        }

        return {this, u, numeric_limits<double>::infinity()};
    }

    bool is(int otherId) const {
        return otherId == this->id;
    }

    int getId() const {
        return id;
    }

    virtual void bouncePod(Unit *u) = 0;

    virtual void bounceWithCheckpoint(int checkpointId) = 0;

    virtual void bounceWithPod(Unit *u) = 0;


    friend ostream &operator<<(ostream &os, const Unit &unit) {
        os << unit.getId() << " (" << unit.x << "," << unit.y << ")" << " Speed: (" << unit.vx << "," << unit.vy << ")";
        return os;
    }


};

int Unit::_id = 0;

class Pod : public Unit {
    double angle;
    int nextCheckpointId;
    int checked, timeout;
    bool shield;

    bool passCheckpoint = false;

    double truncate(double val) {
        if (val >= 0) return floor(val);
        else return ceil(val);
    }

    static double getAngle(const Point &source, const Point &p) {
        double d = source.distance(p);

        double dx = (p.x - source.x) / d;
        double dy = (p.y - source.y) / d;

        // Simple trigonometry. We multiply by 180.0 / PI to convert radiants to degrees.
        double a = acos(dx) * 180.0 / M_PI;

        // If the point I want is below me, I have to shift the angle for it to be correct
        if (dy < 0) {
            a = 360.0 - a;
        }

        return a;
    }

public:

    double diffAngle(const Point &p) const {
        double a = Pod::getAngle(*this, p);
        // To know whether we should turn clockwise or not we look at the two ways and keep the smallest
        // The ternary operators replace the use of a modulo operator which would be slower
        double right = angle <= a ? a - angle : 360.0 - angle + a;
        double left = angle >= a ? angle - a : angle + 360.0 - a;

        if (right < left) {
            return right;
        } else {
            // We return a negative angle if we must rotate to left
            return -left;
        }
    }

    void rotate(const Point &p) {
        double a = diffAngle(p);
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
        double ra = angle * M_PI / 180.0;

        // Trigonometry
        this->vx += cos(ra) * thrust;
        this->vy += sin(ra) * thrust;
    }

    void move(double t) {
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

    void bounceWithCheckpoint(int checkpointId) override {
//        if (nextCheckpointId == checkpointId) {
//            checked++;
//            timeout = 100;
//        }
        timeout = 100;
        passCheckpoint = true;
    }

    void bounceWithPod(Unit *u) override {
        auto p = dynamic_cast<Pod *>(u);

        double m1 = this->shield ? 10 : 1;
        double m2 = p->shield ? 10 : 1;
        double mcoeff = (m1 + m2) / (m1 * m2);

        double nx = this->x - p->x;
        double ny = this->y - p->y;

        // Square of the distance between the 2 pods. This value could be hardcoded because it is always 800²
        double nxnysquare = nx * nx + ny * ny;

        double dvx = this->vx - p->vx;
        double dvy = this->vy - p->vy;

        // fx and fy are the components of the impact vector. product is just there for optimisation purposes
        double product = nx * dvx + ny * dvy;
        double fx = (nx * product) / (nxnysquare * mcoeff);
        double fy = (ny * product) / (nxnysquare * mcoeff);

        // We apply the impact vector once
        this->vx -= fx / m1;
        this->vy -= fy / m1;
        p->vx += fx / m2;
        p->vy += fy / m2;

        // If the norm of the impact vector is less than 120, we normalize it to 120
        double impulse = sqrt(fx * fx + fy * fy);
        if (impulse < 120.0) {
            fx = fx * 120.0 / impulse;
            fy = fy * 120.0 / impulse;
        }

        // We apply the impact vector a second time
        this->vx -= fx / m1;
        this->vy -= fy / m1;
        p->vx += fx / m2;
        p->vy += fy / m2;

        // This is one of the rare places where a Vector class would have made the code more readable.
        // But this place is called so often that I can't pay a performance price to make it more readable.
    }

    void bouncePod(Unit *u) override {
        u->bounceWithPod(u);
    }

    void bounce(Unit *u) {
        u->bouncePod(this);
    }

    Pod(double x, double y, double vx, double vy, double angle, int nextCheckpointId) : Unit(x, y, POD_RADIUS, vx, vy),
                                                                                        angle(angle),
                                                                                        nextCheckpointId(nextCheckpointId){}

    double score() const{
        return (passCheckpoint?50000:0) - this->distance(chec);

    }


};

class Checkpoint : public Unit {
public:
    Checkpoint(double x, double y) : Unit(x, y, CHECKPOINT_RADIUS, 0, 0) {}

    void bouncePod(Unit *u) override {
        u->bounceWithCheckpoint(getId());
    }

    void bounceWithCheckpoint(int checkpointId) override {

    }

    void bounceWithPod(Unit *u) override {

    }

    int location() const {
        return (int) x * 16000 + (int) y;
    }

};


class PodSetup {
    double lastX = 0, lastY = 0;
    bool first = true;

    double computeAngle(double vx, double vy) const {
        double angle = atan2(vy, vx) * 180 / M_PI;
        if (angle < 0) {
            angle = 360 + angle;
        }
        return angle;
    }

    static double truncate(double v) {
        return v >= 0 ? floor(v) : ceil(v);
    }

public:
    Pod getPod(int x, int y,int nextCheckpointId) {
        double vx = 0, vy = 0;

        if (first) {
            first = false;
        } else {
            vx = truncate((x - lastX) * 0.85);
            vy = truncate((y - lastY) * 0.85);
        }

        lastX = x;
        lastY = y;

        double angle = computeAngle(vx, vy);

        return Pod(x, y, vx, vy, angle,nextCheckpointId);
    }
};

class CheckpointSetup {
    map<pair<int, int>, pair<int, int>> next;
    pair<int, int> last;
    int first = true;
public:

    Checkpoint getCheckpoint(int x, int y) {
        pair<int, int> current = make_pair(x, y);

        if (first) {
            first = false;
        } else {
            if (current != last) {
                next[last] = current;
            }
        }

        last = current;

        return Checkpoint(x, y);


    }

    Checkpoint getCurrent() const {
        return Checkpoint(last.first, last.second);
    }

    bool hasNext(int x, int y) const {
        return next.find(make_pair(x, y)) != next.end();
    }

    Checkpoint nextCheckpoint(int x, int y) {
        auto nextPoint = next[make_pair(x, y)];
        return Checkpoint(nextPoint.first, nextPoint.second);
    }
};

class Simulator {
    CheckpointSetup &checkpointSetup;
    Pod &myPod;
    Pod &enemyPod;

public:
    Simulator(Pod &myPod, Pod &enemyPod, CheckpointSetup &checkpointSetup) : myPod(myPod), enemyPod(enemyPod),
                                                                             checkpointSetup(checkpointSetup) {

    }

    void play(Pod &myPod, Pod &enemyPod, CheckpointSetup &checkpointSetup) {
        auto collision = myPod.collision(&enemyPod);
        if (collision.t == numeric_limits<double>::infinity()) {

        }
    }

};

int main() {
    bool boostAvailable = false;

    auto myPodSetup = PodSetup();
    auto enemyPodSetup = PodSetup();
    auto checkpointSetup = CheckpointSetup();

    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle;
        cin.ignore();
        int opponentX;
        int opponentY;
        cin >> opponentX >> opponentY;
        cin.ignore();

        auto myPod = myPodSetup.getPod(x, y);
        auto enemyPod = enemyPodSetup.getPod(opponentX, opponentY);
        auto checkPoint = checkpointSetup.getCheckpoint(nextCheckpointX, nextCheckpointY);


        int nextX = nextCheckpointX, nextY = nextCheckpointY;

        int thrust = abs(nextCheckpointAngle) >= 90 ? 0 : 100;

        if (boostAvailable && abs(nextCheckpointAngle) < 5 && nextCheckpointDist > 1000) {
            boostAvailable = false;
            cout << nextX << " " << nextY << " " << "BOOST" << endl;
        } else {

            cout << nextX << " " << nextY << " " << thrust << endl;

        }

    }
}