#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <time.h>
#include <sys/time.h>

#include "PLProblem.h"

using namespace std;

void remove_files(string prefix)
{
   ifstream sas_file, plan_file, flatzinc_file;

   string sas_filename = prefix + ".sas";
   sas_file.open(sas_filename.c_str(), ios::in);
   if (sas_file)
   {
      sas_file.close();
      if (remove(sas_filename.c_str()) != 0)
         cerr << "Error deleting output.sas" << endl;
   }

   string plan_filename = prefix + ".plan";
   plan_file.open(plan_filename.c_str(), ios::in);
   if (plan_file)
   {
      plan_file.close();
      if (remove(plan_filename.c_str()) != 0)
         cerr << "Error deleting plan.txt" << endl;
   }

   string flatzinc_filename = prefix + ".fzn";
   flatzinc_file.open(flatzinc_filename.c_str(), ios::in);
   if (flatzinc_file)
   {
      flatzinc_file.close();
      if (remove(flatzinc_filename.c_str()) != 0)
         cerr << "Error deleting problem.minion" << endl;
   }
}

int main(int argc, char *argv[])
{
   if (argc < 3)
   {
      cout << " You should provide DOMAIN and PROBLEM files";
      exit(1);
   }

   string prefix = string(tmpnam(nullptr));
   remove_files(prefix);

   string sas_filename = prefix + ".sas";
   string translate_path("");
   translate_path = translate_path +
                    "./utils/fast_downward/fast-downward.py " +
                    "--sas-file " + sas_filename + " " +
                    "--translate " +
                    argv[1] + " " + argv[2];

   const char *c = translate_path.c_str();
   int ret_val;
   ret_val = system(c);

   ifstream Infile;
   Infile.open(sas_filename.c_str(), ios::in);
   if (!Infile)
   {
      cerr << "could not open sas file";
   }

   PLProblem Problem;
   Problem.read_problem(Infile);
   Problem.Solve(prefix);

   string validate_path("");
   validate_path = validate_path + "./utils/VAL/validate" + " " + argv[1] + "  " + argv[2];
   validate_path = validate_path + " " + prefix + ".plan";
   const char *c1 = validate_path.c_str();
   ret_val = system(c1);

   remove_files(prefix);

   return 0;
}
