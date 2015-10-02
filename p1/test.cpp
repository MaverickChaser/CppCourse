// class A;
#include <cstdio>
#include <vector>
using namespace std;


struct Pointer {
    int index;
    int get() { return index; } 
};

Pointer *p;


int main() {
    vector<int> v = {1, 2, 3};
    int *x = new int(500);
    v.push_back(*x);

    //int v[] = {1, 2, 3};
    for (int i = 0; i < v.size(); i++) {
        printf("%lld\n", &v[i]);
    }
    printf("%lld\n", x);
    delete x;
    return 0;
}



