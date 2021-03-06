/**
 * Basic logging macros.
 *
 * I would do `constexpr` functions but I don't know how to represent the `<<`-able type...
 */

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <chrono>
#include <iostream>
#include <string>


// Not sure if this is going to cause a performance issue. If so maybe give up width-saving
// convenience for the simple use of __FILE__?
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define _LOG_LOC \
  __FILENAME__ << "(" << __LINE__ << ")[" << __func__ << "]"


namespace myengine::logging
{

/// Generate a string for time elapsed since the start of logging.
/**
 * TODO: Implement in a CXX.
 *
 * Note that we specifically say "since the start of logging" as opposed to the "start of the
 * program." An internal static variable is utilized and initialized on first call which initializes
 * the basis of timing. This is just for logging so relativity is effectively conserved.
 *
 * @return String representing elapsed time since the start of logging in the format
 *   "HHHH:MM:SS.DDDDDD" (H=hours, M=minutes, S=seconds, D=decimal seconds).
 */
std::string
now_str()
{
  typedef std::chrono::steady_clock clock_t;
  typedef clock_t::rep rep_t;
  static auto const first_t = clock_t::now();
  // This should be in nanoseconds (steady_clock::duration == chrono::nanoseconds)
  auto delta_nano =
      std::chrono::duration_cast<std::chrono::nanoseconds>( clock_t::now() - first_t ).count();
  auto micro = delta_nano % (rep_t)1e9 / 1000,
      seconds = delta_nano / (rep_t)1e9,
      hours = seconds / 3600,
      minutes = seconds % 3600 / 60;
  seconds = seconds % 3600 % 60;
  char buf[18];  // HHHH:MM:SS.DDDDDD<NULL>
  snprintf( buf, 18, "%04ld:%02ld:%02ld.%06ld", hours, minutes, seconds, micro );
  return std::string( buf );
}

}


#ifdef NDEBUG
#define LOG_DEBUG( msg )
#else
#define LOG_DEBUG( msg ) \
  do \
  { \
    std::cerr << "[DEBUG] [" << myengine::logging::now_str() << "] " \
              << _LOG_LOC << " " << msg << std::endl; \
  } while(false)
#endif

#define LOG_INFO( msg ) \
  do \
  { \
    std::cerr << "[ INFO] [" << myengine::logging::now_str() << "] " \
              << _LOG_LOC << " " << msg << std::endl; \
  } while(false)

#define LOG_ERROR( msg ) \
  do \
  { \
    std::cerr << "[ERROR] [" << myengine::logging::now_str() << "] " \
              << _LOG_LOC << " " << msg << std::endl; \
  } while(false)

#endif //LOGGING_HPP
