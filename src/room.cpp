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

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "debug.hpp"
#include "room.hpp"
#include "utility.hpp"

using namespace std;


Room::Room()
{
  if (prog_opts["VERBOSE"] == "TRUE")
    cout << setw (36) << left << "Reading room profiles... ";

  int  line;
  string read_str;
  uint32_t flag;
  ostringstream oss;
  Debug debug;
  room_file.open (FILE_ROOM);

  if (!room_file.is_open() )
  {
    read_str ="Unable to open " + FILE_ROOM;
    debug.push_error (read_str);
  }

  debug.live_or_die();

  for (line = 1, flag = 0; getline (room_file, read_str); line++, flag = 0)
  {
    if (read_str.empty() )
      continue;

    read_str = make_upper (read_str);

    if (token_count (read_str, ",") < 3)
    {
      oss << "Ignoring room entry at line " << line
          << ":  invalid format";
      debug.push_warning (oss.str() );
      oss.str ("");
      continue;
    }

    if (get_token (read_str, 0, ",") != "")
      flag |= 4;

    if (get_token (read_str, 1, ",") != "")
      flag |= 2;

    if (get_token (read_str, 2, ",") != "")
      flag |= 1;

    if (!flag)
    {
      oss << "Ignoring room entry at line " << line << ": empty line";
      debug.push_warning (oss.str() );
      oss.str ("");
      continue;
    }

    if (flag ^ 0x7)
    {
      oss << "Invalid room entry at line " << line << ":  invalid format";
      debug.push_error (oss.str() );
      oss.str ("");
      continue;
    }

    if (get_token (read_str, 2, ",") != "L" && get_token (read_str, 2, ",") != "S")
    {
      oss << "Invalid room entry at line " << line
          << ": invalid room type, use S/L)";
      debug.push_error (oss.str() );
      oss.str ("");
      continue;
    }

    if (get_token (read_str, 2, ",") == "S")
    {
      if (push_stdroom (
            get_token (read_str, 0, ","),
            atoi (get_token (read_str, 1, ",").c_str() ) ) )
      {
        oss << "Ignoring room entry at line " << line << ": duplicate";
        debug.push_warning (oss.str() );
        oss.str ("");
        continue;
      }
    }
    else
    {
      if (push_labroom (
            get_token (read_str, 0, ","),
            atoi (get_token (read_str, 1, ",").c_str() ) ) )
      {
        oss << "Ignoring room entry at line " << line << ": duplicate";
        debug.push_warning (oss.str() );
        oss.str ("");
        continue;
      }
    }
  }

  room_file.close();

  if (prog_opts["VERBOSE"] == "TRUE")
    cout << "done" << endl;

  debug.live_or_die();
}

bool Room::push_stdroom (const string &id, int size)
{
  assert (id != "");
  assert (size > 0);

  if (m_mapstr_stdrooms.find (id) != m_mapstr_stdrooms.end() )
    return true;

  m_mapstr_stdrooms[id].id = id;
  m_mapstr_stdrooms[id].size = size;
  return false;
}

bool Room::push_labroom (const string &id, int size)
{
  assert (id != "");
  assert (size > 0);

  if (m_mapstr_labrooms.find (id) != m_mapstr_labrooms.end() )
    return true;

  m_mapstr_labrooms[id].id = id;
  m_mapstr_labrooms[id].size = size;
  return false;
}
