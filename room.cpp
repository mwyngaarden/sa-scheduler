#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "debug.hpp"
#include "room.hpp"
#include "utility.hpp"

using namespace std;


Room::Room()
{
  if (options[OPT_VERBOSE]) {
    cout << setw(36) << left << "Reading room profiles... ";
  }

  int  line;
  string read_str;
  uint64_t flag;
  
  ostringstream oss;
  Debug debug;
  
  room_file.open("rooms.csv");

  if (!room_file.is_open()) {
    debug.push_error("Unable to open rooms.csv");
  }

  debug.live_or_die();

  for (line = 1, flag = 0; getline(room_file, read_str); line++, flag = 0) {
    if (read_str.empty()) {
      continue;
    }

    if (token_count(read_str, ",") < 3) {
      oss << "Ignoring room entry at line " << line
          << ":  invalid format";
      debug.push_warning(oss.str());
      oss.str("");
      continue;
    }

    if (get_token(read_str, 0, ",") != "") {
      flag |= 4;
    }

    if (get_token(read_str, 1, ",") != "") {
      flag |= 2;
    }

    if (get_token(read_str, 2, ",") != "") {
      flag |= 1;
    }

    if (!flag) {
      oss << "Ignoring room entry at line " << line << ": empty line";
      debug.push_warning(oss.str());
      oss.str("");
      continue;
    }

    if (flag ^ 0x7) {
      oss << "Invalid room entry at line " << line << ":  invalid format";
      debug.push_error(oss.str());
      oss.str("");
      continue;
    }

    if (make_upper(get_token(read_str, 2, ",")) != "L" && 
        make_upper(get_token(read_str, 2, ",")) != "S") {
      oss << "Invalid room entry at line " << line
          << ": invalid room type, use S/L)";
      debug.push_error(oss.str());
      oss.str("");
      continue;
    }

    if (make_upper(get_token(read_str, 2, ",")) == "S") {
      if (push_stdroom(
            make_upper(get_token(read_str, 0, ",")),
            atoi(get_token(read_str, 1, ",").c_str()))) {
        oss << "Ignoring room entry at line " << line << ": duplicate";
        debug.push_warning(oss.str());
        oss.str("");
        continue;
      }

    } else {
      if (push_labroom(
            make_upper(get_token(read_str, 0, ",")),
            atoi(get_token(read_str, 1, ",").c_str()))) {
        oss << "Ignoring room entry at line " << line << ": duplicate";
        debug.push_warning(oss.str());
        oss.str("");
        continue;
      }
    }
  }

  room_file.close();

  if (options[OPT_VERBOSE]) {
    cout << "done" << endl;
  }

  debug.live_or_die();
}

bool Room::push_stdroom(const string &id, int size)
{
  assert(id != "");
  assert(size > 0);

  if (m_mapstr_stdrooms.find(id) != m_mapstr_stdrooms.end()) {
    return true;
  }

  m_mapstr_stdrooms[id].id = id;
  m_mapstr_stdrooms[id].size = size;
  return false;
}

bool Room::push_labroom(const string &id, int size)
{
  assert(id != "");
  assert(size > 0);

  if (m_mapstr_labrooms.find(id) != m_mapstr_labrooms.end()) {
    return true;
  }

  m_mapstr_labrooms[id].id = id;
  m_mapstr_labrooms[id].size = size;
  return false;
}