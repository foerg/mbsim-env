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

#ifndef _FINITE_ELEMENT_1S_21_RCM_H_
#define _FINITE_ELEMENT_1S_21_RCM_H_

#include "mbsimFlexibleBody/discretization_interface.h"
#include "fmatvec/fmatvec.h"

namespace MBSimFlexibleBody {

  /**
   * \brief finite element for planar beam using Redundant Coordinate Method (RCM)
   * \author Roland Zander
   * \author Thorsten Schindler
   * \date 2009-03-23 initial for kernel_dev
   * \date 2009-07-27 implicit integration (Thorsten Schindler)
   * \date 2010-03-07 renamed ElementData to computeAdditionalElementData and wired to class (Roland Zander)
   * \todo transform computeState to Position / Velocity / Orientation / AngularVelocity
   * \todo JacobianMinimalRepresentation
   *
   * model based on
   * Zander, R.; Ulbrich, H.: Reference-free mixed FE-MBS approach for beam structures with constraints, Journal of Nonlinear Dynamics, Kluwer Academic Publishers, 2005
   * Zander, R.; Ulbrich, H.: Impacts on beam structures: Interaction of wave propagationand global dynamics, IUTAM Symposium on Multiscale Problems in Multibody System Contacts Stuttgart, Germany, 2006
   * Zander, R.; Ulbrich, H.: Free plain motion of flexible beams in MBS - A comparison of models, III European Conference on Computational Mechanics Lisbon, Portugal, 2006
   */
  class FiniteElement1s21RCM : public DiscretizationInterface {
    public:
      /*!
       * \brief constructor 
       * \param undeformed lenght of element
       * \param line-density of beam
       * \param longitudinal stiffness
       * \param bending stiffness
       * \param vector of gravitational acceleration
       */
      FiniteElement1s21RCM(double l0_, double  Arho_, double EA_, double EI_, const fmatvec::Vec &g_);

      /**
       * \destructor
       */
       ~FiniteElement1s21RCM() override = default;

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
      virtual double computePhysicalStrain(const fmatvec::Vec& qElement);
       double computeElasticEnergy(const fmatvec::Vec& qElement) override;
      virtual fmatvec::Vec3 getPosition(const fmatvec::Vec& qElement, double s);
      virtual fmatvec::SqrMat3 getOrientation(const fmatvec::Vec& qElement, double s);
      virtual fmatvec::Vec3 getVelocity (const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);
      virtual fmatvec::Vec3 getAngularVelocity(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);
      virtual fmatvec::Mat getJacobianOfMotion(const fmatvec::Vec& qElement, double s) { return JGeneralized(qElement,s); }
      /***************************************************/
      /*!
       * compute additional informations for element
       */
      fmatvec::Vec computeAdditionalElementData(const fmatvec::Vec &qElement, const fmatvec::Vec &qpElement);

      /* GETTER / SETTER */
      void setCurlRadius(double R);
      void setMaterialDamping(double depsilons);
      void setLehrDamping(double D);
      /***************************************************/

      fmatvec::Vec3 getPositions(const fmatvec::Vec& qElement, double s);

      fmatvec::Vec3 getVelocities(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);

      /**
       * \brief return the JACOBIAN of translation and rotation with respect to generalised internal coordinates
       * \param generalised coordinates
       * \param contour point
       * \return JACOBIAN of translation and rotation with respect to generalised internal coordinates
       */
      fmatvec::Mat JGeneralizedInternal(const fmatvec::Vec& qElement, double s);

      /**
       * \brief return the JACOBIAN of translation and rotation with respect to generalised global coordinates
       * \param generalised coordinates
       * \param contour point
       * \return JACOBIAN of translation and rotation with respect to generalised global coordinates
       */
      fmatvec::Mat JGeneralized (const fmatvec::Vec& qElement, double s);

      /**
       * \brief return the derivative of the JACOBIAN of translation and rotation with respect to generalised global coordinates
       * \param generalised coordinates
       * \param generalised velocities
       * \param contour point
       * \param contour point derivative
       * \return derivative of JACOBIAN of translation and rotation with respect to generalised global coordinates
       */
      fmatvec::Mat JpGeneralized(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s, double sp);

      /**
       * \brief return some additional element data
       * \param global positions
       * \param global velocities
       * \return elongation, elongational velocity, cog position, cog velocity, bending angle sum, bending velocity sum
       */
      fmatvec::Vec ElementData(fmatvec::Vec qElement, fmatvec::Vec qpElement);

    protected:
      /** 
       * \brief length, line-density, longitudinal and bending stiffness
       */
      double l0, Arho, EA, EI;

      /**
       * \brief predefined bending curvature
       */
      double wss0;

      /**
       * \brief longitudinal damping
       */
      double depsilon;

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
       * \brief derivative of right hand side with respect to positions and velocities
       */
      fmatvec::SqrMat Dhq, Dhqp;

      /**
       * \brief damping matrix
       */
      fmatvec::SqrMat Damp;

      /**
       * \brief internal position and velocities as well as smooth right hand side
       */
      fmatvec::Vec qLocal, qpLocal, hIntermediate;

      /**
       * \brief local mass matrix
       */
      fmatvec::SymMat MLocal;

      /**
       * \brief transformation global -> internal coordinates coordinates and its derivative
       */
      fmatvec::SqrMat Jeg, Jegp;

      /**
       * \brief global and local state of the last time step 
       */
      fmatvec::Vec qElement_Old, qpElement_Old;

      /**
       * \brief tolerance for comparison of state with old state 
       */
      double tol_comp;

    private:
      /**
       * \brief calculates the local beam coordinates
       * \param global coordinates
       * \param local coordinates
       */
      void BuildqLocal(const fmatvec::Vec& qGlobal, fmatvec::Vec& qLocal);

      /**
       * \brief calculates the JACOBIAN of transformation
       * \param local beam coordinates
       * \param JACOBIAN ot transformation
       */
      void BuildJacobi(const fmatvec::Vec& qLocal, fmatvec::SqrMat& Jeg);

      /**
       * \brief calculates the JACOBIAN of transformation and its time derivative 
       * \param local beam positions
       * \param local beam velocities
       * \param JACOBIAN of transformation
       * \param time derivative of JACOBIAN of transformation
       */
      void BuildJacobi(const fmatvec::Vec& qLocal, const fmatvec::Vec& qpIntern, fmatvec::SqrMat& Jeg, fmatvec::SqrMat& Jegp);

      /** 
       * \brief calculates Cartesian position
       * \param local positions
       * \param contour point
       * \return Cartesian position
       */
      fmatvec::Vec3 evalLocalPositions(const fmatvec::Vec& qLocal, double s);

      /**
       * \brief calculates Cartesian velocity
       * \param local positions
       * \param local velocities
       * \param contour point
       * \return Cartesian velocity
       */
      fmatvec::Vec3 evalLocalVelocities(const fmatvec::Vec& qLocal, const fmatvec::Vec& qpLocal, double s);

      /**
       * \brief calculates JACOBIAN of implicit integration
       * \param global positions
       * \param global velocities
       * \param local positions
       * \param local velocities
       * \param JACOBIAN of transformation
       * \param derivative of JACOBIAN of transformation
       * \param local mass matrix
       * \param local right hand side
       * \return JACOBIAN for implicit integration
       */
      fmatvec::Mat hFullJacobi(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, const fmatvec::Vec& qLocal, const fmatvec::Vec& qpLocal, const fmatvec::SqrMat& Jeg, const fmatvec::SqrMat& Jegp, const fmatvec::SymMat& MLocal, const fmatvec::Vec& hIntermediate);

      /**
       * \brief powers of the beam length
       */
      double l0h2, l0h3, l0h4, l0h5, l0h7, l0h8;
      
      /**
       * \brief constructor is declared private
       */
      FiniteElement1s21RCM() = default;;
  };

  inline double Sec(double alpha) { return 1.0/cos(alpha); }
  inline double Power(double base, int exponent) { return pow(base,exponent); }
}

#endif /* _FINITE_ELEMENT_1S_21_RCM_H_ */

