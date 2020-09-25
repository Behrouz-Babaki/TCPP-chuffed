#include "helper_function.h"
#include "operator.h"
#include <cassert>
#include <iostream>
#include <fstream>

using namespace std;

Operator::Operator()
{
    name = "";
    prevail = NULL;
    pre_post = NULL;
    pv_num = 0;
    pp_num = 0;
    cost = 0;
    CodInVar = 0;
}

void Operator::input(istream &in, int var_size)
{
    check_magic(in, "begin_operator");
    in >> ws;                           /// ?
    getline(in, name);
    in >> pv_num;
    if (pv_num!=0)
    {
       prevail = new struct Prevail[pv_num];
       for (int i = 0; i < pv_num; i++)
          in >> prevail[i].var >> prevail[i].prev;
    }
    in >> pp_num;
    if (pp_num!=0)
    {
       pre_post = new struct PrePost[pp_num];
       for (int i = 0; i < pp_num; i++)
       {
          int x;
          in >> x;               /// ?
          in >> pre_post[i].var >> pre_post[i].pre >> pre_post[i].post;
       }
    }
    in >> cost;
    CodInVar = new int[var_size];
    for(int i=0;i<var_size;i++)
       CodInVar[i] = -1;
    check_magic(in, "end_operator");
}

void Operator::dump(){
    cout << name << ":" << endl;
    cout << "prevail:";
    for (int i = 0; i < pv_num; i++)
        cout << "var" << prevail[i].var << " = " << prevail[i].prev;
    cout << endl;
    cout << "pre-post:";
    for (int i = 0; i < pp_num; i++) {
        cout << " var" << pre_post[i].var << " = " <<
        pre_post[i].pre << " -> " << pre_post[i].post;
    }
    cout << endl;
}

Operator::~Operator()
{
    if (prevail!=NULL)
        delete[] prevail;
    if (pre_post!= NULL)
        delete[] pre_post;
    if(CodInVar)
        delete[] CodInVar;
}
