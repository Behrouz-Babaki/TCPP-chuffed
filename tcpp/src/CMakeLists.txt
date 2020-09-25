cmake_minimum_required(VERSION 3.0.0)

add_executable(tcpp 
               goal_state.cpp       IntPair.cpp      NegConstraint.cpp  
               operator.cpp         PLProblem.cpp    Tuple.cpp
               helper_function.cpp  mutex_group.cpp  NodeList.cpp       
               planner.cpp          state.cpp        Variable.cpp)
