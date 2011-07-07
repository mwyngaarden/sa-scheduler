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
      uint64_t days);

    void set_bias(
      const std::string &instr, 
      double start_time, 
      double end_time,
      uint64_t days, 
      e_bias bias);

  private:
    std::map<std::string, std::vector<e_bias> > m_mapstr_bias;
    std::ifstream bias_file;
};

#endif // !defined(BIAS_HPP)
