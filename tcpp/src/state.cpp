#include "state.h"
#include "helper_function.h"

State::State()
{
    values = NULL;
    size = 0;
}

void State::input(istream &in,int var_size){
    values=new int[var_size];
    size = var_size;
    check_magic(in, "begin_state");
    for (int i = 0; i < var_size; i++) {
        in >> values[i];
    }
    check_magic(in, "end_state");
}

int State::operator[](int var){
    return values[var];
}

void State::dump(){
    for(int i=0;i<size;i++)
        cout<<"var"<<i<<"= "<<values[i]<<endl;
}

State :: ~State()
{
    if (values != NULL)
       delete[] values;
}
