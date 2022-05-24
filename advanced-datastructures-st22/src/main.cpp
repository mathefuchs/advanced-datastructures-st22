#include <string.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "util.hpp"

/**
 * Output the computation's results.
 *
 * @param preproc_start The preprocessing start timestamp.
 * @param query_start The query start timestamp.
 * @param query_stop The query end timestamp.
 * @param result_string The result string.
 * @param argv The cli arguments.
 */
static inline void output_results(
    const ads::util::HighResClockTimepoint &preproc_start,
    const ads::util::HighResClockTimepoint &query_start,
    const ads::util::HighResClockTimepoint &query_stop,
    const std::string &result_string, char **argv) {
  auto preproc_time = ads::util::time_diff(preproc_start, query_start);
  auto query_time = ads::util::time_diff(query_start, query_stop);
  std::cout << "RESULT\talgo=" << argv[1] << "\tname=Tobias Fuchs"
            << "\tconstruction time=" << std::to_string(preproc_time)
            << "\tquery time=" << std::to_string(query_time)
            << "\tsolutions=" << result_string << "\tfile=" << argv[2]
            << std::endl;
}

/**
 * Entry point of program.
 *
 * @param argc The number of cli arguments.
 * @param argv The cli arguments.
 * @return An exit code.
 */
int main(int argc, char **argv) {
  // Check number of cli args
  if (argc != 4) {
    ads::util::malformed_input();
  }

  // Check mode
  bool bv = strcmp(argv[1], "bv") == 0;
  bool bp = bv ? false : strcmp(argv[1], "bp") == 0;
  if (!bv && !bp) {
    ads::util::malformed_input();
  }

  // TODO
  return EXIT_SUCCESS;
}
