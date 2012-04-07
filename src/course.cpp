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
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "course.hpp"
#include "debug.hpp"
#include "schedule.hpp"
#include "utility.hpp"

using namespace std;


Course::Course() : Bias(), Room()
{
  int i, j;
  int line;

  bool core;

  size_t found;

  string avoid;
  string instr;
  string read_str;
  string room_type;
  string str;
  string str_util;

  uint32_t flag;
  course_t course;
  stringstream oss;
  Debug debug;

  if (prog_opts["VERBOSE"] == "TRUE")
    cout << setw (36) << left << "Reading group descriptions... ";

  group_file.open (FILE_GROUP);

  if (!group_file.is_open() )
  {
    str = "Unable to open " + FILE_GROUP;
    debug.push_error (str);
  }

  debug.live_or_die();

  for (line = 1, flag = 0; getline (group_file, read_str); line++, flag = 0)
  {
    if (read_str.empty() )
      continue;

    read_str = make_upper (read_str);

    if (token_count (read_str, ",") < 2)
    {
      oss << "Invalid group description at line " << line
          << ": invalid format";
      debug.push_error (oss.str() );
      oss.str ("");
      continue;
    }

    for (i = 1; i < token_count (read_str, ","); i++)
    {
      str = get_token (read_str, i, ",");

      if ( (get_token (str, 0, "/") != "CORE" && get_token (str, 0, "/") != "ELEC")
           || (token_count (get_token (str, 1, "/"), ":") < 1) )
      {
        oss << "Invalid group description at line " << line
            << ": invalid format, use CORE or ELEC";
        debug.push_error (oss.str() );
        oss.str ("");
        continue;
      }

      core = get_token (str, 0, "/") == "CORE";
      str = get_token (str, 1, "/");

      for (j = 0; j < token_count (str, ":"); j++)
      {
        if (core)
          m_mapstr_core[get_token (read_str, 0, ",") ].push_back (get_token (str, j, ":") );

        else
          m_mapstr_elec[get_token (read_str, 0, ",") ].push_back (get_token (str, j, ":") );

      }
    }
  }

  group_file.close();

  if (prog_opts["VERBOSE"] == "TRUE")
    cout << "done" << endl;

  debug.live_or_die();

  if (prog_opts["VERBOSE"] == "TRUE")
    cout << setw (36) << left << "Reading course descriptions... ";

  course_file.open (FILE_COURSE);

  if (!course_file.is_open() )
  {
    str = "Unable to open " + FILE_COURSE;
    debug.push_error (str);
  }

  debug.live_or_die();

  for (line = 1, flag = 0; getline (course_file, read_str); line++, flag = 0)
  {
    if (read_str.empty() )
      continue;

    read_str = make_upper (read_str);

    if (line == 1)
    {
      if (get_token (read_str, 0, ",") != "COURSE ID")
        debug.push_error ("Invalid file format: courses.csv");

      continue;
    }

    if (token_count (read_str, ",") < COURSE_TOKENS)
    {
      oss << "Invalid course description at line " << line
          << ": invalid format";
      debug.push_error (oss.str() );
      oss.str ("");
      continue;
    }

    if (get_token (read_str, 0, ",") == "")
      continue;

    course.reset();

    if ( (course.id = get_token (read_str, 0, ",") ) != "")
      flag |= COURSE_ID;

    if ( (course.name = get_token (read_str, 1, ",") ) != "")
      flag |= COURSE_NAME;

    if ( (room_type = get_token (read_str, 3, ",") ) != "")
      flag |= COURSE_TYPE;

    if ( (instr = get_token (read_str, 6, ",") ) != "")
      flag |= COURSE_INSTR;

    if ( (course.room_id = get_token (read_str, 7, ",") ) != "")
      flag |= COURSE_ROOM;

    if ( (course.group = get_token (read_str, 10, ",") ) != "")
      flag |= COURSE_GROUP;

    if ( (avoid = get_token (read_str, 11, ",") ) != "")
      flag |= COURSE_AVOID;

    str = get_token (read_str, 2, ",");

    if (str != "")
    {
      course.hours = atoi (str.c_str() );
      flag |= COURSE_HOURS;
    }

    str = get_token (read_str, 4, ",");

    if (str != "")
    {
      if (token_count (str, "/") > 1)
      {
        course.multi_days = true;
        course.vec_days.resize (token_count (str, "/"), 0);
      }

      for (j = 0; j < token_count (str, "/"); j++)
      {
        for (i = 0; i < token_count (get_token (str, j, "/"), ":"); i++)
        {
          str_util = get_token (get_token (str, j, "/"), i, ":");
          found = VALID_DAYS.find (str_util);

          if (found == string::npos || found % 3 != 0 || str_util.size() != 3)
          {
            oss << "Invalid course days at line " << line
                << ": invalid format";
            debug.push_error (oss.str() );
            oss.str ("");
            continue;
          }

          if (course.multi_days)
            course.vec_days[j] |= day_to_flag (str_util);

          else
            course.days |= day_to_flag (str_util);
        }
      }

      flag |= COURSE_DAYS;
      course.const_days = true;
    }

    str = get_token (read_str, 5, ",");

    if (token_count (str, "-") == 2)
    {
      course.start_time = atof (get_token (str, 0, "-").c_str() );
      course.end_time = atof (get_token (str, 1, "-").c_str() );
      flag |= COURSE_TIMES;
      course.const_time = true;
    }

    str = get_token (read_str, 8, ",");

    if (str != "")
    {
      course.size = atoi (str.c_str() );
      flag |= COURSE_SIZE;
    }

    str = get_token (read_str, 9, ",");

    if (str != "")
    {
      course.lectures = atoi (str.c_str() );
      flag |= COURSE_LECTS;
    }

    if ( (COURSE_ID|COURSE_NAME) & ~flag ||
         (COURSE_TYPE & flag && room_type != "L" && room_type != "S") )
    {
      oss << "Invalid course description at line " << line
          << ": invalid course name, ID, or class type";
      debug.push_error (oss.str() );
      oss.str ("");
      continue;
    }

    course.is_lab     = room_type      == "S" ? false : true;
    course.const_room = course.room_id ==  "" ? false : true;

    for (i = 0; i < token_count (avoid, ":"); i++)
      course.vec_avoid.push_back (get_token (avoid, i, ":") );

    for (i = 0; i < token_count (instr, ":"); i++)
      course.vec_instr.push_back (get_token (instr, i, ":") );

    if ( (! ( (COURSE_TIMES|COURSE_DAYS) & ~flag) && !course.size) ||
         ! ( (COURSE_TIMES|COURSE_DAYS|COURSE_ROOM) & ~flag) )
    {
      if (push_const_course (course) )
      {
        oss << "Invalid course description at line " << line
            << ": duplicate";
        debug.push_error (oss.str() );
        oss.str ("");
      }
    }
    else if (! ( (COURSE_HOURS|COURSE_TYPE|COURSE_INSTR|COURSE_SIZE) & ~flag) )
    {
      if (push_course (course) )
      {
        oss << "Invalid course description at line " << line
            << ": duplicate";
        debug.push_error (oss.str() );
        oss.str ("");
      }
    }
    else
    {
      oss << "Invalid course description at line " << line
          << ": invalid format";
      debug.push_error (oss.str() );
      oss.str ("");
    }
  }

  course_file.close();

  if (prog_opts["VERBOSE"] == "TRUE")
    cout << "done" << endl;

  debug.live_or_die();
}

bool Course::push_course (course_t &course)
{
  if (m_mapstr_course.find (course.id) != m_mapstr_course.end() )
    return true;

  double k;
  int i;
  bs_t bs;

  string str;
  string group;

  map<string, room_t>::iterator begin_it;
  map<string, room_t>::iterator end_it;
  map<string, room_t>::iterator it;

  stringstream oss;

  // Test for multiple room entries
  if (course.const_room && token_count (course.room_id, ":") > 1)
  {
    Debug debug;
    // Must be false or vec_prooms wont be looked at
    course.const_room = false;

    for (i = 0; i < token_count (course.room_id, ":"); i++)
    {
      str = get_token (course.room_id, i, ":");

      if (m_mapstr_labrooms.find (str) != m_mapstr_labrooms.end() )
        course.vec_prooms.push_back (m_mapstr_labrooms[str]);

      else if (m_mapstr_stdrooms.find (str) != m_mapstr_stdrooms.end() )
        course.vec_prooms.push_back (m_mapstr_stdrooms[str]);

      else
        oss << "Invalid room for " << course.id << ": " << str;
        debug.push_error (oss.str() );
        oss.str ("");
    }

    course.room_id = "";
    debug.live_or_die();
  }
  else if (!course.const_room)
  {
    if (course.is_lab)
    {
      begin_it = m_mapstr_labrooms.begin();
      end_it   = m_mapstr_labrooms.end();
    }
    else
    {
      begin_it = m_mapstr_stdrooms.begin();
      end_it   = m_mapstr_stdrooms.end();
    }

    for (it = begin_it; it != end_it; it++)
      if (it->second.size >= course.size)
        course.vec_prooms.push_back (it->second);
  }

  /*
    Here we update the possible times a course can be scheduled depending on
    several factors:

    if the lecture is three hours and the instructor specifies he wants to
    teach it over TUE:THU or MON:WED:FRI

    The time was specified in the CSV
    The days were specified in the CSV
    We are using contiguous labs
    We are scheduling a lab and we are using continuous labs
    The user input multiple optional days in the form MON:WED/MON:FRI
  */
  if (course.hours == 3 && course.lectures == 2)
    for (i = 0; i < 9; i++)
      course.vec_avail_times.push_back (sched_bs_idx[3][i]);

  else if (course.hours == 3 && course.lectures == 3)
    for (i = 9; i < 15; i++)
      course.vec_avail_times.push_back (sched_bs_idx[3][i]);

  else if (course.const_time && course.const_days)
  {
    if (course.multi_days)
      for (auto it = course.vec_days.begin(); it != course.vec_days.end(); it++)
        course.vec_avail_times.push_back (make_bitsched (course.start_time, course.end_time, *it) );

    else
      course.vec_avail_times.push_back (make_bitsched (course.start_time, course.end_time, course.days) );
  }

  else if (course.is_lab && prog_opts["CONTIGUOUS-LABS"] == "TRUE")
    for (i = 0; i < 5; i++)
      for (k = atoi (prog_opts["LAB-START-TIME"].c_str() );
           k + course.hours <= atoi (prog_opts["LAB-END-TIME"].c_str() );
           k += 0.5)
      {
        bs = make_bitsched (k, k + course.hours, 2 << i);

        if (course.multi_days)
          for (auto it = course.vec_days.begin(); it != course.vec_days.end(); it++)
            if (! (course.const_time && ( (make_bitsched (course.start_time, course.end_time, 2) ^ bs) & MASK_TIME).any() ) && // dummy day
                ! (course.const_days && ( (make_bitsched (12.0, 13.0, *it) ^ bs) & MASK_DAY).any() ) ) // dummy time
            {
              course.vec_avail_times.push_back (bs);
            }

        else if (! (course.const_time && ( (make_bitsched (course.start_time, course.end_time, 2) ^ bs) & MASK_TIME).any() ) && // dummy day
                 ! (course.const_days && ( (make_bitsched (12.0, 13.0, course.days) ^ bs) & MASK_DAY).any() ) ) // dummy time
        {
          course.vec_avail_times.push_back (bs);
        }
      }

  else if (course.const_time)
  {
    bs = make_bitsched (course.start_time, course.end_time, 2); // dummy day

    for (auto it = sched_bs_idx[course.hours].begin(); it != sched_bs_idx[course.hours].end(); it++)
      if ( ( (*it ^ bs) & MASK_TIME).none() )
        course.vec_avail_times.push_back (*it);
  }

  else if (course.multi_days)
    for (auto it_day = course.vec_days.begin(); it_day != course.vec_days.end(); it_day++)
      for (auto it_bs = sched_bs_idx[course.hours].begin(); it_bs != sched_bs_idx[course.hours].end(); it++)
        if ( (*it_bs >> 56 ^ static_cast<bs_t> (*it_day) ).none() )
          course.vec_avail_times.push_back(*it_bs);

  else if (course.const_days)
    for (auto it = sched_bs_idx[course.hours].begin(); it != sched_bs_idx[course.hours].end(); it++)
      if ( (*it >> 56 ^ static_cast<bs_t> (course.days) ).none() )
        course.vec_avail_times.push_back (*it);

  else
    for (auto it = sched_bs_idx[course.hours].begin(); it != sched_bs_idx[course.hours].end(); it++)
      course.vec_avail_times.push_back (*it);

  /*
    Update vec_core/vec_elec depending on group membership.  The group file was read at
    the beginning of c'tor Course()
  */
  for (i = 0; i < token_count (course.group, ":"); i++)
  {
    group = get_token (course.group, i, ":");

    for (auto it = m_mapstr_core[group].begin(); it != m_mapstr_core[group].end(); it++)
      if (find (course.vec_avoid.begin(), course.vec_avoid.end(), *it) == course.vec_avoid.end() && *it != course.name)
        course.vec_avoid.push_back (*it);

    for (auto it = m_mapstr_elec[group].begin(); it != m_mapstr_elec[group].end(); it++)
      if (find (course.vec_elec.begin(), course.vec_elec.end(), *it) == course.vec_elec.end() && *it != course.name)
        course.vec_elec.push_back (*it);
  }

  m_mapstr_course[course.id] = course;
  return false;
}

bool Course::push_const_course (course_t &course)
{
  if (m_mapstr_const_course.find (course.id) != m_mapstr_const_course.end() )
    return true;

  course.bs_sched = make_bitsched (course.start_time, course.end_time, course.days);
  m_mapstr_const_course[course.id] = course;
  return false;
}
