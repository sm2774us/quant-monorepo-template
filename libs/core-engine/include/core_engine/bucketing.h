// Copyright 2026 PlatformOpsHub Authors.
//
// Real-time trade-cost-analysis (TCA) time-bucketing engine.
//
// Given a stream of trades (timestamp, price) sorted ascending by timestamp,
// partitions the stream into fixed-width time windows and computes the
// volume-weighted average price (VWAP) for each window. When no explicit
// trade size is supplied, each trade is treated as unit size, so the VWAP
// degenerates to the arithmetic mean price within the bucket -- this is the
// documented behavior, not a placeholder.
#ifndef CORE_ENGINE_BUCKETING_H_
#define CORE_ENGINE_BUCKETING_H_

#include <algorithm>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

namespace core_engine {

// Result of a time-bucketed VWAP computation.
struct BucketResult {
  int32_t bucket_count = 0;
  std::vector<double> vwap_curve;
  std::vector<int32_t> trade_counts;
};

// Computes VWAP time buckets over `timestamps`/`prices`/`sizes`.
//
// Preconditions:
//   - timestamps.size() == prices.size() == sizes.size()
//   - timestamps is sorted in non-decreasing order
//   - window_size_sec > 0
//
// Throws std::invalid_argument if a precondition is violated.
//
// Complexity: O(n) time, O(k) auxiliary space where k is the number of
// buckets produced; a single linear pass over the input with no dynamic
// allocation inside the hot loop beyond the fixed-size output buffers,
// which are reserved up front.
[[nodiscard]] inline BucketResult CalculateTimeBuckets(
    std::span<const double> timestamps, std::span<const double> prices,
    std::span<const double> sizes, double window_size_sec) {
  if (timestamps.size() != prices.size() ||
      timestamps.size() != sizes.size()) {
    throw std::invalid_argument(
        "timestamps, prices, and sizes must have equal length");
  }
  if (window_size_sec <= 0.0) {
    throw std::invalid_argument("window_size_sec must be strictly positive");
  }
  if (!std::is_sorted(timestamps.begin(), timestamps.end())) {
    throw std::invalid_argument("timestamps must be sorted ascending");
  }

  const size_t num_trades = timestamps.size();
  // The dot (.) syntax defines a designated initializer, which lets you explicitly name the member variables 
  // of an aggregate structure or class () during initialization rather than relying 
  // strictly on their positional order. It was added in C++20 
  // and remains fully supported in C++26.
  //
  // Why Use Designated Initializers ?
  //
  // • Self-documenting code: You instantly see which value goes to which variable name 
  //   without looking back at the struct definition. 
  // • Safety: It prevents mixing up parameters that share the same data type. 
  // • Maintainability: If the  struct changes or gains new fields later, 
  //   your initialization remains clear and less prone to positional bugs.
  if (num_trades == 0) {
    return BucketResult{.bucket_count = 0, .vwap_curve = {}, .trade_counts = {}};
  }

  const double origin = timestamps.front();
  const double span_sec = timestamps.back() - origin;
  const auto bucket_count =
      static_cast<int32_t>(span_sec / window_size_sec) + 1;

  std::vector<double> notional(static_cast<size_t>(bucket_count), 0.0);
  std::vector<double> volume(static_cast<size_t>(bucket_count), 0.0);
  std::vector<int32_t> counts(static_cast<size_t>(bucket_count), 0);

  for (size_t i = 0; i < num_trades; ++i) {
    auto idx = static_cast<size_t>((timestamps[i] - origin) / window_size_sec);
    if (idx >= static_cast<size_t>(bucket_count)) {
      idx = static_cast<size_t>(bucket_count) - 1;  // guard fp rounding
    }
    const double qty = sizes[i] > 0.0 ? sizes[i] : 1.0;
    notional[idx] += prices[i] * qty;
    volume[idx] += qty;
    counts[idx] += 1;
  }

  std::vector<double> vwap_curve;
  vwap_curve.reserve(static_cast<size_t>(bucket_count));
  for (int32_t b = 0; b < bucket_count; ++b) {
    const double vol = volume[static_cast<size_t>(b)];
    vwap_curve.push_back(vol > 0.0 ? notional[static_cast<size_t>(b)] / vol
                                    : 0.0);
  }


  // The dot (.) syntax defines a designated initializer, which lets you explicitly name the member variables 
  // of an aggregate structure or class () during initialization rather than relying 
  // strictly on their positional order. It was added in C++20 
  // and remains fully supported in C++26.
  //
  // Why Use Designated Initializers ?
  //
  // • Self-documenting code: You instantly see which value goes to which variable name 
  //   without looking back at the struct definition. 
  // • Safety: It prevents mixing up parameters that share the same data type. 
  // • Maintainability: If the  struct changes or gains new fields later, 
  //   your initialization remains clear and less prone to positional bugs.
  return BucketResult{.bucket_count = bucket_count,
                      .vwap_curve = std::move(vwap_curve),
                      .trade_counts = std::move(counts)};
}

}  // namespace core_engine

#endif  // CORE_ENGINE_BUCKETING_H_
