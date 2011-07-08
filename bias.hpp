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


class Bias
{
  public:
    Bias();

    int get_bias(const std::string &instr, uint64_t bit_sched);

    int get_bias(
      const std::string &instr,
      double start_time,
      double end_time,
      uint32_t days);

    void set_bias(
      const std::string &instr,
      double start_time,
      double end_time,
      uint32_t days,
      e_bias bias);

  private:
    std::map<std::string, std::vector<e_bias> > m_mapstr_bias;
    std::ifstream bias_file;
};

#endif // !defined(BIAS_HPP)
