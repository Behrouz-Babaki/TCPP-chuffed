#ifndef PLPROBLEM_H
#define PLPROBLEM_H

#include "Variable.h"
#include "mutex_group.h"
#include "state.h"
#include "operator.h"
#include "goal_state.h"
#include <cstring>

class PLProblem
{
    Variable    *variables;
    int          var_size;
    int          max_range;
    int        **values;
    MutexGroup  *mutexes;
    int          mutex_count;
    State        initial_state;
    GoalState    goal;
    Operator    *operators;
    int          op_count;
    bool         sat;
    int          plan_length;
    double       minion_time;
    NodeList<NegConstraint>  NegCons;
    NodeList<NegConstraint>  MxNegCons;

    public:
        PLProblem();
        void     read_problem(istream &in);
        void     Solve(string minion_switches, string file_attach);
        double   getMinion_time() {return minion_time;};
        virtual ~PLProblem();
    protected:
    private:
        void     read_and_verify_version(istream &in);
        void     read_metric(istream &in);
        void     read_variables(istream &in);
        void     read_mutexes(istream &in);
        void     read_operators(istream &in);
        bool     Check_parallel(int op1, int op2);
        int      smallest_cmg(int var1, int value1, int var2, int value2);
        int      MinDist(int var, int ivalue, int gvalue);
        void     generate_structures();
        void     generate_tuples();
        void     generate_negcons();
        void     generate_mx_negcons();
        void     MaxConsArrities();
        int      MinPlanLen();
        void     generate_MinionFile(string file_attach);
        bool     IsSatisfiable(string file_attach);
        void     extractPlan(string file_attach);
        void     AddNegC(int op1, int op2);
        Node<NegConstraint> *Gen_NodeNegC(int op1, int op2);
};

#endif // PLPROBLEM_H
