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
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>

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
}

void Schedule::optimize()
{
  boost::mt19937 rng;
  rng.seed(static_cast<const uint32_t>(time(NULL)));
  m_start_time = time(NULL);

  double delta;
  double temp;

  int iter;

  health_t health;

  string str;

  state_t cur_state;
  
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

  state_t best_state;
  best_state.health.init();

  // index courses through a vector
  for (course_it = m_mapstr_course.begin(); course_it != m_mapstr_course.end(); course_it++) {
    best_state.state.push_back((*course_it).second);
  }

  assert(best_state.state.size());
  cur_state.state.resize(best_state.state.size());

  // begin simulated annealing
  for (iter = 0, temp = INIT_TEMP; true; iter++, temp *= COOL_RATE) {
    health.reset();

    perturb_state(best_state, crs_name_idx, rng, health, cur_state.state);

    cur_state.health = health;
    cur_state.health.fitness = health.avoid_colls * AVOID_MUL
                             + health.instr_colls * INSTR_MUL
                             + health.room_colls  * ROOM_MUL;

    cur_state.health.ffit = get_score(cur_state.health);
    delta = cur_state.health.ffit - best_state.health.ffit;

    if (delta < 0 || exp(-delta / temp) > rand_unitintvl(rng)) {
      best_state = cur_state;
    }

    if (!((iter + 1) % options[OPT_POLLINTVL])) {
      if (options[OPT_VERBOSE]) {
        display_stats(best_state, iter + 1);
        cout << "temperature: " << temp << endl << endl;
      }

      if (temp < ABS_TEMP) {
        m_end_time = time(NULL);
        save_scheds(best_state);
        return;
      }
    }
  } // end for
}

void Schedule::save_scheds(const state_t &best_state)
{
  cout << endl;

  if (best_state.health.instr_colls) {
    cout << "Collisions with instructors scheduled concurrently:" << endl;

    for (int i = 0; i < best_state.state.size(); i++) {
      if (best_state.state[i].health.instr_colls) {
        cout << best_state.state[i].id << endl;
      }
    }
  }

  if (best_state.health.room_colls) {
    cout << endl << "Collisions with rooms scheduled concurrently:" << endl;

    for (int i = 0; i < best_state.state.size(); i++) {
      if (best_state.state[i].health.room_colls) {
        cout << best_state.state[i].id << ": " << best_state.state[i].room_id << endl;
      }
    }
  }

  if (best_state.health.avoid_colls) {
    cout << endl<< "Collisions with avoidances scheduled concurrently:" << endl;

    for (int i = 0; i < best_state.state.size(); i++) {
      if (best_state.state[i].health.avoid_colls) {
        cout << best_state.state[i].id << endl;
      }
    }
  }

  if (best_state.health.bias_fitness < 0) {
    cout << endl << "Collisions with instructor blocks:" << endl;

    for (int i = 0; i < best_state.state.size(); i++) {
      if (best_state.state[i].health.bias_fitness < 0) {
        cout << best_state.state[i].id << endl;
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

  for (i = 0; i < best_state.state.size(); i++) { // cycle through classes
    if (best_state.state[i].health.avoid_colls ||
        best_state.state[i].health.instr_colls ||
        best_state.state[i].health.room_colls  ||
        best_state.state[i].health.bias_fitness < 0)
    {
      continue;
    }

    bs          = best_state.state[i].bs_sched;
    days        = static_cast<uint8_t>((bs >> 56).to_ulong());
    times       = (bs & MASK_TIME) >> 16;
    start_time  = 16 + get_firstbitpos(times);
    blocks      = static_cast<int>((times).count());
    
    id          = best_state.state[i].id;
    room_id     = best_state.state[i].room_id;
    group       = best_state.state[i].group;
    
    if (best_state.state[i].lectures) {
      lects = best_state.state[i].lectures;
    }

    saved_scheds << fixed
                 << id                                                << ","
                 << best_state.state[i].name                          << ","
                 << best_state.state[i].hours                         << ","
                 << (best_state.state[i].is_lab ? "L" : "S")          << ","
                 << flag_to_str(days)                                 << ","
                 << static_cast<double>(start_time / 2.0)             << "-"
                 << static_cast<double>((start_time + blocks) / 2.0)  << ","
                 << vec_to_str(best_state.state[i].vec_instr)         << ","
                 << room_id                                           << ","
                 << best_state.state[i].size                          << ","
                 << lects                                             << ","
                 << group                                             << ","
                 << vec_to_str(best_state.state[i].vec_avoid)         << endl;

    for (j = 0; j < vec_bitpos_idx[days].size(); j++) { // cycle through bit-days
      idx = vec_bitpos_idx[days][j] + start_time;

      for (k = 0; k < best_state.state[i].vec_instr.size(); k++) {
        str = best_state.state[i].vec_instr[k];
        mapstr_instr[str].m_week_idx[idx].data = id + "<br>" + room_id;
        mapstr_instr[str].m_week_idx[idx].span = blocks;
      }

      mapstr_room[room_id].m_week_idx[idx].data =
        break_instr(best_state.state[i].vec_instr) + id;
      mapstr_room[room_id].m_week_idx[idx].span = blocks;

      for (k = 1; k < blocks; k++) { // extend html table data element
        for (l = 0; l < best_state.state[i].vec_instr.size(); l++) {
          mapstr_instr[best_state.state[i].vec_instr[l]].m_week_idx[idx + k].data = "SPAN";
        }

        mapstr_room [room_id].m_week_idx[idx + k].data = "SPAN";
      }
    }

    for (l = 0; l < token_count(group, ":"); l++) {
      str = get_token(group, l, ":");

      for (j = 0; j < vec_bitpos_idx[days].size(); j++) {
        idx = vec_bitpos_idx[days][j] + start_time;
        mapstr_group[str].m_week_idx[idx].data =
          break_instr(best_state.state[i].vec_instr) + id + "<br>" + room_id;
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

int Schedule::get_duration()
{
  return static_cast<int>(difftime(m_end_time, m_start_time));
};

void Schedule::display_stats(const state_t &state, int iter)
{
  cout << "iteration = " << right << iter
       << " fitness: "
       << "(a=" << state.health.avoid_colls 
       << " i=" << state.health.instr_colls 
       << " r=" << state.health.room_colls << ") "
       << state.health.ffit << endl << endl;
}

void Schedule::get_bitsched(
  course_t              &course,
  map<string, course_t> &crs_name_idx,
  map<string, bs_t>     &u_crs_idx,
  map<string, bs_t>     &u_instr_idx,
  map<string, bs_t>     &u_room_idx,
  boost::mt19937        &rng)
{
  int i, j;
  int idx;
  int avoid_colls;
  int instr_colls;
  int room_colls;

  string str;

  bs_t bs;
  
  pfit_t pfit;

  vector<double> vec_fitness;

  vector<pfit_t> ptime;


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
    pfit.health.fitness     = avoid_colls * AVOID_MUL 
                            + instr_colls * INSTR_MUL 
                            + room_colls  * ROOM_MUL;
    
    ptime.push_back(pfit);
    vec_fitness.push_back(pfit.health.fitness);
  }

  sort(ptime.begin(), ptime.end(), ptime_sort);
  idx = 0;

  if (ptime.size() > 1) {
    sort(ptime.begin(), ptime.end(), ptime_sort);
    
    double fmean   = mean(vec_fitness);
    double fstdevp = stdevp(vec_fitness, fmean);
    
    assert(fmean >= 0);
    assert(fstdevp >= 0);
    
    boost::normal_distribution<> nd(fmean, fstdevp);
    boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > var_nor(rng, nd);
    
    idx = static_cast<int>(fabs(fmean - var_nor()) / 2.0);
    idx = idx >= ptime.size() ? static_cast<int>(ptime.size()) - 1 : idx;
  }

  course.bs_sched = ptime[idx].bs;
  course.health.avoid_colls = ptime[idx].health.avoid_colls;
  course.health.instr_colls = ptime[idx].health.instr_colls;
  course.health.room_colls  = ptime[idx].health.room_colls;
  
  u_crs_idx[course.name]     |= course.bs_sched;
  u_room_idx[course.room_id] |= course.bs_sched;

  for (j = 0; j < course.vec_instr.size(); j++) {
    u_instr_idx[course.vec_instr[j]] |= course.bs_sched;
  }
}

void Schedule::perturb_state(
  const state_t         &best_state,
  map<string, course_t> &crs_name_idx,
  boost::mt19937        &rng,
  health_t              &health,
  vector<course_t>      &cur_state)
{
  bool room_change;

  course_t course;

  double rand_double;

  int i, j;
  int idx;

  bs_t bs;
  
  room_pfit_t new_pfit;

  vector<double> vec_weight;

  vector<room_pfit_t> vec_proom;
  
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

  for (i = 0; i < best_state.state.size(); i++) {
    course      = best_state.state[i];
    room_change = false;
    rand_double = rand_unitintvl(rng);

    if (!course.const_room && 
         (rand_double < PB_RMUT     || 
          course.health.buf_fitness || 
          course.room_id == "")) 
    {
      idx = 0;

      if (course.vec_prooms.size() > 1) {
        for (j = 0; j < course.vec_prooms.size(); j++) {
          new_pfit.id     = course.vec_prooms[j].id;
          new_pfit.weight = 0;

          if (u_room_idx.find(course.vec_prooms[j].id) != u_room_idx.end()) {
            bs = u_room_idx[course.vec_prooms[j].id];
            new_pfit.weight = static_cast<int>((bs & MASK_DAY).count() * (bs & MASK_TIME).count());
          }

          vec_proom.push_back(new_pfit);
          vec_weight.push_back(new_pfit.weight);
        }

        sort(vec_proom.begin(), vec_proom.end(), proom_sort);
        
        double fmean   = mean(vec_weight);
        double fstdevp = stdevp(vec_weight, fmean);
        
        assert(fmean >= 0);
        assert(fstdevp >= 0);
        
        boost::normal_distribution<> nd(fmean, fstdevp);
        boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > var_nor(rng, nd);
        
        idx = static_cast<int>(fabs(fmean - var_nor()) / 5.2);
        idx = idx >= course.vec_prooms.size() ? static_cast<int>(course.vec_prooms.size()) - 1 : idx;
      }

      if (course.vec_prooms[idx].id != course.room_id) {
        room_change = true;
        course.room_id = course.vec_prooms[idx].id;
        course.health.buf_fitness = abs(options[OPT_ROOMBUFF] - (course.vec_prooms[idx].size - course.size));
      }
    }

    rand_double = rand_unitintvl(rng);

    if (rand_double < PB_TMUT           ||
        room_change                     ||
        course.bs_sched.none()          ||
        course.health.bias_fitness < 0  ||
        course.health.fitness) 
    {
      get_bitsched(course, crs_name_idx, u_crs_idx, u_instr_idx, u_room_idx, rng);
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
    assert(course.const_room || (course.vec_prooms.size() && !course.const_room));
    
    health.avoid_colls  += course.health.avoid_colls;
    health.bias_fitness += course.health.bias_fitness;
    health.buf_fitness  += course.health.buf_fitness;
    health.instr_colls  += course.health.instr_colls;
    health.late_penalty += ((course.bs_sched & MASK_TIME) >> 16).to_ulong();
    health.room_colls   += course.health.room_colls;
    
    cur_state[i] = course;
  }
}