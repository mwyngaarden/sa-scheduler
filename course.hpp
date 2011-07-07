#pragma once
#if !defined(COURSE_HPP)
#define COURSE_HPP

#include <fstream>
#include <map>
#include <string>

#include "bias.hpp"
#include "room.hpp"
#include "utility.hpp"


class Course : public Bias, public Room
{
  public:
    Course();
    bool push_course        (course_t &course);
    bool push_const_course  (course_t &course);

    std::map<std::string, course_t> m_mapstr_const_course;
    std::map<std::string, course_t> m_mapstr_course;

  private:
    std::ifstream course_file;
};

namespace
{
const int COURSE_TOKENS = 12;
const uint64_t COURSE_AVOID = 1 <<  0;
const uint64_t COURSE_DAYS  = 1 <<  1;
const uint64_t COURSE_GROUP = 1 <<  2;
const uint64_t COURSE_HOURS = 1 <<  3;
const uint64_t COURSE_ID    = 1 <<  4;
const uint64_t COURSE_INSTR = 1 <<  5;
const uint64_t COURSE_LECTS = 1 <<  6;
const uint64_t COURSE_NAME  = 1 <<  7;
const uint64_t COURSE_ROOM  = 1 <<  8;
const uint64_t COURSE_SIZE  = 1 <<  9;
const uint64_t COURSE_TIMES = 1 << 10;
const uint64_t COURSE_TYPE  = 1 << 11;
}

#endif // !defined(COURSE_HPP)
