#include "NegConstraint.h"
#include <iostream>

using namespace std;

NegConstraint::NegConstraint()
{
    size = 0;
    v_size = 0;
    var_loc = 0;
    var_list = 0;
}
NegConstraint::NegConstraint(int s, int var_size)
{
    size = s;
    v_size = var_size;
    var_loc = new int[2*v_size];
    var_list = new int[size];
}
NegConstraint::NegConstraint(int s, int var_size, int *vlist)
{
    size = s;
    v_size = var_size;
    var_loc = new int[2*var_size];
    var_list = new int[size];
    for (int i=0; i<size; i++)
    {
       var_list[i] = vlist[i];
       var_loc[vlist[i]] = i;
    }
}

NegConstraint::NegConstraint(NegConstraint &C)
{
    *this = C;
}

NegConstraint &NegConstraint::operator=(NegConstraint &C)
{
    size = C.size;
    v_size = C.v_size;
    if (var_loc!=0)
       delete []var_loc;
    if (var_list!=0)
       delete []var_list;
    var_list = new int[size]; //  ? SIGTRAP error
    var_loc = new int[2*v_size];
    for (int i=0; i<size; i++)
    {
        var_list[i] = C.var_list[i];
        var_loc[var_list[i]] = i;
    }
    ///tuples.Destroy();               is it required???
    Node<Tuple> *p = C.tuples.first;
    while (p!=0)
    {
        tuples.AddLast(p->data);
        p = p->next;
    }
    return C;
}

bool  NegConstraint::operator==(NegConstraint &C)
{
    if (size!=C.size)
       return false;
    for (int i=0; i<size; i++)
       if ((var_loc[C.var_list[i]]<0) ||
           (var_loc[C.var_list[i]]>=size) ||
           (var_list[var_loc[C.var_list[i]]]!=C.var_list[i]))
          return false;
    return true;      /// Two var_lists contain same variables but my be in different order.
}
void NegConstraint::permut_tuple(NegConstraint &C, Tuple &T)
{
   int *vlist=new int[T.size]; /// size
   for (int i=0; i<size; i++)
      vlist[i] = T.values[C.var_loc[var_list[i]]];
   delete [] T.values;
   T.values = vlist;  /// Tuple is in the format of the constraint C is converted to *this format
}
void NegConstraint::add_tuple(Tuple &t)
{
    tuples.AddLast(t);
}
NegConstraint::~NegConstraint()
{
    if (var_loc)
        delete [] var_loc;
    if (var_list)
        delete [] var_list;
}

ostream &operator<<(ostream &out,NegConstraint &C)
{
    int i;
    out<<endl<<"--------------------------------------"<<endl;
    out<<"[";
    for ( i=0; i<C.size-1; i++)
       out<<C.var_list[i]<<",";
    out<<C.var_list[i];
    out<<"]"<<endl<<endl<<" ";
    C.tuples.dump();
    return out;
}
