

#
# A) Package-specific configuration options
#

TRIBITS_CONFIGURE_FILE(${PACKAGE_NAME}_Config.h)

#
# B) Define the header and source files (and directories)
#

SET(HEADERS "")
SET(NOINSTALLHEADERS "")
SET(SOURCES "")

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(HEADERS ${HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_Config.h
  )

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# Base headers
APPEND_SET(HEADERS
  NOX.H
  NOX_Common.H
  NOX_Exceptions.H
  )

# Abstract
APPEND_SET(HEADERS
  NOX_Abstract_Vector.H
  NOX_Abstract_MultiVector.H
  NOX_Abstract_Group.H
  NOX_Observer.hpp
  NOX_Abstract_PrePostOperator.H
  NOX_Abstract_ImplicitWeighting.H
  )

APPEND_SET(SOURCES
  NOX_Abstract_Vector.C
  NOX_Abstract_Group.C
  )

# Linesearch
APPEND_SET(HEADERS
  NOX_LineSearch_Generic.H
  NOX_LineSearch_Factory.H
  NOX_LineSearch_UserDefinedFactory.H
  NOX_LineSearch_UserDefinedFactoryT.H
  NOX_LineSearch_Utils_Slope.H
  NOX_LineSearch_Utils_Printing.H
  NOX_LineSearch_Utils_Counters.H
  NOX_LineSearch_FullStep.H
  NOX_LineSearch_Backtrack.H
  NOX_LineSearch_MoreThuente.H
  NOX_LineSearch_NonlinearCG.H
  NOX_LineSearch_Polynomial.H
  NOX_LineSearch_SafeguardedStep.H
)
APPEND_SET(SOURCES
  NOX_LineSearch_Factory.C
  NOX_LineSearch_Utils_Slope.C
  NOX_LineSearch_Utils_Printing.C
  NOX_LineSearch_Utils_Counters.C
  NOX_LineSearch_FullStep.C
  NOX_LineSearch_Backtrack.C
  NOX_LineSearch_MoreThuente.C
  NOX_LineSearch_NonlinearCG.C
  NOX_LineSearch_Polynomial.C
  NOX_LineSearch_SafeguardedStep.C
)

# Direction
APPEND_SET(HEADERS
  NOX_Direction_Generic.H
  NOX_Direction_Factory.H
  NOX_Direction_UserDefinedFactory.H
  NOX_Direction_UserDefinedFactoryT.H
  NOX_Direction_Newton.H
  NOX_Direction_Broyden.H
  NOX_Direction_NonlinearCG.H
  NOX_Direction_SteepestDescent.H
  NOX_Direction_Utils_InexactNewton.H
  )
APPEND_SET(SOURCES
  NOX_Direction_Generic.C
  NOX_Direction_Factory.C
  NOX_Direction_Newton.C
  NOX_Direction_Broyden.C
  NOX_Direction_NonlinearCG.C
  NOX_Direction_SteepestDescent.C
  NOX_Direction_Utils_InexactNewton.C
  )

# Prerelease Code
IF (${PACKAGE_NAME}_BUILD_PRERELEASE)

  APPEND_SET(HEADERS
    NOX_Solver_TensorBasedTest.H
    NOX_LineSearch_Tensor.H
    NOX_Direction_Tensor.H
    NOX_Direction_QuasiNewton.H
    NOX_Direction_ModifiedNewton.H
    )

  APPEND_SET(SOURCES
    NOX_Solver_TensorBasedTest.C
    NOX_LineSearch_Tensor.C
    NOX_Direction_Tensor.C
    NOX_Direction_QuasiNewton.C
    NOX_Direction_ModifiedNewton.C
    )

ENDIF()

# Solvers
APPEND_SET(HEADERS
  NOX_Solver_Generic.H
  NOX_Solver_Factory.H
  NOX_Solver_SolverUtils.H
  NOX_Solver_LineSearchBased.H
  NOX_Solver_TrustRegionBased.H
  NOX_Solver_InexactTrustRegionBased.H
  NOX_Solver_TensorBased.H
  NOX_Solver_AndersonAcceleration.H
  NOX_Solver_SingleStep.H
  NOX_SolverStats.hpp
  )
APPEND_SET(SOURCES
  NOX_Solver_Factory.C
  NOX_Solver_SolverUtils.C
  NOX_Solver_LineSearchBased.C
  NOX_Solver_TrustRegionBased.C
  NOX_Solver_InexactTrustRegionBased.C
  NOX_Solver_TensorBased.C
  NOX_Solver_AndersonAcceleration.C
  NOX_Solver_SingleStep.C
  )

# Merit Function
APPEND_SET(HEADERS
  NOX_MeritFunction_Generic.H
  NOX_MeritFunction_SumOfSquares.H
  )
APPEND_SET(SOURCES
  NOX_MeritFunction_SumOfSquares.C
  )

# Status Tests
APPEND_SET(HEADERS
  NOX_StatusTest_Generic.H
  NOX_StatusTest_Factory.H
  NOX_StatusTest_NormF.H
  NOX_StatusTest_FiniteValue.H
  NOX_StatusTest_NormUpdate.H
  NOX_StatusTest_Combo.H
  NOX_StatusTest_MaxIters.H
  NOX_StatusTest_NormWRMS.H
  NOX_StatusTest_Stagnation.H
  NOX_StatusTest_Divergence.H
  NOX_StatusTest_RelativeNormF.H
  NOX_StatusTest_NStep.H
  )
APPEND_SET(SOURCES
  NOX_StatusTest_Generic.C
  NOX_StatusTest_Factory.C
  NOX_StatusTest_NormF.C
  NOX_StatusTest_FiniteValue.C
  NOX_StatusTest_NormUpdate.C
  NOX_StatusTest_Combo.C
  NOX_StatusTest_MaxIters.C
  NOX_StatusTest_NormWRMS.C
  NOX_StatusTest_Stagnation.C
  NOX_StatusTest_Divergence.C
  NOX_StatusTest_RelativeNormF.C
  NOX_StatusTest_NStep.C
  )

# Utils
APPEND_SET(HEADERS
  NOX_Utils.H
  NOX_SharedObjectTemplate.H
  NOX_Random.H
  NOX_GlobalData.H
  NOX_PrePostOperator_Vector.H
  NOX_Observer_Vector.hpp
  NOX_Observer_Log.hpp
  NOX_Observer_Print.hpp
  NOX_Assert.H
  NOX_GlobalComm.H
  )
APPEND_SET(SOURCES
  NOX_Utils.C
  NOX_Random.C
  NOX_GlobalData.C
  NOX_Observer_Vector.cpp
  NOX_Observer_Log.cpp
  NOX_Observer_Print.cpp
  )

# Multivector
APPEND_SET(HEADERS
  NOX_MultiVector.H
  )
APPEND_SET(SOURCES
  NOX_MultiVector.C
  )

# Multi-physics
APPEND_SET(HEADERS
  NOX_Multiphysics_Group.H
  NOX_Multiphysics_Solver_Manager.H
  NOX_Multiphysics_Solver_Generic.H
  NOX_Multiphysics_Solver_FixedPointBased.H
  NOX_Multiphysics_DataExchange_Interface.H
  )
APPEND_SET(SOURCES
  NOX_Multiphysics_Group.C
  NOX_Multiphysics_Solver_Manager.C
  NOX_Multiphysics_Solver_FixedPointBased.C
  )

IF(NOX_ENABLE_ABSTRACT_IMPLEMENTATION_THYRA)
  TRIBITS_INCLUDE_DIRECTORIES(${PACKAGE_SOURCE_DIR}/src-thyra)
  APPEND_SET(HEADERS
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra.H
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_Vector.H
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_MultiVector.H
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_Group.H
    ${PACKAGE_SOURCE_DIR}/src-thyra/Thyra_NonlinearSolver_NOX.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_PrePostOperator_RowSumScaling.H
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_MeritFunction_Weighted.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Solver_PseudoTransient.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_MatrixFreeJacobianOperator.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_MatrixFreeJacobianOperator_impl.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_MatrixFree_ModelEvaluatorDecorator.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_LineSearch_SafeguardedDirection.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_TOpEleWiseMinSwap.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Observer_ReusePreconditioner.hpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Observer_ReusePreconditionerFactory.hpp
  )
  APPEND_SET(SOURCES
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_Vector.C
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_MultiVector.C
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Thyra_Group.C
    ${PACKAGE_SOURCE_DIR}/src-thyra/Thyra_NonlinearSolver_NOX.cpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_PrePostOperator_RowSumScaling.C
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_MeritFunction_Weighted.cpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Solver_PseudoTransient.cpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_LineSearch_SafeguardedDirection.cpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Observer_ReusePreconditioner.cpp
    ${PACKAGE_SOURCE_DIR}/src-thyra/NOX_Observer_ReusePreconditionerFactory.cpp
  )
ENDIF()

IF(NOX_ENABLE_KOKKOS_SOLVER_STACK)
  TRIBITS_INCLUDE_DIRECTORIES(${PACKAGE_SOURCE_DIR}/src-thyra-tpetra)
  APPEND_SET(HEADERS
    ${PACKAGE_SOURCE_DIR}/src-thyra-tpetra/NOX_TpetraTypedefs.hpp
  )
ENDIF()

# Helper function for ETI processing.
FUNCTION(NOX_TPETRA_PROCESS_ETI_TEMPLATE_SLGN ETI_CLASSES TEMPLATE_FILE SOURCES_LIST NOX_TPETRA_ETI_SCALARS NOX_TPETRA_ETI_LORDS NOX_TPETRA_ETI_GORDS NOX_TPETRA_ETI_NODES)
  SET(SRCS "")
  FOREACH(CLASS ${ETI_CLASSES})
    TPETRA_MANGLE_TEMPLATE_PARAMETER(CLASS_MANGLED ${CLASS})
    string(TOUPPER "${CLASS_MANGLED}" UPPER_CASE_CLASS)
    TPETRA_PROCESS_ALL_SLGN_TEMPLATES(TMP_OUTPUT_FILES ${TEMPLATE_FILE} ${CLASS_MANGLED} ${UPPER_CASE_CLASS} "${NOX_TPETRA_ETI_SCALARS}" "${NOX_TPETRA_ETI_LORDS}" "${NOX_TPETRA_ETI_GORDS}" "${NOX_TPETRA_ETI_NODES}" FALSE)
    LIST(APPEND SRCS ${TMP_OUTPUT_FILES})
  ENDFOREACH()
  SET(${SOURCES_LIST} ${SRCS} PARENT_SCOPE)
ENDFUNCTION(NOX_TPETRA_PROCESS_ETI_TEMPLATE_SLGN)

SET(NOX_TPETRA_CPP_SOURCES "")
IF(NOX_ENABLE_ABSTRACT_IMPLEMENTATION_TPETRA)
  TRIBITS_INCLUDE_DIRECTORIES(${PACKAGE_SOURCE_DIR}/src-tpetra)
  APPEND_SET(HEADERS
    ${PACKAGE_SOURCE_DIR}/src-tpetra/NOX_Tpetra_Vector.hpp
    ${PACKAGE_SOURCE_DIR}/src-tpetra/NOX_Tpetra_Vector_def.hpp
  )
  
  # Do explicit template instantiation (ETI) and testing for the Scalar type
  # double using the same LocalOrdinal, GlobalOrdinal and Node types as Tpetra.

  # Make sure that Tpetra actually defined these variables, even if they are empty.
  ASSERT_DEFINED(TpetraCore_ETI_SCALARS)
  ASSERT_DEFINED(TpetraCore_ETI_LORDS)
  ASSERT_DEFINED(TpetraCore_ETI_GORDS)
  ASSERT_DEFINED(TpetraCore_ETI_NODES)

  # Check if Tpetra's list of ETI Scalars includes double.
  SET(NOX_TPETRA_ETI_SCALARS "")
  if("double" IN_LIST TpetraCore_ETI_SCALARS)
    list(APPEND NOX_TPETRA_ETI_SCALARS "double")
  endif()

  # Copy the LocalOrdinal, GlobalOrdinal and Node types from Tpetra's ETI.
  SET(NOX_TPETRA_ETI_LORDS ${TpetraCore_ETI_LORDS})
  SET(NOX_TPETRA_ETI_GORDS ${TpetraCore_ETI_GORDS})
  SET(NOX_TPETRA_ETI_NODES ${TpetraCore_ETI_NODES})

  # Export the names for use in the ETI system.
  GLOBAL_SET(NOX_TPETRA_ETI_SCALARS ${NOX_TPETRA_ETI_SCALARS})
  GLOBAL_SET(NOX_TPETRA_ETI_LORDS   ${NOX_TPETRA_ETI_LORDS})
  GLOBAL_SET(NOX_TPETRA_ETI_GORDS   ${NOX_TPETRA_ETI_GORDS})
  GLOBAL_SET(NOX_TPETRA_ETI_NODES   ${NOX_TPETRA_ETI_NODES})

  GLOBAL_SET(NOX_TPETRA_ETI_CLASSES
    Vector
  )

  SET(TEMPLATE_FILE "NOX_Tpetra_ETI_SC_LO_GO_NT.tmpl")
  NOX_TPETRA_PROCESS_ETI_TEMPLATE_SLGN("${NOX_TPETRA_ETI_CLASSES}" ${TEMPLATE_FILE} NOX_TPETRA_SRCS "${NOX_TPETRA_ETI_SCALARS}" "${NOX_TPETRA_ETI_LORDS}" "${NOX_TPETRA_ETI_GORDS}" "${NOX_TPETRA_ETI_NODES}")
  LIST(APPEND NOX_TPETRA_CPP_SOURCES ${NOX_TPETRA_SRCS})

  # Generate the NOX_Tpetra_ETIHelperMacros.hpp file.
  TRIBITS_ADD_ETI_SUPPORT()
ENDIF()

#
# C) Define the targets for package's library(s)
#

TRIBITS_ADD_LIBRARY(
  nox
  HEADERS ${HEADERS}
  NOINSTALLHEADERS ${NOINSTALLHEADERS}
  SOURCES ${SOURCES} ${NOX_TPETRA_CPP_SOURCES}
  )
