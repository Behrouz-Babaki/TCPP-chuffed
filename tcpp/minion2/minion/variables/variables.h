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

#define GET_CONTAINER() InternalRefType::getCon_Static()
#define GET_LOCAL_CON() getCon_Static()

#include "VarRefType.h"

#ifdef MORE_SEARCH_INFO
#include "../get_info/info_var_wrapper.h"
#endif

#include "containers/booleanvariables.h"
//#include "containers/intvar.h"
#include "containers/long_intvar.h"
#include "containers/intboundvar.h"
#include "containers/sparse_intboundvar.h"

class VariableContainer {
  // Stop copying!
  VariableContainer(const VariableContainer&);
  void operator=(const VariableContainer&);

public:
  BoundVarContainer<> boundVarContainer;
  BoolVarContainer boolVarContainer;
  BigRangeVarContainer<UnsignedSysInt> bigRangeVarContainer;
  SparseBoundVarContainer<> sparseBoundVarContainer;

  VariableContainer()
      : boundVarContainer(),
        boolVarContainer(),
        bigRangeVarContainer(),
        sparseBoundVarContainer() {}

  inline void lock() {
    boundVarContainer.lock();
    boolVarContainer.lock();
    bigRangeVarContainer.lock();
    sparseBoundVarContainer.lock();
  }
};

#include "mappings/variable_neg.h"
#include "mappings/variable_switch_neg.h"
#include "mappings/variable_stretch.h"
#include "mappings/variable_constant.h"
#include "mappings/variable_not.h"
#include "mappings/variable_shift.h"
