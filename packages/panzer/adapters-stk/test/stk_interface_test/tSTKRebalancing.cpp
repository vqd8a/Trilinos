// @HEADER
// ***********************************************************************
//
//           Panzer: A partial differential equation assembly
//       engine for strongly coupled complex multiphysics systems
//                 Copyright (2011) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Roger P. Pawlowski (rppawlo@sandia.gov) and
// Eric C. Cyr (eccyr@sandia.gov)
// ***********************************************************************
// @HEADER

#include <Teuchos_ConfigDefs.hpp>
#include <Teuchos_UnitTestHarness.hpp>
#include "Teuchos_DefaultComm.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_ParameterList.hpp"

#include "Panzer_STK_Version.hpp"
#include "PanzerAdaptersSTK_config.hpp"
#include "Panzer_STK_Interface.hpp"
#include "Panzer_STK_ExodusReaderFactory.hpp"
#include "Panzer_STK_SetupUtilities.hpp"

#ifdef PANZER_HAVE_PERCEPT
#include "percept/PerceptMesh.hpp"
#endif

#include "stk_mesh/base/GetEntities.hpp"
#include "stk_mesh/base/Selector.hpp"
#include "stk_mesh/base/CreateAdjacentEntities.hpp"

namespace panzer_stk {

// Test to make sure the default option for mesh rebalancing doesn't throw
TEUCHOS_UNIT_TEST(tSTKRebalancing, default)
{
  using namespace Teuchos;
  using Teuchos::RCP;
  using Teuchos::rcp;

  TEST_EQUALITY(Teuchos::DefaultComm<int>::getComm()->getSize(),4);

  {
    out << "\nCreating pamgen mesh" << std::endl;
    RCP<ParameterList> p = parameterList();
    const std::string input_file_name = "pamgen_test.gen";
    p->set("File Name",input_file_name);
    p->set("File Type","Pamgen");
    p->set("Rebalancing","default");

    RCP<STK_ExodusReaderFactory> pamgenFactory = rcp(new STK_ExodusReaderFactory());
    TEST_NOTHROW(pamgenFactory->setParameterList(p));

    RCP<STK_Interface> mesh = pamgenFactory->buildMesh(MPI_COMM_WORLD);
    TEST_ASSERT(mesh!=Teuchos::null);
  }
}

// Test to make sure the none option for mesh rebalancing doesn't throw
TEUCHOS_UNIT_TEST(tSTKRebalancing, none)
{
  using namespace Teuchos;
  using Teuchos::RCP;
  using Teuchos::rcp;

  TEST_EQUALITY(Teuchos::DefaultComm<int>::getComm()->getSize(),4);

  {
    out << "\nCreating pamgen mesh" << std::endl;
    RCP<ParameterList> p = parameterList();
    const std::string input_file_name = "pamgen_test.gen";
    p->set("File Name",input_file_name);
    p->set("File Type","Pamgen");
    p->set("Rebalancing","none");

    RCP<STK_ExodusReaderFactory> pamgenFactory = rcp(new STK_ExodusReaderFactory());
    TEST_NOTHROW(pamgenFactory->setParameterList(p));

    RCP<STK_Interface> mesh = pamgenFactory->buildMesh(MPI_COMM_WORLD);
    TEST_ASSERT(mesh!=Teuchos::null);
  }
}

} // namespace panzer_stk
