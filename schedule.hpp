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
#include <boost/random/mersenne_twister.hpp>
#include <boost/thread/mutex.hpp>

#include "course.hpp"
#include "debug.hpp"
#include "utility.hpp"


class Schedule : public Course
{
  public:
    Schedule();

    void get_bitsched(
      course_t                        &course,
      std::map<std::string, course_t> &crs_name_idx,
      std::map<std::string, bs_t>     &u_crs_idx,
      std::map<std::string, bs_t>     &u_instr_idx,
      std::map<std::string, bs_t>     &u_room_idx,
      boost::mt19937                  &my_rng);

    int get_duration();
    int optimize();
    void perturb_state(
      const state_t                   &best_state,
      std::map<std::string, course_t> &crs_name_idx,
      boost::mt19937                  &my_rng,
      health_t                        &health,
      std::vector<course_t>           &cur_state);

    // TODO: clean up this hack!
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

    inline int num_conflicts(const bs_t &bs) 
    {
      assert((bs & VALID_MASK).none());
      return static_cast<int>((bs & MASK_DAY).count() * (bs & MASK_TIME).count());
    };
        
    void display_stats    (const state_t &state, int iter);
    void save_scheds      (const state_t &best_state);
    void write_html       (std::ofstream &file, std::map<std::string, Week> &mapstr_cal);

  private:
    std::ifstream header_file;
    std::vector<std::string> m_vec_header;

    boost::mutex m_mutex; // thread locking
    boost::mt19937 m_rng;

    // threading
    bool m_finished;
    bool m_started;
    state_t m_shared_state;
    int m_shared_iters;
    int m_shared_last_update;

    time_t m_end_time;
    time_t m_start_time;
};

namespace
{
  // permutation and evaluation
  const int AVOID_MUL    = 1;
  const int INSTR_MUL    = 1;
  const int ROOM_MUL     = 1;

  const double PB_RMUT   =   0.95;
  const double PB_TMUT   =   0.5;

  const double ABS_TEMP    = 1.0e-4;
  const double INIT_TEMP   = 1.0e+6;
  const double COOL_RATE   = 0.9999;
}

#endif // !defined(SCHEDULE_HPP)
