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

#ifndef STANDARD_QUEUE_H
#define STANDARD_QUEUE_H

#define QueueCon deque
#define queueTop front
#define queuePop pop_front

#include <deque>
#include "../solver.h"
#include "../get_info/get_info.h"
#include "../triggering/triggers.h"
#include "../triggering/constraint_abstract.h"
#include "TriggerBacktrackQueue.h"

class Queues {

  QueueCon<DynamicTriggerEvent> dynamic_trigger_list;

  // Special triggers are those which can only be run while the
  // normal queue is empty. This list is at the moment only used
  // by reified constraints when they want to start propagation.
  // I don't like it, but it is necesasary.
  QueueCon<AbstractConstraint*> special_triggers;

  TriggerBacktrackQueue tbq;

public:
  TriggerBacktrackQueue& getTbq() {
    return tbq;
  }

  Queues() {}

  void pushSpecialTrigger(AbstractConstraint* trigger) {
    CON_INFO_ADDONE(AddSpecialToQueue);
    special_triggers.push_back(trigger);
  }

  void pushDynamicTriggers(DynamicTriggerEvent new_dynamic_trig_range) {
    CON_INFO_ADDONE(AddDynToQueue);
    dynamic_trigger_list.push_back(new_dynamic_trig_range);
  }

  void clearQueues() {
    dynamic_trigger_list.clear();

    if(!special_triggers.empty()) {
      SysInt size = special_triggers.size();
      for(SysInt i = 0; i < size; ++i)
        special_triggers[i]->special_unlock();
      special_triggers.clear();
    }
  }

  bool isQueuesEmpty() {
    return dynamic_trigger_list.empty() &&
           special_triggers.empty();
  }

  // next_queue_ptr is defined in constraint_dynamic.
  // It is used if pointers are moved around.

  // Subclass this class and change the following three methods.

  template <bool is_root_node>
  bool propagateDynamicTriggerLists() {
    bool* fail_ptr = getState().getFailedPtr();
    while(!dynamic_trigger_list.empty()) {
      DynamicTriggerEvent dte = dynamic_trigger_list.queueTop();
      dynamic_trigger_list.queuePop();

      DynamicTriggerList& dtl = *(dte.event());
      DomainDelta delta = dte.data;
      SysInt pos = 0;

      while(pos < dtl.size()) {
        if(*fail_ptr) {
          clearQueues();
          return true;
        }

        if(!dtl[pos].empty() && (!is_root_node || dtl[pos].con->full_propagate_done)) {
          dtl[pos].propagate(delta);
        }

#ifdef WDEG
        if(*fail_ptr)
          dtl[pos].constraint()->incWdeg();
#endif

        pos++;
      }
      dtl.tryCompressList();
    }
    return false;
  }

  template <bool is_root_node>
  inline void propagateQueueImpl() {
    while(true) {
      while(!dynamic_trigger_list.empty()) {
        if(propagateDynamicTriggerLists<is_root_node>())
          return;
      }

      if(special_triggers.empty())
        return;

      AbstractConstraint* trig = special_triggers.queueTop();
      special_triggers.queuePop();

      CON_INFO_ADDONE(SpecialTrigger);
      trig->special_check();
#ifdef WDEG
      if(getState().isFailed())
        trig->incWdeg();
#endif

      if(getState().isFailed()) {
        clearQueues();
        return;
      }
    } // while(true)

  } // end Function

  inline void propagateQueueRoot() {
    return propagateQueueImpl<true>();
  }

  inline void propagateQueue() {
    return propagateQueueImpl<false>();
  }
};

#endif
