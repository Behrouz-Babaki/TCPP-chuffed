#ifndef VARIABLE_H_INCLUDED
#define VARIABLE_H_INCLUDED

#include <iostream>

#include "IntPair.h"
#include "NegConstraint.h"

using namespace std;

class Variable
{
    int id;
    int range;
    int nb_num;
    int *nb_loc;
    int *nb_list;
    int **tuples;
    int **auxCol;
    bool *aux;
    int auxDom;
    bool *eq_pp;
    int MaxCoNum;

    unsigned long int edge_num;
    Node<NegConstraint> **MxNegC;
    NodeList<int> *InMxGroups;
    NodeList<int> *InPrevails;
    NodeList<IntPair> *InPosPrePosts;
    NodeList<int> *InNegPrePosts;
    NodeList<int> opList;
    int aux_size;

    NodeList<int>  **OpLinkLists;
    int            **OpListLen;
    IntPair        ***OpLists;

  public:
    Variable();
    void input(istream &in);
    void initialize(int var_size, int mutex_count);
    void initialize_tuples(int var_size);
    int get_range();
    int find_op_index(int op);
    void dump(int var_size, int mutex_count);
    ~Variable();

    friend class PLProblem;
};

#endif // VARIABLE_H_INCLUDED
