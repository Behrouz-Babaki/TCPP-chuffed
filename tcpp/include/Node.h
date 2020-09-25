#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <iostream>
#include <stdio.h>
#include "IntPair.h"

using namespace std;

template <typename T> class NodeList;

template <typename T>
class Node
{
  T    data;
  Node<T> *next;
  public:
    Node()
    {
        next = 0;
    }
    Node(T &d)
    {
        data = d;
        next = 0;
    }
    void dump()
    {
      cout<<data;
    }
    virtual ~Node()
    {
       //cout<<endl<<data<<endl;
       //getchar();
    }
  friend class NodeList<T>;
  friend class NegConstraint;
  friend class Variable;
  friend class PLProblem;
};

#endif // NODE_H_INCLUDED
