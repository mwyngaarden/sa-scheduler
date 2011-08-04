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
#if !defined(ROOM_HPP)
#define ROOM_HPP

#include <fstream>
#include <map>
#include <string>

#include "utility.hpp"

<<<<<<< HEAD
/*!
  Inherited by class Course	

  Room is a simple class that reads in room descriptions from a CSV that 
  specifies rooms names, their type, and size.

  Returns true if the room already exists.
*/
class Room
{
  public:
    /*!
      The constructor does all the work by reading the CSV contents and filling 
      in the public maps.  If the CSV contains an error, then the program will 
      exit after writing the error(s) to debug.log.
    */
=======

class Room
{
  public:
>>>>>>> f13d734e680a209d60f636ed7cba2748acd6bb2b
    Room();
    bool push_labroom(const std::string &id, int size);
    bool push_stdroom(const std::string &id, int size);

    std::map<std::string, room_t> m_mapstr_labrooms;
    std::map<std::string, room_t> m_mapstr_stdrooms;

  private:
    std::ifstream room_file;
};

#endif // !defined(ROOM_HPP)
