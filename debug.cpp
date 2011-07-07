#include <iostream>

#include "debug.hpp"

using namespace std;


void Debug::live_or_die()
{
  string str;

  if (m_vec_warnings.size() || m_vec_errors.size()) {
    debug_log.open("debug.log", ios::app);

  } else {
    return;
  }

  while (m_vec_warnings.size()) {
    str = m_vec_warnings.back();
    m_vec_warnings.pop_back();
    debug_log << str << endl;
  }

  if (m_vec_errors.size()) {
    while (m_vec_errors.size()) {
      str = m_vec_errors.back();
      m_vec_errors.pop_back();
      debug_log << str << endl;
    }

    cout << endl << "Catastrophic failure: see debug.log" << endl;
    debug_log.close();
    exit(1);
  }

  debug_log.close();
}

void Debug::push_error(std::string str) {
  time_t rawtime;
  time(&rawtime);
  std::string push_str = asctime(localtime(&rawtime));
  push_str += "  ERROR: " + str;
  m_vec_errors.push_back(push_str);
};

void Debug::push_warning(std::string str) {
  time_t rawtime;
  time(&rawtime);
  std::string push_str = asctime(localtime(&rawtime));
  push_str += "  WARNING: " + str;
  m_vec_warnings.push_back(push_str);
};