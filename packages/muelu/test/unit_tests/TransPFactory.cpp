// @HEADER
// *****************************************************************************
//        MueLu: A package for multigrid based preconditioning
//
// Copyright 2012 NTESS and the MueLu contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#include <Teuchos_UnitTestHarness.hpp>
#include <Teuchos_ScalarTraits.hpp>

#include <Xpetra_MultiVector.hpp>
#include <Xpetra_MultiVectorFactory.hpp>

#include <MueLu_TestHelpers.hpp>
#include <MueLu_Version.hpp>

#include <MueLu_TransPFactory.hpp>
#include <MueLu_SaPFactory.hpp>

namespace MueLuTests {

TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(TransPFactory, Constructor, Scalar, LocalOrdinal, GlobalOrdinal, Node) {
#include <MueLu_UseShortNames.hpp>
  MUELU_TESTING_SET_OSTREAM;
  MUELU_TESTING_LIMIT_SCOPE(Scalar, GlobalOrdinal, Node);
  out << "version: " << MueLu::Version() << std::endl;

  RCP<TransPFactory> transPFact = rcp(new TransPFactory);
  TEST_EQUALITY(transPFact != Teuchos::null, true);

  out << *transPFact << std::endl;
}

TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL(TransPFactory, Correctness, Scalar, LocalOrdinal, GlobalOrdinal, Node) {
#include <MueLu_UseShortNames.hpp>
  MUELU_TESTING_SET_OSTREAM;
  MUELU_TESTING_LIMIT_SCOPE(Scalar, GlobalOrdinal, Node);
  out << "version: " << MueLu::Version() << std::endl;

  RCP<const Teuchos::Comm<int> > comm = TestHelpers::Parameters::getDefaultComm();

  Level fineLevel, coarseLevel;
  TestHelpers::TestFactory<SC, LO, GO, NO>::createTwoLevelHierarchy(fineLevel, coarseLevel);

  // Test of createTwoLevelHierarchy: to be moved...
  TEST_EQUALITY(fineLevel.GetLevelID(), 0);
  TEST_EQUALITY(coarseLevel.GetLevelID(), 1);
  // compilation warning TEST_EQUALITY(fineLevel.GetPreviousLevel().get(), NULL);
  // TEST_EQUALITY(coarseLevel.GetPreviousLevel().get(), &fineLevel);
  //  --

  RCP<Matrix> Op = TestHelpers::TestFactory<SC, LO, GO, NO>::Build1DPoisson(27 * comm->getSize());
  fineLevel.Set("A", Op);

  SaPFactory sapFactory;
  TransPFactory transPFact;
  transPFact.SetFactory("P", rcpFromRef(sapFactory));
  coarseLevel.Request(sapFactory);
  coarseLevel.Request(transPFact);

  coarseLevel.Request("P", &sapFactory);
  coarseLevel.Request("R", &transPFact);

  sapFactory.BuildP(fineLevel, coarseLevel);
  transPFact.Build(fineLevel, coarseLevel);

  RCP<Matrix> P = coarseLevel.Get<RCP<Matrix> >("P", &sapFactory);
  RCP<Matrix> R = coarseLevel.Get<RCP<Matrix> >("R", &transPFact);

  RCP<MultiVector> result1 = MultiVectorFactory::Build(P->getDomainMap(), 1);
  RCP<MultiVector> result2 = MultiVectorFactory::Build(R->getRangeMap(), 1);
  RCP<MultiVector> X       = MultiVectorFactory::Build(P->getRangeMap(), 1);
  X->randomize();

  // Calculate P^T * X
  P->apply(*X, *result1, Teuchos::TRANS, (SC)1.0, (SC)0.0);
  // Calculate R * X
  R->apply(*X, *result2, Teuchos::NO_TRANS, (SC)1.0, (SC)0.0);

  Teuchos::Array<typename Teuchos::ScalarTraits<SC>::magnitudeType> normX(1), normResult1(1), normResult2(1);
  X->norm2(normX);
  out << "This test checks the correctness of the R created by TransPFactory." << std::endl;
  out << "||X||_2 = " << normX << std::endl;
  result1->norm2(normResult1);
  result2->norm2(normResult2);
  TEST_FLOATING_EQUALITY(normResult1[0], normResult2[0], 1e-12);

}  // Correctness test

#define MUELU_ETI_GROUP(Scalar, LocalOrdinal, GlobalOrdinal, Node)                                            \
  TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(TransPFactory, Constructor, Scalar, LocalOrdinal, GlobalOrdinal, Node) \
  TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT(TransPFactory, Correctness, Scalar, LocalOrdinal, GlobalOrdinal, Node)

#include <MueLu_ETI_4arg.hpp>

}  // namespace MueLuTests
