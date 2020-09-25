#include <string.h>
#include <stdlib.h>
#include <iomanip>

#include "Variable.h"
#include "helper_function.h"

class NegConstraint;
using namespace std;

Variable::Variable()
{
    id = 0;
    range = 0;
    nb_num = 0;
    edge_num = 0;
    nb_loc = 0;
    nb_list = 0;
    tuples = 0;
    eq_pp = 0;
    MxNegC = 0;
    InMxGroups = 0;
    InPrevails = 0;
    InPosPrePosts = 0;
    InNegPrePosts = 0;
    aux = 0;
    auxCol = 0;
    OpLinkLists = 0;
    OpListLen = 0;
    OpLists = 0;
    MaxCoNum = 1;
}
void Variable::initialize(int var_size, int mutex_count)
{
    nb_num = 2;
    nb_loc = new int[2*var_size];
    nb_list = new int[2*var_size];
    eq_pp = new bool[range];
    aux = new bool[var_size];
    auxCol = new int* [var_size];

    for(int i=0;i<var_size;i++)
     auxCol[i]=0;

    if (mutex_count!=0)
    {
        MxNegC = new Node<NegConstraint>*[id];
        for (int i=0; i<id; i++)
           MxNegC[i] = 0;
    }
    InMxGroups = new NodeList<int>[range];
    InPrevails = new NodeList<int>[range];
    InPosPrePosts = new NodeList<IntPair>[range];
    InNegPrePosts = new NodeList<int>[range];

    OpLinkLists = new NodeList<int> *[range+1];
    OpListLen = new int*[range+1];
    OpLists = new IntPair **[range+1];
    for(int i=0;i<=range; i++)
    {
        OpLinkLists[i] = new NodeList<int>[range];
        OpListLen[i] = new int[range];
        OpLists[i] = new IntPair* [range];
    }

    for (int i=0; i<2*var_size; i++)        /// can be ignored
    {
       nb_loc[i] = -1;
       nb_list[i] = -1;
    }
    nb_loc[2*id] = 0;
    nb_list[0] = 2*id;
    nb_loc[2*id+1] = 1;
    nb_list[1] = 2*id+1;
    for (int i=0; i<range; i++)
        eq_pp[i] = false;
    for(int i=0;i<var_size;i++)
        aux[i] = false;
}

void Variable::initialize_tuples(int var_size)
{
    aux_size = var_size;
    Variable *vp;
    unsigned long int MTNum = 1;
    for (int i=0; i<nb_num; i++)
    {
        vp = this+nb_list[i]/2-id;
        MTNum *= vp->range;
    }
    if (edge_num+range!=MTNum)
    {
        tuples = new int *[edge_num];
        for(unsigned long int i=0;i<edge_num;i++)
        {
          tuples[i] = new int[nb_num];
          for(int j=0;j<nb_num;j++)
              tuples[i][j] = -1;
        }
        for(int k=0;k<var_size;k++)
        {
           auxCol[k] = new int[edge_num];
           for(unsigned long int cc=0;cc<edge_num;cc++)
               auxCol[k][cc] = -1;
        }
    }
    else
        edge_num = 0;

}

void Variable::input(istream &in)
{
    string str,h;
    int x;
    check_magic(in, "begin_variable");
    in >> str >> x >> range>>h;
    str.erase(0,3);
    id = atoi(str.c_str());
    for (int i = 0; i < range; i++)
    {
       getline(in, h);
    }
    check_magic(in, "end_variable");
}

int Variable::get_range()
{
    return range;
}

void Variable::dump(int var_size, int mutex_count)
{
    cout<<"-------------------------------------------------------------------------"<<endl;
    cout <<"id = "<<id <<", range= "<<range<<", nb_num="<<nb_num<<endl<<endl;
    cout<<"nb_Loc:  ";
    for (int i=0; i<2*var_size; i++)
       cout<<nb_loc[i]<<" ";
    cout<<endl;
    cout<<"nb_List: ";
    for (int i=0; i<nb_num; i++)
       cout<<setw(3)<<nb_list[i]<<" ";
    cout<<endl<<endl;
    cout<<"tuples:  ";
    for (unsigned long int i=0; i<edge_num; i++)
    {
       for (int j=0; j<nb_num; j++)
          cout<<setw(3)<<tuples[i][j]<<" ";
       cout<<endl<<"         ";
    }
    cout<<endl;
    cout<<"InMxGroups:"<<endl;
    for (int i=0; i<range; i++)
    {
      cout<<"  Value "<<i<<": ";
      InMxGroups[i].dump();
      cout<<endl;
    }
    cout<<"InPrevails:"<<endl;
    for (int i=0; i<range; i++)
    {
      cout<<"  Value "<<i<<": ";
      InPrevails[i].dump();
      cout<<endl;
    }
    cout<<"InPosPrePosts:"<<endl;
    for (int i=0; i<range; i++)
    {
      cout<<"  Value "<<i<<": ";
      InPosPrePosts[i].dump();
      cout<<endl;
    }
    cout<<"InNegPrePosts:"<<endl;
    for (int i=0; i<range; i++)
    {
      cout<<"  Value "<<i<<": ";
      InNegPrePosts[i].dump();
      cout<<endl;
    }
}

int Variable::find_op_index(int op)
{
   int index=0;
   Node<int> *p = opList.first;
   while (p!=0)
   {
      if (p->data==op)
         return index;
      p = p->next;
      index++;
   }
   return -1;
}

Variable::~Variable()
{
    delete [] nb_loc;
    delete [] nb_list;
    for (unsigned long int i = 0; i<edge_num; i++)
    {
        delete[] tuples[i];
    }

    delete[] tuples;
    delete [] MxNegC;
    delete [] InMxGroups;
    delete [] InPrevails;
    delete [] InNegPrePosts;

    if(aux)
        delete [] aux;
    for (int i = 0; i<aux_size; i++)
    {
          if(auxCol[i])
           delete[] auxCol[i];
    }

    delete [] auxCol;

    for(int i=0;i<=range;i++)
    {
        for(int j=0;j<range;j++)
            if(OpListLen[i][j])
                delete[] OpLists[i][j];
        delete[] OpLists[i];
        delete[] OpListLen[i];
    }
    delete[] OpLists;
    delete[] OpListLen;

}
