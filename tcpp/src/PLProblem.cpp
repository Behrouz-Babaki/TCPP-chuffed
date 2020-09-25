#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

using namespace std;

#include "PLProblem.h"
#include "helper_function.h"

bool sm_flag;
static const int SAS_FILE_VERSION = 3;
static const int PRE_FILE_VERSION = SAS_FILE_VERSION;

PLProblem::PLProblem()
{
    variables = NULL;
    values = NULL;
    mutexes = NULL;
    operators = NULL;
    minion_time = 0;
}

void PLProblem::read_and_verify_version(istream &in)
{
    int version;
    check_magic(in, "begin_version");
    in >> version;
    check_magic(in, "end_version");
    if (version != SAS_FILE_VERSION)
    {
        cerr << "Expected translator file version " << SAS_FILE_VERSION
             << ", got " << version << "." << endl;
        cerr << "Exiting." << endl;
        exit(1);
    }
}

void PLProblem::read_metric(istream &in)
{
    bool metric;
    check_magic(in, "begin_metric");
    in >> metric;
    check_magic(in, "end_metric");
}

void PLProblem::read_variables(istream &in)
{
    max_range = 0;
    in >> var_size;
    variables = new Variable[var_size];
    for (int i = 0; i < var_size; i++)
    {
        variables[i].input(in);
        if (variables[i].get_range() > max_range)
            max_range = variables[i].get_range();
    }
}

void PLProblem::read_mutexes(istream &in)
{
    int m_count, index = 0;
    in >> m_count;
    mutexes = new MutexGroup[m_count];
    for (int i = 0; i < m_count; i++)
    {
        if (mutexes[index].input(in))
            index++;
    }
    mutex_count = index;
}

void PLProblem::read_operators(istream &in)
{
    in >> op_count;
    operators = new Operator[op_count];
    for (int i = 0; i < op_count; i++)
        operators[i].input(in, var_size);
}

void PLProblem::read_problem(istream &in)
{
    read_and_verify_version(in);
    read_metric(in);
    read_variables(in);
    read_mutexes(in);
    initial_state.input(in, var_size);
    goal.input(in);
    read_operators(in);
}

bool Consistent(int i, int *Colors, int *DgVars, bool **Connected)
{
    int Vi = DgVars[i];
    for (int j = 0; j < i; j++)
    {
        int Vj = DgVars[j];
        if (Connected[Vi][Vj] && Colors[Vi] == Colors[Vj])
            return false;
    }
    return true;
}

int MinGrColor(int *Colors, int n, bool **Connected, bool ScFlag) /// Single Color Flag for First Node
{

    int *Degree = new int[n];
    for (int i = 0; i < n; i++)
    {
        Degree[i] = 0;
        for (int j = 0; j < n; j++)
        {
            if (Connected[i][j])
                Degree[i] += 1;
        }
    }

    int *DgVars = new int[n];
    for (int i = 0; i < n; i++)
        DgVars[i] = i;

    for (int i = 2; i < n; i++)
    {
        int temp = DgVars[i];
        int j;
        for (j = i - 1; j > 0; j--)
        {
            if (Degree[DgVars[j]] < Degree[temp])
                DgVars[j + 1] = DgVars[j];
            else
                break;
        }
        DgVars[j + 1] = temp;
    }

    int MaxColor = 1;
    for (int i = 0; i < n; i++)
    {
        int Vi = DgVars[i];
        for (int j = 0; j < n; j++)
        {
            Colors[Vi] = j;
            if (Consistent(i, Colors, DgVars, Connected))
            {
                if (j + 1 > MaxColor)
                    MaxColor = j + 1;
                break;
            }
        }
    }
    delete[] Degree;
    delete[] DgVars;
    return MaxColor;

    //    for(int i=0;i<n;i++)
    //       Colors[i] = i;
    //    return n;
}

void PLProblem::generate_structures()
{
    int var, value;
    int var2, value2;
    int loc;
    int pv_num, pp_num;
    struct Prevail pv_temp;
    struct PrePost pp_temp1;
    struct PrePost pp_temp2;
    for (int i = 0; i < var_size; i++)
        variables[i].initialize(var_size, mutex_count);

    for (int i = 0; i < mutex_count; i++)
    {
        for (int j = 0; j < mutexes[i].mg_size; j++)
        {
            var = mutexes[i].facts[j].first;
            value = mutexes[i].facts[j].second;
            variables[var].InMxGroups[value].AddLast(i);
            for (int k = j + 1; k < mutexes[i].mg_size; k++)
            {
                var2 = mutexes[i].facts[k].first;
                value2 = mutexes[i].facts[k].second;
                if (var != var2)
                {
                    Node<Tuple> *NTp = new Node<Tuple>;
                    NTp->data.op1 = 0;
                    NTp->data.op2 = i;
                    NTp->data.size = 2;
                    NTp->data.values = new int[2];
                    if (var2 < var)
                    {
                        if (variables[var].MxNegC[var2] == 0)
                        {
                            variables[var].MxNegC[var2] = new Node<NegConstraint>;
                            variables[var].MxNegC[var2]->data.size = 2;
                            variables[var].MxNegC[var2]->data.var_list = new int[2];
                            variables[var].MxNegC[var2]->data.var_list[0] = var2;
                            variables[var].MxNegC[var2]->data.var_list[1] = var;
                        }
                        NTp->data.values[0] = value2;
                        NTp->data.values[1] = value;
                        if (variables[var].MxNegC[var2]->data.tuples.find(NTp->data) == 0)
                            variables[var].MxNegC[var2]->data.tuples.AddLast(NTp);
                        else
                            delete NTp;
                    }
                    else
                    {
                        if (variables[var2].MxNegC[var] == 0)
                        {
                            variables[var2].MxNegC[var] = new Node<NegConstraint>;
                            variables[var2].MxNegC[var]->data.size = 2;
                            variables[var2].MxNegC[var]->data.var_list = new int[2];
                            variables[var2].MxNegC[var]->data.var_list[0] = var;
                            variables[var2].MxNegC[var]->data.var_list[1] = var2;
                        }
                        NTp->data.values[0] = value;
                        NTp->data.values[1] = value2;
                        if (variables[var2].MxNegC[var]->data.tuples.find(NTp->data) == 0)
                            variables[var2].MxNegC[var]->data.tuples.AddLast(NTp);
                        else
                            delete NTp;
                    }
                }
            }
        } //end for j
    }     // end for i

    for (int i = 0; i < op_count; i++)
    {
        pv_num = operators[i].pv_num;
        pp_num = operators[i].pp_num;
        for (int j = 0; j < pv_num; j++)
        {
            pv_temp = operators[i].prevail[j];
            variables[pv_temp.var].InPrevails[pv_temp.prev].AddLast(i);

            //  variables[pv_temp.var].OpLinkLists[pv_temp.prev][pv_temp.prev].AddLast(i);
        }
        for (int j = 0; j < pp_num; j++)
        {
            pp_temp1 = operators[i].pre_post[j];
            variables[pp_temp1.var].edge_num++;
            if (pp_temp1.pre == -1)
            {
                variables[pp_temp1.var].InNegPrePosts[pp_temp1.post].AddLast(i);

                variables[pp_temp1.var].OpLinkLists[variables[pp_temp1.var].range][pp_temp1.post].AddLast(i);
            }
            else
            {
                if (pp_temp1.pre == pp_temp1.post)
                    variables[pp_temp1.var].eq_pp[pp_temp1.pre] = true;
                IntPair temp(i, pp_temp1.pre);
                variables[pp_temp1.var].InPosPrePosts[pp_temp1.post].AddLast(temp);

                variables[pp_temp1.var].OpLinkLists[pp_temp1.pre][pp_temp1.post].AddLast(i);
            }
            for (int k = 0; k < pv_num; k++)
            {
                pv_temp = operators[i].prevail[k];
                int &nb_num = variables[pp_temp1.var].nb_num;
                loc = variables[pp_temp1.var].nb_loc[2 * pv_temp.var];
                if ((loc < 0) || (loc >= nb_num) || (variables[pp_temp1.var].nb_list[loc] != 2 * pv_temp.var))
                {
                    variables[pp_temp1.var].nb_list[nb_num] = 2 * pv_temp.var;
                    variables[pp_temp1.var].nb_loc[2 * pv_temp.var] = nb_num;
                    nb_num++;
                }
                loc = variables[pp_temp1.var].nb_loc[2 * pv_temp.var + 1];
                if ((loc < 0) || (loc >= nb_num) || (variables[pp_temp1.var].nb_list[loc] != 2 * pv_temp.var + 1))
                {
                    variables[pp_temp1.var].nb_list[nb_num] = 2 * pv_temp.var + 1;
                    variables[pp_temp1.var].nb_loc[2 * pv_temp.var + 1] = nb_num;
                    nb_num++;
                }
            } //end for k
            for (int k = j + 1; k < pp_num; k++)
            {
                pp_temp2 = operators[i].pre_post[k];
                int &nb_num = variables[pp_temp1.var].nb_num;
                loc = variables[pp_temp1.var].nb_loc[2 * pp_temp2.var];
                if ((pp_temp2.pre != -1) &&
                    ((loc < 0) || (loc >= nb_num) ||
                     (variables[pp_temp1.var].nb_list[loc] != 2 * pp_temp2.var)))
                {
                    variables[pp_temp1.var].nb_list[nb_num] = 2 * pp_temp2.var;
                    variables[pp_temp1.var].nb_loc[2 * pp_temp2.var] = nb_num;
                    nb_num++;
                }
                loc = variables[pp_temp1.var].nb_loc[2 * pp_temp2.var + 1];
                if ((loc < 0) || (loc >= nb_num) ||
                    (variables[pp_temp1.var].nb_list[loc] != 2 * pp_temp2.var + 1))
                {
                    variables[pp_temp1.var].nb_list[nb_num] = 2 * pp_temp2.var + 1;
                    variables[pp_temp1.var].nb_loc[2 * pp_temp2.var + 1] = nb_num;
                    nb_num++;
                }
                int &nb_num2 = variables[pp_temp2.var].nb_num;
                loc = variables[pp_temp2.var].nb_loc[2 * pp_temp1.var];
                if ((pp_temp1.pre != -1) &&
                    ((loc < 0) || (loc >= nb_num2) ||
                     (variables[pp_temp2.var].nb_list[loc] != 2 * pp_temp1.var)))
                {
                    variables[pp_temp2.var].nb_list[nb_num2] = 2 * pp_temp1.var;
                    variables[pp_temp2.var].nb_loc[2 * pp_temp1.var] = nb_num2;
                    nb_num2++;
                }
                loc = variables[pp_temp2.var].nb_loc[2 * pp_temp1.var + 1];
                if ((loc < 0) || (loc >= nb_num2) ||
                    (variables[pp_temp2.var].nb_list[loc] != 2 * pp_temp1.var + 1))
                {
                    variables[pp_temp2.var].nb_list[nb_num2] = 2 * pp_temp1.var + 1;
                    variables[pp_temp2.var].nb_loc[2 * pp_temp1.var + 1] = nb_num2;
                    nb_num2++;
                }
            } //end for k
        }     //end of for j
    }         //end of for i

    for (int i = 0; i < var_size; i++)
    {
        int j = 0;
        for (; j <= variables[i].range; j++)
        {
            for (int k = 0; k < variables[i].range; k++)
            {
                if (variables[i].OpLinkLists[j][k].size == 0)
                {
                    variables[i].OpListLen[j][k] = 0;
                    variables[i].OpLists[j][k] = 0;
                }
                else
                {
                    variables[i].OpListLen[j][k] = variables[i].OpLinkLists[j][k].size;
                    variables[i].OpLists[j][k] = new IntPair[variables[i].OpListLen[j][k]];
                    Node<int> *p = variables[i].OpLinkLists[j][k].first;
                    int l = 0;
                    while (p)
                    {
                        variables[i].OpLists[j][k][l].code = p->data;
                        p = p->next;
                        l++;
                    }
                }
            }
        }
    }

    for (int i = 0; i < var_size; i++)
    {
        for (int j = 0; j <= variables[i].range; j++)
            delete[] variables[i].OpLinkLists[j];
        delete[] variables[i].OpLinkLists;
    }

    for (int i = 0; i < var_size; i++)
    {
        int k = 0;
        for (; k < variables[i].range; k++)
        {
            for (int l = 0; l < variables[i].OpListLen[variables[i].range][k]; l++)
            {
                variables[i].OpLists[variables[i].range][k][l].value = 0;
                operators[variables[i].OpLists[variables[i].range][k][l].code].CodInVar[i] = 0;
            }
        }
        //cout<<"bbbbbbbbbbbbbbbbb"<<endl;
        for (int j = 0; j < variables[i].range; j++)
        {
            for (int k = 0; k < variables[i].range; k++)
            {
                if (j == k)
                    continue;
                if (variables[i].OpListLen[j][k])
                {
                    int ColorNum;
                    bool ConFlag = false;
                    int *Colors = new int[variables[i].OpListLen[j][k] + 1];
                    bool **Connected = new bool *[variables[i].OpListLen[j][k] + 1];
                    for (int l = 0; l <= variables[i].OpListLen[j][k]; l++)
                        Connected[l] = new bool[variables[i].OpListLen[j][k] + 1];

                    for (int l = 0; l <= variables[i].OpListLen[j][k]; l++)
                    {
                        for (int h = 0; h <= variables[i].OpListLen[j][k]; h++)
                        {
                            Connected[l][h] = false;
                            Connected[h][l] = false;
                        }
                    }
                    for (int l = 0; l < variables[i].OpListLen[j][k]; l++)
                    {
                        for (int h = 0; h < variables[i].OpListLen[variables[i].range][k]; h++)
                        {
                            if (Check_parallel(variables[i].OpLists[j][k][l].code,
                                               variables[i].OpLists[variables[i].range][k][h].code))
                            {
                                ConFlag = true;
                                Connected[l + 1][0] = true;
                                Connected[0][l + 1] = true;
                                break;
                            }
                        }

                        for (int h = l + 1; h < variables[i].OpListLen[j][k]; h++)
                        {
                            if (Check_parallel(variables[i].OpLists[j][k][l].code,
                                               variables[i].OpLists[j][k][h].code))
                            {
                                ConFlag = true;
                                Connected[l + 1][h + 1] = true;
                                Connected[h + 1][l + 1] = true;
                            }
                        }
                    }

                    if (!ConFlag)
                    {
                        ColorNum = 1;
                        for (int l = 0; l <= variables[i].OpListLen[j][k]; l++)
                            Colors[l] = 0;
                    }
                    else
                        ColorNum = MinGrColor(Colors, variables[i].OpListLen[j][k] + 1, Connected, true);

                    if (ColorNum > variables[i].MaxCoNum)
                        variables[i].MaxCoNum = ColorNum;
                    for (int l = 0; l < variables[i].OpListLen[j][k]; l++)
                    {
                        variables[i].OpLists[j][k][l].value = Colors[l + 1];
                        operators[variables[i].OpLists[j][k][l].code].CodInVar[i] = Colors[l + 1];
                    }

                    for (int l = 0; l <= variables[i].OpListLen[j][k]; l++)
                        delete[] Connected[l];
                    delete[] Connected;
                    delete[] Colors;
                }
            }
        }
    }
}

void PLProblem::generate_tuples()
{
    int tp_num, op, loc;
    int pv_num, pp_num;
    struct Prevail prv;
    struct PrePost pst;
    Node<IntPair> *p;
    Node<int> *q;

    for (int i = 0; i < var_size; i++)
    {
        tp_num = 0;

        variables[i].initialize_tuples(var_size);

        if (variables[i].tuples != 0)
        {
            for (int j = 0; j < variables[i].range; j++)
            {
                p = variables[i].InPosPrePosts[j].first; /// first of its NodeList
                while (p != 0)
                {
                    op = p->data.code;
                    variables[i].opList.AddLast(op);
                    pv_num = operators[op].get_pv_num();
                    pp_num = operators[op].get_pp_num();
                    for (int k = 0; k < pv_num; k++)
                    {
                        prv = operators[op].get_prevail(k);
                        loc = variables[i].nb_loc[2 * prv.var];
                        variables[i].tuples[tp_num][loc] = prv.prev;
                        loc = variables[i].nb_loc[2 * prv.var + 1];
                        variables[i].tuples[tp_num][loc] = prv.prev;
                    }
                    for (int k = 0; k < pp_num; k++)
                    {
                        pst = operators[op].get_pre_post(k);
                        if (pst.pre != -1)
                        {
                            loc = variables[i].nb_loc[2 * pst.var];
                            variables[i].tuples[tp_num][loc] = pst.pre;
                        }
                        loc = variables[i].nb_loc[2 * pst.var + 1];
                        variables[i].tuples[tp_num][loc] = pst.post;
                    }

                    tp_num++;
                    p = p->next;
                }
                q = variables[i].InNegPrePosts[j].first;
                while (q != 0)
                {
                    op = q->data;
                    variables[i].opList.AddLast(op);
                    pv_num = operators[op].get_pv_num();
                    pp_num = operators[op].get_pp_num();
                    for (int k = 0; k < pv_num; k++)
                    {
                        prv = operators[op].get_prevail(k);
                        loc = variables[i].nb_loc[2 * prv.var];
                        variables[i].tuples[tp_num][loc] = prv.prev;
                        loc = variables[i].nb_loc[2 * prv.var + 1];
                        variables[i].tuples[tp_num][loc] = prv.prev;
                    }
                    for (int k = 0; k < pp_num; k++)
                    {
                        pst = operators[op].get_pre_post(k);
                        if (pst.pre != -1)
                        {
                            loc = variables[i].nb_loc[2 * pst.var];
                            variables[i].tuples[tp_num][loc] = pst.pre;
                        }
                        loc = variables[i].nb_loc[2 * pst.var + 1];
                        variables[i].tuples[tp_num][loc] = pst.post;
                    }
                    tp_num++;
                    q = q->next;
                }
            }
        }
    }
}

int PLProblem::smallest_cmg(int var1, int value1, int var2, int value2)
{
    if (!sm_flag)
        return -1;

    Node<int> *p = variables[var1].InMxGroups[value1].first;
    Node<int> *q = variables[var2].InMxGroups[value2].first;
    while ((p != 0) && (q != 0))
    {
        if (p->data == q->data)
            return p->data;
        if (p->data < q->data)
            p = p->next;
        else
            q = q->next;
    }
    return -1;
}

bool PLProblem::Check_parallel(int op1, int op2)
{
    int pv_num1 = operators[op1].get_pv_num();
    int pv_num2 = operators[op2].get_pv_num();
    int pp_num1 = operators[op1].get_pp_num();
    int pp_num2 = operators[op2].get_pp_num();
    struct Prevail pv_temp1, pv_temp2;
    struct PrePost pp_temp1, pp_temp2;

    for (int i = 0; i < pv_num1; i++)
    {
        pv_temp1 = operators[op1].prevail[i];
        for (int j = 0; j < pv_num2; j++)
        {
            pv_temp2 = operators[op2].prevail[j];
            if (pv_temp1.var == pv_temp2.var)
            {
                if (pv_temp1.prev != pv_temp2.prev)
                    return false;
            }
            else
            {
                if (smallest_cmg(pv_temp1.var, pv_temp1.prev, pv_temp2.var, pv_temp2.prev) >= 0)
                    return false;
            }
        }
        for (int j = 0; j < pp_num2; j++)
        {
            pp_temp2 = operators[op2].pre_post[j];
            if (pv_temp1.var == pp_temp2.var)
            {
                if (pv_temp1.prev != pp_temp2.post)
                    return false;
                if ((pp_temp2.pre != -1) && (pv_temp1.prev != pp_temp2.pre))
                    return false;
            }
            else
            {
                if (smallest_cmg(pv_temp1.var, pv_temp1.prev, pp_temp2.var, pp_temp2.post) >= 0)
                    return false;
                if ((pp_temp2.pre != -1) &&
                    (smallest_cmg(pv_temp1.var, pv_temp1.prev, pp_temp2.var, pp_temp2.pre) >= 0))
                    return false;
            }
        }
    }

    for (int i = 0; i < pp_num1; i++)
    {
        pp_temp1 = operators[op1].pre_post[i];
        for (int j = 0; j < pv_num2; j++)
        {
            pv_temp2 = operators[op2].prevail[j];
            if (pv_temp2.var == pp_temp1.var)
            {
                if (pv_temp2.prev != pp_temp1.post)
                    return false;
                if ((pp_temp1.pre != -1) && (pv_temp2.prev != pp_temp1.pre))
                    return false;
            }
            else
            {
                if (smallest_cmg(pv_temp2.var, pv_temp2.prev, pp_temp1.var, pp_temp1.post) >= 0)
                    return false;
                if ((pp_temp1.pre != -1) &&
                    (smallest_cmg(pv_temp2.var, pv_temp2.prev, pp_temp1.var, pp_temp1.pre) >= 0))
                    return false;
            }
        }
        for (int j = 0; j < pp_num2; j++)
        {
            pp_temp2 = operators[op2].pre_post[j];
            if (pp_temp1.var == pp_temp2.var)
            {
                if (pp_temp1.post != pp_temp2.post)
                    return false;
                if (pp_temp1.pre != -1 && pp_temp2.pre != -1)
                {
                    if (pp_temp1.pre != pp_temp2.pre) // ||
                                                      //     operators[op1].CodInVar[pp_temp1.var] != operators[op2].CodInVar[pp_temp1.var])
                        return false;
                }
            }
            else
            {
                if (smallest_cmg(pp_temp1.var, pp_temp1.post, pp_temp2.var, pp_temp2.post) >= 0)
                    return false;
                if ((pp_temp1.pre != -1) && (pp_temp2.pre != -1) &&
                    (smallest_cmg(pp_temp1.var, pp_temp1.pre, pp_temp2.var, pp_temp2.pre) >= 0))
                    return false;
            }
        }
    }
    return true;
}

Node<NegConstraint> *PLProblem::Gen_NodeNegC(int op1, int op2)
{
    int i;
    int pv_num1 = operators[op1].get_pv_num();
    int pv_num2 = operators[op2].get_pv_num();
    int pp_num1 = operators[op1].get_pp_num();
    int pp_num2 = operators[op2].get_pp_num();
    int size = pv_num1 + pv_num2 + 2 * (pp_num1 + pp_num2);
    Node<NegConstraint> *NNegC = new Node<NegConstraint>;
    Node<Tuple> *NTp = new Node<Tuple>;
    NNegC->data.size = size;
    NNegC->data.v_size = var_size;
    NNegC->data.var_loc = new int[2 * var_size];
    NNegC->data.var_list = new int[size];
    NTp->data.op1 = op1;
    NTp->data.op2 = op2;
    NTp->data.size = size;
    NTp->data.values = new int[size];
    struct Prevail pv_temp1, pv_temp2;
    struct PrePost pp_temp1, pp_temp2;

    for (i = 0; i < pv_num1; i++)
    {
        pv_temp1 = operators[op1].prevail[i];
        NNegC->data.var_list[i] = 2 * pv_temp1.var;
        NNegC->data.var_loc[2 * pv_temp1.var] = i;
        NTp->data.values[i] = pv_temp1.prev;
    }
    for (int j = 0; j < pp_num1; j++)
    {
        pp_temp1 = operators[op1].pre_post[j];
        if (pp_temp1.pre != -1)
        {
            NNegC->data.var_list[i] = 2 * pp_temp1.var;
            NNegC->data.var_loc[2 * pp_temp1.var] = i;
            NTp->data.values[i] = pp_temp1.pre;
            i++;
        }
        NNegC->data.var_list[i] = 2 * pp_temp1.var + 1;
        NNegC->data.var_loc[2 * pp_temp1.var + 1] = i;
        NTp->data.values[i] = pp_temp1.post;
        i++;
    }
    for (int j = 0; j < pv_num2; j++)
    {
        pv_temp2 = operators[op2].prevail[j];
        int var = pv_temp2.var;
        int loc = NNegC->data.var_loc[2 * var];
        if ((loc < 0) ||
            (loc >= i) ||
            (NNegC->data.var_list[loc] != 2 * var))
        {
            NNegC->data.var_list[i] = 2 * var;
            NNegC->data.var_loc[2 * var] = i;
            NTp->data.values[i] = pv_temp2.prev;
            i++;
        }
    }
    for (int j = 0; j < pp_num2; j++)
    {
        pp_temp2 = operators[op2].pre_post[j];
        int var = pp_temp2.var;
        int loc = NNegC->data.var_loc[2 * var];
        if ((pp_temp2.pre != -1) &&
            ((loc < 0) ||
             (loc >= i) ||
             (NNegC->data.var_list[loc] != 2 * var)))
        {
            NNegC->data.var_list[i] = 2 * var;
            NNegC->data.var_loc[2 * var] = i;
            NTp->data.values[i] = pp_temp2.pre;
            i++;
        }
        loc = NNegC->data.var_loc[2 * var + 1];
        if ((loc < 0) ||
            (loc >= i) ||
            (NNegC->data.var_list[loc] != 2 * var + 1))
        {
            NNegC->data.var_list[i] = 2 * var + 1;
            NNegC->data.var_loc[2 * var + 1] = i;
            NTp->data.values[i] = pp_temp2.post;
            i++;
        }
    }
    NNegC->data.size = i;
    NTp->data.size = i;
    NNegC->data.tuples.AddLast(NTp);
    return NNegC;
}

void PLProblem::AddNegC(int op1, int op2)
{
    Node<NegConstraint> *NNegC1;
    Node<NegConstraint> *NNegC2;

    NNegC1 = Gen_NodeNegC(op1, op2);
    NNegC2 = NegCons.find(NNegC1->data);
    if (NNegC2 == 0)
        NegCons.AddLast(NNegC1);
    else
    {
        if (NNegC2->data.tuples.find(NNegC1->data.tuples.first->data) == 0)
        {
            NNegC2->data.permut_tuple(NNegC1->data, NNegC1->data.tuples.first->data);
            NNegC2->data.tuples.AddLast(NNegC1->data.tuples.first);
            NNegC1->data.tuples.first = 0; /// AddLast with pointer input gets Node so its pointer
        }                                  /// , i.e. first, should be set to 0 or NULL
        delete NNegC1;
    }
}

void PLProblem::generate_negcons()
{
    //cout<<"llllllllllllll"<<endl;
    //********************************8
    for (int i = 0; i < var_size; i++)
    {
        // auxCol[k] = new int[edge_num];
        // for(int cc=0;cc<edge_num;cc++)
        //  auxCol[k][cc] = -1;
        //variables[i].initialize_tuples(var_size);
        if (variables[i].MaxCoNum > 1)
        {
            variables[i].aux[i] = true;
            variables[i].auxDom = variables[i].MaxCoNum;
            for (int r = 0; r < variables[i].range; r++)
                variables[i].auxCol[i][r] = 0;

            Node<int> *p = variables[i].opList.first;
            int op_ind = 0;
            while (p)
            {
                int op = p->data;
                variables[i].auxCol[i][op_ind] = operators[op].CodInVar[i];

                int pp_num = operators[op].get_pp_num();
                for (int j = 0; j < pp_num; j++)
                {
                    int pstVar = operators[op].get_pre_post(j).var;
                    variables[pstVar].aux[i] = true;
                    int opInd = variables[pstVar].find_op_index(op);
                    variables[pstVar].auxCol[i][opInd] = operators[op].CodInVar[i];
                    //  variables[pstVar].auxCol[i][opInd] = variables[i].auxCol[i][variables[i].find_op_index(op)];
                }

                op_ind++;
                p = p->next;
            }
        }

        // cout<<"kkkkkkkkkkkkkkkkk"<<endl;
        /*






   for(int i=0; i<var_size; i++)
   {
      //variables[i].aux[i] = false;
      for(int j=0; j<variables[i].range; j++)
      {
         NodeList<int>* confOps = new NodeList<int>[variables[i].range];
         Node<int> *q = variables[i].InNegPrePosts[j].first;
         while(q!=0)
         {
            int op1 = q->data;
            Node<IntPair> *b = variables[i].InPosPrePosts[j].first;
            while (b!=0)
            {
               int op2 = b->data.code;
               int value = b->data.value;

               if (Check_parallel(op1,op2))
               {
                   // confOps.Add(op1);
                    if(!variables[i].aux[i])
                    {
                       variables[i].aux[i] = true;
                  //     variables[i].auxCol = new int[variables[i].edge_num];
                       for(int cc=0;cc<variables[i].edge_num;cc++)
                          variables[i].auxCol[i][cc]=0;
                    }

                    confOps[value].Add(op2);
           //         AddNegC(op1,op2);
               }

               b = b->next;
            }
            q = q->next;
         }
         Node<IntPair> *r = variables[i].InPosPrePosts[j].first;
         while (r!=0)
         {
            int op1 = r->data.code;
            int pre1 = r->data.value;
            Node<IntPair> *c = r->next;
            while (c!=0)
            {
               int op2 = c->data.code;
               int  pre2 = c->data.value;
               if ((pre1==pre2) &&
                   (Check_parallel(op1,op2)))
                   {
                     if(!variables[i].aux[i])
                     {
                       variables[i].aux[i] = true;
                      // variables[i].auxCol = new int[variables[i].edge_num];
                       for(int cc=0;cc<variables[i].edge_num;cc++)
                          variables[i].auxCol[i][cc]=0;
                     }
                     confOps[pre1].Add(op1);
                     confOps[pre1].Add(op2);
         //            AddNegC(op1,op2);

                   }

               c = c->next;
            }
            r = r->next;
         }
        // cout<<i<<endl;
       //  cout<<variables[i].aux[i]<<endl;
        // cout<<operators[91].name<<endl;
        // cout<<operators[21].name<<endl;
        // cout<<Check_parallel(91,21)<<endl;
    //    cout<<variables[0].find_op_index(21)<<endl;
      //   cout<<variables[10].find_op_index(21)<<endl;
        //  cout<<variables[8].find_op_index(21)<<endl;

//confOps.dump();
        // int aux_val=1;
         if(variables[i].aux[i])
         {
            int aux_val=1;
            for(int r=0;r<variables[i].range;r++)
            {
                aux_val=1;
                Node<int> *cp = confOps[r].first;
                while(cp)
                {
                   int op = cp->data;
                   int tableIndex= variables[i].find_op_index(op);
                   variables[i].auxCol[i][tableIndex]=aux_val;
                   aux_val++;
                   cp=cp->next;
                }
                if(variables[i].auxDom<aux_val)
                   variables[i].auxDom=aux_val;
            }
*/
        /*           int aux_val=1;
            int op;
            int tableIndex
            for(int r=0;r<variables[i].range;r++)
            {
                aux_val=1;
                Node<int> *cp = confOps[r].first;
                while(cp && cp->next)
                {
                   op = cp->data;
                   tableIndex= variables[i].find_op_index(op);
                   variables[i].auxCol[i][tableIndex]=aux_val;
                   aux_val++;
                   cp=cp->next;
                }
                if(cp)
                {
                   op = cp->data;
                   tableIndex= variables[i].find_op_index(op);
                   variables[i].auxCol[i][tableIndex]=aux_val;
                   aux_val++;
                }
                if(variables[i].auxDom<aux_val)
                   variables[i].auxDom=aux_val;
            }
            */

        /*
            Node<int> *p=variables[i].opList.first;
            int op_ind=0;
            while(p)
            {
              int op = p->data;
              int pp_num = operators[op].get_pp_num();
              for(int p=0; p<pp_num; p++)
              {
                int pstVar = operators[op].get_pre_post(p).var;
                variables[pstVar].aux[i]=true;
                int opInd = variables[pstVar].find_op_index(op);
                variables[pstVar].auxCol[i][opInd] = variables[i].auxCol[i][op_ind];
              //  variables[pstVar].auxCol[i][opInd] = variables[i].auxCol[i][variables[i].find_op_index(op)];
              }
              op_ind++;
              p = p->next;
            }


         }

         for(int r=0;r<variables[i].range;r++)
           confOps[r].Destroy();
        delete[] confOps;
         //cout<<endl;
      //   int yyy;
        // cin>>yyy;
      }
*/
        /*   if(variables[i].aux){
      for(int ccc=0;ccc<variables[i].edge_num;ccc++)
         cout<<variables[i].auxCol[ccc]<<" ";
      cout<<endl;
      cout<<variables[i].opList.size<<endl;
    }
*/
    }
}

void PLProblem::generate_mx_negcons()
{
    for (int j = 0; j < var_size; j++)
    {
        for (int i = j + 1; i < var_size; i++)
        {
            if (variables[i].MxNegC[j] != 0)
            {
                MxNegCons.AddLast(variables[i].MxNegC[j]);
                variables[i].MxNegC[j] = 0;
            }
        }
    }
}

int PLProblem::MinDist(int var, int ivalue, int gvalue)
{
    if (ivalue == gvalue)
        return 0;
    if (variables[var].InNegPrePosts[gvalue].size != 0)
        return 1;
    int opcode;
    int i, mdist;
    int first = 0;
    int last = 1;
    int nvalue, pvalue;
    int range = variables[var].range;
    int *dist = new int[range];
    int *pque = new int[range];
    for (i = 0; i < range; i++)
    {
        dist[i] = -1;
        pque[i] = -1;
    }
    dist[gvalue] = 0;
    pque[0] = gvalue;

    while (dist[ivalue] == -1)
    {
        int m = first;
        for (i = first + 1; i < last; i++)
            if (dist[pque[i]] < dist[pque[m]])
                m = i;
        if (m != first)
        {
            int t = pque[first];
            pque[first] = pque[m];
            pque[m] = t;
        }
        nvalue = pque[first];
        pque[first++] = -1;
        if (variables[var].InNegPrePosts[nvalue].size != 0)
        {
            dist[ivalue] = dist[nvalue] + 1;
            break;
        }
        Node<IntPair> *p = variables[var].InPosPrePosts[nvalue].first;
        while (p != 0)
        {
            opcode = p->data.code;
            pvalue = p->data.value;
            if (dist[pvalue] == -1)
            {
                dist[pvalue] = dist[nvalue] + 1;
                if (pvalue == ivalue)
                    break;
                pque[last++] = pvalue;
            }
            p = p->next;
        }
    }
    mdist = dist[ivalue];
    delete[] dist;
    delete[] pque;
    return mdist;
}
int PLProblem::MinPlanLen()
{
    int i;
    int var;
    int plen;
    int ivalue;
    int gvalue;
    int mplen = 1;
    int size = goal.get_size();
    for (i = 0; i < size; i++)
    {
        var = goal.get_variable(i);
        gvalue = goal.get_value(i);
        ivalue = initial_state[var];
        plen = MinDist(var, ivalue, gvalue);
        if (plen > mplen)
            mplen = plen;
    }
    return mplen;
}

void PLProblem::MaxConsArrities()
{
    int MaxPosAr = 0;
    int MaxNegAr = 0;
    for (int i = 0; i < var_size; i++)
    {
        if (variables[i].nb_num > MaxPosAr)
            MaxPosAr = variables[i].nb_num;
    }
    Node<NegConstraint> *p = NegCons.first;
    while (p != 0)
    {
        if (p->data.size > MaxNegAr)
            MaxNegAr = p->data.size;
        p = p->next;
    }
    cout << endl
         << endl;
    cout << "Max Positive Arrity = " << MaxPosAr << endl;
    cout << "Max Negative Arrity = " << MaxNegAr << endl
         << endl;
}

void PLProblem::generate_MinionFile(string file_attach)
{
    ofstream minionFile;
    // char path[200] = "";
    string path("");
    // strcat(path,file_attach);
    //strcat(path,"problem.minion");
    path = path + file_attach + "problem.minion";
    minionFile.open(path.c_str(), ios::out);

    if (!minionFile)
        cerr << "could not open CSP output file";

    minionFile << "MINION 3" << endl;
    minionFile << "**VARIABLES**" << endl;
    for (int i = 0; i <= plan_length; i++)
    {
        for (int j = 0; j < var_size; j++)
        {
            minionFile << "DISCRETE var" << j << "_" << i << " {0..";
            minionFile << variables[j].get_range() - 1 << "}" << endl;
            if (variables[j].aux[j] && i != plan_length)
            {
                minionFile << "DISCRETE aux" << j << "_" << i << " {0..";
                minionFile << variables[j].auxDom - 1 << "}" << endl;
            }
        }
        /*      for(int j=0;j<var_size;j++)
      {
        // minionFile<<"DISCRETE var"<<j<<"_"<<i<<" {0..";
        // minionFile<<variables[j].get_range()-1<<"}"<<endl;
         if(variables[j].aux[j])
         {
            minionFile<<"DISCRETE aux"<<j<<"_"<<i<<" {0..";
            minionFile<<variables[j].auxDom<<"}"<<endl;
         }
      }*/
    }

    minionFile << "**SHORTTUPLELIST**" << endl;
    for (int i = 0; i < var_size; i++)
    {
        int auxIndex;
        if (variables[i].tuples != 0)
        {
            minionFile << "tl_" << i << " " << variables[i].edge_num + variables[i].range << endl;
            int max_ind = 0;
            for (unsigned int j = 0; j < variables[i].edge_num; j++)
            {
                minionFile << "[";
                int k;
                for (k = 0; k < variables[i].nb_num; k++)
                {
                    if (variables[i].tuples[j][k] != -1)
                        minionFile << "(" << k << "," << variables[i].tuples[j][k] << ")";

                    // if(variables[i].aux[variables[i].nb_list[k]/2])
                    //   minionFile<<"("<<k<<","<<variables[i].auxCol[variables[i].nb_list[k]][j]<<")";
                }
                int ind = variables[i].nb_num;
                for (k = 0; k < var_size; k++)
                {
                    //    if(variables[i].tuples[j][k]!=-1)
                    //      minionFile<<"("<<k<<","<<variables[i].tuples[j][k]<<")";

                    if (variables[i].aux[k])
                    {
                        if (variables[i].auxCol[k][j] != -1)
                            minionFile << "(" << ind << "," << variables[i].auxCol[k][j] << ")";
                        ind++;
                    }
                }
                //    if((ind-1)>max_ind)
                //     max_ind = ind-1;
                //    if(variables[i].aux){
                //         minionFile<<"("<<k<<","<<variables[i].auxCol[j]<<")";
                //         auxIndex=k;
                //   }
                minionFile << "]" << endl;
            }
            for (int k = 0; k < variables[i].get_range(); k++)
            {
                minionFile << "[(" << variables[i].nb_loc[2 * i] << "," << k << ")";
                minionFile << "(" << variables[i].nb_loc[2 * i + 1] << "," << k << ")";
                int ind = 0;

                if (variables[i].aux[i])
                {
                    int ind = 0;
                    for (int c = 0; c < i; c++)
                    {
                        if (variables[i].aux[c])
                            ind++;
                    }
                    minionFile << "(" << ind + variables[i].nb_num << ",0)";
                }
                minionFile << "]" << endl;
            }
        }
    }

    Node<NegConstraint> *p = NegCons.first;
    int ng_count = 0;
    minionFile << "**TUPLELIST**" << endl;
    while (p != 0)
    {
        minionFile << "nl_" << ng_count++ << " " << p->data.tuples.size << "  " << p->data.size << endl;
        Node<Tuple> *q = p->data.tuples.first;
        while (q != 0)
        {
            for (int index = 0; index < q->data.size; index++)
                minionFile << q->data.values[index] << " ";
            minionFile << endl;
            q = q->next;
        }

        p = p->next;
    }

    p = MxNegCons.first;
    int mxng_count = 0;
    while (p != 0)
    {
        minionFile << "mxl_" << mxng_count++ << " " << p->data.tuples.size << "  " << p->data.size << endl;
        Node<Tuple> *q = p->data.tuples.first;
        while (q != 0)
        {
            for (int index = 0; index < q->data.size; index++)
                minionFile << q->data.values[index] << " ";
            minionFile << endl;
            q = q->next;
        }

        p = p->next;
    }

    minionFile << "**CONSTRAINTS**" << endl;
    for (int i = 0; i < var_size; i++)
        minionFile << "eq(var" << i << "_0," << initial_state[i] << ")" << endl;
    for (int i = 0; i < goal.get_size(); i++)
        minionFile << "eq(var" << goal.get_variable(i) << "_" << plan_length << "," << goal.get_value(i) << ")" << endl;
    minionFile << endl;
    for (int i = 0; i < plan_length; i++)
    {
        for (int j = 0; j < var_size; j++)
        {
            if (variables[j].tuples != 0)
            {
                minionFile << "shortstr2([";
                for (int k = 0; k < variables[j].nb_num - 1; k++)
                {
                    if (variables[j].nb_list[k] % 2 == 0)
                        minionFile << "var" << variables[j].nb_list[k] / 2 << "_" << i << ", ";
                    else
                        minionFile << "var" << variables[j].nb_list[k] / 2 << "_" << i + 1 << ", ";
                }
                if (variables[j].nb_list[variables[j].nb_num - 1] % 2 == 0)
                {
                    minionFile << "var" << variables[j].nb_list[variables[j].nb_num - 1] / 2 << "_" << i;
                    for (int v = 0; v < var_size; v++)
                        if (variables[j].aux[v])
                            minionFile << ", aux" << v << "_" << i;
                    minionFile << "],";
                }
                else
                {
                    minionFile << "var" << variables[j].nb_list[variables[j].nb_num - 1] / 2 << "_" << i + 1;
                    for (int v = 0; v < var_size; v++)
                        if (variables[j].aux[v])
                            minionFile << ", aux" << v << "_" << i;
                    minionFile << "],";
                }
                minionFile << "tl_" << j << ")" << endl;
            }
        }
    }

    for (int i = 0; i < plan_length; i++)
    {
        Node<NegConstraint> *p = NegCons.first;
        int ng_count = 0;
        while (p != 0)
        {
            minionFile << "negativetable([";
            for (int index = 0; index < p->data.size - 1; index++)
            {
                if (p->data.var_list[index] % 2 == 0)
                    minionFile << "var" << p->data.var_list[index] / 2 << "_" << i << ",";
                else
                    minionFile << "var" << p->data.var_list[index] / 2 << "_" << i + 1 << ",";
            }
            if (p->data.var_list[p->data.size - 1] % 2 == 0)
                minionFile << "var" << p->data.var_list[p->data.size - 1] / 2 << "_" << i << "],";
            else
                minionFile << "var" << p->data.var_list[p->data.size - 1] / 2 << "_" << i + 1 << "],";
            minionFile << "nl_" << ng_count++ << ")" << endl;
            p = p->next;
        }
    }
    for (int i = 0; i <= plan_length; i++)
    {
        Node<NegConstraint> *p = MxNegCons.first;
        int mxng_count = 0;
        while (p != 0)
        {
            minionFile << "negativetable([";

            minionFile << "var" << p->data.var_list[0] << "_" << i << ",";

            minionFile << "var" << p->data.var_list[1] << "_" << i << "],";

            minionFile << "mxl_" << mxng_count++ << ")" << endl;
            p = p->next;
        }
    }
    minionFile << "**EOF**";
    minionFile.close();
}

bool PLProblem::IsSatisfiable(string file_attach)
{
    string s;
    int satisfiable;
    double m_time;
    ifstream time_file;
    string t_path("");
    t_path = t_path + file_attach + "out.txt";
    // int x;
    // cin>>x;
    time_file.open(t_path.c_str(), ios::in);

    if (!time_file)
        cerr << "could not open out file";
    //for minion 16
    /*
    for(int gh=0;gh<15;gh++)
       time_file>>s;
    for(int j=0;j<9;j++)
       time_file>>s;
    time_file>>satisfiable;
    for(int j=10;j<14;j++)
      time_file>>s;
    time_file>>m_time;
    cout<<"minion time="<<m_time<<endl;
    minion_time+=m_time;
    time_file.close();
*/
    //for minion 2

    for (int gh = 0; gh < 16; gh++)
        time_file >> s;
    for (int j = 0; j < 9; j++)
        time_file >> s;
    time_file >> satisfiable;
    for (int j = 10; j < 15; j++)
        time_file >> s;
    time_file >> m_time;
    cout << "minion time=" << m_time << endl;
    minion_time += m_time;
    time_file.close();

    //int xx;
    //cin>>xx;

    if (remove(t_path.c_str()) != 0)
        cerr << "Error deleting out file" << endl;
    if (satisfiable == 1)
        return true;
    else
        return false;
}
void PLProblem::extractPlan(string file_attach)
{
    values = new int *[plan_length + 1];
    for (int i = 0; i <= plan_length; i++)
        values[i] = new int[2 * var_size];

    ifstream result_file;
    string r_path("");
    r_path = r_path + file_attach + "result.txt";
    result_file.open(r_path.c_str(), ios::in);
    if (!result_file)
        cerr << "could not open result file";

    for (int p = 0; p <= plan_length; p++)
    {
        for (int v = 0; v < var_size; v++)
        {
            result_file >> values[p][v];
            if (variables[v].aux[v] && p != plan_length)
                result_file >> values[p][v + var_size];
        }
    }

    /*  for(int p=0;p<=plan_length;p++)
   {
      for(int v=0;v<var_size;v++)
      {
         cout<<values[p][v]<<" ";
      }
      if(p!=plan_length)
      for(int v=0;v<var_size;v++)
      {
         if(variables[v].aux[v])
           cout<<"aux"<<v<<"="<<values[p][v+var_size]<<" ";
      }
      cout<<endl;
   }
*/

    result_file.close();
    if (remove(r_path.c_str()) != 0)
        cerr << "Error deleting result file" << endl;

    ofstream planFile;
    string p_path("");
    p_path = p_path + file_attach + "plan.txt";
    planFile.open(p_path.c_str(), ios::out);
    if (!planFile)
        cerr << "could not open plan output file";

    NodeList<int> ops;
    int tt = 0;
    for (int pl = 0; pl < plan_length; pl++)
    {
        cout << endl
             << "actions of time" << pl << ":" << endl;
        //      int hhh;
        //      cin>>hhh;
        //   cout<<values[pl][1]<<"  "<<values[pl+1][1]<<endl;
        for (int v = 0; v < var_size; v++)
        {
            int value1 = values[pl][v];
            int value2 = values[pl + 1][v];

            if (value1 != value2)
            {
                int op;
                bool found = false;
                //if(op==214)
                //  cout<<"Heeeeeeeeeeeeeeeeeeeeeeeeeey"<<endl;
                //  cout<<endl;
                Node<IntPair> *p = variables[v].InPosPrePosts[value2].first;
                Node<int> *q = variables[v].InNegPrePosts[value2].first;

                while (p != 0 && !found)
                {

                    if (p->data.value == value1)
                    {
                        op = p->data.code;
                        //  cout<<op<<"  ";
                        int pv_num = operators[op].pv_num;
                        int pp_num = operators[op].pp_num;
                        struct Prevail pv_temp;
                        struct PrePost pp_temp;
                        found = true;
                        for (int pv = 0; pv < pv_num && found; pv++)
                        {
                            pv_temp = operators[op].prevail[pv];
                            if (values[pl][pv_temp.var] != pv_temp.prev ||
                                values[pl + 1][pv_temp.var] != pv_temp.prev)
                                found = false;
                        }
                        for (int pp = 0; pp < pp_num && found; pp++)
                        {
                            pp_temp = operators[op].pre_post[pp];
                            if (values[pl + 1][pp_temp.var] != pp_temp.post ||
                                (pp_temp.pre != -1 && values[pl][pp_temp.var] != pp_temp.pre))
                                found = false;
                            int pp_var = operators[op].pre_post[pp].var;
                            if (found && variables[v].aux[pp_var])
                            {
                                if (variables[v].auxCol[pp_var][variables[v].find_op_index(op)] != -1 &&
                                    variables[v].auxCol[pp_var][variables[v].find_op_index(op)] != values[pl][pp_var + var_size])
                                    found = false;
                            }
                        }
                        if (found && !ops.find(op))
                        {
                            ops.AddLast(op);
                            // found = false;
                        }
                        /*                       if(found)
                       {
                            int pp_num = operators[op].pp_num;
                            //bool varFlag = true;
                            for(int h=0;h<pp_num;h++)
                            {
                                int pp_var = operators[op].pre_post[h].var;
                                if(variables[v].aux[pp_var])
                                {
                                     if(variables[v].auxCol[pp_var][variables[v].find_op_index(op)]!=-1 &&
                                       variables[v].auxCol[pp_var][variables[v].find_op_index(op)]!=values[pl][pp_var+var_size])
                            //ops.AddLast(op);
                                           found=false;
                                }

                             }

                       }*/
                    } //end-if (p->data.value == value1)*/
                    p = p->next;
                } //end-while InPosPrePosts

                while (q != 0 && !found)
                {
                    op = q->data;
                    int pv_num = operators[op].pv_num;
                    int pp_num = operators[op].pp_num;
                    struct Prevail pv_temp;
                    struct PrePost pp_temp;
                    found = true;
                    for (int pv = 0; pv < pv_num && found; pv++)
                    {
                        pv_temp = operators[op].prevail[pv];
                        if (values[pl][pv_temp.var] != pv_temp.prev ||
                            values[pl + 1][pv_temp.var] != pv_temp.prev)
                            found = false;
                    }
                    for (int pp = 0; pp < pp_num && found; pp++)
                    {
                        pp_temp = operators[op].pre_post[pp];
                        if (values[pl + 1][pp_temp.var] != pp_temp.post ||
                            (pp_temp.pre != -1 && values[pl][pp_temp.var] != pp_temp.pre))
                            found = false;
                        int pp_var = operators[op].pre_post[pp].var;
                        if (found && variables[v].aux[pp_var])
                        {
                            if (variables[v].auxCol[pp_var][variables[v].find_op_index(op)] != -1 &&
                                variables[v].auxCol[pp_var][variables[v].find_op_index(op)] != values[pl][pp_var + var_size])
                                found = false;
                        }
                    }
                    if (found && !ops.find(op))
                    {
                        ops.AddLast(op);
                        found = false;
                    }

                    q = q->next;
                } //end-while InNegPrePosts
                //    if (found && !ops.find(op) )
                //    {
                //          ops.AddLast(op);
                //    }
            } //end-if (value1!= value2)
            else
            {
                int op;
                bool found = false;
                //if(op==214)
                //  cout<<"Heeeeeeeeeeeeeeeeeeeeeeeeeey"<<endl;
                //  cout<<endl;
                //    Node<IntPair> *p = variables[v].InPosPrePosts[value2].first;
                Node<int> *q = variables[v].InNegPrePosts[value2].first;

                while (q != 0 && !found)
                {
                    op = q->data;
                    int pv_num = operators[op].pv_num;
                    int pp_num = operators[op].pp_num;
                    struct Prevail pv_temp;
                    struct PrePost pp_temp;
                    found = true;
                    for (int pv = 0; pv < pv_num && found; pv++)
                    {
                        pv_temp = operators[op].prevail[pv];
                        if (values[pl][pv_temp.var] != pv_temp.prev ||
                            values[pl + 1][pv_temp.var] != pv_temp.prev)
                            found = false;
                    }
                    for (int pp = 0; pp < pp_num && found; pp++)
                    {
                        pp_temp = operators[op].pre_post[pp];
                        if (values[pl + 1][pp_temp.var] != pp_temp.post ||
                            (pp_temp.pre != -1 && values[pl][pp_temp.var] != pp_temp.pre))
                            found = false;
                        int pp_var = operators[op].pre_post[pp].var;
                        if (found && variables[v].aux[pp_var])
                        {
                            if (variables[v].auxCol[pp_var][variables[v].find_op_index(op)] != -1 &&
                                variables[v].auxCol[pp_var][variables[v].find_op_index(op)] != values[pl][pp_var + var_size])
                                found = false;
                        }
                    }
                    if (found && !ops.find(op))
                    {
                        ops.AddLast(op);
                        found = false;
                    }

                    q = q->next;
                } //end-while InNegPrePosts
                //    if (found && !ops.find(op) )
                //    {
                //          ops.AddLast(op);
                //    }
            } //end-if-else (value1!= value2)

        } //end-for v
        int op;
        Node<int> *r;
        while (ops.first != 0)
        {
            r = ops.first;
            op = r->data;
            cout << operators[op].get_name() << endl;
            // planFile<<pl<<".000: ("<<operators[op].get_name()<<")[1.000]"<<endl;
            planFile << tt << ".000: (" << operators[op].get_name() << ")[1.000]" << endl;
            //    cout<<endl<<endl;
            //   cout<<op<<endl;
            //   operators[op].dump();
            tt++;
            ops.first = r->next;
            delete r;
        }
        ops.last = 0;
        ops.size = 0;

        //    int hhh;
        //    cin>>hhh;
    } //end-for pl

    planFile.close();
}

void PLProblem::Solve(string minion_switches, string file_attach)
{
    sm_flag = true;
    //   cout<<"ddddddddddddddd"<<endl;
    generate_structures();
    //  cout<<"fffffffffffffffffffff"<<endl;
    generate_tuples();
    // cout<<"gggggggggggggggggggggg"<<endl;
    generate_negcons();

    //    cout<<"8888888888888"<<endl;
    //    cout<<variables[15].MaxCoNum<<endl;

    //  int hhh;
    //   cin>>hhh;

    //  sm_flag = false;

    if (sm_flag && mutex_count != 0)
    {
        //    sm_flag = true;
        generate_mx_negcons();
    }
    //  NegCons.dump();
    ///MaxConsArrities();
    // for (int i=0; i<var_size; i++)
    //   variables[i].dump(var_size, mutex_count);

    plan_length = MinPlanLen();
    cout << endl
         << "Init PlanLen =" << plan_length << endl;

    while (plan_length <= 100)
    {
        generate_MinionFile(file_attach);
        //   cout<<"ddddddddddddd"<<endl;
        /*char *path = NULL;
       path = getcwd(NULL, 0);
       char *out_path =new char[strlen(path)+15];
       strcpy(out_path, path);
       strcat(out_path, "/");
       strcat(out_path, file_attach);
       strcat(out_path, "out.txt");

       char *resl_path =new char[strlen(path)+15];
       strcpy(resl_path, path);
       strcat(resl_path, "/");
       strcat(resl_path, file_attach);
       strcat(resl_path, "result.txt");
       char *pro_path =new char[strlen(path)+15];
       strcpy(pro_path, path);
       strcat(pro_path, "/");
       strcat(pro_path, file_attach);
       strcat(pro_path, "problem.minion");
*/
        string minion_path("");
        minion_path = minion_path + "./minion2/bin/minion " + minion_switches + "  -printsolsonly  -noprintsols -solsout " + file_attach + "result.txt -tableout " + file_attach + "out.txt " + file_attach + "problem.minion"; //+pro_path;
                                                                                                                                                                                                                                //   minion_path = minion_path + "./minion2/bin/minion " + minion_switches + "  -dumptree -solsout  " + file_attach+"result.txt -tableout " + file_attach +"out.txt " + file_attach +"problem.minion";//+pro_path;
        const char *c = minion_path.c_str();
        int result = system(c);

        sat = IsSatisfiable(file_attach);
        if (sat)
            break;
        cout << "no plan of length " << plan_length << " found" << endl;
        //    int xx;
        //   cin>>xx;
        plan_length++;
        //plan_length+=0;
    }

    if (sat)
    {
        cout << "a plan of length " << plan_length << " found" << endl;
        //int xx;
        //    cin>>xx;

        extractPlan(file_attach);
    }
    else
        cout << "no plan of length<100 found" << endl;
}

PLProblem::~PLProblem()
{

    /*   if(variables)
       for(int i=0;i<var_size;i++){
            cout<<"var"<<i<<"  "<<variables[i].aux_size<<endl;
            variables[i].~Variable();

            }*/
    delete[] variables;

    if (mutexes)
        delete[] mutexes;
    if (operators)
        delete[] operators;
    if (values)
    {
        for (int i = 0; i <= plan_length; i++)
            delete[] values[i];
        delete[] values;
    }
}
