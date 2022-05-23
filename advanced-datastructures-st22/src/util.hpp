#ifndef UTIL_HPP
#define UTIL_HPP

#include <chrono>

namespace ads {
namespace util {

using HighResClockTimepoint =
    std::chrono::time_point<std::chrono::high_resolution_clock>;

/**
 * Return time difference in milliseconds.
 *
 * @param start The start time point.
 * @param stop The stop time point.
 * @return int64_t The time difference in milliseconds.
 */
static inline int64_t time_diff(const HighResClockTimepoint& start,
                                const HighResClockTimepoint& stop) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)
      .count();
}

}  // namespace util
}  // namespace ads

#endif  // UTIL_HPP
