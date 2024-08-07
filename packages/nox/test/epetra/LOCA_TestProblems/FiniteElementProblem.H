// @HEADER
// *****************************************************************************
//            LOCA: Library of Continuation Algorithms Package
//
// Copyright 2001-2005 NTESS and the LOCA contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

// Finite Element Problem Class

#ifndef _FINITEELEMENTPROBLEM_H
#define _FINITEELEMENTPROBLEM_H

// Flag to tell the evaluate routine what objects to fill
enum FillType {F_ONLY, MATRIX_ONLY, ALL};

// Finite Element Problem Class
class FiniteElementProblem {

public:

  // Constructor
  FiniteElementProblem() {}

  // Destructor
  virtual ~FiniteElementProblem() {}

  // Evaluates the function (F) and/or the Jacobian using the solution
  // values in solnVector.
  virtual bool evaluate(FillType f, const Epetra_Vector *solnVector,
            Epetra_Vector *rhsVector,
            Epetra_RowMatrix *matrix,
            double jac_coeff = 1.0,
            double mass_coeff = 0.0) = 0;

  // Return a reference to the Epetra_Vector with the initial guess
  // that is generated by the FiniteElementProblem class.
  virtual Epetra_Vector& getSolution() = 0;

  // Return a reference to the Epetra_Vector with the Jacobian
  // that is generated by the FiniteElementProblem class.
  virtual Epetra_CrsMatrix& getJacobian() = 0;

  // Set a bifurcation parameter in the application physics
  virtual bool setParameter(std::string label, double value) = 0;

};
#endif




