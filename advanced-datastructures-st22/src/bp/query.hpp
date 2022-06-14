#ifndef BP_QUERY_HPP
#define BP_QUERY_HPP

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "util.hpp"

namespace ads {
namespace bp {
namespace query {

/**
 * @brief Type of queries.
 *
 * Use 32-bit type for alignment.
 */
enum class BPQueryType : uint32_t {
  DELETE_NODE,
  INSERT_CHILD,
  CHILD,
  SUBTREE_SIZE,
  PARENT,
  QUERY_TYPE_SIZE,
};

/**
 * @brief Get the query type string.
 *
 * @param query_type The query type.
 * @return The string to return.
 */
static inline std::string bp_query_type_str(BPQueryType query_type) {
  switch (query_type) {
    case BPQueryType::DELETE_NODE:
      return "deletenode";
    case BPQueryType::INSERT_CHILD:
      return "insertchild";
    case BPQueryType::CHILD:
      return "child";
    case BPQueryType::SUBTREE_SIZE:
      return "subtree_size";
    case BPQueryType::PARENT:
      return "parent";
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
static inline bool bp_query_type_has_second_arg(BPQueryType query_type) {
  switch (query_type) {
    case BPQueryType::INSERT_CHILD:
    case BPQueryType::CHILD:
      return true;
    default:
      return false;
  }
}

/**
 * @brief Whether the query type has a second argument.
 *
 * @param query_type The query type.
 * @return true if it has a second argument.
 * @return false otherwise.
 */
static inline bool bp_query_type_has_third_arg(BPQueryType query_type) {
  switch (query_type) {
    case BPQueryType::INSERT_CHILD:
      return true;
    default:
      return false;
  }
}

/**
 * @brief Returns the query type for a given string.
 *
 * @param query_type_str The query type string.
 * @return The query type.
 */
static inline BPQueryType string_to_query_type(
    const std::string& query_type_str) {
  for (u_int64_t query_type_id = 0;
       query_type_id < static_cast<uint64_t>(BPQueryType::QUERY_TYPE_SIZE);
       ++query_type_id) {
    const auto type = static_cast<BPQueryType>(query_type_id);
    if (query_type_str == bp_query_type_str(type)) {
      return type;
    }
  }

  std::cerr << "Could not parse query type." << std::endl;
  ads::util::malformed_input();
  return BPQueryType::QUERY_TYPE_SIZE;
}

/**
 * @brief The query object.
 * Width is 16B to align to cache line sizes of typically 64B or 128B.
 */
struct BPQuery {
  uint32_t first_param;
  uint32_t second_param;
  uint32_t third_param;
  BPQueryType type;
};

/**
 * @brief Parses the problem instance.
 *
 * @param input_file_name The input file name.
 * @return The problem instance.
 */
static inline std::vector<BPQuery> parse_bp_input(
    const std::string& input_file_name) {
  // Load input file into memory
  std::ifstream input_stream(input_file_name);
  if (input_stream.is_open()) {
    // Parse queries
    std::vector<BPQuery> queries;
    std::string line;
    while (std::getline(input_stream, line)) {
      // Split string
      std::string type_str, arg1_str, arg2_str, arg3_str;
      std::stringstream query_str(line);
      std::getline(query_str, type_str, ' ');
      std::getline(query_str, arg1_str, ' ');

      // Retrieve query type information
      const auto type = string_to_query_type(type_str);
      const bool second_arg = bp_query_type_has_second_arg(type);
      const bool third_arg = bp_query_type_has_third_arg(type);
      if (second_arg) {
        std::getline(query_str, arg2_str, ' ');
        if (third_arg) {
          std::getline(query_str, arg3_str, ' ');
        }
      }

      // Parse and check types
      const auto arg1 = static_cast<uint32_t>(std::stoul(arg1_str));
      const uint32_t arg2 =
          second_arg ? static_cast<uint32_t>(std::stoul(arg2_str)) : 0u;
      const uint32_t arg3 =
          third_arg ? static_cast<uint32_t>(std::stoul(arg3_str)) : 0u;

      // Append query
      queries.push_back(BPQuery{arg1, arg2, arg3, type});
    }

    // Return problem instance
    queries.shrink_to_fit();
    return queries;
  } else {
    std::cerr << "Could not open file \"" << input_file_name << "\"."
              << std::endl;
    ads::util::malformed_input();
    return {{}, {}};
  }
}

}  // namespace query
}  // namespace bp
}  // namespace ads

#endif
