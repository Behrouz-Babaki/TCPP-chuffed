#include <iostream>
#include "Tuple.h"

using namespace std;

Tuple::Tuple()
{
   op1 = -1;
   op2 = -1;
   size = 0;
   values = 0;
}
Tuple::Tuple(int s)
{
   op1 = -1;
   op2 = -1;
   size = s;
   values = new int [s];
}
Tuple::Tuple(int s, int *v)
{
   op1 = -1;
   op2 = -1;
   size = s;
   values = new int [s];
   for (int i=0; i<s; i++)
      values[i] = v[i];
}
Tuple &Tuple::operator=(Tuple &Tp)
{
    op1 = Tp.op1;
    op2 = Tp.op2;
    size = Tp.size;
    if (values!=0)
       delete [] values;
    values = new int[size];
    for (int i=0; i<size; i++)
       values[i] = Tp.values[i];
    return Tp;
}
bool Tuple::operator==(Tuple &Tp)
{
    if(size != Tp.size)
        return false;
    for(int i=0; i< size; i++)
        if(values[i] != Tp.values[i])
            return false;
    return true;
   //return ((op1==Tp.op1 && op2==Tp.op2) || (op2==Tp.op1 && op1==Tp.op2));
}
Tuple::~Tuple()
{
    delete [] values;
}

ostream &operator<<(ostream &out,Tuple &t)
{
   int i;
   out<<endl<<" "<<t.op1<<","<<t.op2<<",(";
   for ( i=0; i<t.size-1; i++)
      out<<t.values[i]<<",";
   out<<t.values[i];
   out<<")";
   return out;
}
