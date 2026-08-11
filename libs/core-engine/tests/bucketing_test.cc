// Copyright 2026 PlatformOpsHub Authors.
#include "core_engine/bucketing.h"

#include <gtest/gtest.h>

#include <vector>

namespace core_engine {
namespace {

TEST(CalculateTimeBuckets, EmptyInputReturnsZeroBuckets) {
  const BucketResult result = CalculateTimeBuckets({}, {}, {}, 2.0);
  EXPECT_EQ(result.bucket_count, 0);
  EXPECT_TRUE(result.vwap_curve.empty());
}

TEST(CalculateTimeBuckets, SingleTradeProducesOneBucket) {
  const std::vector<double> ts{1.0};
  const std::vector<double> px{100.0};
  const std::vector<double> sz{10.0};
  const BucketResult result = CalculateTimeBuckets(ts, px, sz, 2.0);
  ASSERT_EQ(result.bucket_count, 1);
  EXPECT_DOUBLE_EQ(result.vwap_curve[0], 100.0);
  EXPECT_EQ(result.trade_counts[0], 1);
}

TEST(CalculateTimeBuckets, EqualWeightFallsBackToUnitSize) {
  const std::vector<double> ts{1.0, 1.2};
  const std::vector<double> px{100.0, 200.0};
  const std::vector<double> sz{0.0, 0.0};  // no sizes supplied -> unit weight
  const BucketResult result = CalculateTimeBuckets(ts, px, sz, 2.0);
  ASSERT_EQ(result.bucket_count, 1);
  EXPECT_DOUBLE_EQ(result.vwap_curve[0], 150.0);
}

TEST(CalculateTimeBuckets, VolumeWeightingIsCorrect) {
  const std::vector<double> ts{1.0, 1.5};
  const std::vector<double> px{100.0, 200.0};
  const std::vector<double> sz{3.0, 1.0};
  const BucketResult result = CalculateTimeBuckets(ts, px, sz, 2.0);
  ASSERT_EQ(result.bucket_count, 1);
  // (100*3 + 200*1) / 4 = 125
  EXPECT_DOUBLE_EQ(result.vwap_curve[0], 125.0);
}

TEST(CalculateTimeBuckets, MultipleBucketsSplitOnWindowBoundary) {
  const std::vector<double> ts{0.0, 1.9, 2.1, 3.9, 4.0};
  const std::vector<double> px{100.0, 100.0, 200.0, 200.0, 300.0};
  const std::vector<double> sz{1.0, 1.0, 1.0, 1.0, 1.0};
  const BucketResult result = CalculateTimeBuckets(ts, px, sz, 2.0);
  ASSERT_EQ(result.bucket_count, 3);
  EXPECT_DOUBLE_EQ(result.vwap_curve[0], 100.0);
  EXPECT_DOUBLE_EQ(result.vwap_curve[1], 200.0);
  EXPECT_DOUBLE_EQ(result.vwap_curve[2], 300.0);
  EXPECT_EQ(result.trade_counts[0], 2);
  EXPECT_EQ(result.trade_counts[1], 2);
  EXPECT_EQ(result.trade_counts[2], 1);
}

TEST(CalculateTimeBuckets, MismatchedLengthsThrow) {
  const std::vector<double> ts{1.0, 2.0};
  const std::vector<double> px{1.0};
  const std::vector<double> sz{1.0, 1.0};
  // CalculateTimeBuckets is [[nodiscard]]; under -Werror=unused-result
  // (GCC 14+), EXPECT_THROW's expanded statement discarding that return
  // value fails the build unless explicitly voided.
  EXPECT_THROW((void)CalculateTimeBuckets(ts, px, sz, 1.0),
               std::invalid_argument);
}

TEST(CalculateTimeBuckets, NonPositiveWindowThrows) {
  const std::vector<double> ts{1.0};
  const std::vector<double> px{1.0};
  const std::vector<double> sz{1.0};
  EXPECT_THROW((void)CalculateTimeBuckets(ts, px, sz, 0.0),
               std::invalid_argument);
  EXPECT_THROW((void)CalculateTimeBuckets(ts, px, sz, -1.0),
               std::invalid_argument);
}

TEST(CalculateTimeBuckets, UnsortedTimestampsThrow) {
  const std::vector<double> ts{2.0, 1.0};
  const std::vector<double> px{1.0, 1.0};
  const std::vector<double> sz{1.0, 1.0};
  EXPECT_THROW((void)CalculateTimeBuckets(ts, px, sz, 1.0),
               std::invalid_argument);
}

}  // namespace
}  // namespace core_engine
