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

#pragma once
#if !defined(COURSE_HPP)
#define COURSE_HPP

#include <fstream>
#include <map>
#include <string>

#include "bias.hpp"
#include "room.hpp"
#include "utility.hpp"

/*!
  Course descriptions are read from a CSV and pushed to the appropriate map.
  Returns true if an entry for the course id already exists.  Group descriptions
  are also read and input to the appropriate public vector.

  Of the public maps, one holds the courses to be scheduled, the other courses
  that are already scheduled but are listed for avoidance purposes
*/
class Course : public Bias, public Room
{
public:
  /*!
    The constructor does all the work by reading the CSV contents for courses
    and groups.  If the CSV contains an error, then the program will exit
    after writing the error(s) to debug.log.
  */
  Course();

  /*! Used to save course information read in by the c'tor */
  bool push_course        (course_t &course);

  /*! Used to save course information read in by the c'tor */
  bool push_const_course  (course_t &course);

  /*! Holds data for constant courses that do not need to be schedules */
  std::map<std::string, course_t> m_mapstr_const_course;

  /*! Holds data for courses needing to be scheduled */
  std::map<std::string, course_t> m_mapstr_course;

  /*! Holds group data from the groups CSV file read in by the c'tor */
  std::map<std::string, std::vector<std::string> > m_mapstr_core;
  std::map<std::string, std::vector<std::string> > m_mapstr_elec;


private:
  /*! For referencing the course CSV file */
  std::ifstream course_file;

  /*! For referencing the group CSV file */
  std::ifstream group_file;
};

/*!
  Flags used for IO parsing and error detection
*/
namespace
{
/*! Number of tokens per line in courses.csv*/
const int COURSE_TOKENS = 12;
const uint32_t COURSE_AVOID = 1 <<  0;
const uint32_t COURSE_DAYS  = 1 <<  1;
const uint32_t COURSE_GROUP = 1 <<  2;
const uint32_t COURSE_HOURS = 1 <<  3;
const uint32_t COURSE_ID    = 1 <<  4;
const uint32_t COURSE_INSTR = 1 <<  5;
const uint32_t COURSE_LECTS = 1 <<  6;
const uint32_t COURSE_NAME  = 1 <<  7;
const uint32_t COURSE_ROOM  = 1 <<  8;
const uint32_t COURSE_SIZE  = 1 <<  9;
const uint32_t COURSE_TIMES = 1 << 10;
const uint32_t COURSE_TYPE  = 1 << 11;
}

#endif // !defined(COURSE_HPP)
