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

#pragma once
#if !defined(DEBUG_HPP)
#define DEBUG_HPP

#include <fstream>
#include <string>
#include <vector>

/*!
  Debug class is used to track errors and warnings throughout program execution.
  live_or_die checks for errors and warnings and outputs them to debug.log.
  If an error is found, the program exits after sending a short message to the
  console.
*/
class Debug
{
  public:
    /*! 
      Checks for errors and warnings and outputs them to debug.log.  If an error
      is found, the program exits after sending a short message to the console. 
    */
    void live_or_die();

    /*! Saves an error to m_vec_errors */
    void push_error(std::string str);

    /*! Saves a warning to m_vec_warnings */
    void push_warning(std::string str);

  private:
    /*! Stores errors */
    std::vector<std::string> m_vec_errors;

    /*! Stores warnings */
    std::vector<std::string> m_vec_warnings;

    /*! Used for referencing debug.log */
    std::ofstream debug_log;
};

#endif // !defined(DEBUG_HPP)
