/*
 *    SACS, a Simulated Annealing Class Scheduler
 *    Copyright (C) 2011-2012  Martin Wyngaarden
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "debug.hpp"
#include "schedule.hpp"
#include "utility.hpp"

using namespace std;


int main()
{
  std::cout << "SACS, a Simulated Annealing Class Scheduler" << endl
            << "Copyright (C) 2011  Martin Wyngaarden (wyngaardenm@gmail.com)"
            << endl << endl
            << "Compiled " << COMPILE_DATE << " at " << COMPILE_TIME
            << endl << endl;

  prog_opts["BUFFER"]          = "4";
  prog_opts["CONTIGUOUS-LABS"] = "TRUE";
  prog_opts["LAB-START-TIME"]  = "8";
  prog_opts["LAB-END-TIME"]    = "21";
  prog_opts["POLL"]            = "10";
  prog_opts["REDUCTION"]       = "0.99";
  prog_opts["VERBOSE"]         = "TRUE";

  size_t found;

  string str;
  string option;
  string value;

  fstream config_file;
  stringstream oss;
  Debug debug;
  config_file.open (FILE_CONFIG);

  if (!config_file.is_open() )
    std::cout << "Configuration file (" << FILE_CONFIG << ") not found: using defaults!" << endl;

  // Read options from config file
  else
  {
    while (getline (config_file, str) )
    {
      if (str.empty() )
        continue;

      str = make_upper (str);
      // Remove whitespaces
      found = str.find (" ");

      while (found != string::npos)
      {
        str.erase (found, 1);
        found = str.find (" ");
      }

      // Ignore lines starting with a pound sign
      if (str.substr (0, 1) == "#")
        continue;

      option = get_token (str, 0, "=");
      value  = get_token (str, 1, "=");

      if (option == "" || value == "")
        continue;

      if (prog_opts.find (option) == prog_opts.end() )
      {
        oss << "Invalid option: " << option;
        debug.push_error (oss.str() );
        oss.str ("");
      }

      else
        prog_opts[option] = value;
    }
  }

  config_file.close();
  debug.live_or_die();

  // Precompute certain indices
  util_init();

  Schedule sched;

  std::cout << endl << "Optimizing schedule..." << endl << endl;

  sched.optimize();

  std::cout << endl
            << "Optimization complete (" << sched.duration() << " seconds)"
            << endl << endl;

  std::system ("pause");
  return 0;
}

