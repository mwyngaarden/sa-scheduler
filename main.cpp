/*    
 *    SACS, a Simulated Annealing Class Scheduler
 *    Copyright (C) 2011  Martin Wyngaarden
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
*/

#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/thread/thread.hpp>

#include "debug.hpp"
#include "schedule.hpp"
#include "utility.hpp"

using namespace std;
namespace po = boost::program_options;


int main(int argc, char *argv[])
{
  cout << endl
       << "Advanced Scheduler by Martin Wyngaarden (mwyngaarden@lssu.edu)"
       << endl << "CPU: POPCNT " << (cpu_has_popcnt() ? "" : "not ")
       << "detected and " << cpu_max_threads() << " threads available" << endl
       << "Compiled " << COMPILE_DATE << " at " << COMPILE_TIME << endl
       << endl;

  try {
    string config_file;

    // command line only options
    po::options_description common("Command line options");
    common.add_options()
      ("config,c", po::value<string>(&config_file)->default_value("sched.cfg"),
       "configuration file")
    
      ("help,h", "produce help message");

    // command line and configuration file options
    po::options_description config("Configuration");
    config.add_options()
      ("buffer,b", po::value<int>(&options[OPT_ROOMBUFF])->default_value(4),
       "room size buffer")

      ("contiguous-labs,l", po::value<int>(&options[OPT_CONTIGLABS])->default_value(1),
       "schedule labs in single blocks")

      ("lab-end-time,n", po::value<int>(&options[OPT_CLABETIME])->default_value(21),
       "end time for contiguous labs")

      ("lab-start-time,l", po::value<int>(&options[OPT_CLABSTIME])->default_value(8),
       "start time for contiguous labs")

      ("poll,p", po::value<int>(&options[OPT_POLLINTVL])->default_value(100),
       "interval between status updates")

      ("sync,s", po::value<int>(&options[OPT_SYNCINTVL])->default_value(1),
       "interval between thread synchronization")

      ("threads,t", po::value<int>(&options[OPT_THREADS])->default_value(1),
       "number of threads to use")

      ("verbose,v", po::value<int>(&options[OPT_VERBOSE])->default_value(1),
       "output status updates");
    
    po::options_description cmdline_options;
    cmdline_options.add(common).add(config);

    po::options_description config_file_options;
    config_file_options.add(config);

    po::options_description visible("Allowed options");
    visible.add(common).add(config);

    po::variables_map vm;
    store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
    notify(vm);

    ifstream ifs(config_file.c_str());

    if (!ifs) {
      cout << "Unable to open " << config_file << endl;
      return 0;
    }

    store(parse_config_file(ifs, config_file_options), vm);
    notify(vm);

    if (vm.count("help")) {
      cout << visible << endl;
      return 0;
    }

    options[OPT_HASPOPCNT] = cpu_has_popcnt();

    if (!options[OPT_THREADS]) {
      options[OPT_THREADS] = cpu_max_threads();
    }

  

    util_init();

    
    //boost::mt19937 my_rng(time(NULL));
    //ofstream stato("stato.csv");
    //options[OPT_VERBOSE] = 0;
    //int total = 0;
    //int runs = 0;
    Schedule sched;
    sched.optimize();
    /*
    
    for (int i = 0; i < 1; i++) {

      //MY_DIV = 2.0 + (0.2 * i);

      for (runs = 0, total = 0; runs < 400; runs++) {
        int x = sched.optimize();
        cout << i << ":" << runs << " " << x << endl;
        total += x;
      }

      cout << double(total) / double(runs) << endl;

    }

    return 0;
    */

    /*
    boost::thread_group threads;
    cout << endl << "Optimizing schedule..." << endl << endl;

    for (int i = 0; i < options[OPT_THREADS]; i++) {
      threads.create_thread(boost::bind(&Schedule::optimize, &sched));
    }

    threads.join_all();
    */

    cout << endl << "Optimization complete (" << sched.get_duration()
         << " seconds)" << endl << endl;

    system("pause");

  } catch (exception &e) {
    cout << "Exception: " << e.what() << endl;
    return 1;
  }

  return 0;
}

