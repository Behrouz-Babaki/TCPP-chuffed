#ifndef TUPLE_H
#define TUPLE_H

#include <iostream>

using namespace std;

class Tuple
{
    int op1;
    int op2;
    int size;
    int *values;
    public:
        Tuple();
        Tuple(int s);
        Tuple(int s, int *v);
        virtual ~Tuple();
        Tuple &operator=(Tuple &);
        bool operator==(Tuple&);

    friend ostream &operator<<(ostream &,Tuple &);
    friend class NegConstraint;
    friend class PLProblem;
};

#endif // TUPLE_H
