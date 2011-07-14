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

#include <algorithm>
#include <fstream>

#include "utility.hpp"

using namespace std;


int options[OPT_TOTOPTS];
vector<vector<int> > vec_bitpos_idx;
vector<vector<bs_t> > sched_bs_idx;

void util_init()
{
  int i, j, k;

  string str;

  sched_bs_idx.resize(6);


  for (i = 0; i < 6; i++) {
    for (j = 0; j < sched_count_idx[i]; j++) {
      for (k = 0, str = ""; k < 16; k++) {
        str += hex_to_bin(sched_hex_idx[i][j][k]);
      }

      bs_t bs(str);
      sched_bs_idx[i].push_back(bs);
    }
  }

  vec_bitpos_idx.resize(128);

  for (i = 0; i < 128; i++)  {
    for (j = 0; j < 7; j++) {
      if (i & 1 << j) {
        vec_bitpos_idx[i].push_back(48 * j);
      }
    }
  }
}

string make_upper(const string &str)
{
  string ret_str = str;
  transform(ret_str.begin(), ret_str.end(), ret_str.begin(), toupper);
  return ret_str;
}

int token_count(const string &str, string tok)
{
  if (str == "") {
    return 0;
  }

  int count = 1;

  for (int i = 0; i < str.size(); i++)
    if (!str.compare(i, 1, tok)) {
      count++;
    }

  return count;
}

string get_token(const string &str, int n, string delim)
{
  string ret_str;

  for (int i = 0, t = 1; i < str.size() && t <= (n+1); i++) {
    if (!str.compare(i, 1, delim)) {
      t++;

    } else if (t == n + 1) {
      ret_str.append(str, i, 1);
    }
  }

  return ret_str;
}

string break_instr(const vector<string> &vec_instr)
{
  assert(vec_instr.size());

  string ret_str;

  for (int i = 0; i < vec_instr.size(); i++) {
    ret_str += vec_instr[i] + "<br>";
  }

  return ret_str;
}

bool file_exists(const char *file)
{
  fstream myfile(file);
  return !myfile.fail();
}

bool ptime_sort(const pfit_t &lhs, const pfit_t &rhs)
{
  return lhs.health.fitness < rhs.health.fitness;
}

bool proom_sort(const room_pfit_t &lhs, const room_pfit_t &rhs)
{
  return lhs.weight < rhs.weight;
}

uint8_t day_to_flag(const std::string &day)
{
  assert(day != "");

  return 1 << day_to_int(day);
}

int day_to_int(const std::string &day)
{
  assert(day != "");

  return static_cast<int>(VALID_DAYS.find(day) / 3);
}

string flag_to_str(uint8_t days)
{
  assert(days & 0x3e);
  assert(!(days & 0xc1));
  
  string ret_str;

  for (int i = 1; i < 6; i++) {
    if (days & (1 << i)) {
      ret_str += ":" + VALID_DAYS.substr(3 * i, 3);
    }
  }

  ret_str.erase(0, 1);

  return ret_str;
}

string vec_to_str(const vector<string> &vec_str) 
{
  string ret_str;

  for (int i = 0; i < vec_str.size(); i++) {
    ret_str += ":" + vec_str[i];
  }

  ret_str.erase(0, 1);

  return ret_str;
}

string hex_to_bin(char hex)
{
  assert((hex >= '0' && hex <= '9') || (hex >= 'a' && hex <= 'f'));

  if (hex == '0') {
    return "0000";
  } else if (hex == '1') {
    return "0001";
  } else if (hex == '2') {
    return "0010";
  } else if (hex == '3') {
    return "0011";
  } else if (hex == '4') {
    return "0100";
  } else if (hex == '5') {
    return "0101";
  } else if (hex == '6') {
    return "0110";
  } else if (hex == '7') {
    return "0111";
  } else if (hex == '8') {
    return "1000";
  } else if (hex == '9') {
    return "1001";
  } else if (hex == 'a') {
    return "1010";
  } else if (hex == 'b') {
    return "1011";
  } else if (hex == 'c') {
    return "1100";
  } else if (hex == 'd') {
    return "1101";
  } else if (hex == 'e') {
    return "1110";
  } else {
    return "1111";
  } 
}