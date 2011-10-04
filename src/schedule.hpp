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
#include <fstream>
#include <vector>

#include "course.hpp"
#include "debug.hpp"
#include "utility.hpp"

/*!
  Schedule::optimize is where the program spends most of its time.  Optimization
  is done through simulated annealing.  The starting and ending temperatures are
  defined in the namespace below.

  Public members are for the HTML file and contents, random number generator, 
  and keeping track of the start and end time.  The score for the best solution 
  is saved in m_best_fitness.  These variables are remnants from optimizing 
  coefficients and can safely be ignored.

  Private members are for:
        
    Referencing the HTML header file
    Storing the HTML header for later use
    Random number generator
    Run time 
    Best fitness: 
      Used for parameter tuning and can be ignored for general usage
*/
class Schedule : public Course
{
  public:
    /*!
      Constructor checks for the HTML header and reads it into a vector.  If it's
      not found, as error is output to debug.log and the console before the
      program exits.

      Seeds random number generator with the current time.
    */
    Schedule();

    /*!
      The possible bit schedules are looked at and a score for each is calculated
      for each depending on the number of conflicts ascertained from checking the 
      u indices.
    */
    void get_bitsched(
      course_t                        &const_course,
      std::map<std::string, bs_t>     &u_crs_idx,
      std::map<std::string, bs_t>     &u_instr_idx,
      std::map<std::string, bs_t>     &u_room_idx);

    /*! 
      Function is called from optimize to schedule all courses in state.  
      health is non-const so as to be overwritten, the same for cur_state.
      my_rng is another remnant from when the program was threaded and all
      threads had a different random number generator.  
    */
    void perturb_state(
      const state_t                   &state,
      health_t                        &health,
      std::vector<course_t>           &cur_state,
      prng_t                          &my_rng);

    /*! HACK: For use in outputting schedules to HTML */
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

    /*!
      Returns true if the course is schedule-able given the number of collisions
      and the bias fitness score.
    */
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

    /*! 
      Returns the number of day collisions multiplied by the number of time
      collisions. POPCNT may offer additional speedup?
    */
    inline int num_conflicts(const bs_t &bs) 
    {
      assert((bs & VALID_MASK).none());
      return static_cast<int>((bs & MASK_DAY).count() * (bs & MASK_TIME).count());
    };

    /*! 
      Optimization routine containing the simulated annealing algorithm.  Upon
      completion, data are saved to file and execution returns to main().
    */
    void optimize         (); 

    /*! Sends stats to console: iteration, fitness, temperature, etc. */
    void display_stats    (const state_t &state, int iter);

    /*! 
      Called from optimize to output data to files, that is, html tables and 
      scheduled.csv and failed.csv.
    */
    void save_scheds      (state_t &state);

    /*! Called from save_scheds to output schedules to HTML files using tables */
    void write_html       (std::ofstream &file, std::map<std::string, Week> &mapstr_cal);

    /*! Returns run time */
    int duration          ();

  private:
    /*! Used for referencing html_header.txt */
    std::ifstream header_file;

    /*! Holds data read in from html_header.txt */ 
    std::vector<std::string> m_vec_header;

    /*! Random number generator */
    prng_t m_rng;

    /*! Used to track optimization run time */
    time_t m_end_time;

    /*! Used to track optimization run time */
    time_t m_start_time;
    
    /*! Used to track the best fit schedule */
    int m_best_fitness;
};

namespace
{
  /*! 
    Annealing parameters are sensitive to the schedule fitness and must be
    modified accordingly
  */
  const double TEMP_INIT  = 1.0e+1;     
  const double TEMP_MIN   = 1.0e-5; 

  /*! 
    Coefficients used in get_bitched for ordering candidate schedules for 
    classes.
  */
  const double CMUL_AVOID =  5.062;
  const double CMUL_ELEC  =  1.000;
  const double CMUL_INSTR = 18.148;
  const double CMUL_ROOM  = 18.604;
}

#endif // !defined(SCHEDULE_HPP)
