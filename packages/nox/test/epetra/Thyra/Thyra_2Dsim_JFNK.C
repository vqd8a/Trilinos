// @HEADER
// *****************************************************************************
//            NOX: An Object-Oriented Nonlinear Solver Package
//
// Copyright 2002 NTESS and the NOX contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER
#include <Teuchos_ConfigDefs.hpp>
#include <Teuchos_UnitTestHarness.hpp>

// NOX Objects
#include "NOX.H"
#include "NOX_Thyra.H"

// Trilinos Objects
#ifdef HAVE_MPI
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_RowMatrix.h"
#include "Epetra_CrsMatrix.h"
#include "Epetra_Map.h"
#include "Epetra_LinearProblem.h"

#include "Teuchos_ParameterList.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_CommandLineProcessor.hpp"
#include "Teuchos_FancyOStream.hpp"

#include "Stratimikos_DefaultLinearSolverBuilder.hpp"
#include "Thyra_LinearOpWithSolveFactoryHelpers.hpp"
#include "Thyra_SpmdVectorBase.hpp"
#include "ModelEvaluator2DSim.hpp"

#include "NOX_Thyra_MatrixFreeJacobianOperator.hpp"
#include "NOX_MatrixFree_ModelEvaluatorDecorator.hpp"
#include "ModelEvaluator_LoggerDecorator.hpp"

using namespace std;

TEUCHOS_UNIT_TEST(NOX_Thyra_2DSim_JFNK, perturbation_unit_tests)
{
  Teuchos::TimeMonitor::zeroOutTimers();

  // Create a communicator for Epetra objects
#ifdef HAVE_MPI
  Epetra_MpiComm Comm( MPI_COMM_WORLD );
#else
  Epetra_SerialComm Comm;
#endif

  // Check we have only one processor since this problem doesn't work
  // for more than one proc
  TEST_ASSERT(Comm.NumProc() == 1);

  // Create the model evaluator object
  double d = 10.0;
  double p0 = 2.0;
  double p1 = 0.0;
  double x00 = 0.0;
  double x01 = 1.0;
  Teuchos::RCP<ModelEvaluator2DSim<double> > thyraModel =
    modelEvaluator2DSim<double>(Teuchos::rcp(&Comm,false),d,p0,p1,x00,x01);

  // Create the linear solver type with Stratimikos
  //Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<double> >
  //lowsFactory = rcp(new Thyra::AmesosLinearOpWithSolveFactory());

  ::Stratimikos::DefaultLinearSolverBuilder builder;

  Teuchos::RCP<Teuchos::ParameterList> p =
    Teuchos::rcp(new Teuchos::ParameterList);
  p->set("Linear Solver Type", "AztecOO");
  p->set("Preconditioner Type", "Ifpack");
  //p->set("Enable Delayed Solver Construction", true);
  builder.setParameterList(p);

  Teuchos::RCP< ::Thyra::LinearOpWithSolveFactoryBase<double> >
    lowsFactory = builder.createLinearSolveStrategy("");

  thyraModel->set_W_factory(lowsFactory);

  // Create the initial guess
  Teuchos::RCP< ::Thyra::VectorBase<double> >
    initial_guess = thyraModel->getNominalValues().get_x()->clone_v();

  // Create the NOX::Thyra::Group
  Teuchos::RCP<NOX::Thyra::Group> nox_group =
    Teuchos::rcp(new NOX::Thyra::Group(*initial_guess, thyraModel));

  nox_group->computeF();

  // Test the default JFNK parameter values (LOCA delta evaluation)
  {
    out << "\n **** Testing Salinger LOCA perturbation ****\n" << std::endl;

    // Create the JFNK operator
    Teuchos::ParameterList printParams;
    Teuchos::RCP<Teuchos::ParameterList> jfnkParams = Teuchos::parameterList();
    Teuchos::RCP<NOX::Thyra::MatrixFreeJacobianOperator<double> > jfnkOp =
      Teuchos::rcp(new NOX::Thyra::MatrixFreeJacobianOperator<double>(printParams));
    jfnkOp->setParameterList(jfnkParams);
    jfnkParams->print(out);

    jfnkOp->setBaseEvaluationToNOXGroup(nox_group);

    // Experiment with JFNK object
    Teuchos::RCP< ::Thyra::VectorBase<double> > input = initial_guess->clone_v();
    Teuchos::RCP< ::Thyra::VectorBase<double> > output = initial_guess->clone_v();
    ::Thyra::put_scalar(1.0,input.ptr());
    ::Thyra::put_scalar(0.0,output.ptr());
    ::Thyra::apply(*jfnkOp,::Thyra::NOTRANS,*input,output.ptr());

    output->describe(out,Teuchos::VERB_EXTREME);

    const double tol_mach_eps = 10.0 * std::numeric_limits<double>::epsilon();
    TEST_FLOATING_EQUALITY(jfnkOp->getLambda(),1.0e-6,tol_mach_eps);

    Teuchos::RCP< ::Thyra::SpmdVectorBase<double> > output_spmd =
      Teuchos::rcp_dynamic_cast< ::Thyra::SpmdVectorBase<double> >(output);
    Teuchos::ArrayRCP<const double> data;
    output_spmd->getLocalData(Teuchos::ptrFromRef(data));
    const double tol_perturb = 10.0 * jfnkOp->getDelta();
    TEST_FLOATING_EQUALITY(data[0],3.0,tol_perturb);
    TEST_FLOATING_EQUALITY(data[1],-10.0,tol_perturb);

    TEST_THROW(jfnkOp->setUserDefinedDelta(1.0e-8),std::logic_error);

    double delta_expected = jfnkOp->getLambda() * (jfnkOp->getLambda() + ::Thyra::norm(*initial_guess) / ::Thyra::norm(*input));
    TEST_FLOATING_EQUALITY(jfnkOp->getDelta(),delta_expected,tol_mach_eps);
  }

  // Test the KelleySalinger delta computation
  {
    out << "\n **** Testing Kelley Salinger Pawlowski 2001 perturbation ****\n" << std::endl;

    // Create the JFNK operator
    Teuchos::ParameterList printParams;
    Teuchos::RCP<Teuchos::ParameterList> jfnkParams = Teuchos::parameterList();
    jfnkParams->set("Difference Type","Forward");
    jfnkParams->set("Perturbation Algorithm","KSP NOX 2001");
    jfnkParams->set("lambda",1.0e-4);
    Teuchos::RCP<NOX::Thyra::MatrixFreeJacobianOperator<double> > jfnkOp =
      Teuchos::rcp(new NOX::Thyra::MatrixFreeJacobianOperator<double>(printParams));
    jfnkOp->setParameterList(jfnkParams);
    jfnkParams->print(out);

    jfnkOp->setBaseEvaluationToNOXGroup(nox_group);

    // Experiment with JFNK object
    Teuchos::RCP< ::Thyra::VectorBase<double> > input = initial_guess->clone_v();
    Teuchos::RCP< ::Thyra::VectorBase<double> > output = initial_guess->clone_v();
    ::Thyra::put_scalar(1.0,input.ptr());
    ::Thyra::put_scalar(0.0,output.ptr());
    ::Thyra::apply(*jfnkOp,::Thyra::NOTRANS,*input,output.ptr());

    const double tol_mach_eps = 10.0 * std::numeric_limits<double>::epsilon();
    TEST_FLOATING_EQUALITY(jfnkOp->getLambda(),1.0e-4,tol_mach_eps);

    Teuchos::RCP< ::Thyra::SpmdVectorBase<double> > output_spmd =
      Teuchos::rcp_dynamic_cast< ::Thyra::SpmdVectorBase<double> >(output);
    Teuchos::ArrayRCP<const double> data;
    output_spmd->getLocalData(Teuchos::ptrFromRef(data));
    const double tol_perturb = 10.0 * jfnkOp->getDelta();
    TEST_FLOATING_EQUALITY(data[0],3.0,tol_perturb);
    TEST_FLOATING_EQUALITY(data[1],-10.0,tol_perturb);

    double dotprod = ::Thyra::inner(*initial_guess,*input);
    double vectorNorm = ::Thyra::norm(*input);
    double ks_delta_expected = jfnkOp->getLambda() * (1.0e-12 / jfnkOp->getLambda() + std::fabs(dotprod) / (vectorNorm * vectorNorm) ) * dotprod / std::fabs(dotprod);

    TEST_FLOATING_EQUALITY(jfnkOp->getDelta(),ks_delta_expected,tol_mach_eps);
  }

  // Test the KnollKeyes delta computation
  {
    out << "\n **** Testing Knoll Keyes JCP 2004 perturbation ****\n" << std::endl;

    // Create the JFNK operator
    Teuchos::ParameterList printParams;
    Teuchos::RCP<Teuchos::ParameterList> jfnkParams = Teuchos::parameterList();
    jfnkParams->set("Difference Type","Forward");
    jfnkParams->set("Perturbation Algorithm","Knoll Keyes JCP 2004");
    jfnkParams->set("lambda",1.0e-6);
    Teuchos::RCP<NOX::Thyra::MatrixFreeJacobianOperator<double> > jfnkOp =
      Teuchos::rcp(new NOX::Thyra::MatrixFreeJacobianOperator<double>(printParams));
    jfnkOp->setParameterList(jfnkParams);
    jfnkParams->print(out);

    jfnkOp->setBaseEvaluationToNOXGroup(nox_group);

    // Experiment with JFNK object
    Teuchos::RCP< ::Thyra::VectorBase<double> > input = initial_guess->clone_v();
    Teuchos::RCP< ::Thyra::VectorBase<double> > output = initial_guess->clone_v();
    ::Thyra::put_scalar(1.0,input.ptr());
    ::Thyra::put_scalar(0.0,output.ptr());
    ::Thyra::apply(*jfnkOp,::Thyra::NOTRANS,*input,output.ptr());

    const double tol_mach_eps = 10.0 * std::numeric_limits<double>::epsilon();
    const double tol_perturb = 10.0 * jfnkOp->getDelta();

    Teuchos::RCP< ::Thyra::SpmdVectorBase<double> > output_spmd =
      Teuchos::rcp_dynamic_cast< ::Thyra::SpmdVectorBase<double> >(output);
    Teuchos::ArrayRCP<const double> data;
    output_spmd->getLocalData(Teuchos::ptrFromRef(data));
    TEST_FLOATING_EQUALITY(data[0],3.0,tol_perturb);
    TEST_FLOATING_EQUALITY(data[1],-10.0,tol_perturb);

    double kk_delta_expected = jfnkOp->getLambda() * ::Thyra::norm_1(*initial_guess) / (Teuchos::as<double>(initial_guess->space()->dim()) * ::Thyra::norm(*input)) + jfnkOp->getLambda();

    TEST_FLOATING_EQUALITY(jfnkOp->getDelta(),kk_delta_expected,tol_mach_eps);
  }

  // Test the user defined delta computation
  {
    out << "\n **** Testing User Defined perturbation ****\n" << std::endl;

    // Create the JFNK operator
    Teuchos::ParameterList printParams;
    Teuchos::RCP<Teuchos::ParameterList> jfnkParams = Teuchos::parameterList();
    jfnkParams->set("Difference Type","Forward");
    jfnkParams->set("Perturbation Algorithm","User Defined");
    jfnkParams->set("lambda",1.0e-4);
    jfnkParams->set("User Defined delta Value",1.0e-5);
    Teuchos::RCP<NOX::Thyra::MatrixFreeJacobianOperator<double> > jfnkOp =
      Teuchos::rcp(new NOX::Thyra::MatrixFreeJacobianOperator<double>(printParams));
    jfnkOp->setParameterList(jfnkParams);
    jfnkParams->print(std::cout);

    jfnkOp->setBaseEvaluationToNOXGroup(nox_group);

    // Experiment with JFNK object
    Teuchos::RCP< ::Thyra::VectorBase<double> > input = initial_guess->clone_v();
    Teuchos::RCP< ::Thyra::VectorBase<double> > output = initial_guess->clone_v();
    ::Thyra::put_scalar(1.0,input.ptr());
    ::Thyra::put_scalar(0.0,output.ptr());
    ::Thyra::apply(*jfnkOp,::Thyra::NOTRANS,*input,output.ptr());

    const double tol_mach_eps = 10.0 * std::numeric_limits<double>::epsilon();
    TEST_FLOATING_EQUALITY(jfnkOp->getLambda(),1.0e-4,tol_mach_eps);

    Teuchos::RCP< ::Thyra::SpmdVectorBase<double> > output_spmd =
      Teuchos::rcp_dynamic_cast< ::Thyra::SpmdVectorBase<double> >(output);
    Teuchos::ArrayRCP<const double> data;
    output_spmd->getLocalData(Teuchos::ptrFromRef(data));
    const double tol_perturb = 10.0 * jfnkOp->getDelta();
    TEST_FLOATING_EQUALITY(data[0],3.0,tol_perturb);
    TEST_FLOATING_EQUALITY(data[1],-10.0,tol_perturb);
    double user_defined_delta_expected = 1.0e-5;

    TEST_FLOATING_EQUALITY(jfnkOp->getDelta(),user_defined_delta_expected,tol_mach_eps);

    // test being able to change the value at runtime
    jfnkOp->setUserDefinedDelta(1.0e-7);
    TEST_FLOATING_EQUALITY(jfnkOp->getDelta(),1.0e-5,tol_mach_eps);
    ::Thyra::apply(*jfnkOp,::Thyra::NOTRANS,*input,output.ptr());
    TEST_FLOATING_EQUALITY(jfnkOp->getDelta(),1.0e-7,tol_mach_eps);
  }

  Teuchos::TimeMonitor::summarize();
}

TEUCHOS_UNIT_TEST(NOX_Thyra_2DSim_JFNK, solve_no_prec)
{
  Teuchos::TimeMonitor::zeroOutTimers();

  // Create a communicator for Epetra objects
#ifdef HAVE_MPI
  Epetra_MpiComm Comm( MPI_COMM_WORLD );
#else
  Epetra_SerialComm Comm;
#endif

  // Check we have only one processor since this problem doesn't work
  // for more than one proc
  TEST_ASSERT(Comm.NumProc() == 1);

  // Create the model evaluator object
  double d = 10.0;
  double p0 = 2.0;
  double p1 = 0.0;
  double x00 = 0.0;
  double x01 = 1.0;
  Teuchos::RCP<ModelEvaluator2DSim<double> > model =
    modelEvaluator2DSim<double>(Teuchos::rcp(&Comm,false),d,p0,p1,x00,x01);

  // Create the linear solver type with Stratimikos
  //Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<double> >
  //lowsFactory = rcp(new Thyra::AmesosLinearOpWithSolveFactory());

  ::Stratimikos::DefaultLinearSolverBuilder builder;

  Teuchos::RCP<Teuchos::ParameterList> p =
    Teuchos::rcp(new Teuchos::ParameterList);
  p->set("Linear Solver Type", "AztecOO");
  p->sublist("Linear Solver Types").sublist("AztecOO").sublist("Forward Solve").set("Tolerance",1.0e-1);
  p->sublist("Linear Solver Types").sublist("AztecOO").sublist("Forward Solve").sublist("AztecOO Settings").set("Output Frequency",1);
  //p->set("Linear Solver Type", "Belos");
  p->set("Preconditioner Type", "None");
  //p->set("Enable Delayed Solver Construction", true);
  builder.setParameterList(p);

  Teuchos::RCP< ::Thyra::LinearOpWithSolveFactoryBase<double> >
    lowsFactory = builder.createLinearSolveStrategy("");

  model->set_W_factory(lowsFactory);

  // Create the initial guess
  Teuchos::RCP< ::Thyra::VectorBase<double> >
    initial_guess = model->getNominalValues().get_x()->clone_v();

  // Create the JFNK operator
  Teuchos::ParameterList printParams;
  Teuchos::RCP<Teuchos::ParameterList> jfnkParams = Teuchos::parameterList();
  jfnkParams->set("Difference Type","Forward");
  jfnkParams->set("Perturbation Algorithm","KSP NOX 2001");
  jfnkParams->set("lambda",1.0e-4);
  Teuchos::RCP<NOX::Thyra::MatrixFreeJacobianOperator<double> > jfnkOp =
    Teuchos::rcp(new NOX::Thyra::MatrixFreeJacobianOperator<double>(printParams));
  jfnkOp->setParameterList(jfnkParams);
  jfnkParams->print(out);

  // Wrap the model evaluator in a JFNK Model Evaluator
  Teuchos::RCP< ::Thyra::ModelEvaluator<double> > thyraModel =
    Teuchos::rcp(new NOX::MatrixFreeModelEvaluatorDecorator<double>(model));

  // Wrap model in logger decrator for unit testing
  Teuchos::RCP<NOX_TEST::ModelEvaluatorLoggerDecorator<double> > loggedModel =
    Teuchos::rcp(new NOX_TEST::ModelEvaluatorLoggerDecorator<double>(thyraModel));

  // Create the NOX::Thyra::Group
  Teuchos::RCP<NOX::Thyra::Group> nox_group =
    Teuchos::rcp(new NOX::Thyra::Group(*initial_guess, loggedModel, jfnkOp, lowsFactory, Teuchos::null, Teuchos::null, Teuchos::null));

  nox_group->computeF();

  // VERY IMPORTANT!!!  jfnk object needs base evaluation objects.
  // This creates a circular dependency, so use a weak pointer.
  jfnkOp->setBaseEvaluationToNOXGroup(nox_group.create_weak());

  // Create the NOX status tests and the solver
  // Create the convergence tests
  Teuchos::RCP<NOX::StatusTest::NormF> absresid =
    Teuchos::rcp(new NOX::StatusTest::NormF(1.0e-8));
  Teuchos::RCP<NOX::StatusTest::NormWRMS> wrms =
    Teuchos::rcp(new NOX::StatusTest::NormWRMS(1.0e-2, 1.0e-8));
  Teuchos::RCP<NOX::StatusTest::Combo> converged =
    Teuchos::rcp(new NOX::StatusTest::Combo(NOX::StatusTest::Combo::AND));
  converged->addStatusTest(absresid);
  converged->addStatusTest(wrms);
  Teuchos::RCP<NOX::StatusTest::MaxIters> maxiters =
    Teuchos::rcp(new NOX::StatusTest::MaxIters(20));
  Teuchos::RCP<NOX::StatusTest::FiniteValue> fv =
    Teuchos::rcp(new NOX::StatusTest::FiniteValue);
  Teuchos::RCP<NOX::StatusTest::Combo> combo =
    Teuchos::rcp(new NOX::StatusTest::Combo(NOX::StatusTest::Combo::OR));
  combo->addStatusTest(fv);
  combo->addStatusTest(converged);
  combo->addStatusTest(maxiters);

  // Create nox parameter list
  Teuchos::RCP<Teuchos::ParameterList> nl_params =
    Teuchos::rcp(new Teuchos::ParameterList);
  nl_params->set("Nonlinear Solver", "Line Search Based");

  // Create the solver
  Teuchos::RCP<NOX::Solver::Generic> solver =
    NOX::Solver::buildSolver(nox_group, combo, nl_params);
  NOX::StatusTest::StatusType solvStatus = solver->solve();

  // Final return value (0 = successfull, non-zero = failure)
  TEST_ASSERT(solvStatus == NOX::StatusTest::Converged);
  TEST_EQUALITY(loggedModel->getNumExactCalls(),8);
  TEST_EQUALITY(loggedModel->getNumApproxDerivCalls(),21);
  TEST_EQUALITY(loggedModel->getNumVeryApproxDerivCalls(),0);

  Teuchos::TimeMonitor::summarize();
}

TEUCHOS_UNIT_TEST(NOX_Thyra_2DSim_JFNK, solve_Jacobi_prec_using_ME)
{
  Teuchos::TimeMonitor::zeroOutTimers();

  // Create a communicator for Epetra objects
#ifdef HAVE_MPI
  Epetra_MpiComm Comm( MPI_COMM_WORLD );
#else
  Epetra_SerialComm Comm;
#endif

  // Check we have only one processor since this problem doesn't work
  // for more than one proc
  TEST_ASSERT(Comm.NumProc() == 1);

  // Create the model evaluator object
  double d = 10.0;
  double p0 = 2.0;
  double p1 = 0.0;
  double x00 = 0.0;
  double x01 = 1.0;
  Teuchos::RCP<ModelEvaluator2DSim<double> > model =
    modelEvaluator2DSim<double>(Teuchos::rcp(&Comm,false),d,p0,p1,x00,x01);

  // Create the linear solver type with Stratimikos
  //Teuchos::RCP<Thyra::LinearOpWithSolveFactoryBase<double> >
  //lowsFactory = rcp(new Thyra::AmesosLinearOpWithSolveFactory());

  ::Stratimikos::DefaultLinearSolverBuilder builder;

  Teuchos::RCP<Teuchos::ParameterList> p =
    Teuchos::rcp(new Teuchos::ParameterList);
  p->set("Linear Solver Type", "AztecOO");
  p->sublist("Linear Solver Types").sublist("AztecOO").sublist("Forward Solve").set("Tolerance",1.0e-1);
  p->sublist("Linear Solver Types").sublist("AztecOO").sublist("Forward Solve").sublist("AztecOO Settings").set("Output Frequency",1);
//   p->set("Linear Solver Type", "Belos");
  p->set("Preconditioner Type", "None");
  //p->set("Enable Delayed Solver Construction", true);
  builder.setParameterList(p);

  Teuchos::RCP< ::Thyra::LinearOpWithSolveFactoryBase<double> >
    lowsFactory = builder.createLinearSolveStrategy("");

  model->set_W_factory(lowsFactory);

  // Create the initial guess
  Teuchos::RCP< ::Thyra::VectorBase<double> >
    initial_guess = model->getNominalValues().get_x()->clone_v();

  // Create the JFNK operator
  Teuchos::ParameterList printParams;
  Teuchos::RCP<Teuchos::ParameterList> jfnkParams = Teuchos::parameterList();
  jfnkParams->set("Difference Type","Forward");
  jfnkParams->set("Perturbation Algorithm","KSP NOX 2001");
  jfnkParams->set("lambda",1.0e-4);
  Teuchos::RCP<NOX::Thyra::MatrixFreeJacobianOperator<double> > jfnkOp =
    Teuchos::rcp(new NOX::Thyra::MatrixFreeJacobianOperator<double>(printParams));
  jfnkOp->setParameterList(jfnkParams);
  jfnkParams->print(out);

  // Wrap the model evaluator in a JFNK Model Evaluator
  Teuchos::RCP< ::Thyra::ModelEvaluator<double> > thyraModel =
    Teuchos::rcp(new NOX::MatrixFreeModelEvaluatorDecorator<double>(model));

  // Create the Preconditioner operator
  Teuchos::RCP< ::Thyra::PreconditionerBase<double> > precOp =
    thyraModel->create_W_prec();

  // Create the NOX::Thyra::Group
  Teuchos::RCP<NOX::Thyra::Group> nox_group =
    Teuchos::rcp(new NOX::Thyra::Group(*initial_guess, thyraModel, jfnkOp, lowsFactory, precOp, Teuchos::null, Teuchos::null));

  nox_group->computeF();

  // VERY IMPORTANT!!!  jfnk object needs base evaluation objects.
  // This creates a circular dependency, so use a weak pointer.
  jfnkOp->setBaseEvaluationToNOXGroup(nox_group.create_weak());

  // Create a preconditioner

  // Create the NOX status tests and the solver
  // Create the convergence tests
  Teuchos::RCP<NOX::StatusTest::NormF> absresid =
    Teuchos::rcp(new NOX::StatusTest::NormF(1.0e-8));
  Teuchos::RCP<NOX::StatusTest::NormWRMS> wrms =
    Teuchos::rcp(new NOX::StatusTest::NormWRMS(1.0e-2, 1.0e-8));
  Teuchos::RCP<NOX::StatusTest::Combo> converged =
    Teuchos::rcp(new NOX::StatusTest::Combo(NOX::StatusTest::Combo::AND));
  converged->addStatusTest(absresid);
  converged->addStatusTest(wrms);
  Teuchos::RCP<NOX::StatusTest::MaxIters> maxiters =
    Teuchos::rcp(new NOX::StatusTest::MaxIters(20));
  Teuchos::RCP<NOX::StatusTest::FiniteValue> fv =
    Teuchos::rcp(new NOX::StatusTest::FiniteValue);
  Teuchos::RCP<NOX::StatusTest::Combo> combo =
    Teuchos::rcp(new NOX::StatusTest::Combo(NOX::StatusTest::Combo::OR));
  combo->addStatusTest(fv);
  combo->addStatusTest(converged);
  combo->addStatusTest(maxiters);

  // Create nox parameter list
  Teuchos::RCP<Teuchos::ParameterList> nl_params =
    Teuchos::rcp(new Teuchos::ParameterList);
  nl_params->set("Nonlinear Solver", "Line Search Based");

  // Create the solver
  Teuchos::RCP<NOX::Solver::Generic> solver =
    NOX::Solver::buildSolver(nox_group, combo, nl_params);
  NOX::StatusTest::StatusType solvStatus = solver->solve();

  // Final return value (0 = successfull, non-zero = failure)
  TEST_ASSERT(solvStatus == NOX::StatusTest::Converged);

  Teuchos::TimeMonitor::summarize();
}
