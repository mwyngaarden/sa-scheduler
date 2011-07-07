#include <algorithm>
#include <fstream>

#include "utility.hpp"

using namespace std;


int options[OPT_TOTOPTS];
vector<vector<int> > vec_bitpos_idx;

void util_init()
{
  vec_bitpos_idx.resize(128);

  for (int i = 0; i < 128; i++)  {
    for (int j = 0; j < 7; j++) {
      if (i & 1 << j) {
        vec_bitpos_idx[i].push_back(48 * j);
      }
    }
  }
}

string make_upper(const string &str)
{
  string ret_str = str;
  transform(ret_str.begin(), ret_str.end(), ret_str.begin(), toupper);
  return ret_str;
}

int token_count(const string &str, string tok)
{
  if (str == "") {
    return 0;
  }

  int count = 1;

  for (int i = 0; i < str.size(); i++)
    if (!str.compare(i, 1, tok)) {
      count++;
    }

  return count;
}

string get_token(const string &str, int n, string delim)
{
  string ret_str;

  for (int i = 0, t = 1; i < str.size() && t <= (n+1); i++) {
    if (!str.compare(i, 1, delim)) {
      t++;

    } else if (t == n + 1) {
      ret_str.append(str, i, 1);
    }
  }

  return ret_str;
}

string break_instr(const vector<string> &vec_instr)
{
  string ret_str;

  for (int i = 0; i < vec_instr.size(); i++) {
    ret_str += vec_instr[i] + "<br>";
  }

  return ret_str;
}

bool file_exists(const char *file)
{
  fstream myfile(file);
  return !myfile.fail();
}

bool ptime_sort(const pfit_t &lhs, const pfit_t &rhs)
{
  return lhs.health.fitness < rhs.health.fitness;
}

bool proom_sort(const room_pfit_t &lhs, const room_pfit_t &rhs)
{
  return lhs.weight < rhs.weight;
}

uint64_t day_to_flag(const std::string &day)
{
 return static_cast<uint64_t>(1Ui64 << day_to_int(day));
}

int day_to_int(const std::string &day)
{
  return static_cast<int>(VALID_DAYS.find(day) / 3);
}