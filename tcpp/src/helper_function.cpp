#include <cstdlib>
#include <iostream>
#include <fstream>

#include <string>
using namespace std;

#include "helper_function.h"

/*unsigned __int64    convert( const FILETIME & ac_FileTime )
{
  ULARGE_INTEGER    lv_Large ;

  lv_Large.LowPart  = ac_FileTime.dwLowDateTime   ;
  lv_Large.HighPart = ac_FileTime.dwHighDateTime  ;

  return lv_Large.QuadPart ;
}*/

void check_magic(istream &in, string magic) {
    string word;
    in >> word;
    if (word != magic) {
        cerr << "Failed to match magic word '" << magic << "'." << endl;
        cerr << "Got '" << word << "'." << endl;
        if (magic == "begin_version") {
            cerr << "Possible cause: you are running the preprocessor "
                 << "on a translator file from an " << endl
                 << "older version." << endl;
        }
        exit(1);
    }
}
