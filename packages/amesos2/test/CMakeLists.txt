
ADD_SUBDIRECTORIES(
  solvers
  adapters
  )

TRIBITS_COPY_FILES_TO_BINARY_DIR(CopyMatrixTestFiles
  SOURCE_FILES ./matrices/arc130.mtx
               ./matrices/orsirr_2.mtx
               ./matrices/gemat12.mtx
               ./matrices/sherman3.mtx
               ./matrices/young1c.mtx
               ./matrices/amesos2_test_mat0.mtx
               ./matrices/amesos2_test_mat0_complex.mtx
               ./matrices/amesos2_test_mat1.mtx
               ./matrices/amesos2_test_mat2.mtx
               ./matrices/amesos2_test_mat3.mtx
               ./matrices/amesos2_test_mat4.mtx
               ./matrices/tacho_test.mtx
               ./matrices/tacho_test_complex.mtx
               ./matrices/gap-ids-1procA.mm
               ./matrices/gap-ids-1proc-rowmap-tpetra.mm
               ./matrices/gap-ids-1proc-rowmap-epetra.mm
  )

