// @HEADER
// *****************************************************************************
//               Rapid Optimization Library (ROL) Package
//
// Copyright 2014 NTESS and the ROL contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

/*! \file  test_04.cpp
    \brief Unit test for the FE_CURL class.
*/

#include "ROL_Types.hpp"

#include "ROL_Stream.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

#include "Intrepid_HCURL_HEX_I1_FEM.hpp"
#include "Intrepid_DefaultCubatureFactory.hpp"

#include "../TOOLS/meshmanager.hpp"
#include "../TOOLS/dofmanager.hpp"
#include "../TOOLS/fe_curl.hpp"

typedef double RealT;

int main(int argc, char *argv[]) {

  Teuchos::GlobalMPISession mpiSession(&argc, &argv);
  // This little trick lets us print to std::cout only if a (dummy) command-line argument is provided.
  int iprint     = argc - 1;
  ROL::Ptr<std::ostream> outStream;
  ROL::nullstream bhs; // outputs nothing
  if (iprint > 0)
    outStream = ROL::makePtrFromRef(std::cout);
  else
    outStream = ROL::makePtrFromRef(bhs);

  int errorFlag  = 0;

  // *** Example body.
  try {

    /*** Read in XML input ***/
    std::string filename = "input_05.xml";
    Teuchos::RCP<Teuchos::ParameterList> parlist
      = Teuchos::rcp( new Teuchos::ParameterList() );
    Teuchos::updateParametersFromXmlFile( filename, parlist.ptr() );

    /*** Initialize mesh / degree-of-freedom manager. ***/
    MeshManager_Brick<RealT> meshmgr(*parlist);
    ROL::Ptr<Intrepid::FieldContainer<RealT> > nodesPtr = meshmgr.getNodes();
    ROL::Ptr<Intrepid::FieldContainer<int> >   cellToNodeMapPtr = meshmgr.getCellToNodeMap();

    Intrepid::FieldContainer<RealT> &nodes = *nodesPtr;
    Intrepid::FieldContainer<int>   &cellToNodeMap = *cellToNodeMapPtr;
    *outStream << "Number of nodes = " << meshmgr.getNumNodes() << std::endl;
    *outStream << "Number of cells = " << meshmgr.getNumCells() << std::endl;
    *outStream << "Number of edges = " << meshmgr.getNumEdges() << std::endl;

    ROL::Ptr<MeshManager<RealT> > meshmgrPtr = ROL::makePtrFromRef(meshmgr);

    // Basis.
    ROL::Ptr<Intrepid::Basis<RealT, Intrepid::FieldContainer<RealT> > >    basis =
      ROL::makePtr<Intrepid::Basis_HCURL_HEX_I1_FEM<RealT, Intrepid::FieldContainer<RealT> >>();
    // Cubature.
    ROL::Ptr<Intrepid::Cubature<RealT, Intrepid::FieldContainer<RealT> > > cubature;
    shards::CellTopology cellType = basis->getBaseCellTopology();                        // get the cell type from any basis
    Intrepid::DefaultCubatureFactory<RealT> cubFactory;                                  // create cubature factory
    int cubDegree = 4;                                                                   // set cubature degree, e.g., 2
    cubature = cubFactory.create(cellType, cubDegree);                                   // create default cubature
    // Cell nodes.
    Intrepid::FieldContainer<RealT> cellNodes(meshmgr.getNumCells(), cellType.getNodeCount(), cellType.getDimension());
    for (int i=0; i<meshmgr.getNumCells(); ++i) {
      for (unsigned j=0; j<cellType.getNodeCount(); ++j) {
        for (unsigned k=0; k<cellType.getDimension(); ++k) {
          cellNodes(i,j,k) = nodes(cellToNodeMap(i,j), k);
        }
      }
    }
    // FE object.
    FE_CURL<RealT> fe(ROL::makePtrFromRef(cellNodes), basis, cubature);

    // Check integration.
    Intrepid::FieldContainer<RealT> feVals1(meshmgr.getNumCells(), cubature->getNumPoints(), cubature->getDimension());
    Intrepid::FieldContainer<RealT> feVals2(meshmgr.getNumCells(), cubature->getNumPoints(), cubature->getDimension());
    Intrepid::FieldContainer<RealT> feCurls1(meshmgr.getNumCells(), cubature->getNumPoints(), cubature->getDimension());
    Intrepid::FieldContainer<RealT> feCurls2(meshmgr.getNumCells(), cubature->getNumPoints(), cubature->getDimension());
    Intrepid::FieldContainer<RealT> feCoeffs1(meshmgr.getNumCells(), basis->getCardinality());
    Intrepid::FieldContainer<RealT> feCoeffs2(meshmgr.getNumCells(), basis->getCardinality());
    Intrepid::FieldContainer<RealT> feIntegral(meshmgr.getNumCells());
    // values
    feCoeffs1.initialize(static_cast<RealT>(0));
    feCoeffs2.initialize(static_cast<RealT>(0));
    for (int i=0; i<meshmgr.getNumCells(); ++i) {
      feCoeffs1(i, 0) = static_cast<RealT>(1);
      feCoeffs2(i, 0) = static_cast<RealT>(1);
    }
    fe.evaluateValue(ROL::makePtrFromRef(feVals1), ROL::makePtrFromRef(feCoeffs1));
    fe.evaluateValue(ROL::makePtrFromRef(feVals2), ROL::makePtrFromRef(feCoeffs2));
    fe.computeIntegral(ROL::makePtrFromRef(feIntegral), ROL::makePtrFromRef(feVals1), ROL::makePtrFromRef(feVals2));
    RealT valval(0);
    for (int i=0; i<meshmgr.getNumCells(); ++i) {
      valval += feIntegral(i);
    }
    if (std::abs(valval - static_cast<RealT>(1)/static_cast<RealT>(9)) > std::sqrt(ROL::ROL_EPSILON<RealT>())) {
      errorFlag = -1;
    }
    // curls
    fe.evaluateCurl(ROL::makePtrFromRef(feCurls1), ROL::makePtrFromRef(feCoeffs1));
    fe.evaluateCurl(ROL::makePtrFromRef(feCurls2), ROL::makePtrFromRef(feCoeffs2));
    fe.computeIntegral(ROL::makePtrFromRef(feIntegral), ROL::makePtrFromRef(feCurls1), ROL::makePtrFromRef(feCurls2));
    RealT curlcurl(0);
    for (int i=0; i<meshmgr.getNumCells(); ++i) {
      curlcurl += feIntegral(i);
    }
    if (std::abs(curlcurl - static_cast<RealT>(2)/static_cast<RealT>(3)) > std::sqrt(ROL::ROL_EPSILON<RealT>())) {
      errorFlag = -1;
    }
    *outStream << std::setprecision(14) << "\nvalvalIntegral=" << valval << "  "
                                        << "curlcurlIntegral=" << curlcurl << std::endl;

  }
  catch (std::logic_error& err) {
    *outStream << err.what() << "\n";
    errorFlag = -1000;
  }; // end try

  if (errorFlag != 0)
    std::cout << "End Result: TEST FAILED\n";
  else
    std::cout << "End Result: TEST PASSED\n";

  return 0;
}
