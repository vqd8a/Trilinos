// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_TimeMonitor.hpp"
#include "Teuchos_DefaultComm.hpp"

#include "Thyra_VectorStdOps.hpp"

#include "Tempus_IntegratorBasic.hpp"
#include "Tempus_StepperExplicitRK.hpp"

#include "../TestModels/SinCosModel.hpp"
#include "../TestModels/VanDerPolModel.hpp"
#include "../TestUtils/Tempus_ConvergenceTestUtils.hpp"

#include <fstream>
#include <vector>

namespace Tempus_Unit_Test {

using Teuchos::RCP;
using Teuchos::rcp;
using Teuchos::rcp_const_cast;
using Teuchos::ParameterList;
using Teuchos::sublist;
using Teuchos::getParametersFromXmlFile;

using Tempus::IntegratorBasic;
using Tempus::SolutionHistory;
using Tempus::SolutionState;

// Comment out any of the following tests to exclude from build/run.
#define SETTING_PARAMETERS


#ifdef SETTING_PARAMETERS
// ************************************************************
// ************************************************************
TEUCHOS_UNIT_TEST(ExplicitRKUnitTest, SETTING_PARAMETERS)
{
  // Default Stepper construction -----------------------------
  auto stepper = rcp(new Tempus::StepperExplicitRK<double>());
  auto defaultPL = stepper->getParameterList();

  auto tableau = rcp(new Tempus::ExplicitBogackiShampine32_RKBT<double>());

  stepper->setTableau(tableau);
  auto setPL = stepper->getParameterList();

  stepper->initialize();
  auto initializePL = stepper->getParameterList();


}
#endif // SETTING_PARAMETERS


} // namespace Tempus_Test
