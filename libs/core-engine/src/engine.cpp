// Copyright 2026 PlatformOpsHub Authors.
//
// nanobind bindings exposing the zero-copy C++26 TCA bucketing engine to
// Python. All numeric buffers are ingested as contiguous C arrays directly
// from NumPy without an intermediate copy.
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/vector.h>

#include <span>
#include <stdexcept>

#include "core_engine/bucketing.h"

namespace nb = nanobind;

namespace {

core_engine::BucketResult CalculateTimeBucketsPy(
    nb::ndarray<const double, nb::c_contig, nb::device::cpu> timestamps,
    nb::ndarray<const double, nb::c_contig, nb::device::cpu> prices,
    nb::ndarray<const double, nb::c_contig, nb::device::cpu> sizes,
    double window_size_sec) {
  // Release the GIL for the duration of the pure C++ computation so the
  // Python interpreter can service other threads concurrently.
  nb::gil_scoped_release release;

  const std::span<const double> ts_span(timestamps.data(), timestamps.size());
  const std::span<const double> px_span(prices.data(), prices.size());
  const std::span<const double> sz_span(sizes.data(), sizes.size());

  return core_engine::CalculateTimeBuckets(ts_span, px_span, sz_span,
                                            window_size_sec);
}

}  // namespace

NB_MODULE(core_engine, m) {
  m.doc() = "C++26 zero-copy VWAP time-bucketing engine (nanobind)";

  nb::class_<core_engine::BucketResult>(m, "BucketResult")
      .def_ro("bucket_count", &core_engine::BucketResult::bucket_count)
      .def_ro("vwap_curve", &core_engine::BucketResult::vwap_curve)
      .def_ro("trade_counts", &core_engine::BucketResult::trade_counts);

  m.def("calculate_time_buckets", &CalculateTimeBucketsPy,
        "Compute VWAP time buckets from raw trade arrays "
        "(timestamps, prices, sizes, window_size_sec).",
        nb::arg("timestamps").noconvert(), nb::arg("prices").noconvert(),
        nb::arg("sizes").noconvert(), nb::arg("window_size_sec"));
}
