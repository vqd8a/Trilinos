// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#ifndef Tempus_StepperNewmarkExplicitAForm_impl_hpp
#define Tempus_StepperNewmarkExplicitAForm_impl_hpp

#include "Teuchos_VerboseObjectParameterListHelpers.hpp"
#include "Thyra_VectorStdOps.hpp"

//#define DEBUG_OUTPUT

namespace Tempus {


template<class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::
predictVelocity(Thyra::VectorBase<Scalar>& vPred,
                const Thyra::VectorBase<Scalar>& v,
                const Thyra::VectorBase<Scalar>& a,
                const Scalar dt) const
{
  //vPred = v + dt*(1.0-gamma_)*a
  Thyra::V_StVpStV(Teuchos::ptrFromRef(vPred), 1.0, v, dt*(1.0-gamma_), a);
}

template<class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::
predictDisplacement(Thyra::VectorBase<Scalar>& dPred,
                    const Thyra::VectorBase<Scalar>& d,
                    const Thyra::VectorBase<Scalar>& v,
                    const Thyra::VectorBase<Scalar>& a,
                    const Scalar dt) const
{
  Teuchos::RCP<const Thyra::VectorBase<Scalar> > tmp =
    Thyra::createMember<Scalar>(dPred.space());
  //dPred = dt*v + dt*dt/2.0*a
  Scalar aConst = dt*dt/2.0;
  Thyra::V_StVpStV(Teuchos::ptrFromRef(dPred), dt, v, aConst, a);
  //dPred += d;
  Thyra::Vp_V(Teuchos::ptrFromRef(dPred), d, 1.0);
}

template<class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::
correctVelocity(Thyra::VectorBase<Scalar>& v,
                const Thyra::VectorBase<Scalar>& vPred,
                const Thyra::VectorBase<Scalar>& a,
                const Scalar dt) const
{
  //v = vPred + dt*gamma_*a
  Thyra::V_StVpStV(Teuchos::ptrFromRef(v), 1.0, vPred, dt*gamma_, a);
}


template<class Scalar>
StepperNewmarkExplicitAForm<Scalar>::StepperNewmarkExplicitAForm()
{
  this->setParameterList(Teuchos::null);
  this->modelWarning();
}


template<class Scalar>
StepperNewmarkExplicitAForm<Scalar>::StepperNewmarkExplicitAForm(
  const Teuchos::RCP<const Thyra::ModelEvaluator<Scalar> >& appModel,
  Teuchos::RCP<Teuchos::ParameterList> pList)
{
  this->setParameterList(pList);

  if (appModel == Teuchos::null) {
    this->modelWarning();
  }
  else {
    this->setModel(appModel);
    this->initialize();
  }
}


template<class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::initialize()
{
  TEUCHOS_TEST_FOR_EXCEPTION(
    this->appModel_ == Teuchos::null, std::logic_error,
    "Error - Need to set the model, setModel(), before calling "
    "StepperNewmarkExplicitAForm::initialize()\n");

  this->setParameterList(this->stepperPL_);
}

template<class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::setInitialConditions(
  const Teuchos::RCP<SolutionHistory<Scalar> >& solutionHistory)
{
  using Teuchos::RCP;

  int numStates = solutionHistory->getNumStates();

  TEUCHOS_TEST_FOR_EXCEPTION(numStates < 1, std::logic_error,
    "Error - setInitialConditions() needs at least one SolutionState\n"
    "        to set the initial condition.  Number of States = " << numStates);

  if (numStates > 1) {
    RCP<Teuchos::FancyOStream> out = this->getOStream();
    Teuchos::OSTab ostab(out,1,
      "StepperNewmarkExplicitAForm::setInitialConditions()");
    *out << "Warning -- SolutionHistory has more than one state!\n"
         << "Setting the initial conditions on the currentState.\n"<<std::endl;
  }

  RCP<SolutionState<Scalar> > initialState = solutionHistory->getCurrentState();
  RCP<Thyra::VectorBase<Scalar> > x    = initialState->getX();
  RCP<Thyra::VectorBase<Scalar> > xDot = initialState->getXDot();

  // If initialState has x and xDot set, treat them as the initial conditions.
  // Otherwise use the x and xDot from getNominalValues() as the ICs.
  TEUCHOS_TEST_FOR_EXCEPTION(
    !((initialState->getX() != Teuchos::null &&
       initialState->getXDot() != Teuchos::null) ||
      (this->inArgs_.get_x() != Teuchos::null &&
       this->inArgs_.get_x_dot() != Teuchos::null)), std::logic_error,
    "Error - We need to set the initial conditions for x and xDot from\n"
    "        either initialState or appModel_->getNominalValues::InArgs\n"
    "        (but not from a mixture of the two).\n");

  this->inArgs_ = this->appModel_->getNominalValues();
  using Teuchos::rcp_const_cast;
  // Use the x and xDot from getNominalValues() as the ICs.
  if ( initialState->getX() == Teuchos::null ||
       initialState->getXDot() == Teuchos::null ) {
    TEUCHOS_TEST_FOR_EXCEPTION( (this->inArgs_.get_x() == Teuchos::null) ||
      (this->inArgs_.get_x_dot() == Teuchos::null), std::logic_error,
      "Error - setInitialConditions() needs the ICs from the SolutionHistory\n"
      "        or getNominalValues()!\n");
    x    =rcp_const_cast<Thyra::VectorBase<Scalar> >(this->inArgs_.get_x());
    initialState->setX(x);
    xDot =rcp_const_cast<Thyra::VectorBase<Scalar> >(this->inArgs_.get_x_dot());
    initialState->setXDot(xDot);
  } else {
    this->inArgs_.set_x(x);
    this->inArgs_.set_x_dot(xDot);
  }

  // Check if we need Stepper storage for xDotDot
  if (initialState->getXDotDot() == Teuchos::null)
    initialState->setXDotDot(initialState->getX()->clone_v());

  // Perform IC Consistency
  std::string icConsistency = this->getICConsistency();
  if (icConsistency == "None") {
    if (initialState->getXDotDot() == Teuchos::null) {
      RCP<Teuchos::FancyOStream> out = this->getOStream();
      Teuchos::OSTab ostab(out,1,"StepperForwardEuler::setInitialConditions()");
      *out << "Warning -- Requested IC consistency of 'None' but\n"
           << "           initialState does not have an xDotDot.\n"
           << "           Setting a 'Consistent' xDotDot!\n" << std::endl;
      this->evaluateExplicitODE(initialState->getXDotDot(), x, xDot,
                                initialState->getTime());

      initialState->setIsSynced(true);
    }
  }
  else if (icConsistency == "Zero")
    Thyra::assign(initialState->getXDotDot().ptr(), Scalar(0.0));
  else if (icConsistency == "App") {
    auto xDotDot = Teuchos::rcp_const_cast<Thyra::VectorBase<Scalar> >(
                     this->inArgs_.get_x_dot_dot());
    TEUCHOS_TEST_FOR_EXCEPTION(xDotDot == Teuchos::null, std::logic_error,
      "Error - setInitialConditions() requested 'App' for IC consistency,\n"
      "        but 'App' returned a null pointer for xDotDot!\n");
    Thyra::assign(initialState->getXDotDot().ptr(), *xDotDot);
  }
  else if (icConsistency == "Consistent") {
    // Evaluate xDotDot = f(x,t).
    this->evaluateExplicitODE(initialState->getXDotDot(), x, xDot,
                              initialState->getTime());

    // At this point, x, xDot and xDotDot are sync'ed or consistent
    // at the same time level for the initialState.
    initialState->setIsSynced(true);
  }
  else {
    TEUCHOS_TEST_FOR_EXCEPTION(true, std::logic_error,
      "Error - setInitialConditions() invalid IC consistency, "
      << icConsistency << ".\n");
  }

  // Test for consistency.
  if (this->getICConsistencyCheck()) {
    auto xDotDot = initialState->getXDotDot();
    auto f       = initialState->getX()->clone_v();
    this->evaluateExplicitODE(f, x, xDot, initialState->getTime());
    Thyra::Vp_StV(f.ptr(), Scalar(-1.0), *(xDotDot));
    Scalar reldiff = Thyra::norm(*f);
    Scalar normxDotDot = Thyra::norm(*xDotDot); 
    //The following logic is to prevent FPEs  
    Scalar eps = Scalar(100.0)*std::abs(Teuchos::ScalarTraits<Scalar>::eps());
    if (normxDotDot > eps*reldiff) reldiff /= normxDotDot;  

    if (reldiff > eps) {
      RCP<Teuchos::FancyOStream> out = this->getOStream();
      Teuchos::OSTab ostab(out,1,"StepperForwardEuler::setInitialConditions()");
      *out << "Warning -- Failed consistency check but continuing!\n"
         << "  ||xDotDot-f(x,t)||/||xDotDot|| > eps" << std::endl
         << "  ||xDotDot-f(x,t)||             = " << Thyra::norm(*f)
         << std::endl
         << "  ||xDotDot||                    = " << Thyra::norm(*xDotDot)
         << std::endl
         << "  ||xDotDot-f(x,t)||/||xDotDot|| = " << reldiff << std::endl
         << "                             eps = " << eps     << std::endl;
    }
  }
}


template<class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::takeStep(
  const Teuchos::RCP<SolutionHistory<Scalar> >& solutionHistory)
{
  using Teuchos::RCP;

  TEMPUS_FUNC_TIME_MONITOR("Tempus::StepperNewmarkExplicitAForm::takeStep()");
  {
    TEUCHOS_TEST_FOR_EXCEPTION(solutionHistory->getNumStates() < 2,
      std::logic_error,
      "Error - StepperNewmarkExplicitAForm<Scalar>::takeStep(...)\n"
      "Need at least two SolutionStates for NewmarkExplicitAForm.\n"
      "  Number of States = " << solutionHistory->getNumStates() << "\n"
      "Try setting in \"Solution History\" \"Storage Type\" = \"Undo\"\n"
      "  or \"Storage Type\" = \"Static\" and \"Storage Limit\" = \"2\"\n");

    RCP<SolutionState<Scalar> > currentState=solutionHistory->getCurrentState();
    RCP<SolutionState<Scalar> > workingState=solutionHistory->getWorkingState();

    //Get values of d, v and a from previous step
    RCP<const Thyra::VectorBase<Scalar> > d_old = currentState->getX();
    RCP<const Thyra::VectorBase<Scalar> > v_old = currentState->getXDot();
    RCP<      Thyra::VectorBase<Scalar> > a_old = currentState->getXDotDot();

    //Get dt and time
    const Scalar dt = workingState->getTimeStep();
    const Scalar time_old = currentState->getTime();

    if ( !(this->getUseFSAL()) ) {
      // Evaluate xDotDot = f(x, xDot, t).
      this->evaluateExplicitODE(a_old, d_old, v_old, time_old);

      // For UseFSAL=false, x and xDot sync'ed or consistent
      // at the same time level for the currentState.
      currentState->setIsSynced(true);
    }

    //New d, v and a to be computed here
    RCP<Thyra::VectorBase<Scalar> > d_new = workingState->getX();
    RCP<Thyra::VectorBase<Scalar> > v_new = workingState->getXDot();
    RCP<Thyra::VectorBase<Scalar> > a_new = workingState->getXDotDot();

    //compute displacement and velocity predictors
    predictDisplacement(*d_new, *d_old, *v_old, *a_old, dt);
    predictVelocity(*v_new, *v_old, *a_old, dt);

    // Evaluate xDotDot = f(x, xDot, t).
    this->evaluateExplicitODE(a_new, d_new, v_new, time_old);

    // Set xDot in workingState to velocity corrector
    correctVelocity(*v_new, *v_new, *a_new, dt);

    if ( this->getUseFSAL() ) {
      // Evaluate xDotDot = f(x, xDot, t).
      const Scalar time_new = workingState->getTime();
      this->evaluateExplicitODE(a_new, d_new, v_new, time_new);

      // For UseFSAL=true, x, xDot and xDotxDot are now sync'ed or consistent
      // for the workingState.
      workingState->setIsSynced(true);
    } else {
      assign(a_new.ptr(), Teuchos::ScalarTraits<Scalar>::zero());
      workingState->setIsSynced(false);
    }

    workingState->setSolutionStatus(Status::PASSED);
    workingState->setOrder(this->getOrder());
  }
  return;
}


/** \brief Provide a StepperState to the SolutionState.
 *  This Stepper does not have any special state data,
 *  so just provide the base class StepperState with the
 *  Stepper description.  This can be checked to ensure
 *  that the input StepperState can be used by this Stepper.
 */
template<class Scalar>
Teuchos::RCP<Tempus::StepperState<Scalar> > StepperNewmarkExplicitAForm<Scalar>::
getDefaultStepperState()
{
  Teuchos::RCP<Tempus::StepperState<Scalar> > stepperState =
    rcp(new StepperState<Scalar>(description()));
  return stepperState;
}


template<class Scalar>
std::string StepperNewmarkExplicitAForm<Scalar>::description() const
{
  std::string name = "Newmark Explicit a-Form";
  return(name);
}


template<class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::describe(
   Teuchos::FancyOStream               &out,
   const Teuchos::EVerbosityLevel      /* verbLevel */) const
{
  out << description() << "::describe:" << std::endl
      << "appModel_ = " << this->appModel_->description() << std::endl;
}


template <class Scalar>
void StepperNewmarkExplicitAForm<Scalar>::setParameterList(
  const Teuchos::RCP<Teuchos::ParameterList> & pList)
{
  if (pList == Teuchos::null) {
    // Create default parameters if null, otherwise keep current parameters.
    if (this->stepperPL_ == Teuchos::null)
      this->stepperPL_ = this->getDefaultParameters();
  } else {
    this->stepperPL_ = pList;
  }
  this->stepperPL_->validateParametersAndSetDefaults(*this->getValidParameters());

  std::string stepperType =
    this->stepperPL_->template get<std::string>("Stepper Type");
  TEUCHOS_TEST_FOR_EXCEPTION( stepperType != "Newmark Explicit a-Form",
    std::logic_error,
       "Error - Stepper Type is not 'Newmark Explicit a-Form'!\n"
    << "  Stepper Type = "
    << this->stepperPL_->template get<std::string>("Stepper Type") << "\n");

  gamma_ = this->stepperPL_->sublist("Newmark Explicit Parameters")
                            .template get<double>("Gamma");
  TEUCHOS_TEST_FOR_EXCEPTION( (gamma_ > 1.0) || (gamma_ < 0.0),
    std::logic_error,
    "Error in 'Newmark Explicit a-Form' stepper: invalid value of Gamma = "
     << gamma_ << ".  Please select 0 <= Gamma <= 1. \n");

}


template<class Scalar>
Teuchos::RCP<const Teuchos::ParameterList>
StepperNewmarkExplicitAForm<Scalar>::getValidParameters() const
{
  Teuchos::RCP<Teuchos::ParameterList> pl = Teuchos::parameterList();
  pl->setName("Default Stepper - " + this->description());
  pl->set<std::string>("Stepper Type", "Newmark Explicit a-Form",
                       "'Stepper Type' must be 'Newmark Explicit a-Form'.");
  this->getValidParametersBasic(pl);
  pl->set<bool>("Use FSAL", true);
  pl->set<std::string>("Initial Condition Consistency", "Consistent");
  pl->sublist("Newmark Explicit Parameters", false, "");
  pl->sublist("Newmark Explicit Parameters", false, "").set("Gamma",
               0.5, "Newmark Explicit parameter");

  return pl;
}


template<class Scalar>
Teuchos::RCP<Teuchos::ParameterList>
StepperNewmarkExplicitAForm<Scalar>::getDefaultParameters() const
{
  using Teuchos::RCP;
  using Teuchos::ParameterList;
  using Teuchos::rcp_const_cast;

  RCP<ParameterList> pl =
    rcp_const_cast<ParameterList>(this->getValidParameters());

  return pl;
}


template <class Scalar>
Teuchos::RCP<Teuchos::ParameterList>
StepperNewmarkExplicitAForm<Scalar>::getNonconstParameterList()
{
  return(this->stepperPL_);
}


template <class Scalar>
Teuchos::RCP<Teuchos::ParameterList>
StepperNewmarkExplicitAForm<Scalar>::unsetParameterList()
{
  Teuchos::RCP<Teuchos::ParameterList> temp_plist = this->stepperPL_;
  this->stepperPL_ = Teuchos::null;
  return(temp_plist);
}


} // namespace Tempus
#endif // Tempus_StepperNewmarkExplicitAForm_impl_hpp