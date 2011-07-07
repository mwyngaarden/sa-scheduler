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

#pragma once
#if !defined(ROOM_HPP)
#define ROOM_HPP

#include <fstream>
#include <map>
#include <string>

#include "utility.hpp"


class Room
{
  public:
    Room();
    bool push_labroom(const std::string &id, int size);
    bool push_stdroom(const std::string &id, int size);

    std::map<std::string, room_t> m_mapstr_labrooms;
    std::map<std::string, room_t> m_mapstr_stdrooms;

  private:
    std::ifstream room_file;
};

#endif // !defined(ROOM_HPP)
