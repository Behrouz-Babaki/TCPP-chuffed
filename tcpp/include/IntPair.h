#ifndef INTPAIR_H
#define INTPAIR_H
#include <iostream>

using namespace std;

class IntPair
{
    int code;
    int value;
    public:
        IntPair()
        {
          code = 0;
          value = 0;
        }
        IntPair(int c, int v)
        {
          code = c;
          value = v;
        }

    friend ostream &operator<<(ostream &out, IntPair &p);
    friend class PLProblem;
};

#endif // INTPAIR_H
