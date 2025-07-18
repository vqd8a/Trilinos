// @HEADER
// *****************************************************************************
//        MueLu: A package for multigrid based preconditioning
//
// Copyright 2012 NTESS and the MueLu contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef MUELU_INITIALBLOCKNUMBER_FACTORY_DEF_HPP
#define MUELU_INITIALBLOCKNUMBER_FACTORY_DEF_HPP

#include "Xpetra_VectorFactory.hpp"
#include "Xpetra_Map.hpp"
#include "Xpetra_Matrix.hpp"
#include "Xpetra_IO.hpp"

#include "MueLu_InitialBlockNumberFactory_decl.hpp"

#include "MueLu_MasterList.hpp"
#include "MueLu_Level.hpp"
#include "MueLu_Monitor.hpp"

namespace MueLu {

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<const ParameterList> InitialBlockNumberFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::GetValidParameterList() const {
  RCP<ParameterList> validParamList = rcp(new ParameterList());

#define SET_VALID_ENTRY(name) validParamList->setEntry(name, MasterList::getEntry(name))
  SET_VALID_ENTRY("aggregation: block diagonal: interleaved blocksize");
#undef SET_VALID_ENTRY

  validParamList->set<RCP<const FactoryBase>>("A", Teuchos::null, "Generating factory of the matrix A");
  validParamList->set<RCP<const FactoryBase>>("BlockNumber", Teuchos::null, "Generating factory of the BlockNumber vector");

  return validParamList;
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void InitialBlockNumberFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::DeclareInput(Level& currentLevel) const {
  Input(currentLevel, "A");

  if (currentLevel.GetLevelID() == 0 && currentLevel.IsAvailable("BlockNumber", NoFactory::get()))
    Input(currentLevel, "BlockNumber");
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void InitialBlockNumberFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node>::Build(Level& currentLevel) const {
  FactoryMonitor m(*this, "Build", currentLevel);
  const ParameterList& pL = GetParameterList();

  RCP<LocalOrdinalVector> BlockNumber;

  if (currentLevel.IsAvailable("BlockNumber", NoFactory::get())) {
    // Set already provided BlockNumber
    BlockNumber = Get<RCP<LocalOrdinalVector>>(currentLevel, "BlockNumber");
    GetOStream(Statistics1) << "Use user-given blocknumber with length=" << BlockNumber->getGlobalLength() << std::endl;
  } else {
    // Creating interleaved BlockNumber from scratch
    RCP<Matrix> A = Get<RCP<Matrix>>(currentLevel, "A");
    LO blocksize  = as<LO>(pL.get<int>("aggregation: block diagonal: interleaved blocksize"));

    GetOStream(Statistics1) << "Generating new interleaved blocking with " << blocksize << " equations" << std::endl;
    BlockNumber                   = LocalOrdinalVectorFactory::Build(A->getRowMap(), false);
    Teuchos::ArrayRCP<LO> bn_data = BlockNumber->getDataNonConst(0);
    for (LO i = 0; i < (LO)A->getRowMap()->getLocalNumElements(); i++)
      bn_data[i] = i % blocksize;
  }

  Set(currentLevel, "BlockNumber", BlockNumber);
}

}  // namespace MueLu

#endif  // MUELU_INITIALBLOCKNUMBER_FACTORY_DEF_HPP
