#ifndef NODELIST_H_INCLUDED
#define NODELIST_H_INCLUDED

#include "Node.h"

template <typename T>
class NodeList
{
   Node<T> *first;
   Node<T> *last;
   int size;
   public:
     NodeList(){first = 0; last = 0; size = 0;}
     void AddLast(T &d);
     void AddLast(Node<T> *);
     bool Add(T &d);
     void Destroy();
     Node<T> *find(T &d);
     void dump();
     ~NodeList();

   friend class NegConstraint;
   friend class PLProblem;
   friend class Variable;
};

template <typename T>
void NodeList<T>::AddLast(T &d)
{
    Node<T> *temp = new Node<T>(d);
    if (first == 0)
    {
        first = temp;
        last = temp;
        size = 1;
    }
    else
    {
        last->next = temp;
        last = temp;
        size++;
    }
}
template <typename T>
void NodeList<T>::AddLast(Node<T> *temp)
{
    if (first == 0)
    {
        first = temp;
        last = temp;
        size = 1;
    }
    else
    {
        last->next = temp;
        last = temp;
        size++;
    }
}
template <typename T>
Node<T> *NodeList<T>::find(T &d)
{
   Node<T> *p = first;
   while (p!=0)
   {
      if (p->data==d)
         break;
      p = p->next;
   }
   return p;
}
template <typename T>
void NodeList<T>::dump()
{
    Node<T> *temp;
    temp = first;
    while (temp != 0)
    {
        cout<<temp->data<<" ";
        temp = temp->next;
    }
    cout<<"["<<size<<"]";
}

template <typename T>
void NodeList<T>::Destroy()
{
    Node<T> *temp;
    while (first != 0)
    {
       temp = first;
       first = first->next;
       delete temp;
    }
    last = 0;
    size = 0;
}

template <typename T>
bool NodeList<T>::Add(T &d)
{
   Node<T> *p = first;
   while (p!=0)
   {
      if (p->data==d)
         return false;
      p = p->next;
   }
   Node<T> *NTp = new Node<T>(d);
   AddLast(NTp);
   return true;
}



template <typename T>
NodeList<T>::~NodeList()
{
    Destroy();
}

#endif // NODELIST_H_INCLUDED
