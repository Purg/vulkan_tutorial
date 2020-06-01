/**
 * Basic logging macros.
 */

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <iostream>


#ifdef NDEBUG
#define LOG_DEBUG( msg )
#else
#define LOG_DEBUG( msg ) \
  do \
  { \
    std::cerr << "[BASIC][DEBUG] " << msg << std::endl; \
  } while(false)
#endif

#define LOG_INFO( msg ) \
  do \
  { \
    std::cerr << "[BASIC][ INFO] " << msg << std::endl; \
  } while(false)

#define LOG_ERROR( msg ) \
  do \
  { \
    std::cerr << "[BASIC][ERROR] " << msg << std::endl; \
  } while(false)

#endif //LOGGING_HPP
