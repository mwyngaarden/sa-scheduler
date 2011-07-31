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
#if !defined(UTILITY_HPP)
#define UTILITY_HPP

#include <bitset>
#include <cassert>
#include <cmath>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>


typedef std::bitset<64> bs_t;
const bs_t MASK_DAY   ("0011111000000000000000000000000000000000000000000000000000000000");
const bs_t MASK_TIME  ("0000000000000000000000111111111111111111111111110000000000000000");
const bs_t VALID_MASK ("1100000111111111111111000000000000000000000000001111111111111111");

const std::string COMPILE_TIME = __TIME__;
const std::string COMPILE_DATE = __DATE__;

const std::string FILE_BIAS   = "instructor.csv";
const std::string FILE_COURSE = "courses.csv";
const std::string FILE_GROUP  = "groups.csv";
const std::string FILE_HTML   = "html_header.txt";
const std::string FILE_ROOM   = "rooms.csv";
const std::string FILE_CONFIG = "sched.cfg";

const std::string VALID_DAYS = "SUNMONTUEWEDTHUFRISATALL";

const int SCORE_VOID = -100000;
const int INF = 0x7fffffff;

// Mersenne Twister
class prng_t {
  public:
    prng_t() {
      seed(0x84db26a9); // default seed
    }

    prng_t(uint32_t s) {
      seed(s);
    };

    void seed(uint32_t s) {
      MT[0] = s;
      for (int i = 1; i < 624; i++) {
        MT[i] = 0x6c078965 * (MT[i-1] >> 30 ^ MT[i-1]) + i;
      }

      index = -1;
    };

    uint32_t operator()() {
      
      uint32_t y;

      index++;

      if (index > 623) {
        index = 0;
      }

      if (!index) {
        for (int i = 0; i < 624; i++) {
          y = (MT[i] & 0x80000000) | (MT[(i + 1) % 624] & 0x7fffffff);
          MT[i] = MT[(i + 397) % 624] ^ y >> 1;

          if (y & 0x1) {
            MT[i] ^= 0x9908b0df;
          }
        }
      }

      y = MT[index];
      y ^= y >> 11;
      y ^= y <<  7 & 0x9d2c5680;
      y ^= y << 15 & 0xefc60000;
      y ^= y >> 18;

      return y;      
    };

  private:
    uint32_t MT[624];
    int index;
};

enum e_bias {
  AVR6, AVR5, AVR4, AVR3, AVR2, AVR1,
  OPEN,
  PRF1, PRF2, PRF3, PRF4, PRF5, PRF6,
  VOID
};

struct room_t {
  std::string id;
  int size;
};

class health_t
{
  public:
    void reset() {
      avoid_colls  = 0;
      bias_fitness = 0;
      buf_fitness  = 0;
      instr_colls  = 0;
      late_penalty = 0;
      room_colls   = 0;
      sched        = 0;

      fitness      = 0;
      ffit         = 0;
    };

    void init() {
      avoid_colls  =  INF;
      bias_fitness = -INF;
      buf_fitness  =  INF;
      instr_colls  =  INF;
      late_penalty =  INF;
      room_colls   =  INF;
      sched        = -INF;

      fitness      =  1.0e+30;
      ffit         =  1.0e+30;
    };

    int avoid_colls;
    int bias_fitness;
    int buf_fitness;
    int instr_colls;
    int late_penalty;
    int room_colls;
    int sched;

    double fitness;
    double ffit;
};

class course_t
{
  public:
    void reset() {
      const_days   = false;
      const_room   = false;
      const_time   = false;
      days         = 0;
      end_time     = 0.0;
      group        = "";
      hours        = 0;
      id           = "";
      is_lab       = false;
      lectures     = 0;
      multi_days   = false;
      name         = "";
      room_id      = "";
      size         = 0;
      start_time   = 0.0;
      bs_sched.reset();
      health.init();
      vec_avail_times.clear();
      vec_avoid.clear();
      vec_days.clear();
      vec_instr.clear();
      vec_prooms.clear();
    };

    bool const_days;
    bool const_room;
    bool const_time;
    bool is_lab;
    bool multi_days;

    double end_time;
    double start_time;

    health_t health;

    int hours;
    int lectures;
    int size;

    std::bitset<64> bs_sched;

    std::string group;
    std::string id;
    std::string name;
    std::string room_id;

    std::vector<bs_t> vec_avail_times;
    std::vector<std::string> vec_avoid;
    std::vector<std::string> vec_instr;
    std::vector<room_t> vec_prooms;
    std::vector<uint8_t> vec_days;

    uint8_t days;
};

struct state_t {
  health_t health;
  std::vector<course_t> vec_crs;
};

struct pfit_t {
  health_t health;
  bs_t bs;
};

struct room_pfit_t {
  std::string id;
  int weight;
};

const int sched_count_idx[6] = { 0, 25, 20, 15, 35, 7 };
const std::string sched_hex_idx[6][35] = {
  {},
  {
    "0200000003000000", //M
    "020000000c000000",
    "0200000030000000",
    "02000000c0000000",
    "0200000300000000",
    "0400000003000000", //T
    "040000000c000000",
    "0400000030000000",
    "04000000c0000000",
    "0400000300000000",
    "0800000003000000", //W
    "080000000c000000",
    "0800000030000000",
    "08000000c0000000",
    "0800000300000000",
    "1000000003000000", //T
    "100000000c000000",
    "1000000030000000",
    "10000000c0000000",
    "1000000300000000",
    "2000000003000000", //F
    "200000000c000000",
    "2000000030000000",
    "20000000c0000000",
    "2000000300000000"
  },
  {
    "1400000003000000", //TT
    "140000000c000000",
    "1400000030000000",
    "14000000c0000000",
    "1400000300000000",
    "2200000003000000", //MF
    "220000000c000000",
    "2200000030000000",
    "22000000c0000000",
    "2200000300000000",
    "0a00000003000000", //MW
    "0a0000000c000000",
    "0a00000030000000",
    "0a000000c0000000",
    "0a00000300000000",
    "2800000003000000", //WF
    "280000000c000000",
    "2800000030000000",
    "28000000c0000000",
    "2800000300000000"
  },
  {
    "1400000000070000", //TT
    "1400000000380000",
    "1400000001c00000",
    "140000000e000000",
    "1400000070000000",
    "1400000380000000",
    "2a00000000030000", //MWF
    "2a000000000c0000",
    "2a00000000300000",
    "2a00000000c00000",
    "2a00000003000000",
    "2a0000000c000000",
    "2a00000030000000",
    "2a000000c0000000",
    "2a00000300000000",
    "2a00000c00000000"
  },
  {
    "1e00000000030000", //MTWT
    "1e00000000300000",
    "1e00000000c00000",
    "1e0000000c000000",
    "1e00000030000000",
    "1e000000c0000000",
    "1e00000300000000",
    "2e00000000030000", //MTWF
    "2e00000000300000",
    "2e00000000c00000",
    "2e0000000c000000",
    "2e00000030000000",
    "2e000000c0000000",
    "2e00000300000000",
    "3600000000030000", //MTTF
    "3600000000300000",
    "3600000000c00000",
    "360000000c000000",
    "3600000030000000",
    "36000000c0000000",
    "3600000300000000",
    "3a00000000030000", //MWTF
    "3a00000000300000",
    "3a00000000c00000",
    "3a0000000c000000",
    "3a00000030000000",
    "3a000000c0000000",
    "3a00000300000000",
    "3c00000000030000", //TWTF
    "3c00000000300000",
    "3c00000000c00000",
    "3c0000000c000000",
    "3c00000030000000",
    "3c000000c0000000",
    "3c00000300000000"
  },
  {
    "3e00000000030000", //MTWTF
    "3e00000000300000",
    "3e00000000c00000",
    "3e0000000c000000",
    "3e00000030000000",
    "3e000000c0000000",
    "3e00000300000000"
  }
};

void util_init            ();

bool ptime_sort           (const pfit_t &lhs, const pfit_t &rhs);

bool file_exists          (const char *file);

int token_count           (const std::string &str, std::string tok);
std::string get_token     (const std::string &str, int n, std::string delim);
std::string make_upper    (const std::string &str);
std::string break_instr   (const std::vector<std::string> &vec_instr);
std::string break_days    (const std::vector<uint8_t> &vec_days);

uint8_t day_to_flag       (const std::string &day);
int day_to_int            (const std::string &day);
std::string flag_to_str   (uint8_t days);
std::string vec_to_str    (const std::vector<std::string> &vec_instr);
std::string hex_to_bin    (char hex);

extern std::map<std::string, std::string> prog_opts;
extern std::vector<std::vector<int> > vec_bitpos_idx;
extern std::vector<std::vector<bs_t> > sched_bs_idx;

inline int cpu_max_threads()
{
  int CPUInfo[4] = {-1};
  __cpuid(CPUInfo, 0x00000001Ui32);
  return CPUInfo[0] >> 14 & 0x00000fffUi32;
}

inline bs_t make_bitsched(double start_time, double end_time, uint8_t days)
{
  assert(start_time >= 8.0);
  assert(start_time < 21.0);
  assert(end_time > start_time);
  assert(end_time <= 21.0);
  assert(!(0xc1 & days));
  assert(0x3e & days);

  bs_t bs(days);
  bs <<= 56;

  for (int i = static_cast<int>(2 * start_time); i < static_cast<int>(2 * end_time); i++) {
    bs.set(i, 1);
  }

  return bs;
};

inline double rand_unitintvl(prng_t &rng)
{
  return (rng() & 0x7fffffff) / 2147483648.0;
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

inline double get_score(const health_t &health)
{
  double score = 0.0;
  
  score += health.buf_fitness  * 1.0e-1;
  score += health.late_penalty * 1.0e-6;
  score += (1.0e+6 - health.bias_fitness) * 1.0e-2;
  score += health.fitness      * 1.0e+9;

  return score;
}

inline int get_firstbitpos(const bs_t &bs)
{
  assert (bs.any());

  for (int i = 0; i < 47; i++) {
    if (bs[i]) { 
      return i;
    }
  }

  return 47;
}

#endif // !defined(UTILITY_HPP)
