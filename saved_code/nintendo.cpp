#include <iostream>
#include <algorithm>
#include <bitset>
#include <cmath>
#include <random>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace chrono;

class GF2 {
    static const unsigned int SIZE = 1024;
    bitset<SIZE> repr = bitset<SIZE>();
    unsigned int degree;

    static bitset<SIZE> fromInt(unsigned int value) {
        bitset<SIZE> newBitset = bitset<SIZE>();

        for (unsigned int i = 0; i < 32; i++) {
            newBitset[i] = value & (1u << i);
        }

        return newBitset;
    }

    void recomputeDegree() {
        for (unsigned int i = 0; i < SIZE; i++) {
            if (repr[SIZE - i - 1]) {
                degree = SIZE - i - 1;
                return;
            }
        }
        degree = 0;
    }

public:
    unsigned int getDegree() const {
        return degree;
    }

    explicit GF2(unsigned int value) : repr(fromInt(value)), degree((int) log2(value)) {}

    GF2(bitset<SIZE> repr, unsigned int degree) : repr(repr), degree(degree) {}

    GF2(unsigned int targetDegree, double p) {

        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution d(p);

        degree = 0;
        for (int n = 0; n < targetDegree; ++n) {
            repr[n] = d(gen);
            if (repr[n]) degree = n;
        }

    }

    GF2(const GF2 &other) = default;

    friend ostream &operator<<(ostream &os, const GF2 &ff2) {
        os << ff2.repr;
        return os;
    }

    GF2 diff() const {
        auto o = (*this) >> 1;
        for (int i = 0; i < SIZE; i++) {
            if (i % 2 == 0) {
                o.repr[i] = false;
            }
        }
        return o;
    }

    bool isSquareFree() const;

    GF2 friend operator+(const GF2 &lhs, const GF2 &rhs) {
        auto output = lhs;
        output.repr ^= rhs.repr;
        output.recomputeDegree();
        return output;
    }

    GF2 &operator+=(const GF2 &rhs) {
        repr ^= rhs.repr;
        recomputeDegree();
        return *this;
    }

    friend GF2 operator<<(const GF2 &lhs, unsigned int rhs) {
        return GF2(lhs.repr << rhs, lhs.degree + 1);
    }

    friend GF2 operator>>(const GF2 &lhs, unsigned int rhs) {
        return GF2(lhs.repr >> rhs, max(lhs.degree - 1, 0u));
    }

    friend GF2 operator*(const GF2 &lhs, const GF2 &rhs) {
        bitset<SIZE> repr;
        for (int i = 0; i <= rhs.degree; i++) {
            if (rhs.repr[i]) {
                repr ^= (lhs.repr << i);
            }
        }
        return GF2(repr, lhs.degree + rhs.degree);
    }

    friend GF2 operator/(const GF2 &lhs, const GF2 &rhs) {

        if (lhs < rhs) { return GF2(0); }
        GF2 op = lhs;
        bitset<SIZE> repr;

        while (op.repr.any() && op >= rhs) {
            auto degDiff = op.degree - rhs.degree;
            repr[degDiff] = true;
            op.repr ^= (rhs.repr << degDiff);
            op.recomputeDegree();

        }

        return GF2(repr, lhs.degree - rhs.degree);
    }

    friend GF2 operator%(const GF2 &lhs, const GF2 &rhs) {

        if (lhs < rhs) { return GF2(lhs); }

        GF2 op = lhs;
        while (op.repr.any() && op >= rhs) {
            op.repr ^= (rhs.repr << (op.degree - rhs.degree));
            op.recomputeDegree();
        }

        return op;
    }

    friend GF2 operator&(const GF2 &lhs, const GF2 &rhs) {

        auto o = lhs;
        o.repr &= rhs.repr;
        o.recomputeDegree();
        return o;
    }

    bool operator<(const GF2 &rhs) const {

        if (degree == rhs.degree) {
            unsigned int d = degree;
            while (d > 0) {
                d--;
                if (repr[d] && !rhs.repr[d]) {
                    return false;
                } else if (!repr[d] && rhs.repr[d]) {
                    return true;
                }
            }
            return false;
        } else {
            return degree < rhs.degree;
        }
    }

    bool operator>(const GF2 &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const GF2 &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const GF2 &rhs) const {
        return !(*this < rhs);
    }

    bool operator==(const GF2 &rhs) const {
        return rhs.repr == repr;
    }

    bool operator[](unsigned int index) const {
        return repr[index];
    }

    void setBit(unsigned int index) {
        repr[index] = true;
        recomputeDegree();
    }

    bool equalsOne() const {
        return degree == 0 && repr[0];
    }

    bool equalsZero() const {
        return degree == 0 && !repr[0];
    }

    bool operator==(const unsigned int &rhs) const {
        return (repr == bitset<SIZE>(rhs));
    }

    unsigned int to_ulong() const {
        return repr.to_ulong();
    }

    [[nodiscard]] unsigned int *split(unsigned int parts) const {
        auto *arr = new unsigned int[parts];
        unsigned int mask = 4294967295;
        for (int i = 0; i < parts; i++) {
            arr[i] = ((repr << SIZE - 32 * (i + 1)) >> (SIZE - 32)).to_ulong() & mask;
        }
        return arr;
    }

};

GF2 gcd(const GF2 &lhs, const GF2 &rhs) {
    if (rhs.equalsZero()) {
        return GF2(lhs);
    } else {
        return gcd(rhs, lhs % rhs);
    }
}

bool GF2::isSquareFree() const {
    GF2 g = gcd(*this, diff());
    return g.equalsOne() || g == (*this);
}

GF2 rs(const GF2 &base, const GF2 & fx, const GF2 &exp) {

    auto g = exp[0] ? base : GF2(1);
    auto gi = base;

    for (int i = 1; i <= exp.getDegree(); i++) {
        gi = (gi * gi) % fx;
        if (exp[i]) {
            g = (g * gi) % fx;
        }
    }

    return g;

}

vector<GF2> ddf(const GF2 &f) {

    auto fi = f;
    vector<GF2> res;
    auto si  = GF2(2);
    for (unsigned int i = 0; i < f.getDegree(); i++) {

        auto exp = GF2(0);
        exp.setBit(i + 1);
        si = (si * si)%f;
        auto gi = gcd(si + GF2(2), fi);
        res.emplace_back(gi);
        if (gi.getDegree() > 0){
            return res;
        }
        fi = fi / gi;
    }

    return res;
}

void cz(const GF2 &A, unsigned int d, vector<GF2> &res) {
    if (A.getDegree() == d) {
        res.emplace_back(A);
        return;
    }

    bool notFinished = true;

    while (notFinished) {
        auto T = GF2(A.getDegree(), 0.5);
        auto W = T;
        for (int i = 0; i < (d - 1); i++) {
            T = (T * T) % A;
            W = W + T;
        }
        auto U = gcd(A, W);
        if (U.getDegree() > 0 && U.getDegree() < A.getDegree()) {
            cz(U, d, res);
            cz(A / U, d, res);
            notFinished = false;
        }
    }

}

vector<GF2> factorize(const GF2 &A) {
    vector<GF2> res;
    cz(A, A.getDegree() / 2, res);
    return res;

}


int main() {


    int S;
    cin >> S;
    cin.ignore();
    string line;
    getline(cin, line);

    GF2 val = GF2(0);

    bool hasZero = false;

    //pack
    for (int i = 0; i < S / 16; i++) {
        unsigned int x;
        stringstream ss;
        ss << std::hex << line.substr(9 * i, 8);
        ss >> x;
        if ( x == 0){
            hasZero = true;
        }
        val += (GF2(x) << (i * 32));
    }

    auto res = factorize(val);

    auto mask = GF2(pow(2, 32) - 1);

    vector<string> answer;

    vector<string> parts;
    for (auto r:res) {
        stringstream ss;
        for (int i = 0; i < (S / 32); i++) {
            ss << setfill('0') << setw(8) << hex << ((r & (mask << (i * 32))) >> (i * 32)).to_ulong() << " ";
        }
        parts.emplace_back(ss.str());
    }

    answer.emplace_back(parts[0] + parts[1].substr(0,S/32 * 9-1));
    answer.emplace_back(parts[1] + parts[0].substr(0,S/32 * 9-1));

    if (hasZero){
        stringstream ss;
        ss << setfill('0') << setw(8) << hex << val.to_ulong();
        answer.emplace_back("00000001 " + ss.str());
        answer.emplace_back(ss.str() + " " + "00000001");
    }
    sort(answer.begin(), answer.end());

    for (auto a : answer) {
        cout << a << endl;
    }


}


