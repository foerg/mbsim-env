/* Copyright (C) 2004-2015 MBSim Development Team
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

#ifndef _FINITE_ELEMENT_1S_33_RCM_H_
#define _FINITE_ELEMENT_1S_33_RCM_H_

#include "mbsimFlexibleBody/discretization_interface.h"
#include "mbsimFlexibleBody/pointer.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_rcm/weight33RCM.h"
#include "fmatvec/fmatvec.h"

namespace MBSimFlexibleBody {

  /**
   * \brief finite element for spatial beam using Redundant Coordinate Method (RCM)
   * \author Thorsten Schindler
   * \date 2009-04-17 initial commit kernel_dev (Thorsten Schindler)
   * \date 2009-07-27 implicit integration (Thorsten Schindler)
   * \todo transform computeState to Position / Velocity / Orientation / AngularVelocity 
   * \todo JacobianOfMotion 
   * \todo computeM 
   */
  class FiniteElement1s33RCM : public DiscretizationInterface {
    public:
      /**
       * \brief constructor
       * \param length of finite element
       * \param density
       * \param cross-sectional area
       * \param Young's modulus
       * \param shear modulus
       * \param first area moment of inertia
       * \param second area moment of inertia
       * \param torsional moment of inertia
       * \param acceleration of gravity
       * \param cardan object
       */
      FiniteElement1s33RCM(double l0_, double rho_, double A_, double E_, double G_, double I1_, double I2_, double I0_, const fmatvec::Vec& g_, RevCardanPtr ag_);

      /* INHERITED INTERFACE OF DISCRETIZATIONINTERFACE */
       const fmatvec::SymMat& getM() const override;		
       const fmatvec::Vec& geth() const override;
       const fmatvec::SqrMat& getdhdq() const override;
       const fmatvec::SqrMat& getdhdu() const override;
       int getqSize() const override;
       int getuSize() const override;

       void computeM(const fmatvec::Vec& qG) override;
       void computeh(const fmatvec::Vec& qG, const fmatvec::Vec& qGt) override;
       void computedhdz(const fmatvec::Vec& qG, const fmatvec::Vec& qGt) override;
       double computeKineticEnergy(const fmatvec::Vec& qG, const fmatvec::Vec& qGt) override;
       double computeGravitationalEnergy(const fmatvec::Vec& qG) override;		
       double computeElasticEnergy(const fmatvec::Vec& qG) override;

      virtual fmatvec::Vec3 getPosition(const fmatvec::Vec& qElement, double s);
      virtual fmatvec::SqrMat3 getOrientation(const fmatvec::Vec& qElement, double s);
      virtual fmatvec::Vec3 getVelocity(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);
      virtual fmatvec::Vec3 getAngularVelocity(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);
      virtual fmatvec::Mat computeJacobianOfMotion(const fmatvec::Vec& qG, double s) { return computeJXqG(qG,s); }
      /*****************************************************/ 

      /* GETTER / SETTER */
      void setGauss(int nGauss);
      void setCurlRadius(double R1, double R2);
      void setMaterialDamping(double epstD_, double k0D_);
      void setLehrDamping(double epstL, double k0L);
      double getl0() const;
      /*****************************************************/

      fmatvec::Vector<fmatvec::Fixed<6>, double> getPositions(const fmatvec::Vec& qElement, double s);

      fmatvec::Vector<fmatvec::Fixed<6>, double> getVelocities(const fmatvec::Vec& qElement, const fmatvec::Vec& qpElement, double s);

      /**
       * \brief compute JACOBIAN of contact description in global coordinates
       * \param global coordinates
       * \param LAGRANGIAN parameter
       */
      fmatvec::Mat computeJXqG(const fmatvec::Vec& qG, double x);

      /**
       * \brief compute interesting data 
       * \param global coordinates
       * \param global velocities
       */
      fmatvec::Vec computeData(const fmatvec::Vec& qG, const fmatvec::Vec& qGt);

      /*!
       * \brief compute the physical strain as defined in Schindler2010, p. 25, eq. (2.52)
       * \param global coordinates
       * \param global velocities
       */
      double computePhysicalStrain(const fmatvec::Vec& qG, const fmatvec::Vec& qGt);

    private:
      /**
       * \brief length of finite element
       */
      double l0;

      /**
       * \brief density
       */
      double rho;

      /**
       * \brief cross sectional area 
       */
      double A;

      /**
       * \brief Young's modulus 
       */
      double E;

      /**
       * \brief shear modulus 
       */
      double G;

      /**
       * \brief geometrical moment of inertia 
       */
      double I1, I2, I0;

      /**
       * \brief gravitation
       */
      fmatvec::Vec g;

      /**
       * \brief predefined bending 
       */
      double k10, k20;

      /**
       * \brief prolongational and torsional damping
       */
      double epstD, k0D;

      /**
       * \brief global system description 
       */
      fmatvec::SymMat M;
      fmatvec::Vec h;

      /**
       * \brief matrices for implicit integration 
       */
      fmatvec::SqrMat dhdq, dhdu;

      /**
       * \brief internal system description 
       */
      fmatvec::SymMat MI;

      /**
       * \brief internal damping matrix
       */
      fmatvec::SqrMat Damp;

      /**
       * \brief beam length powers 
       */
      double l0h2, l0h3;

      /**
       * \brief last Lagrangian coordinate in state calculation 
       */
      double x_Old;

      /**
       * \brief global and local state of the last time step 
       */
      fmatvec::Vec qG_Old, qGt_Old;

      /**
       * \brief tolerance for comparison of state with old state 
       */
      double tol_comp;

      /**
       * \brief delta matrices
       */
      fmatvec::Mat drS, drSH;
      fmatvec::RowVec depstil, dk0;

      /**
       * \brief reversed Cardan-object 
       */
      RevCardanPtr ag;

      /**
       * \brief trafo-object
       */
      Trafo33RCMPtr tf;

      /**
       * \brief weight-function-object
       */
      Weight33RCMPtr wt;

      /**
       * \brief compute delta matrix for CP with respect to translation
       */
      void computedrS();

      /**
       * \brief compute delta matrix for elongation
       */
      void computedepstil();
      
      /**
       * \brief compute delta matrix for torsion
       */
      void computedk0();	
  };

  inline void FiniteElement1s33RCM::setGauss(int nGauss) { wt->setGauss(nGauss); }
  inline double FiniteElement1s33RCM::getl0() const { return l0; }
  inline const fmatvec::SymMat& FiniteElement1s33RCM::getM() const { return M; }
  inline const fmatvec::Vec& FiniteElement1s33RCM::geth() const { return h; }
  inline const fmatvec::SqrMat& FiniteElement1s33RCM::getdhdq() const { return dhdq; }
  inline const fmatvec::SqrMat& FiniteElement1s33RCM::getdhdu() const { return dhdu; }
  inline int FiniteElement1s33RCM::getqSize() const { return 16; }
  inline int FiniteElement1s33RCM::getuSize() const { return 16; }

}

#endif /* _FINITE_ELEMENT_1S_33_RCM_H_ */
