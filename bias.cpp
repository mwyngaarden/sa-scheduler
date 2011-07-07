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

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "debug.hpp"
#include "bias.hpp"
#include "schedule.hpp"
#include "utility.hpp"

using namespace std;


Bias::Bias()
{
  if (options[OPT_VERBOSE]) {
    cout << setw(36) << left << "Reading instructor preferences... ";
  }

  e_bias bias;
  int i, line;
  uint64_t days;
  uint64_t flag;

  double end_time;
  double start_time;
  string instr;
  string read_str;
  string str;

  ostringstream oss;
  size_t found;
  string valid_status = "AVR6AVR5AVR4AVR3AVR2AVR1OPENPRF1PRF2PRF3PRF4PRF5PRF6VOID";

  Debug debug;

  bias_file.open("bias.csv");

  if (!bias_file.is_open()) {
    debug.push_error("Unable to open bias.csv");
  }

  debug.live_or_die();

  for (line = 1, flag = 0; getline(bias_file, read_str); line++, flag = 0) {
    if (read_str.empty()) {
      continue;
    }

    for (i = 1; i < token_count(read_str, ","); i++) {
      str = get_token(read_str, i, ",");

      if (str == "") {
        continue;
      }

      if (token_count(str, ":") != 3) {
        oss << "Invalid schedule descriptor at line " << line
            << ": invalid format";
        debug.push_error(oss.str());
        oss.str("");
        continue;
      }

      str = make_upper(get_token(get_token(read_str, i, ","), 0, ":"));
      found = valid_status.find(str);

      if (found == string::npos || found % 4 != 0 || str.size() != 4) {
        oss << "Invalid schedule descriptor at line " << line
            << ": invalid status, use AVR[6-1]/PRF[1-6]/VOID";
        debug.push_error(oss.str());
        oss.str("");
        continue;
      }

      bias = e_bias(found / 4);
      str = make_upper(get_token(get_token(read_str, i, ","), 1, ":"));
      found = VALID_DAYS.find(str);

      if (found == string::npos || found % 3 != 0 || str.size() != 3) {
        oss << "Invalid schedule descriptor at line " << line
            << ": invalid day, use ALL/SUN/MON/TUE/WED/THU/FRI/SAT";
        debug.push_error(oss.str());
        oss.str("");
        continue;
      }

      days = str == "ALL" ? 127 : day_to_flag(str);
      str = make_upper(get_token(get_token(read_str, i, ","), 2, ":"));

      if (str != "ALL" && 
          (token_count(str, "-") != 2 || get_token(str, 0, "-") == "" || 
           get_token(str, 1, "-") == "")) {
        oss << "Invalid schedule descriptor at line " << line
            << ": invalid time";
        debug.push_error(oss.str());
        oss.str("");
        continue;
      }

      start_time = str == "ALL" ?  0.0 : atof(get_token(str, 0, "-").c_str());
      end_time   = str == "ALL" ? 24.0 : atof(get_token(str, 1, "-").c_str());
      instr = make_upper(get_token(read_str, 0, ","));

      if (instr == "") {
        oss << "Invalid schedule descriptor at line " << line
            << ": invalid name";
        debug.push_error(oss.str());
        oss.str("");
        continue;
      }

      set_bias(instr, start_time, end_time, days, bias);
    }
  }

  bias_file.close();
  debug.live_or_die();

  if (options[OPT_VERBOSE]) {
    cout << "done" << endl;
  }
}

int Bias::get_bias(
  const string &instr,
  double start_time,
  double end_time, uint64_t days)
{
  assert(instr != "");
  assert(start_time >= 0.0);
  assert(start_time < 24.0);
  assert(end_time > start_time);
  assert(end_time <= 24.0);
  assert(days & 0x3e);
  assert(!(days & 0xc1));

  if (m_mapstr_bias.find(instr) == m_mapstr_bias.end()) {
    return 0;
  }

  e_bias bias;
  int level = 0;

  for (int i = 0; i < vec_bitpos_idx[days].size(); i++)
    for (int j = 0; j < 2 * (end_time - start_time); j++) {
      bias = m_mapstr_bias[instr][static_cast<int>(vec_bitpos_idx[days][i] + 2 * start_time + j)];

      if (bias == VOID) {
        return SCORE_VOID;
      }

      level += bias;
    }

  return level;
}

int Bias::get_bias(const string &instr, uint64_t bit_sched)
{
  assert(instr != "");
  assert(bit_sched & MASK_DAY);
  assert(bit_sched & MASK_TIME);
  assert(!(bit_sched & VALID_MASK));

  if (m_mapstr_bias.find(instr) == m_mapstr_bias.end()) {
    return 0;
  }

  uint64_t days     = bit_sched >> 56;
  bit_sched         = (bit_sched & MASK_TIME) >> 16;
  double start_time = 8 + 0.5 * POPCNT(~bit_sched & bit_sched - 1);

  return get_bias(instr, start_time, start_time + POPCNT(bit_sched) * 0.5, days);
}

void Bias::set_bias(
  const string &instr,
  double start_time,
  double end_time,
  uint64_t days,
  e_bias bias)
{
  assert(instr != "");
  assert(start_time >= 0.0);
  assert(start_time < 24.0);
  assert(end_time > start_time);
  assert(end_time <= 24.0);
  assert(days > 0);
  assert(days < 128);

  if (m_mapstr_bias.find(instr) == m_mapstr_bias.end()) {
    m_mapstr_bias[instr].resize(336, OPEN);
  }

  for (int i = 0; i < vec_bitpos_idx[days].size(); i++)
    for (int j = 0; j < 2 * (end_time - start_time); j++) {
      m_mapstr_bias[instr][static_cast<int>(vec_bitpos_idx[days][i] + 2 * start_time + j)] = bias;
    }
}