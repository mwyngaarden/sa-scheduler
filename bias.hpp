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
#if !defined(BIAS_HPP)
#define BIAS_HPP

#include <fstream>
#include <map>
#include <string>

#include "debug.hpp"
#include "utility.hpp"

/*!
  Inherited by class Course	

  Bias is for setting aversions, preferences, or blocks.  Is is also used to
  set global blocks using ALL as the instructor name.  As elsewhere, all times
  should be in 24 hour format, using .5 as the 30 minute mark.  For example,
  13.5 would be 1:30 PM.

  days is an 8-bit flag where the LSB represents Sunday, followed by Monday, etc.
  The MSB is unused.

  See e_bias in utility.hpp for a more details.

  The two overloaded functions are for inputting either a type bs_t or raw 
  day and time format using the start and end time and an 8-bit day flag.

  See the typedef declaration in utility.hpp for more details.

  The return type is int, which will be the bias score for those times and days.
  If the instructor does not have a listing, 0 is returned.  For evaluating
  schedules, this score should be maximized.

  m_mapstr_bias uses a 7*48 sized vector to index preferences, aversions, and
  blocks.  The zeroth element is Sunday at 00:00 hours, the first 00:30, the
  48th element is therefore Monday at 00:00 hours.
*/
class Bias
{
  public:
    /*!
      The constructor does all the work by reading the CSV contents and setting 
      the instructor preferences, aversions, and blocks.  If the CSV contains an 
      error, then the program will exit after writing the error(s) to debug.log.
    */ 
    Bias();

    int get_bias(const std::string &instr, const bs_t &bs);

    int get_bias(
      const std::string &instr,
      double start_time,
      double end_time,
      uint8_t days);

    void set_bias(
      const std::string &instr,
      double start_time,
      double end_time,
      uint8_t days,
      e_bias bias);

  private:
    std::map<std::string, std::vector<e_bias> > m_mapstr_bias;
    std::ifstream bias_file;
};

#endif // !defined(BIAS_HPP)
