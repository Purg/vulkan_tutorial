/**
 * Basic logging macros.
 *
 * I would do `constexpr` functions but I don't know how to represent the
 * `<<`-able type...
 */

#ifndef LOGGING_H
#define LOGGING_H

#include <chrono>
#include <iostream>
#include <string>

// Not sure if this is going to cause a performance issue. If so maybe give up
// width-saving
// convenience for the simple use of __FILE__?
#define __FILENAME__ \
  ( strrchr( __FILE__, '/' ) ? strrchr( __FILE__, '/' ) + 1 : __FILE__ )
#define _LOG_LOC \
  __FILENAME__ << "(" << __LINE__ << ")[" << __func__ << "]"

namespace myengine::logging {

/**
 * Generate a string for time elapsed since the start of logging.
 * TODO: Implement in a CXX.
 *
 * Note that we specifically say "since the start of logging" as opposed to the
 * "start of the program."
 * An internal static variable is utilized and initialized on first call which
 * initializes the basis of timing.
 * This is just for logging so relativity is effectively conserved.
 *
 * @return String representing elapsed time since the start of logging in the
 * format:
 *   "HHHH:MM:SS.DDDDDD" (H=hours, M=minutes, S=seconds, D=decimal seconds).
 */
char const* now_str();

} // namespace myengine::logging

#ifdef NDEBUG
# define LOG_DEBUG( msg )
#else
# define LOG_DEBUG( msg ) \
  do \
  { \
    std::cerr   << "[DEBUG] [" << myengine::logging::now_str() << "] " \
                << _LOG_LOC << " " << msg << std::endl; \
  } while( false )
#endif

#define LOG_INFO( msg ) \
  do \
  { \
    std::cerr   << "[ INFO] [" << myengine::logging::now_str() << "] " \
                << _LOG_LOC << " " << msg << std::endl; \
  } while( false )

#define LOG_WARN( msg ) \
  do \
  { \
    std::cerr   << "[ WARN] [" << myengine::logging::now_str() << "] " \
                << _LOG_LOC << " " << msg << std::endl; \
  } while( false )

#define LOG_ERROR( msg ) \
  do \
  { \
    std::cerr   << "[ERROR] [" << myengine::logging::now_str() << "] " \
                << _LOG_LOC << " " << msg << std::endl; \
  } while( false )

#endif //LOGGING_H
