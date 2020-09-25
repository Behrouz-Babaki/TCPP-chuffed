#include "mutex_group.h"

#include "helper_function.h"                   /// check_magic

MutexGroup::MutexGroup()
{
    facts = NULL;
    mg_size = 0;
}

bool MutexGroup::input(istream &in)
{
    bool flag=false;
    check_magic(in, "begin_mutex_group");
    in >> mg_size;
    facts=new struct Pair[mg_size];
    in >> facts[0].first >> facts[0].second;
    for (int i = 1; i < mg_size; ++i)
    {
       in >> facts[i].first >> facts[i].second;
       if (facts[i].first!=facts[i-1].first)
            flag = true;
    }
    check_magic(in, "end_mutex_group");
    if (!flag)                              /// Useless
    {
        delete[] facts;
        facts = NULL;
        mg_size=0;
    }
    return flag;
}

void MutexGroup::dump(){
    cout << "mutex group of size " << mg_size << ":" << endl;
    for (int i = 0; i < mg_size; ++i)
        cout << "varID = " << facts[i].first << "  value = " << facts[i].second<<endl;
    cout<<endl;
}

int MutexGroup::get_variable(int i)
{
    return facts[i].first;
}

int MutexGroup::get_value(int i)
{
    return facts[i].second;
}

int MutexGroup::get_size()
{
    return mg_size;
}

MutexGroup :: ~MutexGroup()
{
    if (facts!= NULL)
       delete[] facts;
}
