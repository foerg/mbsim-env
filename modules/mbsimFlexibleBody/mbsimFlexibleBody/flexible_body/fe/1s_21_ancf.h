/* Copyright (C) 2004-2014 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: thorsten.schindler@mytum.de
 */

#ifndef _FINITE_ELEMENT_1S_21_ANCF_H_
#define _FINITE_ELEMENT_1S_21_ANCF_H_

#include "mbsimFlexibleBody/discretization_interface.h"
#include "fmatvec/fmatvec.h"

namespace MBSimFlexibleBody {

  /**
   * \brief differentiates a vector defined by its norm with respect to the vector
   * \param vector to be differentiated
   * \return Jacobian matrix
   */
  fmatvec::SqrMat differentiate_normalized_vector_respective_vector(const fmatvec::Vec &vector);

  /**
   * \brief finite element for planar beam using Absolute Nodal Coordinate Formulation (ANCF)
   * \author Roland Zander
   * \author Thorsten Schindler
   *
   * \date 2014-02-27 basic revision
   * \date 2014-03-23 damping added
   * \date 2014-05-09 Euler perspective added as an option
   *
   * model based on
   * SHABANA, A. A.: Computer Implementation of the Absolute Nodal Coordinate Formulation for Flexible Multibody Dynamics. In: Nonlinear Dynamics 16 (1998), S. 293-306
   * SHABANA, A. A.: Definition of the Slopes and the Finite Element Absolute Nodal Coordinate Formulation. In: Nonlinear Dynamics 1 (1997), S. 339-348
   * SHABANE, A. A.: Dynamics of Multibody Systems. Cambridge University Press (2005)
   */
  class FiniteElement1s21ANCF : public DiscretizationInterface
  {
    public:
      /*!
       * \brief constructor 
       * \param undeformed lenght of element
       * \param line-density of beam
       * \param longitudinal stiffness
       * \param bending stiffness
       * \param vector of gravitational acceleration
       */
      explicit FiniteElement1s21ANCF(double sl0, double sArho, double sEA, double sEI, const fmatvec::Vec &sg, bool sEuler=false, double sv0=0.);

      /**
       * \destructor
       */
       ~FiniteElement1s21ANCF() override;

      /* INHERITED INTERFACE */
       const fmatvec::SymMat& getM() const override { return M; }
       const fmatvec::Vec& geth() const override { return h; }
       const fmatvec::SqrMat& getdhdq() const override { return Dhq; }
       const fmatvec::SqrMat& getdhdu() const override { return Dhqp; }
       int getqSize() const override { return 8; }
       int getuSize() const override { return 8; }
       void computeM(const fmatvec::Vec& qElement) override;
       void computeh(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement) override;
       void computedhdz(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement) override;
       double computeKineticEnergy(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement) override;
       double computeGravitationalEnergy(const fmatvec::Vec& qElement) override;
       double computeElasticEnergy(const fmatvec::Vec& qElement) override;
      virtual fmatvec::Vec3 getPosition(const fmatvec::Vec& qElement, double s);
      virtual fmatvec::SqrMat3 getOrientation(const fmatvec::Vec& qElement, double s);
      virtual fmatvec::Vec3 getVelocity(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);
      virtual fmatvec::Vec3 getAngularVelocity(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);
      virtual fmatvec::Mat getJacobianOfMotion(const fmatvec::Vec& qElement, double s) { return JGeneralized(qElement,s); }
      /***************************************************/
      
//      /**
//       * compute additional informations for element
//       */
//      fmatvec::Vec computeAdditionalElementData(fmatvec::Vec &qElement, fmatvec::Vec &qpElement);

      /* GETTER / SETTER */
      void setCurlRadius(double R);
      void setMaterialDamping(double depsilon_, double dkappa_);
      /***************************************************/

      /**
       * \brief calculate constant mass matrix
       */
      void initM();

      /**
       * \brief return the planar position and angle at a contour point (Lagrange/Euler)
       * \param generalised coordinates
       * \param contour point (Lagrange/Euler)
       * \return planar position and angle
       */
      fmatvec::Vec LocateBalken(const fmatvec::Vec& qElement, double s);

      /**
       * \brief return the planar state at a contour point (Lagrange/Euler)
       * \param generalised positions
       * \param generalised velocities
       * \param contour point (Lagrange/Euler)
       * \return planar state
       */
      fmatvec::Vec StateBalken(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);

      /**
       * \brief return the JACOBIAN of translation and rotation with respect to generalised coordinates
       * \param generalised coordinates
       * \param contour point (Lagrange/Euler)
       * \return JACOBIAN of translation and rotation with respect to generalised coordinates
       */
      fmatvec::Mat JGeneralized(const fmatvec::Vec& qElement, double s);

      /**
       * \brief return the matrix of global shape functions
       * \param contour point (Lagrange/Euler)
       * \return matrix of global shape functions
       */
      fmatvec::Mat GlobalShapeFunctions(double s);

      /**
       * \brief return 1st derivative of the matrix of global shape functions
       * \param contour point (Lagrange/Euler)
       * \return 1st derivative of the matrix of global shape functions
       */
      fmatvec::Mat GlobalShapeFunctions_1stDerivative(double s);

      /**
       * \brief returns the tangent
       * \param generalised coordinates
       * \param contour point (Lagrange/Euler)
       * \return tangent
       * */
      fmatvec::Vec3 getTangent(const fmatvec::Vec& qElement, double s);

    private:
      /**
       * \brief beam element length
       */
      double l0;

      /** 
       * \brief line-density
       */
      double Arho;

      /**
       * \brief longitudinal stiffness
       */
      double EA;

      /**
       * \brief bending stiffness
       */
      double EI;

      /** 
       * \brief Euler perspective: true if set
       */
      bool Euler;

      /**
       * \brief Euler perspective: constant longitudinal velocity
       */
      double v0;

      /**
       * \brief predefined bending curvature
       */
      double wss0;

      /**
       * \brief longitudinal damping
       */
      double depsilon;

      /**
       * \brief curvature damping
       */
      double dkappa;

      /**
       * \brief gravitation
       */
      fmatvec::Vec g;

      /**
       * \brief mass matrix
       */
      fmatvec::SymMat M;

      /**
       * \brief right hand side
       */
      fmatvec::Vec h;

      /**
       * \brief derivative of right hand side with respect to positions
       */
      fmatvec::SqrMat Dhq;

      /**
       * \brief derivative of right hand side with respect to velocities
       */
      fmatvec::SqrMat Dhqp;
  };

}

#endif /* _FINITE_ELEMENT_1S_21_ANCF_H_ */
