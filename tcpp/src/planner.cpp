#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <time.h>
#include <sys/time.h>

#include "PLProblem.h"

using namespace std;


int main(int argc, char *argv[])
{
   double minion_time;
   double cpu_time1,cpu_time2,cpu_time;
   cpu_time1 = (double) clock();

   if (argc<3)
    {
        cout<<" You should provide DOMAIN and PROBLEM files";
        exit(1);
    }
    string minion_switches("");
    for(int i=3;i<argc-1;i++)
       minion_switches= minion_switches + "  " + argv[i];

    string file_attach("");
    file_attach = file_attach + argv[argc-1]+ "_" ;

    string translate_path("");
    translate_path = translate_path + "python ./translate/translate.py"+ " "+ argv[1]+"  "+argv[2];

    ifstream sas_file,plan_file,minion_file,result_file,out_file;
    sas_file.open("output.sas", ios::in);
    if(sas_file)
    {
        sas_file.close();
        if( remove( "output.sas" ) != 0 )
           cerr<<"Error deleting output.sas"<<endl;
    }
    string pl_path("");
    pl_path = pl_path + file_attach + "plan.txt";
    plan_file.open(pl_path.c_str(), ios::in);
    if(plan_file)
    {
       plan_file.close();
       if( remove( pl_path.c_str() ) != 0 )
         cerr<<"Error deleting plan.txt"<<endl;
    }
    string min_path("");
    min_path = min_path + file_attach + "problem.minion";
    minion_file.open(min_path.c_str(), ios::in);
    if(minion_file)
    {
       minion_file.close();
       if( remove( min_path.c_str() ) != 0 )
          cerr<<"Error deleting problem.minion"<<endl;
    }
    string res_path("");
    res_path = res_path + file_attach + "result.txt";
    result_file.open(res_path.c_str(), ios::in);
    if(result_file)
    {
       result_file.close();
       if( remove(res_path.c_str()) != 0 )
          cerr<<"Error deleting result.txt"<<endl;
    }
    string out_path("");
    out_path = out_path + file_attach + "out.txt";
    out_file.open(out_path.c_str(), ios::in);
    if(out_file)
    {
       out_file.close();
      if( remove( out_path.c_str()) != 0 )
         cerr<<"Error deleting out.txt"<<endl;
   }




    const char * c = translate_path.c_str();
    system(c);

    ifstream Infile;
    Infile.open("output.sas", ios::in);
    if(!Infile)
    {
        cerr<<"could not open sas file";

    }

    PLProblem Problem;
    Problem.read_problem(Infile);
    Problem.Solve(minion_switches,file_attach);
    minion_time = Problem.getMinion_time();
    cpu_time2 = (double) clock();
    cpu_time = (cpu_time2-cpu_time1)/CLOCKS_PER_SEC;

    string validate_path("");
    validate_path = validate_path + "./VAL/validate"+ " "+ argv[1]+"  "+argv[2];
    validate_path = validate_path +" " +file_attach + "plan.txt ";
    const char * c1 = validate_path.c_str();
    system(c1);
    //system("./../VAL-4.2.09/validate ../benchmarks/airport/p07-domain.pddl ../benchmarks/airport/p07-airport2-p2.pddl ../files/plan.txt" );

    cout<<endl<<"modelling time = "<<cpu_time<<endl;
    cout<<"minion time = "<<minion_time<<endl;
    cout<<"total time = "<<minion_time + cpu_time<<endl;

    return 0;
}
