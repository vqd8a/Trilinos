// @HEADER
// *****************************************************************************
//             Xpetra: A linear algebra interface package
//
// Copyright 2012 NTESS and the Xpetra contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef XPETRA_MAPFACTORY_DEF_HPP
#define XPETRA_MAPFACTORY_DEF_HPP

#include "Teuchos_CompilerCodeTweakMacros.hpp"
#include "Teuchos_ENull.hpp"
#include "Xpetra_MapFactory_decl.hpp"

#ifdef HAVE_XPETRA_TPETRA
#include "Xpetra_TpetraMap.hpp"
#endif
#ifdef HAVE_XPETRA_EPETRA
#include "Xpetra_EpetraMap.hpp"
#endif

#include "Xpetra_BlockedMap.hpp"

namespace Xpetra {

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    Build(UnderlyingLib lib,
          global_size_t numGlobalElements,
          GlobalOrdinal indexBase,
          const Teuchos::RCP<const Teuchos::Comm<int>>& comm,
          LocalGlobal lg) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra)
    return Teuchos::rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(numGlobalElements, indexBase, comm, lg));
#endif

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    Build(UnderlyingLib lib,
          global_size_t numGlobalElements,
          size_t numLocalElements,
          GlobalOrdinal indexBase,
          const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra)
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(numGlobalElements, numLocalElements, indexBase, comm));
#endif

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    Build(UnderlyingLib lib,
          global_size_t numGlobalElements,
          const Teuchos::ArrayView<const GlobalOrdinal>& elementList,
          GlobalOrdinal indexBase,
          const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra)
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(numGlobalElements, elementList, indexBase, comm));
#endif

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    Build(const Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>& nodeMap,
          const LocalOrdinal numDofPerNode, const GlobalOrdinal gidOffset) {
  XPETRA_MONITOR("MapFactory::Build");

  RCP<const BlockedMap<LocalOrdinal, GlobalOrdinal, Node>> bmap =
      Teuchos::rcp_dynamic_cast<const BlockedMap<LocalOrdinal, GlobalOrdinal, Node>>(nodeMap);

  if (!bmap.is_null()) {
    TEUCHOS_TEST_FOR_EXCEPTION(numDofPerNode != 1, Xpetra::Exceptions::RuntimeError,
                               "Xpetra::MapFactory::Build: When provided a BlockedMap numDofPerNode must set to be one. It is set to "
                                   << numDofPerNode << ".");
    return rcp(new Xpetra::BlockedMap<LocalOrdinal, GlobalOrdinal, Node>(*bmap));
  }

#ifdef HAVE_XPETRA_TPETRA
  LocalOrdinal numLocalElements                       = nodeMap->getLocalNumElements();
  Teuchos::ArrayView<const GlobalOrdinal> oldElements = nodeMap->getLocalElementList();
  Teuchos::Array<GlobalOrdinal> newElements(nodeMap->getLocalNumElements() * numDofPerNode);
  for (LocalOrdinal i = 0; i < numLocalElements; i++) {
    for (LocalOrdinal j = 0; j < numDofPerNode; j++) {
      newElements[i * numDofPerNode + j] = oldElements[i] * numDofPerNode + j + gidOffset;
    }
  }
  if (nodeMap->lib() == UseTpetra) {
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(nodeMap->getGlobalNumElements() * numDofPerNode, newElements, nodeMap->getIndexBase(), nodeMap->getComm()));
  }
#endif

  XPETRA_FACTORY_ERROR_IF_EPETRA(nodeMap->lib());
  XPETRA_FACTORY_END;
  TEUCHOS_UNREACHABLE_RETURN(Teuchos::null);
}

#ifdef HAVE_XPETRA_TPETRA
template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    Build(UnderlyingLib lib,
          global_size_t numGlobalElements,
          const Kokkos::View<const GlobalOrdinal*, typename Node::device_type>& indexList,
          GlobalOrdinal indexBase,
          const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");
  if (lib == UseTpetra)
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(numGlobalElements, indexList, indexBase, comm));
  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}
#endif  // HAVE_XPETRA_TPETRA

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    createLocalMap(UnderlyingLib lib,
                   size_t numElements,
                   const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra) {
    // Pre-ETI code called Tpetra::createLocalMap() but this can result in compile erros
    // when Trilinos is built with multiple node-types, specifically the GCC 4.8.4 PR
    // build generates an error because it would try to match Tpetra::Map objects where
    // Node is Serial in one and OpenMP in the other. See Issue #5672 / PR #5723 for more
    // information.
    // return rcp(new Xpetra::TpetraMap<LocalOrdinal,GlobalOrdinal,Node>(Tpetra::createLocalMapWithNode<LocalOrdinal,GlobalOrdinal,Node>(numElements, comm))); // (old version)
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(Tpetra::createLocalMapWithNode<LocalOrdinal, GlobalOrdinal, Node>(numElements, comm)));
  }
#endif  // HAVE_XPETRA_TPETRA

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    createLocalMapWithNode(UnderlyingLib lib,
                           size_t numElements,
                           const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra) {
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(Tpetra::createLocalMapWithNode<LocalOrdinal, GlobalOrdinal, Node>(numElements, comm)));
  }
#endif  // HAVE_XPETRA_TPETRA

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    createUniformContigMapWithNode(UnderlyingLib lib,
                                   global_size_t numElements,
                                   const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra)
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(
        Tpetra::createUniformContigMapWithNode<LocalOrdinal, GlobalOrdinal, Node>(numElements, comm)));
#endif  // HAVE_XPETRA_TPETRA

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    createUniformContigMap(UnderlyingLib lib,
                           global_size_t numElements,
                           const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra)
    return rcp(new Xpetra::TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(
        Tpetra::createUniformContigMapWithNode<LocalOrdinal, GlobalOrdinal, Node>(numElements, comm)));
#endif  // HAVE_XPETRA_TPETRA

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    createContigMap(UnderlyingLib lib,
                    global_size_t numElements,
                    size_t localNumElements,
                    const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra)
    return rcp(new Xpetra::TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(
        Tpetra::createContigMapWithNode<LocalOrdinal, GlobalOrdinal, Node>(numElements, localNumElements, comm)));
#endif  // HAVE_XPETRA_TPETRA

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    createContigMapWithNode(UnderlyingLib lib,
                            global_size_t numElements,
                            size_t localNumElements,
                            const Teuchos::RCP<const Teuchos::Comm<int>>& comm) {
  XPETRA_MONITOR("MapFactory::Build");

#ifdef HAVE_XPETRA_TPETRA
  if (lib == UseTpetra) {
    return rcp(new TpetraMap<LocalOrdinal, GlobalOrdinal, Node>(
        Tpetra::createContigMapWithNode<LocalOrdinal, GlobalOrdinal, Node>(numElements, localNumElements, comm)));
  }
#endif  // HAVE_XPETRA_TPETRA

  XPETRA_FACTORY_ERROR_IF_EPETRA(lib);
  XPETRA_FACTORY_END;
}

template <class LocalOrdinal, class GlobalOrdinal, class Node>
Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>
MapFactory<LocalOrdinal, GlobalOrdinal, Node>::
    copyMapWithNewComm(const Teuchos::RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>>& oldmap,
                       const Teuchos::RCP<const Teuchos::Comm<int>>& newComm) {
  XPETRA_MONITOR("MapFactory::Build");
  using XMF             = Xpetra::MapFactory<LocalOrdinal, GlobalOrdinal, Node>;
  global_size_t INVALID = Teuchos::OrdinalTraits<global_size_t>::invalid();

  size_t Nlocal         = oldmap->getLocalNumElements();
  global_size_t Nglobal = oldmap->getGlobalNumElements();

  // Sanity check -- if there's no comm, we can't keep elements on the map  (vice versa is OK)
  TEUCHOS_TEST_FOR_EXCEPTION(Nlocal && newComm.is_null(),
                             std::logic_error, "MapFactory::copyMapWithNewComm needs the comm to match the map.");

  // We'll return null if we don't have a Comm on this rank
  RCP<const Map<LocalOrdinal, GlobalOrdinal, Node>> newMap;
  if (!newComm.is_null()) {
    if (oldmap->isContiguous()) {
      newMap = XMF::Build(oldmap->lib(), INVALID, Nlocal, oldmap->getIndexBase(), newComm);
    } else {
      newMap = XMF::Build(oldmap->lib(), Nglobal, oldmap->getLocalElementList(), oldmap->getIndexBase(), newComm);
    }
  }

  return newMap;
  XPETRA_FACTORY_END;
}

}  // namespace Xpetra

#endif  // XPETRA_MAPFACTORY_DEF_HPP

// TODO: remove unused methods
