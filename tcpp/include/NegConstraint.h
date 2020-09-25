#ifndef NEGCONSTRAINT_H
#define NEGCONSTRAINT_H

#include <iostream>

#include "NodeList.h"
#include "Tuple.h"

using namespace std;

class NegConstraint
{
    int size;     /// size of var_list
    int v_size;   /// (size of var_loc)/2 = var_size of PLProblem
    int *var_loc;
    int *var_list;
    NodeList<Tuple> tuples;
    public:
        NegConstraint();
        NegConstraint(NegConstraint &C);
        NegConstraint(int s, int var_size);
        NegConstraint(int s, int var_size, int *vlist);
        NegConstraint &operator=(NegConstraint &);
        bool operator==(NegConstraint &);
        void permut_tuple(NegConstraint &, Tuple &);
        void add_tuple(Tuple &);
        virtual ~NegConstraint();

    friend ostream &operator<<(ostream &,NegConstraint &);
    friend class PLProblem;
};

#endif // NEGCONSTRAINT_H
