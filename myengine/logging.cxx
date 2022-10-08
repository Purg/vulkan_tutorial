// Copyright (c) 2021 paul.tunison
// Distributed under the OSI-approved Apache 2.0 License.
// See top-level LICENSE file details.

#include "logging.h"

namespace myengine::logging {

std::string
now_str()
{
  typedef std::chrono::steady_clock clock_t;
  typedef clock_t::rep rep_t;

  // Static "start" time to delta from
  static auto const first_t = clock_t::now();

  // This should be in nanoseconds (steady_clock::duration ==
  // chrono::nanoseconds)
  auto delta_nano =
    std::chrono::duration_cast< std::chrono::nanoseconds >(
      clock_t::now() - first_t ).count();
  auto micro = delta_nano % (rep_t) 1e9 / 1000, // microseconds truncation
       seconds = delta_nano / (rep_t) 1e9,      // total seconds
       hours = seconds / 3600,                  // minutes truncation
       minutes = seconds % 3600 / 60;           // hours truncation

  // seconds is assigned above to *total* seconds, this is the truncation to
  // the seconds slice of the time.
  seconds = seconds % 3600 % 60;

  char buf[ 18 ];  // HHHH:MM:SS.DDDDDD<NULL>
  snprintf( buf, 18, "%04lld:%02lld:%02lld.%06lld", hours, minutes, seconds,
            micro );
  return { buf };
}

} // namespace myengine::logging
