#ifndef EMPTY_HPP
#define EMPTY_HPP

namespace ads {
namespace meta {

/**
 * @brief Empty struct to pass as template argument if needed.
 */
template <class T>
struct Empty {
  Empty() {}
  explicit Empty(T) {}
};

}  // namespace meta
}  // namespace ads

#endif
