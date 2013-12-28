/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Author : Benoit Mathieu
 *
 * This file is part of Yaafe.
 *
 * Yaafe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Yaafe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>
#include <vector>
#include <map>

#ifdef WITH_TIMERS
#include <unistd.h>
#endif

namespace YAAFE
{

  struct ParameterDescriptor
  {
    std::string m_identifier;
    std::string m_description;
    std::string m_defaultValue;
  };
  typedef std::vector<ParameterDescriptor> ParameterDescriptorList;
  typedef std::map<std::string,std::string> ParameterMap;
  std::string getParam(const std::string& key, const ParameterMap& params, const ParameterDescriptorList& pList);

  std::string encodeParameterMap(const ParameterMap& params);
  ParameterMap decodeParameterMap(const std::string& str);

  /**
   * verbose flag
   */
  extern bool verboseFlag;


#ifdef WITH_TIMERS

  class Timer
  {
   public:
     virtual ~Timer()
     {
     }

     void start();
     void stop();

     static Timer* get_timer(const std::string& name);
     static void print_all_timers();

   private:
     Timer(const std::string& name);

     std::string m_name;
     double m_totalTime;
     double m_lastStart;

     static std::vector<Timer*> s_allTimers;
  };

  static double getCPUTime()
  {
    timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return double(ts.tv_sec) + 1e-9 * double(ts.tv_nsec);
  }

  inline void Timer::start()
  {
    m_lastStart = getCPUTime();
  }

  inline void Timer::stop()
  {
    m_totalTime += (getCPUTime() - m_lastStart);
  }

#endif

} // end YAAFE

#endif /* UTILS_H_ */
