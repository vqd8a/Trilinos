TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
TRIBITS_INCLUDE_DIRECTORIES(${PARENT_PACKAGE_SOURCE_DIR}/disc-fe/test/equation_set)
TRIBITS_INCLUDE_DIRECTORIES(${PARENT_PACKAGE_SOURCE_DIR}/adapters-stk/test/evaluator_tests)

SET(UNIT_TEST_DRIVER ${PANZER_UNIT_TEST_MAIN})

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  interpolation
  SOURCES interpolation.cpp ${UNIT_TEST_DRIVER}
  COMM serial mpi
  NUM_MPI_PROCS 4
  )
