#pragma once
#if !defined(DEBUG_HPP)
#define DEBUG_HPP

#include <ctime>
#include <fstream>
#include <string>
#include <vector>


class Debug
{
  public:
    void live_or_die();
    void push_error(std::string str);
    void push_warning(std::string str);

  private:
    std::vector<std::string> m_vec_errors;
    std::vector<std::string> m_vec_warnings;
    std::ofstream debug_log;
};

#endif // !defined(DEBUG_HPP)
