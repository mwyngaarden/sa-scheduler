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
      course_t &course,
      std::map<std::string, course_t> &crs_name_idx,
      std::map<std::string, uint64_t> &u_crs_idx,
      std::map<std::string, uint64_t> &u_instr_idx,
      std::map<std::string, uint64_t> &u_room_idx,
      boost::mt19937 &my_rng);

    int get_duration();
    int optimize();
    void perturb_state(
      const state_t &best_state,
      std::map<std::string, course_t> &crs_name_idx,
      boost::mt19937 &my_rng,
      health_t &health,
      std::vector<course_t> &cur_state);

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

    bool done             (double temp, time_t dur);
    int num_conflicts     (uint64_t bs);
    void display_stats    (const state_t &state, int iter);
    void save_scheds      (const state_t &best_state);
    void write_html       (std::ofstream &file, std::map<std::string, Week> &mapstr_cal);

  private:
    std::ifstream header_file;
    std::vector<std::string> vec_header;

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

const double ABS_TEMP    = 1.0e-6;
const double INIT_TEMP   = 1.0e+5;
const double COOL_RATE   = 1.0 - 1.0e-3;
const double DELTA_SCALE = 1.0e-13;

}

#endif // !defined(SCHEDULE_HPP)
