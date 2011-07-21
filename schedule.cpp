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

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>

#include "debug.hpp"
#include "schedule.hpp"
#include "utility.hpp"

using namespace std;


Schedule::Schedule() : Course()
{
  string read_str;
  Debug debug;
  header_file.open(FILE_HTML);

  if (!header_file.is_open()) {
    read_str = "Unable to open " + FILE_HTML;
    debug.push_error(read_str);
  }

  debug.live_or_die();

  while (getline(header_file, read_str)) {
    m_vec_header.push_back(read_str);
  }

  header_file.close();

  m_rng.seed(static_cast<const uint32_t>(time(NULL)));
}

void Schedule::optimize()
{
  boost::mt19937 my_rng;

  my_rng.seed(m_rng());
  m_start_time = time(NULL);

  double delta;
  double reduction;
  double temp;

  int i;

  health_t health;

  string str;

  state_t cur_state;
  state_t best_state;

  best_state.health.init();

  map<string, course_t> crs_name_idx;
  map<string, course_t>::iterator course_it;

  map<string, room_t>::iterator room_it;


  // for referencing constant courses by name rather than id
  for (course_it = m_mapstr_const_course.begin(); course_it != m_mapstr_const_course.end(); course_it++) {
    str = (*course_it).second.name;
    crs_name_idx[str] = (*course_it).second;

    crs_name_idx[str].bs_sched |=
      make_bitsched(crs_name_idx[str].start_time,
                    crs_name_idx[str].end_time,
                    crs_name_idx[str].days);
  }

  // index courses through a vector
  for (course_it = m_mapstr_course.begin(); course_it != m_mapstr_course.end(); course_it++) {
    best_state.vec_crs.push_back((*course_it).second);
  }

  assert(best_state.vec_crs.size());
  cur_state.vec_crs.resize(best_state.vec_crs.size());

  state_t i_state;

  cout.precision(1);

  // begin annealing
  reduction = options[OPT_TEMPRED] / 1000000.0;
  temp = TEMP_INIT;

  for (i = 0; ; i++, temp *= reduction) {
    health.reset();

    perturb_state(best_state, crs_name_idx, health, cur_state.vec_crs, my_rng);

    cur_state.health = health;
    cur_state.health.fitness = static_cast<int>(cur_state.vec_crs.size()) - cur_state.health.sched;
    cur_state.health.ffit = cur_state.health.fitness; // get_score(cur_state.health);
    
    delta = cur_state.health.ffit - best_state.health.ffit;

    if (delta < 0 || exp(-delta / temp) > rand_unitintvl(my_rng)) {
      best_state = cur_state;
    }

    if (!((i + 1) % options[OPT_POLLINTVL])) {
      if (options[OPT_VERBOSE]) {
        cout << fixed << scientific << "temp = " << temp << endl;
        display_stats(best_state, i + 1);
      }

      if (temp < TEMP_MIN) {
        m_end_time = time(NULL);
        m_best_fitness = best_state.health.sched;
        //save_scheds(best_state);
        return;
      }
    } 
  } // end for
}

void Schedule::save_scheds(const state_t &state)
{
  cout << endl;

  if (state.health.instr_colls) {
    cout << "Collisions with instructors scheduled concurrently:" << endl;

    for (int i = 0; i < state.vec_crs.size(); i++) {
      if (state.vec_crs[i].health.instr_colls) {
        cout << state.vec_crs[i].id << endl;
      }
    }
  }

  if (state.health.room_colls) {
    cout << endl << "Collisions with rooms scheduled concurrently:" << endl;

    for (int i = 0; i < state.vec_crs.size(); i++) {
      if (state.vec_crs[i].health.room_colls) {
        cout << state.vec_crs[i].id << ": " << state.vec_crs[i].room_id << endl;
      }
    }
  }

  if (state.health.avoid_colls) {
    cout << endl<< "Collisions with avoidances scheduled concurrently:" << endl;

    for (int i = 0; i < state.vec_crs.size(); i++) {
      if (state.vec_crs[i].health.avoid_colls) {
        cout << state.vec_crs[i].id << endl;
      }
    }
  }

  if (state.health.bias_fitness < 0) {
    cout << endl << "Collisions with instructor blocks:" << endl;

    for (int i = 0; i < state.vec_crs.size(); i++) {
      if (state.vec_crs[i].health.bias_fitness < 0) {
        cout << state.vec_crs[i].id << endl;
      }
    }
  }

  int i, j, k, l;
  int blocks;
  int idx;
  int start_time;

  string group;
  string id;
  string lects;
  string room_id;
  string str;
  string yesno_str;

  bs_t bs;
  bs_t times;

  uint8_t days;
  
  map<string, Week> mapstr_group;
  map<string, Week> mapstr_instr;
  map<string, Week> mapstr_room;
  
  ofstream group_html;
  ofstream instr_html;
  ofstream room_html;
  ofstream saved_scheds;


  if (file_exists("group.html") ||
      file_exists("instr.html") ||
      file_exists("room.html")  ||
      file_exists("scheduled.csv")) 
  {
    while (true) {
      cout << endl << "Overwrite existing schedules (y/n)? ";
      cin >> yesno_str;

      if (make_upper(yesno_str) == "Y") {
        break;

      } else if (make_upper(yesno_str) == "N") {
        return;
      }
    }
  }

  group_html.open("group.html");
  instr_html.open("instr.html");
  room_html.open("room.html");
  saved_scheds.open("scheduled.csv");

  saved_scheds.precision(1);

  // indices are built using an array of 7*48=336 m_week_t structures for all
  // groups, instructors, and rooms using the day and time as the index, for
  // example, monday (=1) at 15hrs (=30) = [1 * 48 + 30]

  for (i = 0; i < state.vec_crs.size(); i++) { // cycle through classes
    if (state.vec_crs[i].health.avoid_colls ||
        state.vec_crs[i].health.instr_colls ||
        state.vec_crs[i].health.room_colls  ||
        state.vec_crs[i].health.bias_fitness < 0)
    {
      continue;
    }

    bs          = state.vec_crs[i].bs_sched;
    days        = static_cast<uint8_t>((bs >> 56).to_ulong());
    times       = (bs & MASK_TIME) >> 16;
    start_time  = 16 + get_firstbitpos(times);
    blocks      = static_cast<int>((times).count());
    
    id          = state.vec_crs[i].id;
    room_id     = state.vec_crs[i].room_id;
    group       = state.vec_crs[i].group;
    
    if (state.vec_crs[i].lectures) {
      lects = state.vec_crs[i].lectures;
    }

    saved_scheds << fixed
                 << id                                                << ","
                 << state.vec_crs[i].name                             << ","
                 << state.vec_crs[i].hours                            << ","
                 << (state.vec_crs[i].is_lab ? "L" : "S")             << ","
                 << flag_to_str(days)                                 << ","
                 << static_cast<double>(start_time / 2.0)             << "-"
                 << static_cast<double>((start_time + blocks) / 2.0)  << ","
                 << vec_to_str(state.vec_crs[i].vec_instr)            << ","
                 << room_id                                           << ","
                 << "0"                                               << ","
                 << lects                                             << ","
                 << group                                             << ","
                 << vec_to_str(state.vec_crs[i].vec_avoid)            << endl;

    for (j = 0; j < vec_bitpos_idx[days].size(); j++) { // cycle through bit-days
      idx = vec_bitpos_idx[days][j] + start_time;

      for (k = 0; k < state.vec_crs[i].vec_instr.size(); k++) {
        str = state.vec_crs[i].vec_instr[k];
        mapstr_instr[str].m_week_idx[idx].data = id + "<br>" + room_id;
        mapstr_instr[str].m_week_idx[idx].span = blocks;
      }

      mapstr_room[room_id].m_week_idx[idx].data =
        break_instr(state.vec_crs[i].vec_instr) + id;
      mapstr_room[room_id].m_week_idx[idx].span = blocks;

      for (k = 1; k < blocks; k++) { // extend html table data element
        for (l = 0; l < state.vec_crs[i].vec_instr.size(); l++) {
          mapstr_instr[state.vec_crs[i].vec_instr[l]].m_week_idx[idx + k].data = "SPAN";
        }

        mapstr_room [room_id].m_week_idx[idx + k].data = "SPAN";
      }
    }

    for (l = 0; l < token_count(group, ":"); l++) {
      str = get_token(group, l, ":");

      for (j = 0; j < vec_bitpos_idx[days].size(); j++) {
        idx = vec_bitpos_idx[days][j] + start_time;
        mapstr_group[str].m_week_idx[idx].data =
          break_instr(state.vec_crs[i].vec_instr) + id + "<br>" + room_id;
        mapstr_group[str].m_week_idx[idx].span = blocks;

        for (k = 1; k < blocks; k++) {
          mapstr_group[str].m_week_idx[idx + k].data = "SPAN";
        }
      }
    }
  }

  write_html(group_html, mapstr_group);
  write_html(instr_html, mapstr_instr);
  write_html(room_html, mapstr_room);

  if (options[OPT_VERBOSE]) {
    cout << endl << "Schedules saved!" << endl << endl;
  }

  group_html.close();
  instr_html.close();
  room_html.close();
}

void Schedule::write_html(ofstream &file, map<string, Week> &mapstr_cal)
{
  int idx;
  int span;
  int time;

  string member;
  string str;
  stringstream oss;


  for (vector<string>::iterator it=m_vec_header.begin(); it != m_vec_header.end(); it++) {
    file << (*it) << endl;
  }

  for (map<string, Week>::iterator it = mapstr_cal.begin(); it != mapstr_cal.end(); it++) {
    member = (*it).first;
    file << "<h1>" << member << "</h1>\n"
         << "<table>\n"
         << "  <tr>\n"
         << "    <th>Time</th>\n"
         << "    <th>Monday</th>\n"
         << "    <th>Tuesday</th>\n"
         << "    <th>Wednesday</th>\n"
         << "    <th>Thursday</th>\n"
         << "    <th>Friday</th>\n"
         << "  </tr>\n";

    for (int i = 0; i < 26; i++) {
      if ((time = 8 + ( i / 2)) > 12) {
        time -= 12;
      }

      oss << time << ":" << (i % 2 ? "30" : "00");
      file << "  <tr>\n    <td>" << oss.str() << "</td>\n";
      oss.str("");

      for (int j = 1; j < 6; j++) {
        idx = vec_bitpos_idx[1 << j][0] + 16 + i;
        str  = mapstr_cal[member].m_week_idx[idx].data;
        span = mapstr_cal[member].m_week_idx[idx].span;

        if (span > 0) {
          file << "    <td rowspan=\"" << span << "\">" << str << "</td>\n";

        } else if (str != "SPAN") {
          file << "    <td></td>\n";
        }
      }

      file << "  </tr>\n";
    }

    file << "</table>\n\n<p style=\"page-break-before: always\"></p>\n\n";
  }

  file << "</body>\n</html>\n";
}

int Schedule::duration()
{
  return static_cast<int>(difftime(m_end_time, m_start_time));
};

void Schedule::display_stats(const state_t &state, int iter)
{
  cout << ""
       << "iter = " << right << iter
       << " fit: "
       << "(a=" << state.health.avoid_colls
       << " i=" << state.health.instr_colls
       << " r=" << state.health.room_colls << ") "
       << " ( " << state.health.sched << " / " << state.vec_crs.size() << " )"
       << endl << endl;
}

void Schedule::get_bitsched(
  course_t              &course,
  map<string, course_t> &crs_name_idx,
  map<string, bs_t>     &u_crs_idx,
  map<string, bs_t>     &u_instr_idx,
  map<string, bs_t>     &u_room_idx,
  boost::mt19937        &my_rng)
{
  int i, j;
  int avoid_colls;
  int instr_colls;
  int room_colls;

  size_t idx;

  string str;

  bs_t bs;
  
  pfit_t pfit;

  vector<pfit_t> ptime;

  double best = INF;


  for (i = 0; i < course.vec_avail_times.size(); i++) {
    avoid_colls = 0;
    instr_colls = 0;
    room_colls  = 0;
    bs = course.vec_avail_times[i];

    for (j = 0; j < course.vec_avoid.size(); j++) {
      str = course.vec_avoid[j];

      if (crs_name_idx.find(str) != crs_name_idx.end()) {
        avoid_colls += num_conflicts(crs_name_idx[str].bs_sched & bs);

      } else if (u_crs_idx.find(str) != u_crs_idx.end()) {
        avoid_colls += num_conflicts(u_crs_idx[str] & bs);
      }
    }

    if (u_room_idx.find(course.room_id) != u_room_idx.end()) {
      room_colls += num_conflicts(u_room_idx[course.room_id] & bs);
    }

    for (j = 0; j < course.vec_instr.size(); j++) {
      str = course.vec_instr[j];

      if (u_instr_idx.find(str) != u_instr_idx.end()) {
        instr_colls += num_conflicts(u_instr_idx[str] & bs);
      }
    }

    if (u_room_idx.find(course.room_id) != u_room_idx.end()) {
      course.health.room_colls += num_conflicts(u_room_idx[course.room_id] & bs);
    }

    pfit.bs = bs;
    pfit.health.avoid_colls = avoid_colls;
    pfit.health.instr_colls = instr_colls;
    pfit.health.room_colls  = room_colls;
    pfit.health.fitness     = CMUL_AVOID * avoid_colls + 
                              CMUL_INSTR * instr_colls + 
                              CMUL_ROOM  * room_colls;
    
    if (pfit.health.fitness < best) {
      best = pfit.health.fitness;
      idx = ptime.size();
    }
    
    ptime.push_back(pfit);
  }

  course.bs_sched = ptime[idx].bs;
  course.health.avoid_colls = ptime[idx].health.avoid_colls;
  course.health.instr_colls = ptime[idx].health.instr_colls;
  course.health.room_colls  = ptime[idx].health.room_colls;
  
  u_crs_idx[course.name] |= course.bs_sched;
  u_room_idx[course.room_id] |= course.bs_sched;

  for (j = 0; j < course.vec_instr.size(); j++) {
    u_instr_idx[course.vec_instr[j]] |= course.bs_sched;
  }
}

void Schedule::perturb_state(
  const state_t         &const_state,
  map<string, course_t> &crs_name_idx,
  health_t              &health,
  vector<course_t>      &cur_state,
  boost::mt19937        &my_rng)
{
  bool room_change;

  course_t course;

  int i, j;
  int idx;

  bs_t bs;

  state_t state = const_state;

 
  // swap
  size_t size = state.vec_crs.size();
  i = my_rng() % size;
  j = my_rng() % size;

  while (j == i) {
    j = my_rng() % size;
  }
  
  swap(state.vec_crs[i], state.vec_crs[j]);


  map<string, bs_t> u_crs_idx;
  map<string, bs_t> u_instr_idx;
  map<string, bs_t> u_room_idx;

  map<string, course_t>::iterator course_it;


  // set constants for room and instructor times in indices
  for (course_it = m_mapstr_const_course.begin(); course_it != m_mapstr_const_course.end(); course_it++) {
    if ((*course_it).second.room_id != "") {
      u_room_idx[(*course_it).second.room_id] = (*course_it).second.bs_sched;
    }

    for (i = 0; i < (*course_it).second.vec_instr.size(); i++) {
      u_instr_idx[(*course_it).second.vec_instr[i]] = (*course_it).second.bs_sched;
    }
  }

  for (i = 0; i < state.vec_crs.size(); i++) {
    course      = state.vec_crs[i];
    room_change = false;

    if (!course.const_room && (course.health.buf_fitness || course.room_id == "")) {
      idx = 0;

      if (course.vec_prooms[idx].id != course.room_id) {
        room_change = true;
        course.room_id = course.vec_prooms[idx].id;
        course.health.buf_fitness = abs(options[OPT_ROOMBUFF] - (course.vec_prooms[idx].size - course.size));
      }
    }

    if (room_change                     ||
        course.bs_sched.none()          ||
        course.health.bias_fitness < 0  ||
        course.health.fitness > 0) 
    {
      get_bitsched(course, crs_name_idx, u_crs_idx, u_instr_idx, u_room_idx, my_rng);
      course.health.bias_fitness = 0;

      for (j = 0; j < course.vec_instr.size(); j++) {
        course.health.bias_fitness += get_bias(course.vec_instr[j], course.bs_sched);
      }

      if (get_bias("ALL", course.bs_sched) == SCORE_VOID) {
        course.health.bias_fitness -= SCORE_VOID;
      }
    }

    // sanity check!
    assert(course.hours > 0);
    assert(course.hours < 6);
    assert(course.id != "");
    assert(course.lectures == 0 || ((course.lectures == 2 || course.lectures == 3) && course.hours == 3));
    assert(course.name != "");
    assert(course.room_id != "");
    assert((course.bs_sched & MASK_DAY).any());
    assert((course.bs_sched & MASK_TIME).any());
    assert((course.bs_sched & VALID_MASK).none());
    assert(course.vec_avail_times.size());
    assert(course.vec_instr.size());
    assert(course.const_room || (!course.const_room && course.vec_prooms.size()));
    
    health.avoid_colls  += course.health.avoid_colls;
    health.bias_fitness += course.health.bias_fitness;
    health.buf_fitness  += course.health.buf_fitness;
    health.instr_colls  += course.health.instr_colls;
    health.late_penalty += ((course.bs_sched & MASK_TIME) >> 16).to_ulong();
    health.room_colls   += course.health.room_colls;
    health.sched        += !(course.health.avoid_colls  || 
                             course.health.instr_colls  ||
                             course.health.room_colls   ||
                             course.health.bias_fitness  < 0)
                         ? 1 : 0;
    
    cur_state[i] = course;
  }
}