

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(UNIT_TEST_DRIVER ${PANZER_UNIT_TEST_MAIN})

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tFieldPattern
  SOURCES tFieldPattern.cpp ${UNIT_TEST_DRIVER}
  COMM serial mpi
  )
  
TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tGeometricAggFieldPattern
  SOURCES tGeometricAggFieldPattern.cpp ${UNIT_TEST_DRIVER}
  COMM serial mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tIntrepidFieldPattern
  SOURCES tIntrepidFieldPattern.cpp ${UNIT_TEST_DRIVER}
  COMM serial mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tNodalFieldPattern
  SOURCES tNodalFieldPattern.cpp ${UNIT_TEST_DRIVER}
  COMM serial mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tFieldAggPattern
  SOURCES tFieldAggPattern.cpp ${UNIT_TEST_DRIVER}
  COMM serial mpi
  )

#TRIBITS_ADD_EXECUTABLE_AND_TEST(
#  tDOFManager_SimpleTests
#  SOURCES tDOFManager_SimpleTests.cpp ${UNIT_TEST_DRIVER}
#  COMM serial mpi
#  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tUniqueGlobalIndexerUtilities
  SOURCES tGlobalIndexerUtilities.cpp UnitTest_GlobalIndexer.cpp ${UNIT_TEST_DRIVER}
  NUM_MPI_PROCS 2
  COMM mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tBlockedDOFManager
  SOURCES tBlockedDOFManager.cpp UnitTest_ConnManager.cpp UnitTest_ConnManager.hpp ${UNIT_TEST_DRIVER}
  NUM_MPI_PROCS 2
  COMM mpi
  )

#TRIBITS_ADD_EXECUTABLE_AND_TEST(
#  tBlockedDOFManager_fei
#  SOURCES tBlockedDOFManager_fei.cpp UnitTest_ConnManager.cpp UnitTest_ConnManager.hpp ${UNIT_TEST_DRIVER}
#  NUM_MPI_PROCS 2
#  COMM mpi
#  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tOrientations
  SOURCES tOrientations.cpp ${UNIT_TEST_DRIVER}
  NUM_MPI_PROCS 1
  COMM mpi
  )

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  tFilteredUGI
  SOURCES tFilteredUGI.cpp UnitTest_ConnManager.cpp UnitTest_ConnManager.hpp ${UNIT_TEST_DRIVER}
  NUM_MPI_PROCS 2
  COMM mpi
  )
