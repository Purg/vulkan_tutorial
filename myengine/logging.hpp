/**
 * Basic logging macros.
 *
 * I would do `constexpr` functions but I don't know how to represent the `<<`-able type...
 */

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <cstring>
#include <iostream>


// Not sure if this is going to cause a performance issue. If so maybe give up width-saving
// convenience for the simple use of __FILE__?
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define _LOG_LOC \
  __FILENAME__ << "(" << __LINE__ << ")[" << __func__ << "]"


#ifdef NDEBUG
#define LOG_DEBUG( msg )
#else
#define LOG_DEBUG( msg ) \
  do \
  { \
    std::cerr << "[DEBUG] " << _LOG_LOC << " " << msg << std::endl; \
  } while(false)
#endif

#define LOG_INFO( msg ) \
  do \
  { \
    std::cerr << "[ INFO] " << _LOG_LOC << " " << msg << std::endl; \
  } while(false)

#define LOG_ERROR( msg ) \
  do \
  { \
    std::cerr << "[ERROR] " << _LOG_LOC << " " << msg << std::endl; \
  } while(false)

#endif //LOGGING_HPP
