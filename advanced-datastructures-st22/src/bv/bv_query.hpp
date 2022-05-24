#ifndef BV_QUERY_HPP
#define BV_QUERY_HPP

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "bv/simple_bitvector.hpp"
#include "util.hpp"

namespace ads {
namespace bv {
namespace query {

/**
 * @brief Type of bit-vector queries.
 *
 * Use 64-bit type for alignment.
 */
enum class BVQueryType : uint64_t {
  INSERT,
  DELETE,
  FLIP,
  RANK,
  SELECT,
  QUERY_TYPE_SIZE
};

/**
 * @brief Get the query type string.
 *
 * @param query_type The query type.
 * @return The string to return.
 */
static inline std::string bv_query_type_str(BVQueryType query_type) {
  switch (query_type) {
    case BVQueryType::INSERT:
      return "insert";
    case BVQueryType::DELETE:
      return "delete";
    case BVQueryType::FLIP:
      return "flip";
    case BVQueryType::RANK:
      return "rank";
    case BVQueryType::SELECT:
      return "select";
    default:
      std::cerr << "Could not parse query type." << std::endl;
      ads::util::malformed_input();
      return "nop";
  }
}

/**
 * @brief Whether the query type has a second argument.
 *
 * @param query_type The query type.
 * @return true if it has a second argument.
 * @return false otherwise.
 */
static inline bool bv_query_type_has_second_arg(BVQueryType query_type) {
  switch (query_type) {
    case BVQueryType::INSERT:
    case BVQueryType::RANK:
    case BVQueryType::SELECT:
      return true;
    case BVQueryType::DELETE:
    case BVQueryType::FLIP:
      return false;
    default:
      std::cerr << "Could not parse query type." << std::endl;
      ads::util::malformed_input();
      return false;
  }
}

/**
 * @brief Whether the query type's first argument has a binary value.
 *
 * @param query_type The query type.
 * @return true if the query type's first argument has a binary value.
 * @return false otherwise.
 */
static inline bool bv_query_type_first_arg_binary(BVQueryType query_type) {
  switch (query_type) {
    case BVQueryType::RANK:
    case BVQueryType::SELECT:
      return true;
    case BVQueryType::INSERT:
    case BVQueryType::DELETE:
    case BVQueryType::FLIP:
      return false;
    default:
      std::cerr << "Could not parse query type." << std::endl;
      ads::util::malformed_input();
      return false;
  }
}

/**
 * @brief Whether the query type's second argument has a binary value.
 *
 * @param query_type The query type.
 * @return true if the query type's second argument has a binary value.
 * @return false otherwise.
 */
static inline bool bv_query_type_second_arg_binary(BVQueryType query_type) {
  switch (query_type) {
    case BVQueryType::INSERT:
      return true;
    case BVQueryType::RANK:
    case BVQueryType::SELECT:
    case BVQueryType::DELETE:
    case BVQueryType::FLIP:
      return false;
    default:
      std::cerr << "Could not parse query type." << std::endl;
      ads::util::malformed_input();
      return false;
  }
}

/**
 * @brief Returns the bit-vector query type for a given string.
 *
 * @param query_type_str The bit-vector query type string.
 * @return The bit-vector query type.
 */
static inline BVQueryType string_to_query_type(
    const std::string& query_type_str) {
  for (u_int64_t query_type_id = 0;
       query_type_id < static_cast<uint64_t>(BVQueryType::QUERY_TYPE_SIZE);
       ++query_type_id) {
    const auto type = static_cast<BVQueryType>(query_type_id);
    if (query_type_str == bv_query_type_str(type)) {
      return type;
    }
  }

  std::cerr << "Could not parse query type." << std::endl;
  ads::util::malformed_input();
  return BVQueryType::QUERY_TYPE_SIZE;
}

/**
 * @brief Bit-vector query object.
 * Width is 16B to align to cache line sizes of typically 64B or 128B.
 */
struct BVQuery {
  uint32_t first_param;
  uint32_t second_param;
  BVQueryType type;
};

/**
 * @brief Represents a bit-vector problem instance.
 */
struct BVProblemInstance {
  SimpleBitVector<uint64_t> bv;
  std::vector<BVQuery> queries;
};

/**
 * @brief Parses the problem instance.
 *
 * @param input_file_name The input file name.
 * @return The problem instance.
 */
static inline BVProblemInstance parse_bv_input(
    const std::string& input_file_name) {
  // Load input file into memory
  std::ifstream input_stream(input_file_name);
  if (input_stream.is_open()) {
    // Get initial bit-vector size
    std::string line;
    std::getline(input_stream, line);
    size_t initial_size = std::stoul(line);

    // Create problem instance object
    BVProblemInstance instance{SimpleBitVector<uint64_t>(initial_size), {}};

    // Parse initial bit-vector
    size_t i = 0;
    while (i < initial_size && std::getline(input_stream, line)) {
      instance.bv.set(i, static_cast<bool>(std::stoi(line)));
      ++i;
    }
    if (i != initial_size) {
      std::cerr << "File \"" << input_file_name << "\" ended unexpectedly."
                << std::endl;
      ads::util::malformed_input();
    }

    // Parse queries
    while (std::getline(input_stream, line)) {
      // Split string
      std::string type_str, arg1_str, arg2_str;
      std::stringstream query_str(line);
      std::getline(query_str, type_str, ' ');
      std::getline(query_str, arg1_str, ' ');

      // Retrieve query type information
      const auto type = string_to_query_type(type_str);
      const auto first_arg_binary = bv_query_type_first_arg_binary(type);
      const auto second_arg_binary = bv_query_type_second_arg_binary(type);
      const auto has_second_arg = bv_query_type_has_second_arg(type);
      if (has_second_arg) {
        std::getline(query_str, arg2_str, ' ');
      }

      // Parse and check types
      const auto arg1 = static_cast<uint32_t>(std::stoul(arg1_str));
      const auto arg2 =
          has_second_arg ? static_cast<uint32_t>(std::stoul(arg2_str)) : 0u;
      if ((first_arg_binary && !(arg1 == 0 || arg1 == 1)) ||
          (second_arg_binary && !(arg2 == 0 || arg2 == 1))) {
        std::cerr << "Could not parse query arguments." << std::endl;
        ads::util::malformed_input();
      }

      // Append query
      instance.queries.emplace_back(arg1, arg2, type);
    }

    // Return problem instance
    instance.queries.shrink_to_fit();
    return instance;
  } else {
    std::cerr << "Could not open file \"" << input_file_name << "\"."
              << std::endl;
    ads::util::malformed_input();
    return {SimpleBitVector<uint64_t>(0), {}};
  }
}

}  // namespace query
}  // namespace bv
}  // namespace ads

#endif
