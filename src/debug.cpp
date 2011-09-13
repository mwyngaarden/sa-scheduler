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
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctime>
#include <iostream>

#include "debug.hpp"

using namespace std;


void Debug::live_or_die()
{
  string str;


  if (!m_vec_warnings.size() && !m_vec_errors.size()) {
    return;
  }
  
  debug_log.open("debug.log", ios::app);

  while (m_vec_warnings.size()) {
    str = m_vec_warnings.back();
    m_vec_warnings.pop_back();
    debug_log << str << endl;
  }

  if (m_vec_errors.size()) {
    while (m_vec_errors.size()) {
      str = m_vec_errors.back();
      m_vec_errors.pop_back();
      debug_log << str << endl;
    }

    cout << endl << "Catastrophic failure: see debug.log" << endl;
    debug_log.close();
    system("pause");
    exit(1);
  }

  debug_log.close();
}

void Debug::push_error(std::string str)
{
  time_t rawtime;
  time(&rawtime);
  string push_str = asctime(localtime(&rawtime));
  push_str += "  ERROR: " + str;
  m_vec_errors.push_back(push_str);
};

void Debug::push_warning(std::string str)
{
  time_t rawtime;
  time(&rawtime);
  string push_str = asctime(localtime(&rawtime));
  push_str += "  WARNING: " + str;
  m_vec_warnings.push_back(push_str);
};