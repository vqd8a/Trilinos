
SET(HEADERS "")
SET(SOURCES "")

TRIBITS_CONFIGURE_FILE(${PACKAGE_NAME}_config.hpp)
TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
SET(HEADERS ${HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_config.hpp
  )

TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
#TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/stk_interface)
#TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/evaluators)
#TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/responses)

# auto generate headers, add to dependencies and include directories
TRILINOS_CREATE_CLIENT_TEMPLATE_HEADERS(${CMAKE_CURRENT_SOURCE_DIR})
#TRILINOS_CREATE_CLIENT_TEMPLATE_HEADERS(${CMAKE_CURRENT_SOURCE_DIR}/stk_interface)
#TRILINOS_CREATE_CLIENT_TEMPLATE_HEADERS(${CMAKE_CURRENT_SOURCE_DIR}/evaluators)
#TRILINOS_CREATE_CLIENT_TEMPLATE_HEADERS(${CMAKE_CURRENT_SOURCE_DIR}/responses)
APPEND_GLOB(HEADERS ${CMAKE_CURRENT_BINARY_DIR}/*.hpp)
TRIBITS_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

APPEND_GLOB(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
#APPEND_GLOB(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/stk_interface/*.hpp)
#IF( NOT TPL_ENABLE_CAMAL)
#list( REMOVE_ITEM HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/stk_interface/Panzer_STK_SculptMeshFactory.hpp )
#ENDIF( NOT TPL_ENABLE_CAMAL )
#APPEND_GLOB(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/evaluators/*.hpp)
#APPEND_GLOB(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/responses/*.hpp)

APPEND_GLOB(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
#APPEND_GLOB(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/stk_interface/*.cpp)
#IF( NOT TPL_ENABLE_CAMAL)
#list( REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/stk_interface/Panzer_STK_SculptMeshFactory.cpp )
#ENDIF( NOT TPL_ENABLE_CAMAL )
#APPEND_GLOB(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/evaluators/*.cpp)
#APPEND_GLOB(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/responses/*.cpp)

#
# C) Define the targets for package's library(s)
#

TRIBITS_ADD_LIBRARY(
  panzer-ioss
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  )




