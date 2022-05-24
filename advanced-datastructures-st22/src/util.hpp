#ifndef UTIL_HPP
#define UTIL_HPP

#include <chrono>
#include <cstdint>

namespace ads {
namespace util {

using HighResClockTimepoint =
    std::chrono::time_point<std::chrono::high_resolution_clock>;

/**
 * @brief Return time difference in milliseconds.
 *
 * @param start The start time point.
 * @param stop The stop time point.
 * @return The time difference in milliseconds.
 */
static inline int64_t time_diff(const HighResClockTimepoint& start,
                                const HighResClockTimepoint& stop) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)
      .count();
}

/**
 * @brief Output help text and exit program with error code EXIT_FAILURE.
 */
static inline void malformed_input() {
  std::cerr << "Required> ./ads_programm_a [bv|bp] eingabe_datei ausgabe_datei"
            << std::endl;
  std::exit(EXIT_FAILURE);
}

}  // namespace util
}  // namespace ads

#endif  // UTIL_HPP
