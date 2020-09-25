/*
* Minion http://minion.sourceforge.net
* Copyright (C) 2006-09
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
* USA.
*/

/** @help constraints;difference Description
The constraint

   difference(x,y,z)

ensures that z=|x-y| in any solution.
*/

/** @help constraints;difference Notes
This constraint can be expressed in a much longer form, this form both avoids
requiring an extra variable, and also gets better propagation. It gets bounds
consistency.
*/

#ifndef CONSTRAINT_DIFFERENCE_H
#define CONSTRAINT_DIFFERENCE_H

#include "constraint_checkassign.h"

#ifdef P
#undef P
#endif

//#define P(x) cout << x << endl
#define P(x)

/// |var1 - var2| = var3
template <typename VarRef1, typename VarRef2, typename VarRef3>
struct DifferenceConstraint : public AbstractConstraint {
  virtual string constraint_name() {
    return "difference";
  }

  CONSTRAINT_ARG_LIST3(var1, var2, var3);

  VarRef1 var1;
  VarRef2 var2;
  VarRef3 var3;
  DifferenceConstraint(VarRef1 _var1, VarRef2 _var2, VarRef3 _var3)
      : var1(_var1), var2(_var2), var3(_var3) {}

  virtual SysInt dynamic_trigger_count() {
    return 6;
  }

  template <typename Var>
  void remove_range(DomainInt low, DomainInt high, Var& v) {
    P("Remove Range" << low << high);
    D_ASSERT(low <= high);
    if(!v.isBound()) {
      for(DomainInt i = low + 1; i < high; ++i)
        v.removeFromDomain(i);
    } else {
      if(v.getMax() < high)
        v.setMax(low + 1);

      if(v.getMin() > low)
        v.setMin(high - 1);
    }
  }

  virtual void propagateDynInt(SysInt, DomainDelta) {
    PROP_INFO_ADDONE(Difference);

    DomainInt var1_min = var1.getMin();
    DomainInt var1_max = var1.getMax();
    DomainInt var2_min = var2.getMin();
    DomainInt var2_max = var2.getMax();

    P(var1_min << var1_max << var2_min << var2_max << var3.getMin() << var3.getMax());

    var3.setMax(max(var2_max, var1_max) - min(var1_min, var2_min));

    var1.setMin(var2.getMin() - var3.getMax());
    var2.setMin(var1.getMin() - var3.getMax());
    var1.setMax(var2.getMax() + var3.getMax());
    P(var2.getMax());
    var2.setMax(var1.getMax() + var3.getMax());
    P(var2.getMax());

    if(var1_max < var2_min) {
      var3.setMin(var2_min - var1_max);
      var2.setMin(var1.getMin() + var3.getMin());
      var1.setMax(var2.getMax() - var3.getMin());
    }

    if(var2_max < var1_min) {
      var3.setMin(var1_min - var2_max);
      var1.setMin(var2.getMin() + var3.getMin());
      P(var2.getMax());
      var2.setMax(var1.getMax() - var3.getMin());
      P(var2.getMax());
    }

    if(var1_max - var1_min < var3.getMin()) {
      remove_range(var1_max - var3.getMin(), var1_min + var3.getMin(), var2);
    }

    if(var2_max - var2_min < var3.getMin()) {
      remove_range(var2_max - var3.getMin(), var2_min + var3.getMin(), var1);
    }
  }

  void trigger_setup() {
    moveTriggerInt(var1, 0, LowerBound);
    moveTriggerInt(var1, 1, UpperBound);
    moveTriggerInt(var2, 2, LowerBound);
    moveTriggerInt(var2, 3, UpperBound);
    moveTriggerInt(var3, 4, LowerBound);
    moveTriggerInt(var3, 5, UpperBound);
  }

  virtual void full_propagate() {
    trigger_setup();
    var3.setMin(0);
    propagateDynInt(0, DomainDelta::empty());
  }

  virtual BOOL check_assignment(DomainInt* v, SysInt v_size) {
    D_ASSERT(v_size == 3);
    DomainInt abs_val = v[0] - v[1];
    if(abs_val < 0)
      abs_val = -abs_val;
    return abs_val == v[2];
  }

  virtual bool get_satisfying_assignment(box<pair<SysInt, DomainInt>>& assignment) {
    for(DomainInt i = var1.getMin(); i <= var1.getMax(); ++i) {
      if(var1.inDomain(i)) {
        for(DomainInt j = var2.getMin(); j <= var2.getMax(); ++j) {
          if(var2.inDomain(j) && var3.inDomain(abs(i - j))) {
            assignment.push_back(make_pair(0, i));
            assignment.push_back(make_pair(1, j));
            assignment.push_back(make_pair(2, abs(i - j)));
            return true;
          }
        }
      }
    }
    return false;
  }

  virtual vector<AnyVarRef> get_vars() {
    vector<AnyVarRef> v;
    v.push_back(var1);
    v.push_back(var2);
    v.push_back(var3);
    return v;
  }

  // Function to make it reifiable in the lousiest way.
  virtual AbstractConstraint* reverse_constraint() {
    return forward_check_negation(this);
  }
};

template <typename VarRef1, typename VarRef2>
AbstractConstraint* BuildCT_DIFFERENCE(const vector<VarRef1>& vars, const vector<VarRef2>& var2,
                                       ConstraintBlob&) {
  D_ASSERT(vars.size() == 2);
  D_ASSERT(var2.size() == 1);
  return new DifferenceConstraint<VarRef1, VarRef1, VarRef2>(vars[0], vars[1], var2[0]);
}

/* JSON
{ "type": "constraint",
  "name": "difference",
  "internal_name": "CT_DIFFERENCE",
  "args": [ "read_2_vars", "read_var" ]
}
*/

#endif
