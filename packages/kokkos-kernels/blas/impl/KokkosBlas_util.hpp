//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//@HEADER

#ifndef KOKKOSBLAS_UTIL_HPP
#define KOKKOSBLAS_UTIL_HPP

#include "Kokkos_ArithTraits.hpp"

namespace KokkosBlas {
namespace Impl {
struct OpID {
  template <typename ValueType>
  KOKKOS_INLINE_FUNCTION ValueType operator()(ValueType v) const {
    return v;
  }
};

struct OpConj {
  template <typename ValueType>
  KOKKOS_INLINE_FUNCTION ValueType operator()(ValueType v) const {
    using KAT = Kokkos::ArithTraits<ValueType>;
    return KAT::conj(v);
  }
};

struct OpReal {
  template <typename ValueType>
  KOKKOS_INLINE_FUNCTION typename Kokkos::ArithTraits<ValueType>::mag_type operator()(ValueType v) const {
    using KAT = Kokkos::ArithTraits<ValueType>;
    return KAT::real(v);
  }
};
}  // namespace Impl

//////// Tags for BLAS ////////

struct Mode {
  struct Serial {
    static const char *name() { return "Serial"; }
  };
  struct Team {
    static const char *name() { return "Team"; }
  };
  struct TeamVector {
    static const char *name() { return "TeamVector"; }
  };
};

template <class T>
struct is_mode : std::false_type {};

template <>
struct is_mode<Mode::Serial> : std::true_type {};

template <>
struct is_mode<Mode::Team> : std::true_type {};

template <>
struct is_mode<Mode::TeamVector> : std::true_type {};

template <class T>
static constexpr bool is_mode_v = is_mode<T>::value;

struct Trans {
  struct Transpose {};
  struct NoTranspose {};
  struct ConjTranspose {};
};

template <class T>
struct is_trans : std::false_type {};

template <>
struct is_trans<Trans::Transpose> : std::true_type {};

template <>
struct is_trans<Trans::NoTranspose> : std::true_type {};

template <>
struct is_trans<Trans::ConjTranspose> : std::true_type {};

template <class T>
static constexpr bool is_trans_v = is_trans<T>::value;

struct Algo {
  struct Level3 {
    struct Unblocked {
      static const char *name() { return "Unblocked"; }
    };
    struct Blocked {
      static const char *name() { return "Blocked"; }
      // TODO:: for now harwire the blocksizes; this should reflect
      // register blocking (not about team parallelism).
      // this mb should vary according to
      // - team policy (smaller) or range policy (bigger)
      // - space (gpu vs host)
      // - blocksize input (blk <= 4 mb = 2, otherwise mb = 4), etc.
      static constexpr KOKKOS_FUNCTION int mb() {
        KOKKOS_IF_ON_HOST((return 4;))
        KOKKOS_IF_ON_DEVICE((return 2;))
      }
    };
    struct MKL {
      static const char *name() { return "MKL"; }
    };
    struct CompactMKL {
      static const char *name() { return "CompactMKL"; }
    };

    // When this is first developed, unblocked algorithm is a naive
    // implementation and blocked algorithm uses register blocking variant of
    // algorithm (manual unrolling). This distinction is almost meaningless and
    // it just adds more complications. Eventually, the blocked version will be
    // removed and we only use the default algorithm. For testing and
    // development purpose, we still leave algorithm tag in the template
    // arguments.
    using Default = Unblocked;
  };

  using Gemm      = Level3;
  using Trsm      = Level3;
  using Trmm      = Level3;
  using Trtri     = Level3;
  using LU        = Level3;
  using InverseLU = Level3;
  using SolveLU   = Level3;
  using QR        = Level3;
  using UTV       = Level3;
  using Pttrf     = Level3;
  using Pttrs     = Level3;
  using Getrf     = Level3;
  using Getrs     = Level3;
  using Gbtrf     = Level3;

  struct Level2 {
    struct Unblocked {};
    struct Blocked {
      // TODO:: for now hardwire the blocksizes; this should reflect
      // register blocking (not about team parallelism).
      // this mb should vary according to
      // - team policy (smaller) or range policy (bigger)
      // - space (cuda vs host)
      // - blocksize input (blk <= 4 mb = 2, otherwise mb = 4), etc.
      static constexpr KOKKOS_FUNCTION int mb() {
        KOKKOS_IF_ON_HOST((return 4;))
        KOKKOS_IF_ON_DEVICE((return 1;))
      }
    };
    struct MKL {};
    struct CompactMKL {};

    // When this is first developed, unblocked algorithm is a naive
    // implementation and blocked algorithm uses register blocking variant of
    // algorithm (manual unrolling). This distinction is almost meaningless and
    // it just adds more complications. Eventually, the blocked version will be
    // removed and we only use the default algorithm. For testing and
    // development purpose, we still leave algorithm tag in the template
    // arguments.
    using Default = Unblocked;
  };

  using Gemv   = Level2;
  using Trsv   = Level2;
  using ApplyQ = Level2;
  using Tbsv   = Level2;
  using Pbtrf  = Level2;
  using Pbtrs  = Level2;
};

template <class T>
struct is_level2 : std::false_type {};

template <>
struct is_level2<Algo::Level2::Unblocked> : std::true_type {};

template <>
struct is_level2<Algo::Level2::Blocked> : std::true_type {};

template <>
struct is_level2<Algo::Level2::MKL> : std::true_type {};

template <>
struct is_level2<Algo::Level2::CompactMKL> : std::true_type {};

template <class T>
static constexpr bool is_level2_v = is_level2<T>::value;

namespace Impl {

// Helper to choose the work distribution for a TeamPolicy computing multiple
// reductions. Each team computes a partial reduction and atomically contributes
// to the final result.
//
// This was originally written for dot-based GEMM, but can also be applied to
// multivector dots/norms.

// Input params:
//  * length: size of each vector to reduce
//  * numReductions: number of reductions to compute
// Output params:
//  * teamsPerReduction: number of teams to use for each reduction
template <typename ExecSpace, typename size_type>
void multipleReductionWorkDistribution(size_type length, size_type numReductions, size_type &teamsPerDot) {
  constexpr size_type workPerTeam = 4096;                                    // Amount of work per team
  size_type appxNumTeams          = (length * numReductions) / workPerTeam;  // Estimation for appxNumTeams

  // Adjust appxNumTeams in case it is too small or too large
  if (appxNumTeams < 1) appxNumTeams = 1;
  if (appxNumTeams > 1024) appxNumTeams = 1024;

  // If there are more reductions than the number of teams,
  // then set the number of teams to be number of reductions.
  // We don't want a team to contribute to more than one reduction.
  if (numReductions >= appxNumTeams) {
    teamsPerDot = 1;
  }
  // If there are more teams than reductions, each reduction can
  // potentially be performed by multiple teams. First, compute
  // teamsPerDot as an integer (take the floor, not ceiling), then,
  // compute actual number of teams by using this factor.
  else {
    teamsPerDot = appxNumTeams / numReductions;
  }
}

// Functor to apply sqrt() to each element of a 1D view.

template <class RV>
struct TakeSqrtFunctor {
  TakeSqrtFunctor(const RV &r_) : r(r_) {}

  KOKKOS_INLINE_FUNCTION void operator()(int i) const {
    r(i) = Kokkos::ArithTraits<typename RV::non_const_value_type>::sqrt(r(i));
  }

  RV r;
};

}  // namespace Impl
}  // namespace KokkosBlas

#endif  // KOKKOSBLAS_UTIL_HPP
