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
#if !defined(UTILITY_HPP)
#define UTILITY_HPP

#include <cassert>
#include <cmath>
#include <intrin.h>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>
#include <boost/random/mersenne_twister.hpp>


const uint64_t MASK_DAY   = 0x3e00000000000000Ui64;
const uint64_t MASK_TIME  = 0x000003ffffff0000Ui64;
const uint64_t VALID_MASK = ~(MASK_DAY|MASK_TIME);

const std::string COMPILE_TIME = __TIME__;
const std::string COMPILE_DATE = __DATE__;

const std::string VALID_DAYS = "SUNMONTUEWEDTHUFRISATALL";

// TODO: duh!
const int SCORE_VOID = 1;
const int INF = 0x7fffffff;

enum e_bias {
  AVR6, AVR5, AVR4, AVR3, AVR2, AVR1,
  OPEN,
  PRF1, PRF2, PRF3, PRF4, PRF5, PRF6,
  VOID
};

enum e_opts {
  OPT_CLABETIME,
  OPT_CLABSTIME,
  OPT_CONTIGLABS,
  OPT_HASPOPCNT,
  OPT_POLLINTVL,
  OPT_ROOMBUFF,
  OPT_SYNCINTVL,
  OPT_THREADS,
  OPT_VERBOSE,
  OPT_TOTOPTS
};

struct room_t {
  std::string id;
  int size;
};

class health_t {
  public:
    void reset() {
      avoid_colls  = 0;
      bias_fitness = 0;
      buf_fitness = 0;
      fitness      = 0;
      instr_colls  = 0;
      late_penalty = 0;
      room_colls   = 0;

      ffit         = 0;
    };

    void init() {
      avoid_colls  =  INF;
      bias_fitness = -INF;
      buf_fitness =  INF;
      fitness      =  INF;
      instr_colls  =  INF;
      late_penalty =  INF;
      room_colls   =  INF;

      ffit         = 0x7fffffffffffffffUi64;
    };

    int avoid_colls;
    int bias_fitness;
    int buf_fitness;
    int fitness;
    int instr_colls;
    int late_penalty;
    int room_colls;

    int64_t ffit;
};

class course_t
{
  public:
    void reset() {
      const_days   = false;
      const_room   = false;
      const_time   = false;
      days         = 0;  // uint64_t
      end_time     = 0.0;
      group        = "";
      hours        = 0;
      id           = "";
      is_lab       = false;
      lectures     = 0;
      name         = "";
      room_id      = "";
      rsrv_blks    = 0;  // uint64_t
      size         = 0;
      start_time   = 0.0;

      health.init();

      vec_avail_times.clear();
      vec_avoid.clear();
      vec_instr.clear();
      vec_prooms.clear();
    };

    bool const_days;
    bool const_room;
    bool const_time;
    bool is_lab;
    double end_time;
    double start_time;
    health_t health;
    int hours;
    int lectures;
    int size;
    std::string group;
    std::string id;
    std::string name;
    std::string room_id;
    std::vector<uint64_t> vec_avail_times;
    std::vector<std::string> vec_avoid;
    std::vector<std::string> vec_instr;
    std::vector<room_t> vec_prooms;
    uint64_t days;
    uint64_t rsrv_blks;
};

struct state_t {
  health_t health;
  std::vector<course_t> state;
};

struct pfit_t {
  health_t health;
  uint64_t bs;
};

struct room_pfit_t {
  std::string id;
  int weight;
};

const int randsched_idx[6] = { 0, 25, 20, 15, 35, 7 };
const uint64_t rand_sched[6][35] = {
  {},
  {
    0x0200000003000000Ui64, //M
    0x020000000c000000Ui64,
    0x0200000030000000Ui64,
    0x02000000c0000000Ui64,
    0x0200000300000000Ui64,
    0x0400000003000000Ui64, //T
    0x040000000c000000Ui64,
    0x0400000030000000Ui64,
    0x04000000c0000000Ui64,
    0x0400000300000000Ui64,
    0x0800000003000000Ui64, //W
    0x080000000c000000Ui64,
    0x0800000030000000Ui64,
    0x08000000c0000000Ui64,
    0x0800000300000000Ui64,
    0x1000000003000000Ui64, //T
    0x100000000c000000Ui64,
    0x1000000030000000Ui64,
    0x10000000c0000000Ui64,
    0x1000000300000000Ui64,
    0x2000000003000000Ui64, //F
    0x200000000c000000Ui64,
    0x2000000030000000Ui64,
    0x20000000c0000000Ui64,
    0x2000000300000000Ui64
  },
  {
    0x1400000003000000Ui64, //TT
    0x140000000c000000Ui64,
    0x1400000030000000Ui64,
    0x14000000c0000000Ui64,
    0x1400000300000000Ui64,
    0x2200000003000000Ui64, //MF
    0x220000000c000000Ui64,
    0x2200000030000000Ui64,
    0x22000000c0000000Ui64,
    0x2200000300000000Ui64,
    0x0a00000003000000Ui64, //MW
    0x0a0000000c000000Ui64,
    0x0a00000030000000Ui64,
    0x0a000000c0000000Ui64,
    0x0a00000300000000Ui64,
    0x2800000003000000Ui64, //WF
    0x280000000c000000Ui64,
    0x2800000030000000Ui64,
    0x28000000c0000000Ui64,
    0x2800000300000000Ui64
  },
  {
    0x1400000000070000Ui64, //TT
    0x1400000000380000Ui64,
    0x1400000001c00000Ui64,
    0x140000000e000000Ui64,
    0x1400000070000000Ui64,
    0x1400000380000000Ui64,
    0x2a00000000030000Ui64, //MWF
    0x2a000000000c0000Ui64,
    0x2a00000000300000Ui64,
    0x2a00000000c00000Ui64,
    0x2a00000003000000Ui64,
    0x2a0000000c000000Ui64,
    0x2a00000030000000Ui64,
    0x2a000000c0000000Ui64,
    0x2a00000300000000Ui64,
    0x2a00000c00000000Ui64
  },
  {
    0x1e00000000030000Ui64, //MTWT
    0x1e00000000300000Ui64,
    0x1e00000000c00000Ui64,
    0x1e0000000c000000Ui64,
    0x1e00000030000000Ui64,
    0x1e000000c0000000Ui64,
    0x1e00000300000000Ui64,
    0x2e00000000030000Ui64, //MTWF
    0x2e00000000300000Ui64,
    0x2e00000000c00000Ui64,
    0x2e0000000c000000Ui64,
    0x2e00000030000000Ui64,
    0x2e000000c0000000Ui64,
    0x2e00000300000000Ui64,
    0x3600000000030000Ui64, //MTTF
    0x3600000000300000Ui64,
    0x3600000000c00000Ui64,
    0x360000000c000000Ui64,
    0x3600000030000000Ui64,
    0x36000000c0000000Ui64,
    0x3600000300000000Ui64,
    0x3a00000000030000Ui64, //MWTF
    0x3a00000000300000Ui64,
    0x3a00000000c00000Ui64,
    0x3a0000000c000000Ui64,
    0x3a00000030000000Ui64,
    0x3a000000c0000000Ui64,
    0x3a00000300000000Ui64,
    0x3c00000000030000Ui64, //TWTF
    0x3c00000000300000Ui64,
    0x3c00000000c00000Ui64,
    0x3c0000000c000000Ui64,
    0x3c00000030000000Ui64,
    0x3c000000c0000000Ui64,
    0x3c00000300000000Ui64
  },
  {
    0x3e00000000030000Ui64, //MTWTF
    0x3e00000000300000Ui64,
    0x3e00000000c00000Ui64,
    0x3e0000000c000000Ui64,
    0x3e00000030000000Ui64,
    0x3e000000c0000000Ui64,
    0x3e00000300000000Ui64
  }
};

void util_init            ();

bool ptime_sort           (const pfit_t &lhs, const pfit_t &rhs);
bool proom_sort           (const room_pfit_t &lhs, const room_pfit_t &rhs);

bool file_exists          (const char *file);

int token_count           (const std::string &str, std::string tok);
std::string get_token     (const std::string &str, int n, std::string delim);
std::string make_upper    (const std::string &str);
std::string break_instr   (const std::vector<std::string> &vec_instr);

uint64_t day_to_flag      (const std::string &day);
int day_to_int            (const std::string &day);

extern int options[OPT_TOTOPTS];
extern std::vector<std::vector<int> > vec_bitpos_idx;

inline int cpu_max_threads() 
{
  int CPUInfo[4] = {-1};
  __cpuid(CPUInfo, 0x00000001Ui32);
  return CPUInfo[0] >> 14 & 0x00000fffUi32;
}

inline bool cpu_has_popcnt()
{
  int CPUInfo[4] = {-1};
  __cpuid(CPUInfo, 0x00000001Ui32);
  return CPUInfo[2] >> 23 & 0x00000001Ui32;
}

inline uint64_t make_bitsched(double start_time, double end_time, uint64_t days)
{
  assert(start_time >= 8.0);
  assert(start_time < 21.0);
  assert(end_time > start_time);
  assert(end_time <= 21.0);
  assert(!(0xc1 & days));
  assert(0x3e & days);

  uint64_t bit_sched = days << 56;

  for (int i = static_cast<int>(2 * start_time); i < static_cast<int>(2 * end_time); i++) {
    bit_sched |= 1Ui64 << i;
  }

  return bit_sched;
};

inline int POPCNT(uint64_t v)
{
  if (options[OPT_HASPOPCNT]) {
    return static_cast<int>(__popcnt64(v));
  }

  v -= (v >> 1 & 0x5555555555555555Ui64);
  v  = (v >> 2 & 0x3333333333333333Ui64) + (v & 0x3333333333333333Ui64);
  v  = (v >> 4) + v & 0x0f0f0f0f0f0f0f0fUi64;
  v  = v * 0x0101010101010101Ui64 >> 56;
  return static_cast<int>(v);
}

inline double rand_unitintvl(boost::mt19937 &my_rng)
{
  return my_rng() / 4294967296.0;
}  

inline double mean(const std::vector<double> &n)
{
  double sum = 0;

  for (int i = 0; i < n.size(); i++) {
    sum += n[i];
  }

  return sum / n.size();
}

inline double stdevp(const std::vector<double> &n, const double mean)
{
  double sum_sq = 0;

  for (int i = 0; i < n.size(); i++) {
    sum_sq += pow(n[i] - mean, 2);
  }

  return sqrt(sum_sq / n.size());
}

// TODO: fix bias scoring
inline uint64_t get_score(const health_t &health)
{
  uint64_t score = static_cast<uint64_t>(health.fitness)      << 54 
                 | static_cast<uint64_t>(health.bias_fitness) << 40
                 | static_cast<uint64_t>(health.late_penalty) << 10
                 | static_cast<uint64_t>(health.buf_fitness)  <<  0;

  return score;
}

#endif // !defined(UTILITY_HPP)
