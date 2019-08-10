// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <regex>
#include <fstream>
#include <cerrno>
#include <iomanip>
#include <typeinfo>
#include <cassert>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

const string cin_name = "-";

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}

void read_lines (istream& infile, const string& filename,
        str_str_map& listmap, str_str_map::iterator& itor) {

   regex comment_regex {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex {R"(^\s*([^=]+?)\s*$)"};
   int linenr = 0;

   for (;;) {
      string line;
      getline (infile, line);
      if (infile.eof()) break;
      smatch result;
      cout << filename << ": " << ++linenr << line << endl;
      if (regex_search (line, result, comment_regex)) {
         continue;
      }
      else if (regex_search (line, result, key_value_regex)) {
         // _ = _
         if (result[1] == "" and result[2] == "") {
            itor = listmap.begin();
            while (itor != listmap.end()) {
               cout << itor->first << " = " << itor->second << endl;
               ++itor;
            }
         }
         // _ = value
         else if (result[1] == "") {
            itor = listmap.begin();
            while (itor != listmap.end()) {
               if (itor->second == result[2]) {
                  cout << itor->first << " = " << itor->second << endl;
               }
               ++itor;
            }
         }
         // key = _
         else if (result[2] == "") {
            itor = listmap.find (result[1]);
            if (itor != listmap.end()) {
               listmap.erase (itor);
            }
         }
         // key = value
         else {
            str_str_pair pair (result[1], result[2]);
            itor = listmap.insert (pair);
            cout << itor->first << " = " << itor->second << endl;
         }
      }
      else if(regex_search (line, result, trimmed_regex)) {
         itor = listmap.find (result[1]);
         if (itor == listmap.end()) {
            cout << result[1] << ": " << "key not found" << endl;
         }
         else {
            cout << itor->first << " = " << itor->second << endl;
         }
      }
   }
}

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);

   int status = 0;
   string progname (basename (argv[0]));
   str_str_map listmap;
   str_str_map::iterator itor;

   // no files given, read from cin
   if (argc == 1) read_lines (cin, cin_name, listmap, itor);
   for (char **argp = &argv[optind]; argp != &argv[argc]; ++argp) {
      if (*argp == cin_name) read_lines(cin, cin_name, listmap, itor);
      else {
         ifstream infile (*argp);
         if (infile.fail()) {
            status = 1;
            cerr << progname << ": " << *argp << ": "
                 << strerror (errno) << endl;
         }
         else {
            read_lines(infile, progname, listmap, itor);
            infile.close();
         }
      }
   }
   itor = listmap.begin();
   while (not listmap.empty()) {
      itor = listmap.erase(itor);
   }
   return status;
}

