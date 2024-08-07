// @HEADER
// *****************************************************************************
//                           Stokhos Package
//
// Copyright 2009 NTESS and the Stokhos contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

#ifndef STOKHOS_PRODUCT_EPETRA_VECTOR_HPP
#define STOKHOS_PRODUCT_EPETRA_VECTOR_HPP

#include "Stokhos_ProductContainer.hpp"
#include "Stokhos_VectorOrthogPolyTraitsEpetra.hpp"
#include "Epetra_BlockMap.h"
#include "EpetraExt_MultiComm.h"
#include "EpetraExt_BlockMultiVector.h"

namespace Stokhos {

  /*! 
   * \brief A container class for products of Epetra_Vector's.  
   */
  class ProductEpetraVector : 
    public virtual ProductContainer<Epetra_Vector> {
  public:

    //! Typename of values
    typedef double value_type;

    //! Typename of ordinals
    typedef int ordinal_type;

    //! Default constructor
    /*!
     * Use with care!  Generally you will want to call reset() before using
     * any of the methods on this class.
     */
    ProductEpetraVector();

    /*! 
     * \brief Create a container with container map \c block_map
     */
    ProductEpetraVector(const Teuchos::RCP<const Epetra_BlockMap>& block_map);

    /*! 
     * \brief Create a container with container map \c block_map where each 
     * coefficient is generated from the supplied coefficient map \c coeff_map
     */
    ProductEpetraVector(
      const Teuchos::RCP<const Epetra_BlockMap>& block_map,
      const Teuchos::RCP<const Epetra_BlockMap>& coeff_map,
      const Teuchos::RCP<const EpetraExt::MultiComm>& product_comm);

    /*! 
     * \brief Create a container with container map \c block_map where each 
     * coefficient is generated from the supplied coefficient map \c coeff_map
     */
    /*
     * This version supplies the generated product map \c product_map
     */
    ProductEpetraVector(
      const Teuchos::RCP<const Epetra_BlockMap>& block_map,
      const Teuchos::RCP<const Epetra_BlockMap>& coeff_map,
      const Teuchos::RCP<const Epetra_BlockMap>& product_map,
      const Teuchos::RCP<const EpetraExt::MultiComm>& product_comm);

    /*! 
     * \brief Create a container with container map \c block_map where each 
     * coefficient is given by the supplied block vector.
     */
    ProductEpetraVector(
      const Teuchos::RCP<const Epetra_BlockMap>& block_map,
      const Teuchos::RCP<const Epetra_BlockMap>& coeff_map,
      const Teuchos::RCP<const Epetra_BlockMap>& product_map,
      const Teuchos::RCP<const EpetraExt::MultiComm>& product_comm,
      Epetra_DataAccess CV,
      const Epetra_Vector& block_vector);
    
    //! Copy constructor
    /*!
     * NOTE:  This is a shallow copy
     */
    ProductEpetraVector(const ProductEpetraVector& v);

    //! Destructor
    virtual ~ProductEpetraVector();

    //! Assignment
    /*!
     * NOTE:  This is a shallow copy
     */
    ProductEpetraVector& operator=(const ProductEpetraVector& v);

    //! Assignment
    ProductEpetraVector& operator=(const Epetra_Vector& v);

    //! Assignment
    void assignToBlockVector(Epetra_Vector& v) const;

    //! Assignment
    void assignFromBlockVector(const Epetra_Vector& v);

    //! Get coefficient map
    Teuchos::RCP<const Epetra_BlockMap> coefficientMap() const;

    //! Get product map
    Teuchos::RCP<const Epetra_BlockMap> productMap() const;

    //! Get product comm
    Teuchos::RCP<const EpetraExt::MultiComm> productComm() const;

    //! Reset to a new size
    /*!
     * This resizes array to fit new size.
     */
    void reset(
      const Teuchos::RCP<const Epetra_BlockMap>& block_map,
      const Teuchos::RCP<const Epetra_BlockMap>& coeff_map,
      const Teuchos::RCP<const EpetraExt::MultiComm>& product_comm);

    //! Reset to a new size
    /*!
     * This resizes array to fit new size.
     */
    void reset(
      const Teuchos::RCP<const Epetra_BlockMap>& block_map,
      const Teuchos::RCP<const Epetra_BlockMap>& coeff_map,
      const Teuchos::RCP<const Epetra_BlockMap>& product_map,
      const Teuchos::RCP<const EpetraExt::MultiComm>& product_comm);

    //! Reset vector cofficients
    void resetCoefficients(Epetra_DataAccess CV,
			   const Epetra_Vector& block_vector);

    //! Get block vector
    Teuchos::RCP<EpetraExt::BlockVector> getBlockVector();

    //! Get block vector
    Teuchos::RCP<const EpetraExt::BlockVector> getBlockVector() const;

    //! Set block vector
    void setBlockVector(const Teuchos::RCP<EpetraExt::BlockVector>& block_vec);

    //! Sum coefficients across processors, storing result in this
    /*!
     * This is only appropriate for local-replicated parallel maps
     */
    void sumAll();

  protected:

    //! Product map of block vector
    Teuchos::RCP<const Epetra_BlockMap> coeff_map;

    //! Product multi-level communicator
    Teuchos::RCP<const EpetraExt::MultiComm> product_comm;

    //! Product map of block vector
    Teuchos::RCP<const Epetra_BlockMap> product_map;

    //! Block vector storing coefficients
    Teuchos::RCP<EpetraExt::BlockVector> bv;    

  }; // class ProductEpetraVector

} // end namespace Stokhos

#endif  // STOKHOS_PRODUCT_EPETRA_VECTOR_HPP
