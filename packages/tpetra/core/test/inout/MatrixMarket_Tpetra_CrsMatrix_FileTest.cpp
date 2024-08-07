// @HEADER
// *****************************************************************************
//          Tpetra: Templated Linear Algebra Services Package
//
// Copyright 2008 NTESS and the Tpetra contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include <MatrixMarket_Tpetra.hpp>
#include <Tpetra_CrsMatrix.hpp>
#include <Tpetra_Core.hpp>
#include <Tpetra_Util.hpp> // sort2
#include <Tpetra_TestingUtilities.hpp> 
#include <Teuchos_UnitTestHarness.hpp>

using Tpetra::global_size_t;
using Tpetra::TestingUtilities::arcp_from_view;
using Teuchos::Array;
using Teuchos::ArrayRCP;
using Teuchos::ArrayView;
using Teuchos::as;
using Teuchos::Comm;
using Teuchos::null;
using Teuchos::OSTab;
using Teuchos::ParameterList;
using Teuchos::ptr;
using Teuchos::RCP;
using Teuchos::rcp;
using Teuchos::rcp_dynamic_cast;
using Teuchos::rcpFromRef;
using Teuchos::REDUCE_MAX;
using Teuchos::REDUCE_MIN;
using Teuchos::reduceAll;
using std::endl;


namespace {

// Command-line arguments
std::string mapOutputFilename;
std::string matrixOutputFilename;
std::string mapInputFilename;
std::string matrixInputFilename;
bool debug = false;

TEUCHOS_STATIC_SETUP()
{
  Teuchos::CommandLineProcessor& clp = Teuchos::UnitTestRepository::getCLP ();
  clp.setOption ("mapOutputFilename", &mapOutputFilename,
                 "Name of file to which to write temporary Map output.  "
                 "If not provided, we will not test Map output.");
  clp.setOption ("matrixOutputFilename", &matrixOutputFilename,
                 "Name of file to which to write temporary CrsMatrix output.  "
                 "If not provided, we write output to an std::ostringstream in "
                 "memory.  WARNING: If you don't provide an output file name, "
                 "the test may run out of memory if the input file is large, "
                 "since in that case we have to store two copies of the "
                 "matrix's data in memory at once.");
  clp.setOption ("mapInputFilename", &mapInputFilename,
                 "Name of file from which to read the initial Map");
  clp.setOption ("matrixInputFilename", &matrixInputFilename,
                 "Name of file from which to read the initial CrsMatrix "
                 "(using the input Map as its row Map)");
  clp.setOption ("debug", "release", &debug, "If true, print copious debugging "
                 "output to stderr on all processes.");
}


// Input matrices must be fill complete.
template<class CrsMatrixType>
bool
compareCrsMatrixMaps (const CrsMatrixType& A_orig, const CrsMatrixType& A, Teuchos::FancyOStream& out)
{
  Teuchos::OSTab tab (out);

  bool globalAllSame = true;
  if (! A_orig.getRowMap ()->isSameAs (* (A.getRowMap ()))) {
    out << "Row Maps are not the same" << endl;
    globalAllSame = false;
  }
  if (! A_orig.getColMap ()->isSameAs (* (A.getColMap ()))) {
    out << "Column Maps are not the same" << endl;
    globalAllSame = false;
  }
  if (! A_orig.getDomainMap ()->isSameAs (* (A.getDomainMap ()))) {
    out << "Domain Maps are not the same" << endl;
    globalAllSame = false;
  }
  if (! A_orig.getRangeMap ()->isSameAs (* (A.getRangeMap ()))) {
    out << "Range Maps are not the same" << endl;
    globalAllSame = false;
  }
  if (globalAllSame) {
    out << "All Maps are the same" << endl;
  }
  return globalAllSame;
}

// Input matrices must be fill complete, and all four of their Maps
// (row, column, domain, and range) must be the same.
template<class CrsMatrixType>
bool
compareCrsMatrix (const CrsMatrixType& A_orig, const CrsMatrixType& A, Teuchos::FancyOStream& out)
{
  typedef typename CrsMatrixType::global_ordinal_type GO;
  typedef typename ArrayView<const GO>::size_type size_type;
  typedef typename CrsMatrixType::nonconst_global_inds_host_view_type gids_type;
  typedef typename CrsMatrixType::nonconst_values_host_view_type vals_type;

  OSTab tab (out);
  int localEqual = 1;

  //
  // Are my local matrices equal?
  //
  gids_type indOrig, ind;
  vals_type valOrig, val;
  size_t numEntriesOrig = 0;
  size_t numEntries = 0;

  ArrayView<const GO> localElts = A.getRowMap ()->getLocalElementList ();
  const size_type numLocalElts = localElts.size ();
  for (size_type i = 0; i < numLocalElts; ++i) {
    const GO globalRow = localElts[i];
    numEntriesOrig = A_orig.getNumEntriesInGlobalRow (globalRow);
    numEntries = A.getNumEntriesInGlobalRow (globalRow);

    if (numEntriesOrig != numEntries) {
      localEqual = 0;
      break;
    }
    Kokkos::resize(indOrig,numEntriesOrig);
    Kokkos::resize(valOrig,numEntriesOrig);
    A_orig.getGlobalRowCopy (globalRow, indOrig, valOrig, numEntriesOrig);
    Kokkos::resize(ind,numEntries);
    Kokkos::resize(val,numEntries);
    A.getGlobalRowCopy (globalRow, ind, val, numEntries);

    // Global row entries are not necessarily sorted.  Sort them so
    // we can compare them.
    Tpetra::sort2 (indOrig, indOrig.extent(0), valOrig);
    Tpetra::sort2 (ind, ind.extent(0), val);

    for (size_t k = 0; k < numEntries; ++k) {
      // Values should be _exactly_ equal.
      if (indOrig[k] != ind[k] || valOrig[k] != val[k]) {
        localEqual = 0;
        break;
      }
    }
  }

  RCP<const Comm<int> > comm = A.getRowMap ()->getComm ();
  int globalEqual = 0;
  reduceAll<int, int> (*comm, REDUCE_MIN, 1, &localEqual, &globalEqual);
  return globalEqual == 1;
}

// Input matrices must be fill complete, and all four of their Maps
// (row, column, domain, and range) must be the same.
template<class CrsMatrixType>
bool
compareCrsMatrixValues (const CrsMatrixType& A_orig,
                        const CrsMatrixType& A,
                        Teuchos::FancyOStream& out)
{
  typedef typename CrsMatrixType::scalar_type ST;
  typedef typename CrsMatrixType::global_ordinal_type GO;
  typedef typename ArrayView<const GO>::size_type size_type;
  typedef Teuchos::ScalarTraits<ST> STS;
  typedef typename STS::magnitudeType MT;
  typedef Teuchos::ScalarTraits<MT> STM;
  typedef typename CrsMatrixType::nonconst_global_inds_host_view_type gids_type;
  typedef typename CrsMatrixType::nonconst_values_host_view_type vals_type;

  OSTab tab (out);
  //
  // Are my local matrices equal?
  //

  gids_type indOrig_v, ind_v;
  vals_type valOrig_v, val_v;
  size_t numEntriesOrig = 0;
  size_t numEntries = 0;

  ArrayView<const GO> localElts = A.getRowMap ()->getLocalElementList ();
  const size_type numLocalElts = localElts.size ();
  MT localDiff = STM::zero (); // \sum_{i,j} |A_orig(i,j) - A(i,j)| locally
  for (size_type i = 0; i < numLocalElts; ++i) {
    const GO globalRow = localElts[i];
    numEntriesOrig = A_orig.getNumEntriesInGlobalRow (globalRow);
    numEntries = A.getNumEntriesInGlobalRow (globalRow);

    Kokkos::resize(indOrig_v,numEntriesOrig);
    Kokkos::resize(valOrig_v,numEntriesOrig);
    A_orig.getGlobalRowCopy (globalRow, indOrig_v, valOrig_v, numEntriesOrig);
    Kokkos::resize(ind_v,numEntries);
    Kokkos::resize(val_v,numEntries);
    A.getGlobalRowCopy (globalRow, ind_v, val_v, numEntries);

    // Global row entries are not necessarily sorted.  Sort them
    // (and their values with them) so we can merge their values.
    Tpetra::sort2 (indOrig_v, indOrig_v.extent(0), valOrig_v);
    Tpetra::sort2 (ind_v, ind_v.extent(0), val_v);
    auto indOrig = arcp_from_view(indOrig_v);
    auto ind = arcp_from_view(ind_v);
    auto valOrig = arcp_from_view(valOrig_v);
    auto val = arcp_from_view(val_v);

    //
    // Merge repeated values in each set of indices and values.
    //
    typename ArrayRCP<GO>::iterator indOrigIter = indOrig.begin ();
    typename ArrayRCP<ST>::iterator valOrigIter = valOrig.begin ();
    typename ArrayRCP<GO>::iterator indOrigEnd = indOrig.end ();
    typename ArrayRCP<ST>::iterator valOrigEnd = valOrig.end ();
    Tpetra::merge2 (indOrigEnd, valOrigEnd, indOrigIter, indOrigEnd, valOrigIter, valOrigEnd);

    typename ArrayRCP<GO>::iterator indIter = ind.begin ();
    typename ArrayRCP<ST>::iterator valIter = val.begin ();
    typename ArrayRCP<GO>::iterator indEnd = ind.end ();
    typename ArrayRCP<ST>::iterator valEnd = val.end ();
    Tpetra::merge2 (indEnd, valEnd, indIter, indEnd, valIter, valEnd);

    //
    // Compare the merged sets of entries.
    //
    indOrigIter = indOrig.begin ();
    indIter = ind.begin ();
    valOrigIter = valOrig.begin ();
    valIter = val.begin ();
    while (indOrigIter != indOrigEnd && indIter != indEnd) {
      const GO j_orig = *indOrigIter;
      const GO j = *indIter;

      if (j_orig < j) { // entry is in A_orig, not in A
        localDiff += STS::magnitude (*valOrigIter);
        ++indOrigIter;
        ++valOrigIter;
      } else if (j_orig > j) { // entry is in A, not A_orig
        localDiff += STS::magnitude (*valIter);
        ++indIter;
        ++valIter;
      } else { // j_orig == j: entry is in both matrices
        localDiff += STS::magnitude (*valOrigIter - *valIter);
        ++indOrigIter;
        ++valOrigIter;
        ++indIter;
        ++valIter;
      }
    }
  }

  RCP<const Comm<int> > comm = A.getRowMap ()->getComm ();
  const int myRank = comm->getRank ();
  std::ostringstream os;
  os << "Values are ";
  if (localDiff == STM::zero ()) {
    os << "the same on process " << myRank << endl;
  } else {
    os << "NOT the same on process " << myRank
       << ": \\sum_{i,j} |A_orig(i,j) - A(i,j)| = " << localDiff << endl;
  }
  out << os.str ();

  int globalEqual = 0;
  int localEqual = (localDiff == STM::zero ()) ? 1 : 0;
  reduceAll<int, int> (*comm, REDUCE_MIN, 1, &localEqual, &globalEqual);
  return globalEqual == 1;
}


bool
testReadAndWriteFile (Teuchos::FancyOStream& out,
                      const std::string& mapOutFile,
                      const std::string& matrixOutFile,
                      const std::string& mapInFile,
                      const std::string& matrixInFile)
{
  using map_type = Tpetra::Map<>;
  using crs_matrix_type = Tpetra::CrsMatrix<>;
  using reader_type = Tpetra::MatrixMarket::Reader<crs_matrix_type>;
  using writer_type = Tpetra::MatrixMarket::Writer<crs_matrix_type>;
  const bool tolerant = false;

  bool result = true; // current Boolean result; reused below
  bool success = true; // used by TEST_EQUALITY

  out << "Test: Matrix Market, CrsMatrix from row Map" << endl;
  OSTab tab1 (out);

  RCP<const Comm<int> > comm = Tpetra::getDefaultComm ();
  const int myRank = comm->getRank ();

  out << "Reading the (row) Map" << endl;
  RCP<const map_type> rowMap =
    reader_type::readMapFile (mapInFile, comm, tolerant, debug);

  if (mapOutFile != "") {
    out << "Writing the Map" << endl;
    writer_type::writeMapFile (mapOutFile, *rowMap);

    out << "Reading the written Map back in" << endl;
    RCP<const map_type> rowMap_out =
      reader_type::readMapFile (mapOutFile, comm, tolerant, debug);

    out << "Comparing the two Maps" << endl;
    result = rowMap->isSameAs (*rowMap_out);
    TEST_EQUALITY( result, true );
  }

  out << "Reading the CrsMatrix" << endl;
  const bool callFillComplete = true;
  RCP<const map_type> colMap;
  RCP<const map_type> domainMap = rowMap;
  RCP<const map_type> rangeMap = rowMap;
  RCP<crs_matrix_type> A_in =
    reader_type::readSparseFile (matrixInFile,
                                 rowMap, colMap, domainMap, rangeMap,
                                 callFillComplete, tolerant, debug);
  RCP<std::ostream> matrixOutStream;
  if (matrixOutFile == "") {
    if (myRank == 0) {
      out << "Process 0: No CrsMatrix output file provided; storing output in "
        "memory." << endl << "WARNING: This may run out of memory if the input "
        "file is large, since we have to store two copies of the matrix's data "
        "in memory at once." << endl;
      matrixOutStream = rcp (new std::ostringstream);
    }
  } else {
    if (myRank == 0) {
      out << "Opening CrsMatrix output file \"" << matrixOutFile
          << "\" on Process 0 only" << endl;
      matrixOutStream = rcp (new std::ofstream (matrixOutFile.c_str ()));
    }
  }
  out << "Executing barrier to ensure no deadlock" << endl;
  comm->barrier ();
  out << "Writing the CrsMatrix" << endl;
  writer_type::writeSparse (*matrixOutStream, A_in, debug);

  out << "Reading the written CrsMatrix back in" << endl;
  RCP<std::istream> matrixInStream;
  if (matrixOutFile == "") {
    if (myRank == 0) {
      // We have to store two copies of the output in memory, so that
      // we can construct the input stream from the output stream.
      RCP<std::ostringstream> matOss =
        rcp_dynamic_cast<std::ostringstream> (matrixOutStream, true);
      matrixInStream = rcp (new std::istringstream (matOss->str ()));
      matrixOutStream = null;
    }
  } else {
    if (myRank == 0) {
      out << "Process 0: Closing CrsMatrix output file" << endl;
      matrixOutStream = null;
      out << "Process 0: Reopening CrsMatrix output file \""
          << matrixOutFile << "\"" << endl;
      matrixInStream = rcp (new std::ifstream (matrixOutFile.c_str ()));
    }
  }

  RCP<crs_matrix_type> A_out =
    reader_type::readSparse (*matrixInStream,
                             rowMap, colMap, domainMap, rangeMap,
                             callFillComplete, tolerant, debug);

  out << "Test the two matrices for exact equality of structure and values" << endl;
  result = compareCrsMatrix<crs_matrix_type> (*A_in, *A_out, out);
  {
    OSTab tab (out);
    out << "- Matrices are " << (result ? "" : "NOT ") << "equal" << endl;
  }
  TEST_EQUALITY( result, true );

  out << "Test the two matrices for exact equality of values, independent of local structure" << endl;
  result = compareCrsMatrixValues<crs_matrix_type> (*A_in, *A_out, out);
  TEST_EQUALITY( result, true );

  // mfh 24 Feb 2014: Apparently, "set but not used" errors may still
  // show up on some compilers, even with use of the "(void) success"
  // idiom.  We can fix this by noting that success has to be true in
  // order for this function to return true, so we can just Boolean
  // AND the return value with success.
  //
  //(void) success; // silence compile warning ("set but not used")
  //return result;
  return result && success;
}

} // namespace (anonymous)


TEUCHOS_UNIT_TEST( MatrixMarket, CrsMatrixFileTest )
{
  success = testReadAndWriteFile (out, mapOutputFilename, matrixOutputFilename,
                                  mapInputFilename, matrixInputFilename);
}



