// @HEADER
// *****************************************************************************
//                           Intrepid2 Package
//
// Copyright 2007 NTESS and the Intrepid2 contributors.
// SPDX-License-Identifier: BSD-3-Clause
// *****************************************************************************
// @HEADER

/** \file   Intrepid2_HVOL_TRI_Cn_FEM.hpp
    \brief  Header file for the Intrepid2::Basis_HVOL_TRI_Cn_FEM class.
    \author Created by M. Perego, based on the Intrepid2::HGRAD_TRI_Cn_FEM class
*/

#ifndef __INTREPID2_HVOL_TRI_CN_FEM_HPP__
#define __INTREPID2_HVOL_TRI_CN_FEM_HPP__

#include "Intrepid2_Basis.hpp"

#include "Intrepid2_PointTools.hpp"
#include "Teuchos_LAPACK.hpp"

namespace Intrepid2 {

  /** \class  Intrepid2::Basis_HVOL_TRI_Cn_FEM
      \brief  Implementation of the default HVOL-compatible Lagrange basis of arbitrary degree  on Triangle cell

      Implements Lagrangian basis of degree n on the reference Triangle cell. The basis has
      cardinality (n+1)(n+2)/2 and spans a COMPLETE polynomial space of degree n.
      Basis functions are dual to a unisolvent set of degrees-of-freedom (DoF) defined on a lattice of order n (see PointTools).
      In particular, the degrees of freedom are point evaluation at points in the interior of the Triangle.

      The distribution of these points is specified by the pointType argument to the class constructor.
      Currently, either equispaced lattice points or Warburton's warp-blend points are available.

      The dof are enumerated according to the ordering on the lattice (see PointTools).
  */

  namespace Impl {

    /**
      \brief See Intrepid2::Basis_HVOL_TRI_Cn_FEM
    */
    class Basis_HVOL_TRI_Cn_FEM {
    public:
      typedef struct Triangle<3> cell_topology_type;
      /**
        \brief See Intrepid2::Basis_HVOL_TRI_Cn_FEM
      */
      template<EOperator opType>
      struct Serial {
        template<typename outputValueViewType,
                 typename inputPointViewType,
                 typename workViewType,
                 typename vinvViewType>
        KOKKOS_INLINE_FUNCTION
        static void
        getValues(       outputValueViewType outputValues,
                   const inputPointViewType  inputPoints,
                         workViewType        work,
                   const vinvViewType        vinv );
                   
                   
        KOKKOS_INLINE_FUNCTION
        static ordinal_type
        getWorkSizePerPoint(ordinal_type order) {
          auto cardinality = getPnCardinality<2>(order);
          switch (opType) {
          case OPERATOR_GRAD:
          case OPERATOR_CURL:
          case OPERATOR_D1:
            return 5*cardinality;
          default:
            return getDkCardinality<opType,2>()*cardinality;
          }
        }
      };

      template<typename DeviceType, ordinal_type numPtsPerEval,
               typename outputValueValueType, class ...outputValueProperties,
               typename inputPointValueType,  class ...inputPointProperties,
               typename vinvValueType,        class ...vinvProperties>
      static void
      getValues(        Kokkos::DynRankView<outputValueValueType,outputValueProperties...> outputValues,
                  const Kokkos::DynRankView<inputPointValueType, inputPointProperties...>  inputPoints,
                  const Kokkos::DynRankView<vinvValueType,       vinvProperties...>        vinv,
                  const EOperator operatorType);

      /**
        \brief See Intrepid2::Basis_HVOL_TRI_Cn_FEM
      */
      template<typename outputValueViewType,
               typename inputPointViewType,
               typename vinvViewType,
               typename workViewType,
               EOperator opType,
               ordinal_type numPtsEval>
      struct Functor {
              outputValueViewType _outputValues;
        const inputPointViewType  _inputPoints;
        const vinvViewType        _vinv;
              workViewType        _work;

        KOKKOS_INLINE_FUNCTION
        Functor(       outputValueViewType outputValues_,
                       inputPointViewType  inputPoints_,
                       vinvViewType        vinv_,
                       workViewType        work_)
          : _outputValues(outputValues_), _inputPoints(inputPoints_),
            _vinv(vinv_), _work(work_) {}

        KOKKOS_INLINE_FUNCTION
        void operator()(const size_type iter) const {
          const auto ptBegin = Util<ordinal_type>::min(iter*numPtsEval,    _inputPoints.extent(0));
          const auto ptEnd   = Util<ordinal_type>::min(ptBegin+numPtsEval, _inputPoints.extent(0));

          const auto ptRange = Kokkos::pair<ordinal_type,ordinal_type>(ptBegin, ptEnd);
          const auto input   = Kokkos::subview( _inputPoints, ptRange, Kokkos::ALL() );

          typename workViewType::pointer_type ptr = _work.data() + _work.extent(0)*ptBegin*get_dimension_scalar(_work);

          auto vcprop = Kokkos::common_view_alloc_prop(_work);
          workViewType  work(Kokkos::view_wrap(ptr,vcprop), (ptEnd-ptBegin)*_work.extent(0));

          switch (opType) {
          case OPERATOR_VALUE : {
            auto output = Kokkos::subview( _outputValues, Kokkos::ALL(), ptRange );
            Serial<opType>::getValues( output, input, work, _vinv );
            break;
          }
          case OPERATOR_D1:
          case OPERATOR_D2:  {
            auto output = Kokkos::subview( _outputValues, Kokkos::ALL(), ptRange, Kokkos::ALL() );
            Serial<opType>::getValues( output, input, work, _vinv );
            break;
          }
          default: {
            INTREPID2_TEST_FOR_ABORT( true,
                                      ">>> ERROR: (Intrepid2::Basis_HVOL_TRI_Cn_FEM::Functor) operator is not supported");

          }
          }
        }
      };
    };
  }

  template<typename DeviceType = void,
           typename outputValueType = double,
           typename pointValueType = double>
  class Basis_HVOL_TRI_Cn_FEM
    : public Basis<DeviceType,outputValueType,pointValueType> {
  public:
    using BasisBase = Basis<DeviceType,outputValueType,pointValueType>;
    using HostBasis = Basis_HVOL_TRI_Cn_FEM<typename Kokkos::HostSpace::device_type,outputValueType,pointValueType>;

    using typename BasisBase::OrdinalTypeArray1DHost;
    using typename BasisBase::OrdinalTypeArray2DHost;
    using typename BasisBase::OrdinalTypeArray3DHost;

    using typename BasisBase::OutputViewType;
    using typename BasisBase::PointViewType ;
    using typename BasisBase::ScalarViewType;

    /** \brief  Constructor.
     */
    Basis_HVOL_TRI_Cn_FEM(const ordinal_type order,
                           const EPointType   pointType = POINTTYPE_EQUISPACED);
    
    using scalarType = typename BasisBase::scalarType;
    using BasisBase::getValues;

    virtual
    void
    getValues(       OutputViewType outputValues,
               const PointViewType  inputPoints,
               const EOperator operatorType = OPERATOR_VALUE) const override {
#ifdef HAVE_INTREPID2_DEBUG
      Intrepid2::getValues_HVOL_Args(outputValues,
                                      inputPoints,
                                      operatorType,
                                      this->getBaseCellTopology(),
                                      this->getCardinality() );
#endif
      constexpr ordinal_type numPtsPerEval = Parameters::MaxNumPtsPerBasisEval;
      Impl::Basis_HVOL_TRI_Cn_FEM::
        getValues<DeviceType,numPtsPerEval>( outputValues,
                                                inputPoints,
                                                this->vinv_,
                                                operatorType);
    }

    virtual void 
    getScratchSpaceSize(      ordinal_type& perTeamSpaceSize,
                              ordinal_type& perThreadSpaceSize,
                        const PointViewType inputPoints,
                        const EOperator operatorType = OPERATOR_VALUE) const override;

    KOKKOS_INLINE_FUNCTION
    virtual void 
    getValues(       
          OutputViewType outputValues,
      const PointViewType  inputPoints,
      const EOperator operatorType,
      const typename Kokkos::TeamPolicy<typename DeviceType::execution_space>::member_type& team_member,
      const typename DeviceType::execution_space::scratch_memory_space & scratchStorage, 
      const ordinal_type subcellDim = -1,
      const ordinal_type subcellOrdinal = -1) const override;


    virtual
    void
    getDofCoords( ScalarViewType dofCoords ) const override {
#ifdef HAVE_INTREPID2_DEBUG
      // Verify rank of output array.
      INTREPID2_TEST_FOR_EXCEPTION( dofCoords.rank() != 2, std::invalid_argument,
                                    ">>> ERROR: (Intrepid2::Basis_HVOL_TRI_Cn_FEM::getDofCoords) rank = 2 required for dofCoords array");
      // Verify 0th dimension of output array.
      INTREPID2_TEST_FOR_EXCEPTION( static_cast<ordinal_type>(dofCoords.extent(0)) != this->getCardinality(), std::invalid_argument,
                                    ">>> ERROR: (Intrepid2::Basis_HVOL_TRI_Cn_FEM::getDofCoords) mismatch in number of dof and 0th dimension of dofCoords array");
      // Verify 1st dimension of output array.
      INTREPID2_TEST_FOR_EXCEPTION( dofCoords.extent(1) != this->getBaseCellTopology().getDimension(), std::invalid_argument,
                                    ">>> ERROR: (Intrepid2::Basis_HVOL_TRI_Cn_FEM::getDofCoords) incorrect reference cell (1st) dimension in dofCoords array");
#endif
      Kokkos::deep_copy(dofCoords, this->dofCoords_);
    }

    virtual
    void
    getDofCoeffs( ScalarViewType dofCoeffs ) const override {
#ifdef HAVE_INTREPID2_DEBUG
      // Verify rank of output array.
      INTREPID2_TEST_FOR_EXCEPTION( dofCoeffs.rank() != 1, std::invalid_argument,
                                    ">>> ERROR: (Intrepid2::Basis_HVOL_TRI_Cn_FEM::getdofCoeffs) rank = 1 required for dofCoeffs array");
      // Verify 0th dimension of output array.
      INTREPID2_TEST_FOR_EXCEPTION( static_cast<ordinal_type>(dofCoeffs.extent(0)) != this->getCardinality(), std::invalid_argument,
                                    ">>> ERROR: (Intrepid2::Basis_HVOL_TRI_Cn_FEM::getdofCoeffs) mismatch in number of dof and 0th dimension of dofCoeffs array");
#endif
      Kokkos::deep_copy(dofCoeffs, 1.0);
    }

    void
    getVandermondeInverse( ScalarViewType vinv ) const {
      // has to be same rank and dimensions
      Kokkos::deep_copy(vinv, this->vinv_);
    }

    virtual
    const char*
    getName() const override {
      return "Intrepid2_HVOL_TRI_Cn_FEM";
    }

    virtual
    bool
    requireOrientation() const override {
      return false;
    }

    virtual HostBasisPtr<outputValueType,pointValueType>
    getHostBasis() const override{
      return Teuchos::rcp(new Basis_HVOL_TRI_Cn_FEM<typename Kokkos::HostSpace::device_type,outputValueType,pointValueType>(this->basisDegree_,pointType_));
    }

  private:

    /** \brief inverse of Generalized Vandermonde matrix, whose columns store the expansion
        coefficients of the nodal basis in terms of phis_ */
    Kokkos::DynRankView<scalarType,DeviceType> vinv_;
    EPointType   pointType_;

  };

}// namespace Intrepid2

#include "Intrepid2_HVOL_TRI_Cn_FEMDef.hpp"

#endif
