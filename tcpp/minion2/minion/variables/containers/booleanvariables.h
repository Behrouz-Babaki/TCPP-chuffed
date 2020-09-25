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

/** @help variables;01 Description
01 variables are used very commonly for logical expressions, and for
encoding the characteristic functions of sets and relations. Note that
wherever a 01 variable can appear, the negation of that variable can
also appear. A boolean variable x's negation is identified by !x.
*/

/** @help variables;01 Example
Declaration of a 01 variable called bool in input file:

BOOL bool

Use of this variable in a constraint:

eq(bool, 0) #variable bool equals 0
*/

#ifndef _BOOLEANVARIABLES_H
#define _BOOLEANVARIABLES_H

#include "../../system/system.h"

#include "../../memory_management/backtrackable_memory.h"

#include "../../triggering/constraint_abstract.h"

/// Standard data type used for storing compressed booleans
typedef unsigned long data_type;
static const data_type one = 1;
static const data_type max_data = one << (sizeof(data_type) - 1);

struct BoolVarContainer;

/// A reference to a boolean variable
struct BoolVarRef_internal {
  static const BOOL isBool = true;
  static const BoundType isBoundConst = Bound_No;
  static string name() {
    return "Bool";
  }
  BOOL isBound() const {
    return false;
  }

  AnyVarRef popOneMapper() const {
    FATAL_REPORTABLE_ERROR();
  }

  data_type shift_offset;
  SysInt var_num;
  void* data_position;
  void* value_position;

  UnsignedSysInt data_offset() const {
    return var_num / (sizeof(data_type) * 8);
  }

  static BoolVarContainer& getCon_Static();
  BoolVarRef_internal(const BoolVarRef_internal& b)
      : shift_offset(b.shift_offset),
        var_num(b.var_num),
        data_position(b.data_position),
        value_position(b.value_position) {}

  BoolVarRef_internal() : shift_offset(~1), var_num(~1) {}

  BoolVarRef_internal(DomainInt value, BoolVarContainer* b_con);

  data_type& assign_ptr() const {
    return *static_cast<data_type*>(data_position);
  }

  data_type& value_ptr() const {
    return *static_cast<data_type*>(value_position);
  }

  BOOL isAssigned() const {
    return assign_ptr() & shift_offset;
  }

  DomainInt getAssignedValue() const {
    D_ASSERT(isAssigned());
    return (bool)(value_ptr() & shift_offset);
  }

  BOOL inDomain(DomainInt b) const {
    if((checked_cast<SysInt>(b) | 1) != 1)
      return false;
    return (!isAssigned()) || (b == getAssignedValue());
  }

  BOOL inDomain_noBoundCheck(DomainInt b) const {
    D_ASSERT(b == 0 || b == 1);
    return (!isAssigned()) || (b == getAssignedValue());
  }

  DomainInt getMin() const {
    if(!isAssigned())
      return 0;
    return getAssignedValue();
  }

  DomainInt getDomSize() const {
    if(isAssigned())
      return 1;
    else
      return 2;
  }

  DomainInt getMax() const {
    if(!isAssigned())
      return 1;
    return getAssignedValue();
  }

  DomainInt getInitialMin() const {
    return 0;
  }

  DomainInt getInitialMax() const {
    return 1;
  }

  DomainInt getBaseVal(DomainInt v) const {
    D_ASSERT(inDomain(v));
    return v;
  }

  Var getBaseVar() const {
    return Var(VAR_BOOL, var_num);
  }

  vector<Mapper> getMapperStack() const {
    return vector<Mapper>();
  }

  friend std::ostream& operator<<(std::ostream& o, const BoolVarRef_internal& b) {
    return o << "Bool:" << b.var_num;
  }
};

struct GetBoolVarContainer;

#ifdef MORE_SEARCH_INFO
typedef InfoRefType<QuickVarRefType<GetBoolVarContainer, BoolVarRef_internal>, VAR_INFO_BOOL>
    BoolVarRef;
#else
typedef QuickVarRefType<GetBoolVarContainer, BoolVarRef_internal> BoolVarRef;
#endif

/// Container for boolean variables
struct BoolVarContainer {

  BoolVarContainer() : var_count_m(0), trigger_list(false), lock_m(false) {}

  static const SysInt width = 7;
  ExtendableBlock assign_offset;
  void* values_mem;
  vector<vector<AbstractConstraint*>> constraints;
#ifdef WDEG
  vector<DomainInt> wdegs;
#endif
  UnsignedSysInt var_count_m;
  TriggerList trigger_list;
  /// When false, no variable can be altered. When true, no variables can be
  /// created.
  BOOL lock_m;

  data_type* value_ptr() {
    return static_cast<data_type*>(values_mem);
  }

  const data_type* value_ptr() const {
    return static_cast<const data_type*>(values_mem);
  }

  data_type* assign_ptr() {
    return (data_type*)(assign_offset());
  }

  const data_type* assign_ptr() const {
    return (const data_type*)(assign_offset());
  }

  void lock() {
    lock_m = true;
    // Min domain value = 0, max domain val = 1.
    std::vector<std::pair<DomainInt, DomainInt> > doms(var_count_m,
        make_pair(DomainInt(0), DomainInt(1)));
    trigger_list.addVariables(doms);
  }

  /// Returns a new Boolean Variable.
  // BoolVarRef get_new_var();

  void addVariables(SysInt new_bools) {
    D_ASSERT(!lock_m);
    var_count_m += new_bools;

    SysInt required_mem = var_count_m / 8 + 1;
    // Round up to nearest data_type block
    required_mem += sizeof(data_type) - (required_mem % sizeof(data_type));
    if(assign_offset.empty()) {
      assign_offset = getMemory().backTrack().requestBytesExtendable(required_mem);
      values_mem = checked_malloc(10*1024*1024);
      CHECK(required_mem < 10*1024*1024, "Bool mem overflow");
    }
    else {
      getMemory().backTrack().resizeExtendableBlock(assign_offset, required_mem);
    }
    constraints.resize(var_count_m);
#ifdef WDEG
    wdegs.resize(var_count_m);
#endif
  }

  /// Returns a reference to the ith Boolean variable which was previously
  /// created.
  BoolVarRef get_var_num(DomainInt i);

  UnsignedSysInt var_count() {
    return var_count_m;
  }

  void setMax(const BoolVarRef_internal& d, DomainInt i) {
    if(i < 0) {
      getState().setFailed(true);
      return;
    }

    D_ASSERT(i >= 0);
    if(i == 0)
      propagateAssign(d, 0);
  }

  void setMin(const BoolVarRef_internal& d, DomainInt i) {
    if(i > 1) {
      getState().setFailed(true);
      return;
    }
    D_ASSERT(i <= 1);
    if(i == 1)
      propagateAssign(d, 1);
  }

  void removeFromDomain(const BoolVarRef_internal& d, DomainInt b) {
    D_ASSERT(lock_m && d.var_num < (SysInt)var_count_m);
    if((checked_cast<SysInt>(b) | 1) != 1)
      return;

    if(d.isAssigned()) {
      if(b == d.getAssignedValue())
        getState().setFailed(true);
    } else
      uncheckedAssign(d, 1 - b);
  }

  void internalAssign(const BoolVarRef_internal& d, DomainInt b) {
    D_ASSERT(lock_m && d.var_num < (SysInt)var_count_m);
    D_ASSERT(!d.isAssigned());
    if((checked_cast<SysInt>(b) | 1) != 1) {
      getState().setFailed(true);
      return;
    }
    assign_ptr()[d.data_offset()] |= d.shift_offset;

    trigger_list.push_assign(d.var_num, b);
    trigger_list.push_domain_changed(d.var_num);
    trigger_list.push_domain_removal(d.var_num, 1 - b);

    if(b == 1) {
      trigger_list.push_lower(d.var_num, 1);
      value_ptr()[d.data_offset()] |= d.shift_offset;
    } else {
      trigger_list.push_upper(d.var_num, 1);
      value_ptr()[d.data_offset()] &= ~d.shift_offset;
    }
  }

  void uncheckedAssign(const BoolVarRef_internal& d, DomainInt b) {
    internalAssign(d, b);
  }

  void propagateAssign(const BoolVarRef_internal& d, DomainInt b) {
    if(!d.isAssigned())
      internalAssign(d, b);
    else {
      if(d.getAssignedValue() != b)
        getState().setFailed(true);
    }
  }

  void decisionAssign(const BoolVarRef_internal& d, DomainInt b) {
    internalAssign(d, b);
  }

  void addDynamicTrigger(BoolVarRef_internal& b, Trig_ConRef t, TrigType type,
                         DomainInt pos = NoDomainValue, TrigOp op = TO_Default) {
    D_ASSERT(pos == NoDomainValue || (type == DomainRemoval && pos != NoDomainValue));
    D_ASSERT(lock_m);

    trigger_list.addDynamicTrigger(b.var_num, t, type, pos, op);
  }

  vector<AbstractConstraint*>* getConstraints(const BoolVarRef_internal& b) {
    return &constraints[b.var_num];
  }

  void addConstraint(const BoolVarRef_internal& b, AbstractConstraint* c) {
    constraints[b.var_num].push_back(c);
#ifdef WDEG
    wdegs[b.var_num] += checked_cast<SysInt>(c->getWdeg()); // add constraint score to base var wdeg
#endif
  }

#ifdef WDEG
  DomainInt getBaseWdeg(const BoolVarRef_internal& b) {
    return wdegs[b.var_num];
  }

  void incWdeg(const BoolVarRef_internal& b) {
    wdegs[b.var_num]++;
  }
#endif
};

inline BoolVarRef BoolVarContainer::get_var_num(DomainInt i) {
  D_ASSERT(i < (SysInt)var_count_m);
  return BoolVarRef(BoolVarRef_internal(i, this));
}

inline BoolVarRef_internal::BoolVarRef_internal(DomainInt value, BoolVarContainer* b_con)
    : var_num(checked_cast<UnsignedSysInt>(value)),
      data_position((char*)(b_con->assign_offset()) + data_offset() * sizeof(data_type)),
      value_position((char*)(b_con->values_mem) + data_offset() * sizeof(data_type)) {
  shift_offset = one << (checked_cast<UnsignedSysInt>(value) % (sizeof(data_type) * 8));
}

#endif
