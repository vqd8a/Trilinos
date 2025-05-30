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

#ifndef KOKKOS_TEST_SIMD_WHERE_EXPRESSIONS_HPP
#define KOKKOS_TEST_SIMD_WHERE_EXPRESSIONS_HPP

#include <Kokkos_SIMD.hpp>
#include <SIMDTesting_Utilities.hpp>

template <typename Abi, typename DataType>
inline void host_check_where_expr_scatter_to() {
  if constexpr (is_type_v<Kokkos::Experimental::basic_simd<DataType, Abi>>) {
    using simd_type  = Kokkos::Experimental::basic_simd<DataType, Abi>;
    using index_type = Kokkos::Experimental::basic_simd<std::int32_t, Abi>;
    using mask_type  = typename simd_type::mask_type;

    constexpr std::size_t nlanes = simd_type::size();
    DataType init[]              = {11, 13, 17, 19, 23, 29, 31, 37,
                                    53, 71, 79, 83, 89, 93, 97, 103};
    simd_type src;
    src.copy_from(init, Kokkos::Experimental::simd_flag_default);

    for (std::size_t idx = 0; idx < nlanes; ++idx) {
      if constexpr (std::is_same_v<Abi,
                                   Kokkos::Experimental::simd_abi::scalar>) {
        mask_type mask(KOKKOS_LAMBDA(std::size_t i) { return i != idx; });

        DataType dst[nlanes] = {0};
        for (std::size_t i = 0; i < nlanes; ++i) {
          dst[i] = (2 + (i * 2));
        }
        index_type index(KOKKOS_LAMBDA(std::size_t i) { return i; });
        simd_type expected_result(KOKKOS_LAMBDA(std::size_t i) {
          return (mask[i]) ? src[index[i]] : dst[i];
        });
        where(mask, src).scatter_to(dst, index);

        simd_type dst_simd;
        dst_simd.copy_from(dst, Kokkos::Experimental::simd_flag_default);

        host_check_equality(expected_result, dst_simd, nlanes);
      } else {
        mask_type mask([=](std::size_t i) { return i != idx; });

        DataType dst[nlanes] = {0};
        for (std::size_t i = 0; i < nlanes; ++i) {
          dst[i] = (2 + (i * 2));
        }
        index_type index([=](std::size_t i) { return i; });
        simd_type expected_result(
            [=](std::size_t i) { return (mask[i]) ? src[index[i]] : dst[i]; });
        where(mask, src).scatter_to(dst, index);

        simd_type dst_simd;
        dst_simd.copy_from(dst, Kokkos::Experimental::simd_flag_default);

        host_check_equality(expected_result, dst_simd, nlanes);
      }
    }
  }
}

template <typename Abi, typename DataType>
inline void host_check_where_expr_gather_from() {
  if constexpr (is_type_v<Kokkos::Experimental::basic_simd<DataType, Abi>>) {
    using simd_type  = Kokkos::Experimental::basic_simd<DataType, Abi>;
    using index_type = Kokkos::Experimental::basic_simd<std::int32_t, Abi>;
    using mask_type  = typename simd_type::mask_type;

    std::size_t nlanes = simd_type::size();
    DataType src[]     = {11, 13, 17, 19, 23, 29, 31, 37,
                          53, 71, 79, 83, 89, 93, 97, 103};

    for (std::size_t idx = 0; idx < nlanes; ++idx) {
      if constexpr (std::is_same_v<Abi,
                                   Kokkos::Experimental::simd_abi::scalar>) {
        mask_type mask(KOKKOS_LAMBDA(std::size_t i) { return i != idx; });

        simd_type dst(KOKKOS_LAMBDA(std::size_t i) { return (2 + (i * 2)); });
        index_type index(KOKKOS_LAMBDA(std::size_t i) { return i; });
        simd_type expected_result(KOKKOS_LAMBDA(std::size_t i) {
          return (mask[i]) ? src[index[i]] : dst[i];
        });
        where(mask, dst).gather_from(src, index);

        host_check_equality(expected_result, dst, nlanes);
      } else {
        mask_type mask([=](std::size_t i) { return i != idx; });

        simd_type dst([=](std::size_t i) { return (2 + (i * 2)); });
        index_type index([=](std::size_t i) { return i; });
        simd_type expected_result(
            [=](std::size_t i) { return (mask[i]) ? src[index[i]] : dst[i]; });
        where(mask, dst).gather_from(src, index);

        host_check_equality(expected_result, dst, nlanes);
      }
    }
  }
}

template <class Abi, typename DataType>
inline void host_check_where_expr() {
  host_check_where_expr_scatter_to<Abi, DataType>();
  host_check_where_expr_gather_from<Abi, DataType>();
}

template <typename Abi, typename... DataTypes>
inline void host_check_where_expr_all_types(
    Kokkos::Experimental::Impl::data_types<DataTypes...>) {
  (host_check_where_expr<Abi, DataTypes>(), ...);
}

template <typename... Abis>
inline void host_check_where_expr_all_abis(
    Kokkos::Experimental::Impl::abi_set<Abis...>) {
  using DataTypes = Kokkos::Experimental::Impl::data_type_set;
  (host_check_where_expr_all_types<Abis>(DataTypes()), ...);
}

template <typename Abi, typename DataType>
KOKKOS_INLINE_FUNCTION void device_check_where_expr_scatter_to() {
  if constexpr (is_type_v<Kokkos::Experimental::basic_simd<DataType, Abi>>) {
    using simd_type  = Kokkos::Experimental::basic_simd<DataType, Abi>;
    using index_type = Kokkos::Experimental::basic_simd<std::int32_t, Abi>;
    using mask_type  = typename simd_type::mask_type;

    constexpr std::size_t nlanes = simd_type::size();
    DataType init[]              = {11, 13, 17, 19, 23, 29, 31, 37,
                                    53, 71, 79, 83, 89, 93, 97, 103};
    simd_type src;
    src.copy_from(init, Kokkos::Experimental::simd_flag_default);

    for (std::size_t idx = 0; idx < nlanes; ++idx) {
      mask_type mask(KOKKOS_LAMBDA(std::size_t i) { return i != idx; });

      DataType dst[nlanes] = {0};
      for (std::size_t i = 0; i < nlanes; ++i) {
        dst[i] = (2 + (i * 2));
      }
      index_type index([=](std::size_t i) { return i; });
      simd_type expected_result(KOKKOS_LAMBDA(std::size_t i) {
        return (mask[i]) ? src[index[i]] : dst[i];
      });
      where(mask, src).scatter_to(dst, index);

      simd_type dst_simd;
      dst_simd.copy_from(dst, Kokkos::Experimental::simd_flag_default);

      device_check_equality(expected_result, dst_simd, nlanes);
    }
  }
}

template <typename Abi, typename DataType>
KOKKOS_INLINE_FUNCTION void device_check_where_expr_gather_from() {
  using simd_type  = Kokkos::Experimental::basic_simd<DataType, Abi>;
  using index_type = Kokkos::Experimental::basic_simd<std::int32_t, Abi>;
  using mask_type  = typename simd_type::mask_type;

  std::size_t nlanes = simd_type::size();
  DataType src[]     = {11, 13, 17, 19, 23, 29, 31, 37,
                        53, 71, 79, 83, 89, 93, 97, 103};

  for (std::size_t idx = 0; idx < nlanes; ++idx) {
    mask_type mask(KOKKOS_LAMBDA(std::size_t i) { return i != idx; });

    simd_type dst([=](std::size_t i) { return (2 + (i * 2)); });
    index_type index([=](std::size_t i) { return i; });
    simd_type expected_result(KOKKOS_LAMBDA(std::size_t i) {
      return (mask[i]) ? src[index[i]] : dst[i];
    });
    where(mask, dst).gather_from(src, index);

    device_check_equality(expected_result, dst, nlanes);
  }
}

template <class Abi, typename DataType>
KOKKOS_INLINE_FUNCTION void device_check_where_expr() {
  device_check_where_expr_scatter_to<Abi, DataType>();
  device_check_where_expr_gather_from<Abi, DataType>();
}

template <typename Abi, typename... DataTypes>
KOKKOS_INLINE_FUNCTION void device_check_where_expr_all_types(
    Kokkos::Experimental::Impl::data_types<DataTypes...>) {
  (device_check_where_expr<Abi, DataTypes>(), ...);
}

template <typename... Abis>
KOKKOS_INLINE_FUNCTION void device_check_where_expr_all_abis(
    Kokkos::Experimental::Impl::abi_set<Abis...>) {
  using DataTypes = Kokkos::Experimental::Impl::data_type_set;
  (device_check_where_expr_all_types<Abis>(DataTypes()), ...);
}

class simd_device_where_expr_functor {
 public:
  KOKKOS_INLINE_FUNCTION void operator()(int) const {
    device_check_where_expr_all_abis(
        Kokkos::Experimental::Impl::device_abi_set());
  }
};

TEST(simd, host_where_expressions) {
  host_check_where_expr_all_abis(Kokkos::Experimental::Impl::host_abi_set());
}

TEST(simd, device_where_expressions) {
  Kokkos::parallel_for(1, simd_device_where_expr_functor());
}

#endif
