INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

IF(Kokkos_ENABLE_Serial)
  ADD_SUBDIRECTORY(Serial)
ENDIF()

# IF(Kokkos_ENABLE_OpenMP)
#   ADD_SUBDIRECTORY(OpenMP)
# ENDIF()

# IF(Kokkos_ENABLE_Cuda)
#   ADD_SUBDIRECTORY(Cuda)
# ENDIF()

