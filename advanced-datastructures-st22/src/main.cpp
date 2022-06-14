#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "bp/dynamic_bp_tree.hpp"
#include "bp/query.hpp"
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
 * @param params Additional parameter information.
 */
static inline void print_results(const std::string &algo, int64_t time,
                                 int64_t space, const std::string &params) {
  std::cout << "RESULT" << RESULT_SEP << "algo=" << algo << RESULT_SEP
            << "name=Tobias Fuchs" << RESULT_SEP << "time=" << time
            << RESULT_SEP << "space=" << space << RESULT_SEP << params
            << std::endl;
}

/**
 * @brief Runs the bitvector benchmark.
 *
 * @tparam BlockType The block type.
 * @tparam SizeType The size type.
 * @tparam SignedIntType The signed integer type.
 * @tparam MinLeafSizeBlocks The minimum leaf size in blocks.
 * @tparam InitialLeafSizeBlocks The initial leaf size in blocks.
 * @tparam MaxLeafSizeBlocks The maximum leaf size in blocks.
 * @param input_file The input file with queries.
 * @param output_file The output file.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks>
static inline void run_bv(const std::string &input_file,
                          const std::string &output_file) {
  // Prepare everything
  const auto problem_instance =
      ads::bv::query::parse_bv_input<BlockType, SizeType, SignedIntType>(
          input_file);

  // Open output file
  std::ofstream ofs(output_file, std::ios::trunc);
  if (!ofs.is_open()) {
    std::cerr << "Could not open result file." << std::endl;
    ads::util::malformed_input();
  }

  // Run all queries
  const auto start = std::chrono::high_resolution_clock::now();
  ads::bv::DynamicBitVector<BlockType, SizeType, SignedIntType,
                            MinLeafSizeBlocks, InitialLeafSizeBlocks,
                            MaxLeafSizeBlocks>
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
        ofs << bv.rank(query.first_param, query.second_param) << "\n";
        break;
      case ads::bv::query::BVQueryType::SELECT:
        ofs << bv.select(query.first_param, query.second_param) << "\n";
        break;
      default:
        break;
    }
  }

  // Print result line
  const auto end = std::chrono::high_resolution_clock::now();
  ofs.close();
  const auto time = ads::util::time_diff(start, end);
  const auto space = bv.space_used();
  std::ostringstream oss;
  oss << "param_block_type=" << typeid(BlockType).name() << RESULT_SEP
      << "param_size_type=" << typeid(SizeType).name() << RESULT_SEP
      << "param_signed_type=" << typeid(SignedIntType).name() << RESULT_SEP
      << "param_min_leaf=" << MinLeafSizeBlocks << RESULT_SEP
      << "param_initial_leaf=" << InitialLeafSizeBlocks << RESULT_SEP
      << "param_max_leaf=" << MaxLeafSizeBlocks;
  print_results("bv", time, space, oss.str());
}

/**
 * @brief Runs the BP benchmark.
 *
 * @param input_file The input file with queries.
 * @param output_file The output file to write results to.
 */
template <class BlockType, class SizeType, class SignedIntType,
          SizeType MinLeafSizeBlocks, SizeType InitialLeafSizeBlocks,
          SizeType MaxLeafSizeBlocks, SizeType BlocksPerChunk>
static inline void run_bp(const std::string &input_file,
                          const std::string &output_file) {
  // Prepare everything
  const auto problem_instance = ads::bp::query::parse_bp_input(input_file);

  // Open output file
  std::ofstream ofs(output_file, std::ios::trunc);
  if (!ofs.is_open()) {
    std::cerr << "Could not open result file." << std::endl;
    ads::util::malformed_input();
  }

  // Run all queries
  const auto start = std::chrono::high_resolution_clock::now();
  ads::bp::DynamicBPTree<BlockType, SizeType, SignedIntType, MinLeafSizeBlocks,
                         InitialLeafSizeBlocks, MaxLeafSizeBlocks,
                         BlocksPerChunk>
      bp_tree;
  for (const auto &query : problem_instance) {
    switch (query.type) {
      case ads::bp::query::BPQueryType::DELETE_NODE:
        bp_tree.delete_node(query.first_param);
        break;
      case ads::bp::query::BPQueryType::INSERT_CHILD:
        bp_tree.insert_node(query.first_param, query.second_param,
                            query.third_param);
        break;
      case ads::bp::query::BPQueryType::CHILD:
        ofs << bp_tree.i_th_child(query.first_param, query.second_param)
            << "\n";
        break;
      case ads::bp::query::BPQueryType::SUBTREE_SIZE:
        ofs << bp_tree.subtree_size(query.first_param) << "\n";
        break;
      case ads::bp::query::BPQueryType::PARENT:
        ofs << bp_tree.parent(query.first_param) << "\n";
        break;
      default:
        break;
    }
  }

  // Write pre-order-traversal children sizes to file
  bp_tree.pre_order_children_sizes(ofs);

  // Print result line
  const auto end = std::chrono::high_resolution_clock::now();
  ofs.close();
  const auto time = ads::util::time_diff(start, end);
  const auto space = bp_tree.space_used();
  std::ostringstream oss;
  oss << "param_block_type=" << typeid(BlockType).name() << RESULT_SEP
      << "param_size_type=" << typeid(SizeType).name() << RESULT_SEP
      << "param_signed_type=" << typeid(SignedIntType).name() << RESULT_SEP
      << "param_min_leaf=" << MinLeafSizeBlocks << RESULT_SEP
      << "param_initial_leaf=" << InitialLeafSizeBlocks << RESULT_SEP
      << "param_max_leaf=" << MaxLeafSizeBlocks << RESULT_SEP
      << "param_chunk_size=" << BlocksPerChunk;
  print_results("bp", time, space, oss.str());
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
    run_bv<uint64_t, uint32_t, int32_t, 32, 64, 128>(input_file, output_file);
  } else if (bp) {
    run_bp<uint64_t, uint32_t, int32_t, 16, 32, 64, 8>(input_file, output_file);
  } else {
    std::cerr << "Mode has to be either \"bv\" or \"bp\"." << std::endl;
    ads::util::malformed_input();
  }

  return EXIT_SUCCESS;
}
