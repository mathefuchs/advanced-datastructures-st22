#include <string.h>

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bv/dynamic_bitvector.hpp"
#include "bv/query.hpp"
#include "bv/simple_bitvector.hpp"
#include "util.hpp"

#define RESULT_SEP ("\t")

/**
 * @brief Prints a result line for the given performances.
 *
 * @param algo The algorithm used.
 * @param time The time results.
 * @param space The space results.
 */
static inline void print_results(const std::string &algo, int64_t time,
                                 int64_t space) {
  std::cout << "RESULT" << RESULT_SEP << "algo=" << algo << RESULT_SEP
            << "name=Tobias Fuchs" << RESULT_SEP << "time=" << time
            << RESULT_SEP << "space=" << space << std::endl;
}

/**
 * @brief Runs the bitvector benchmark.
 *
 * @tparam BlockType The block type.
 * @tparam SizeType The size type.
 * @tparam MinLeafSizeBlocks The minimum leaf size in blocks.
 * @tparam InitialLeafSizeBlocks The initial leaf size in blocks.
 * @tparam MaxLeafSizeBlocks The maximum leaf size in blocks.
 * @param input_file The input file with queries.
 * @param output_file The output file.
 */
template <class BlockType, class SizeType, SizeType MinLeafSizeBlocks,
          SizeType InitialLeafSizeBlocks, SizeType MaxLeafSizeBlocks>
static inline void run_bv(const std::string &input_file,
                          const std::string &output_file) {
  // Prepare everything
  const auto problem_instance =
      ads::bv::query::parse_bv_input<BlockType, SizeType>(input_file);
  std::vector<SizeType> output;
  output.reserve(problem_instance.queries.size());

  // Run all queries
  const auto start = std::chrono::high_resolution_clock::now();
  ads::bv::DynamicBitVector<BlockType, SizeType, MinLeafSizeBlocks,
                            InitialLeafSizeBlocks, MaxLeafSizeBlocks>
      bv(problem_instance.bv);
  for (const auto &query : problem_instance.queries) {
    switch (query.type) {
      case ads::bv::query::BVQueryType::INSERT:
        bv.insert(query.first_param, query.second_param);
        break;
      case ads::bv::query::BVQueryType::DELETE:
        bv.delete_element(query.first_param);
        break;
      case ads::bv::query::BVQueryType::FLIP:
        bv.flip(query.first_param);
        break;
      case ads::bv::query::BVQueryType::RANK:
        output.push_back(bv.rank(query.first_param, query.second_param));
        break;
      case ads::bv::query::BVQueryType::SELECT:
        output.push_back(bv.select(query.first_param, query.second_param));
        break;
      default:
        break;
    }
  }
  const auto end = std::chrono::high_resolution_clock::now();

  // Write output to file
  std::ofstream ofs(output_file);
  if (ofs.is_open()) {
    for (const auto &query_output : output) {
      ofs << query_output << "\n";
    }
    ofs.close();
  } else {
    std::cerr << "Could not open result file." << std::endl;
    ads::util::malformed_input();
  }

  // Print result line
  const auto time = ads::util::time_diff(start, end);
  const auto space = bv.space_used();
  print_results("bv", time, space);
}

/**
 * @brief Runs the BP benchmark.
 *
 * @param input_file The input file with queries.
 * @param output_file The output file to write results to.
 */
static inline void run_bp(const std::string &input_file,
                          const std::string &output_file) {
  std::cout << input_file << RESULT_SEP << output_file << std::endl;
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

  // Parse input
  std::string algo(argv[1]);
  std::string input_file(argv[2]);
  std::string output_file(argv[3]);
  bool bv = algo == "bv";
  bool bp = bv ? false : algo == "bp";

  // Run program
  if (bv) {
    run_bv<uint64_t, uint64_t, 32, 64, 128>(input_file, output_file);
  } else if (bp) {
    run_bp(input_file, output_file);
  } else {
    std::cerr << "Mode has to be either \"bv\" or \"bp\"." << std::endl;
    ads::util::malformed_input();
  }

  return EXIT_SUCCESS;
}
