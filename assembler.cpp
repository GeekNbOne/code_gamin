#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;


class Compute{

    vector<Compute*> overlaps;
    bool active = true;

public:

    void unsubscribe(Compute* ref){
        overlaps.erase(remove(overlaps.begin(), overlaps.end(), ref), overlaps.end());
    }

    int overlapCount() const{
        return overlaps.size();
    }

    void deactivate(){
        active = false;
        for (auto overlap:overlaps){
            unsubscribe(this);
        }
        overlaps.clear();
    }

    void choose(){
        for (auto overlap:overlaps){
            overlap->deactivate();
        }
    }

};


int main()
{
    int N;
    cin >> N; cin.ignore();
    
    for (int i = 0; i < N; i++) {
        int J;
        int D;
        cin >> J >> D; cin.ignore();

    }


}