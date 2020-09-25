#ifndef OPERATOR_H_INCLUDED
#define OPERATOR_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>

using namespace std;


struct Prevail {
   int var;
   int prev;
};
struct PrePost {
   int var;
   int pre, post;
};

class Operator
{

private:
    string name;
    Prevail *prevail;  // var, val
    PrePost *pre_post; // var, old-val, new-val
    int     *CodInVar;
    int pv_num;
    int pp_num;
    int cost;
public:
    Operator();
    void input(istream &in, int var_size);
    void dump();
    string get_name(){return name; }
    int get_pp_num() {return pp_num;}
    int get_pv_num() {return pv_num;}
    struct Prevail get_prevail(int i) {return prevail[i]; }
    struct PrePost get_pre_post(int i) {return pre_post[i]; }
    ~Operator();

    friend class PLProblem;
};

#endif // OPERATOR_H_INCLUDED
