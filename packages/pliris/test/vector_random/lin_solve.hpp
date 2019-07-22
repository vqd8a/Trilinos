#pragma once
#include <Kokkos_Core.hpp>
#include <mpi.h>
#include "Pliris.hpp"

namespace linearAlgebra {

template<class ZDView>
void lin_solve( const ZDView& Z, int my_rows, int my_cols, int matrix_size, int num_procs, int num_rhs, double* secs )
{
  //// Test distribution
  ///*  Determine who I am and the number of processors that are being used    */
  int me,nprocs;

  MPI_Comm_rank(MPI_COMM_WORLD, &me) ;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);  

  // Solver code
   
  printf("Solve in rank %d -- rows %u , cols %u , matrix_size %u, num_procs_per_row %u, num_rhs %u\n", me, my_rows, my_cols, matrix_size, num_procs, num_rhs);
  
  Pliris_new::FactorSolve (Z,
                           my_rows,
                           my_cols,
                           &matrix_size,
                           &num_procs,
                           &num_rhs,
                           secs);

}
// End lin_solve function

} // linearAlgebra
