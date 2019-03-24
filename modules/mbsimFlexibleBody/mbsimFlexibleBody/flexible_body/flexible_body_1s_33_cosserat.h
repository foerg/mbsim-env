/* Copyright (C) 2004-2012 MBSim Development Team
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

#ifndef _FLEXIBLE_BODY_1S_33_COSSERAT_H_
#define _FLEXIBLE_BODY_1S_33_COSSERAT_H_

#include "flexible_body_1s_cosserat.h"
#include "mbsimFlexibleBody/pointer.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_translation.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_rotation.h"

namespace MBSimFlexibleBody {

  class NodeFrame;

//  class NurbsCurve1s;

  /**
   * \brief finite element for spatial beam using Cosserat model
   * \author Thorsten Schindler
   * \author Christian Käsbauer
   * \author Thomas Cebulla
   * \date 2011-09-10 initial commit (Thorsten Schindler)
   * \data 2011-10-08 basics derived and included (Thorsten Schindler)
   * \date 2011-10-12 rotation grid added (Thorsten Schindler)
   * \date 2012-03-15 updateKinematicsForFrame and contact Jacobians (Cebulla / Schindler)
   * \date 2012-05-10 added initInfo()-function and Contour1sFlexible for perlchain example (Thomas Cebulla)
   * \date 2012-05-25 added export and import position velocity function (Thomas Cebulla)
   * \todo compute boundary conditions TODO
   * \todo check open structure in contact kinematics TODO
   *
   * Cosserat model based on
   * H. Lang, J. Linn, M. Arnold: Multi-body dynamics simulation of geometrically exact Cosserat rods
   * but with 
   *  - Kirchhoff assumption (-> less stiff)
   *  - Cardan parametrisation (-> less problems with condition and drift for quaternion dae system)
   *  - piecewise constant Darboux vector with evaluation according to
   *    I. Romero: The interpolation of rotations and its application to finite element models of
   *    geometrically exact beams
   */
  class FlexibleBody1s33Cosserat : public FlexibleBody1sCosserat {
    public:

      /**
       * \brief constructor
       * \param name of body
       * \param bool to specify open (cantilever) or closed (ring) structure
       */
      FlexibleBody1s33Cosserat(const std::string &name, bool openStructure);

      /**
       * \brief destructor
       */
      virtual ~FlexibleBody1s33Cosserat();

      /* INHERITED INTERFACE OF FLEXIBLE BODY */
      virtual void BuildElements();
      virtual void GlobalVectorContribution(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec);
      virtual void GlobalMatrixContribution(int n, const fmatvec::Mat& locMat, fmatvec::Mat& gloMat);
      virtual void GlobalMatrixContribution(int n, const fmatvec::SymMat& locMat, fmatvec::SymMat& gloMat);
      virtual void exportPositionVelocity(const std::string & filenamePos, const std::string & filenameVel = std::string(), const int & deg = 3, const bool & writePsFile = false);
      virtual void importPositionVelocity(const std::string & filenamePos, const std::string & filenameVel = std::string());
      /***************************************************/

      virtual void updatePositions(Frame1s* frame);
      virtual void updateVelocities(Frame1s* frame);
      virtual void updateAccelerations(Frame1s* frame);
      virtual void updateJacobians(Frame1s* frame, int j=0);
      virtual void updateGyroscopicAccelerations(Frame1s* frame);

      void updatePositions(int node) override;
      void updateVelocities(int node) override;
      void updateAccelerations(int node) override;
      void updateJacobians(int node, int j=0) override;
      void updateGyroscopicAccelerations(int node) override;

      virtual fmatvec::Vec3 getAngles(int i);
      virtual fmatvec::Vec3 getDerivativeOfAngles(int i);

      /* INHERITED INTERFACE OF OBJECT */
      virtual void init(InitStage stage, const MBSim::InitConfigSet &config);
      virtual double computePotentialEnergy();
      /***************************************************/

      /* INHERITED INTERFACE OF OBJECTINTERFACE */
      virtual void updateLLM();

      /* INHERITED INTERFACE OF ELEMENT */
      /***************************************************/

      /* GETTER / SETTER */
      void setNumberElements(int n);

      void setMomentsInertia(double I1_, double I2_, double I0_);

      void setCurlRadius(double R1_, double R2_);
      void setMaterialDamping(double cEps0D_, double cEps1D_, double cEps2D_);

      virtual int getNumberOfElementDOF() const { return 6; }

      int getNumberElements() const { return Elements; }
      int getNumberDOFs() const { return qSize; }
      /***************************************************/

      /**
       * \brief compute positions and angle at Lagrangian coordinate in local FE coordinates
       * \param Lagrangian coordinate
       */
      fmatvec::Vector<fmatvec::Fixed<6>, double> getPositions(double x);

      /**
       * \brief compute velocities and differentiated angles at Lagrangian coordinate in local FE coordinates
       * \param Lagrangian coordinate
       */
      fmatvec::Vector<fmatvec::Fixed<6>, double> getVelocities(double x);

      /**
       * \brief compute angles at Lagrangian coordinate in local FE coordinates
       * \param Lagrangian coordinate
       * \param vector to be interpolated
       */
      fmatvec::Vec3 computeAngles(double sGlobal, const fmatvec::Vec & vec);

      /**
       * \brief initialise beam only for giving information with respect to state, number elements, length, (not for simulation)
       */
      void initInfo();

      /**
       * \brief detect current finite element (t
       void setOpenMBVSpineExtrusion(OpenMBV::SpineExtrusion* spineExtrusion) {
       openMBVSpineExtrusion = spineExtrusion;
       }
       OpenMBV::Body* getOpenMBVSpineExtrusion() {
       return openMBVSpineExtrusion;
       }
       * \param global parametrisation
       * \param local parametrisation
       * \param finite element number
       */
      void BuildElementTranslation(const double& sGlobal, double& sLocal, int& currentElementTranslation);

    protected:

      /*!
       * \brief marker if Jacobians already interpolated
       */
      bool JTransInterp;

      /**
       * \brief area moments of inertia
       * I0: around torsional axis
       * I1: in t-b-plane
       * I2: in t-n-plane
       */
      double I2, I0;

      /**
       * \brief radius of undeformed shape
       * R1: in t-b-plane
       * R2: in t-n-plane
       */
      double R2;

      /**
       * \brief strain damping 
       */
      double cEps2D;

      FlexibleBody1s33Cosserat(); // standard constructor
      FlexibleBody1s33Cosserat(const FlexibleBody1s33Cosserat&); // copy constructor
      FlexibleBody1s33Cosserat& operator=(const FlexibleBody1s33Cosserat&); // assignment operator

      /**
       * \brief initialize translational part of mass matrix and calculate Cholesky decomposition
       */
      void initM();

      /**
       * \brief compute boundary conditions for rotation grid
       * first and last finite difference rotation beam element refer to values not directly given by dof in open structure
       * they have to be estimated in the following function
       */
      void computeBoundaryCondition();

      /** 
       * \brief insert 'local' information in global vectors for rotation grid
       * \param number of finite element
       * \param local vector
       * \param global vector
       */
      void GlobalVectorContributionRotation(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec);
  };

  inline void FlexibleBody1s33Cosserat::setMomentsInertia(double I1_, double I2_, double I0_) {
    I1 = I1_;
    I2 = I2_;
    I0 = I0_;
  }

  inline void FlexibleBody1s33Cosserat::setCurlRadius(double R1_, double R2_) {
    R1 = R1_;
    R2 = R2_;
    if (initialised)
      for (int i = 0; i < Elements; i++)
        static_cast<FiniteElement1s33CosseratRotation*>(rotationDiscretization[i])->setCurlRadius(R1, R2);
  }
  inline void FlexibleBody1s33Cosserat::setMaterialDamping(double cEps0D_, double cEps1D_, double cEps2D_) {
    cEps0D = cEps0D_;
    cEps1D = cEps1D_;
    cEps2D = cEps2D_;
    if (initialised)
      for (int i = 0; i < Elements; i++)
        static_cast<FiniteElement1s33CosseratTranslation*>(discretization[i])->setMaterialDamping(Elements * cEps0D, cEps1D, cEps2D);
  }

}

#endif /* _FLEXIBLE_BODY_1S_33_COSSERAT_H_ */
