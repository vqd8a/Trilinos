// @HEADER
// *****************************************************************************
//          Tpetra: Templated Linear Algebra Services Package
//
// Copyright 2008 NTESS and the Tpetra contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef TPETRA_DETAILS_EQUILIBRATIONINFO_HPP
#define TPETRA_DETAILS_EQUILIBRATIONINFO_HPP

/// \file Tpetra_Details_EquilibrationInfo.hpp
/// \brief Declaration of Tpetra::Details::EquilibrationInfo

#include "TpetraCore_config.h"
#include "Kokkos_ArithTraits.hpp"
#include "Kokkos_Core.hpp"

namespace Tpetra {
namespace Details {

/// \brief Struct storing results of Tpetra::computeRowAndColumnOneNorms.
///
/// \tparam ScalarType Type of the entries in the Tpetra::RowMatrix /
///   Tpetra::CrsMatrix.  This must be the Kokkos-ized version, that
///   is, <tt>Kokkos::ArithTraits<ScalarType>::val_type</tt>.
///
/// \tparam DeviceType Kokkos::Device specialization.
///
/// Tpetra users may NOT do anything with this struct other than get
/// it as the return value of Tpetra::computeRowAndColumnOneNorms, and
/// pass it into Tpetra::leftAndOrRightScaleCrsMatrix.
///
/// Tpetra developers may use the fields in this struct, to implement
/// equilibration, balancing (in the symmetric / Hermitian positive
/// definite case only), or to analyze the matrix (e.g., whether it is
/// diagonally dominant).
///
/// If this is the return value of computeRowAndColumnOneNorms,
/// results are always global.  computeLocalRowAndColumnOneNorms only
/// computes local results, where "local" means "to an MPI process."
/// globalizeRowOneNorms and globalizeColumnOneNorms "globalize" the
/// results, so they refer to the entire matrix, globally distributed
/// over an MPI communicator.
template<class ScalarType, class DeviceType>
struct EquilibrationInfo {
  using val_type = typename Kokkos::ArithTraits<ScalarType>::val_type;
  using mag_type = typename Kokkos::ArithTraits<val_type>::mag_type;
  using device_type = typename DeviceType::device_type;
  using host_device_type = typename Kokkos::View<mag_type*, device_type>::HostMirror::device_type;
  using HostMirror = EquilibrationInfo<val_type, host_device_type>;

  EquilibrationInfo () :
    foundInf (false),
    foundNan (false),
    foundZeroDiag (false),
    foundZeroRowNorm (false)
  {}

  EquilibrationInfo (const std::size_t lclNumRows,
                     const std::size_t lclNumCols,
                     const bool assumeSymmetric_) :
    rowNorms (Kokkos::View<mag_type*, device_type> ("rowNorms", lclNumRows)),
    rowDiagonalEntries (Kokkos::View<val_type*, device_type> ("rowDiagonalEntries", lclNumRows)),
    colNorms (Kokkos::View<mag_type*, device_type> ("colNorms", lclNumCols)),
    colDiagonalEntries (Kokkos::View<val_type*, device_type> ("colDiagonalEntries",
                                                              assumeSymmetric_ ?
                                                              std::size_t (0) :
                                                              lclNumCols)),
    rowScaledColNorms (Kokkos::View<mag_type*, device_type> ("rowScaledColNorms",
                                                             assumeSymmetric_ ?
                                                             std::size_t (0) :
                                                             lclNumCols)),
    assumeSymmetric (assumeSymmetric_),
    foundInf (false),
    foundNan (false),
    foundZeroDiag (false),
    foundZeroRowNorm (false)
  {}

  EquilibrationInfo (const Kokkos::View<mag_type*, device_type>& rowNorms_,
                     const Kokkos::View<val_type*, device_type>& rowDiagonalEntries_,
                     const Kokkos::View<mag_type*, device_type>& colNorms_,
                     const Kokkos::View<val_type*, device_type>& colDiagonalEntries_,
                     const Kokkos::View<mag_type*, device_type>& rowScaledColNorms_,
                     const bool assumeSymmetric_,
                     const bool foundInf_,
                     const bool foundNan_,
                     const bool foundZeroDiag_,
                     const bool foundZeroRowNorm_) :
    rowNorms (rowNorms_),
    rowDiagonalEntries (rowDiagonalEntries_),
    colNorms (colNorms_),
    colDiagonalEntries (colDiagonalEntries_),
    rowScaledColNorms (rowScaledColNorms_),
    assumeSymmetric (assumeSymmetric_),
    foundInf (foundInf_),
    foundNan (foundNan_),
    foundZeroDiag (foundZeroDiag_),
    foundZeroRowNorm (foundZeroRowNorm_)
  {}

  //! Deep-copy src into *this.
  template<class SrcDeviceType>
  void
  assign (const EquilibrationInfo<ScalarType, SrcDeviceType>& src)
  {
    using execution_space = typename device_type::execution_space;
    // DEEP_COPY REVIEW - DEVICE-TO-DEVICE
    Kokkos::deep_copy (execution_space(), rowNorms, src.rowNorms);
    // DEEP_COPY REVIEW - DEVICE-TO-DEVICE
    Kokkos::deep_copy (execution_space(), rowDiagonalEntries, src.rowDiagonalEntries);
    // DEEP_COPY REVIEW - DEVICE-TO-DEVICE
    Kokkos::deep_copy (execution_space(), colNorms, src.colNorms);
    if (src.colDiagonalEntries.extent (0) == 0) {
      colDiagonalEntries =
        Kokkos::View<val_type*, device_type> ("colDiagonalEntries", 0);
    }
    else {
      // DEEP_COPY REVIEW - DEVICE-TO-DEVICE
      Kokkos::deep_copy (execution_space(), colDiagonalEntries, src.colDiagonalEntries);
    }
    if (src.rowScaledColNorms.extent (0) == 0) {
      rowScaledColNorms =
        Kokkos::View<mag_type*, device_type> ("rowScaledColNorms", 0);
    }
    else {
      // DEEP_COPY REVIEW - DEVICE-TO-DEVICE
      Kokkos::deep_copy (execution_space(), rowScaledColNorms, src.rowScaledColNorms);
    }

    assumeSymmetric = src.assumeSymmetric;
    foundInf = src.foundInf;
    foundNan = src.foundNan;
    foundZeroDiag = src.foundZeroDiag;
    foundZeroRowNorm = src.foundZeroRowNorm;
  }

  typename EquilibrationInfo<val_type, device_type>::HostMirror
  createMirrorView ()
  {
    auto rowNorms_h = Kokkos::create_mirror_view (rowNorms);
    auto rowDiagonalEntries_h = Kokkos::create_mirror_view (rowDiagonalEntries);
    auto colNorms_h = Kokkos::create_mirror_view (colNorms);
    auto colDiagonalEntries_h = Kokkos::create_mirror_view (colDiagonalEntries);
    auto rowScaledColNorms_h = Kokkos::create_mirror_view (rowScaledColNorms);

    return HostMirror {rowNorms_h, rowDiagonalEntries_h, colNorms_h,
        colDiagonalEntries_h, rowScaledColNorms_h, assumeSymmetric,
        foundInf, foundNan, foundZeroDiag, foundZeroRowNorm};
  }

  // We call a row a "diagonally dominant row" if the absolute value
  // of the diagonal entry is >= the sum of the absolute values of the
  // off-diagonal entries.  The row norm is the sum of those two
  // things, so this means diagAbsVal >= rowNorm - diagAbsVal.  Ditto
  // for a column.

  //! One-norms of the matrix's rows, distributed via the row Map.
  Kokkos::View<mag_type*, device_type> rowNorms;

  //! Diagonal entries of the matrix, distributed via the row Map.
  Kokkos::View<val_type*, device_type> rowDiagonalEntries;

  /// \brief One-norms of the matrix's columns, distributed via the
  ///   column Map.
  ///
  /// If assumeSymmetric is true, this is just a redistributed version
  /// of rowNorms.
  Kokkos::View<mag_type*, device_type> colNorms;

  /// \brief Diagonal entries of the matrix, distributed via the column Map.
  ///
  /// Only use this if assumeSymmetric is false.
  Kokkos::View<val_type*, device_type> colDiagonalEntries;

  /// \brief One-norms of the matrix's columns, after the matrix's
  ///   rows have been scaled by rowNorms.
  ///
  /// This is only valid if assumeSymmetric is false.
  ///
  /// For the nonsymmetric case, we imitate LAPACK's DGEEQU, in doing
  /// the row scaling first, then the column scaling.  Thus, the
  /// column norms are "scaled" by the row norms (above).  We still
  /// keep the unscaled column norms (colNorms; see above) in that
  /// case, because they are diagnostic.
  Kokkos::View<mag_type*, device_type> rowScaledColNorms;

  /// \brief Whether to assume that the matrix is (globally) symmetric.
  ///
  /// This affects whether colDiagonalEntries and rowScaledColNorms
  /// are valid.
  bool assumeSymmetric;

  //! Found an Inf somewhere in the matrix.
  bool foundInf;

  //! Found a NaN somewhere in the matrix.
  bool foundNan;

  //! Found a zero diagonal entry somewhere in the matrix.
  bool foundZeroDiag;

  //! At least one row of the matrix has a zero norm.
  bool foundZeroRowNorm;
};

} // namespace Details
} // namespace Tpetra

#endif // TPETRA_DETAILS_EQUILIBRATIONINFO_HPP
