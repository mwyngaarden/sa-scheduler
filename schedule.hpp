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
#if !defined(SCHEDULE_HPP)
#define SCHEDULE_HPP

#include <cassert>
#include <ctime>
#include <fstream>
#include <vector>

#include "course.hpp"
#include "debug.hpp"
#include "utility.hpp"


class Schedule : public Course
{
  public:
    Schedule();

    void get_bitsched(
      course_t                        &const_course,
      std::map<std::string, bs_t>     &u_crs_idx,
      std::map<std::string, bs_t>     &u_instr_idx,
      std::map<std::string, bs_t>     &u_room_idx,
      prng_t                          &my_rng);

    void perturb_state(
      const state_t                   &state,
      health_t                        &health,
      std::vector<course_t>           &cur_state,
      prng_t                          &my_rng);

    class Week
    {
      public:
        Week() {
          for (int i = 0; i < 336; i++) {
            m_week_idx[i].span = 0;
            m_week_idx[i].data = "";
          }
        };

        struct m_week_t {
          int span;
          std::string data;
        } m_week_idx[336];
    };

    bool can_schedule(const course_t &course) {
      return 
        !(course.health.avoid_colls  || 
          course.health.instr_colls  ||
          course.health.room_colls   ||
          course.health.bias_fitness < 0);
    };

    int best() {
      return m_best_fitness;
    };

    void reset() {
      m_best_fitness = 0;
    };

    inline int num_conflicts(const bs_t &bs) 
    {
      assert((bs & VALID_MASK).none());
      return static_cast<int>((bs & MASK_DAY).count() * (bs & MASK_TIME).count());
    };

    void optimize         ();        
    void display_stats    (const state_t &state, int iter);
    void save_scheds      (state_t &state);
    void write_html       (std::ofstream &file, std::map<std::string, Week> &mapstr_cal);
    int duration          ();

  private:
    std::ifstream header_file;
    std::vector<std::string> m_vec_header;

    prng_t m_rng;

    time_t m_end_time;
    time_t m_start_time;
    
    int m_best_fitness;
};

namespace
{
  // permutation and evaluation
  const double TEMP_INIT  = 1.0e+1;     
  const double TEMP_MIN   = 1.0e-5; 

  const double CMUL_AVOID = 1.0;
  const double CMUL_INSTR = 1.0;
  const double CMUL_ROOM  = 1.0;
}

#endif // !defined(SCHEDULE_HPP)
