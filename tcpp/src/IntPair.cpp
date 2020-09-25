#include "IntPair.h"

ostream &operator<<(ostream &out, IntPair &p)
{
  out<<"("<<p.code<<","<<p.value<<")";
  return out;
}
