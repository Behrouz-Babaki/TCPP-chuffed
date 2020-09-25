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

#ifndef CHECK_GSA_H_HIUO
#define CHECK_GSA_H_HIUO

#include "../triggering/constraint_abstract.h"
#include "../memory_management/reversible_vals.h"
#include "../get_info/get_info.h"
#include "../queue/standard_queue.h"

#ifdef P
#undef P
#endif

//#define P(x) cout << x << endl
#define P(x)

struct Check_GSA : public AbstractConstraint {
  virtual string extended_name() {
    return constraint_name() + ":" + child->extended_name();
  }

  virtual string constraint_name() {
    return "check[gsa]";
  }

  CONSTRAINT_ARG_LIST1(child);

  AbstractConstraint* child;

  Check_GSA(AbstractConstraint* _con) : child(_con) {}

  virtual ~Check_GSA() {
    delete child;
  }

  virtual AbstractConstraint* reverse_constraint() {
    return new Check_GSA(child->reverse_constraint());
  }

  virtual SysInt dynamic_trigger_count() {
    return child->get_vars_singleton()->size() * 2;
  }

  virtual bool get_satisfying_assignment(box<pair<SysInt, DomainInt>>& assignment) {
    return child->get_satisfying_assignment(assignment);
  }

  virtual BOOL check_assignment(DomainInt* v, SysInt v_size) {
    return child->check_assignment(v, v_size);
  }

  virtual vector<AnyVarRef> get_vars() {
    return child->get_vars();
  }

  virtual void propagateDynInt(SysInt, DomainDelta) {
    bool flag = false;
    GET_ASSIGNMENT(assignment, child);
    if(!flag) {
      getState().setFailed(true);
    } else {
      watch_assignment(assignment, *(child->get_vars_singleton()), 0);
    }
  }

  template <typename T, typename Vars>
  void watch_assignment(const T& assignment, Vars& vars, DomainInt trig) {
    for(SysInt i = 0; i < (SysInt)assignment.size(); ++i) {
      D_ASSERT(vars[assignment[i].first].inDomain(assignment[i].second));
      if(vars[assignment[i].first].isBound()) {
        moveTriggerInt(vars[assignment[i].first], trig + i, DomainChanged);
      } else {
        moveTriggerInt(vars[assignment[i].first], trig + i, DomainRemoval, assignment[i].second);
      }
    }
  }

  virtual void full_propagate() {
    propagateDynInt(0, DomainDelta::empty());
  }
};

AbstractConstraint* checkGSACon(AbstractConstraint* c) {
  return new Check_GSA(c);
}

inline AbstractConstraint* BuildCT_CHECK_GSA(ConstraintBlob& bl) {
  D_ASSERT(bl.internal_constraints.size() == 1);
  return checkGSACon(build_constraint(bl.internal_constraints[0]));
}

/* JSON
{ "type": "constraint",
  "name": "check[gsa]",
  "internal_name": "CT_CHECK_GSA",
  "args": [ "read_constraint" ]
}
*/

#endif
