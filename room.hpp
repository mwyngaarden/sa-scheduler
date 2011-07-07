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
