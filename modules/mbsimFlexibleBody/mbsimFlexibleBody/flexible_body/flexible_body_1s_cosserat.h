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

#ifndef FLEXIBLE_BODY_1S_COSSERAT_H_
#define FLEXIBLE_BODY_1S_COSSERAT_H_

#include "mbsimFlexibleBody/flexible_body/flexible_body_1s.h"
#include "mbsimFlexibleBody/pointer.h"
//#include <mbsimFlexibleBody/contours/neutral_contour/contour_1s_neutral_cosserat.h>
//#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_translation.h"
//#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_rotation.h"
//#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_21_cosserat_translation.h"
//#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_21_cosserat_rotation.h"
//#include <openmbvcppinterface/spineextrusion.h>
//
//#include "mbsimFlexibleBody/contours/nurbs_curve_1s.h"

namespace MBSimFlexibleBody {

  /**
   * \brief finite element for spatial beam using Cosserat model
   * \author Robert von Zitzewitz
   * \date 2013-02-04 on parent class for 2D and 3D Cosserat beam for nurbs contour (Robert von Zitzewitz)
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
  class Contour1sNeutralCosserat;

  class FlexibleBody1sCosserat : public FlexibleBody1s {
    public:

      /**
       * \brief constructor
       * \param name of body
       * \param bool to specify open (cantilever) or closed (ring) structure
       */
      FlexibleBody1sCosserat(const std::string &name, bool openStructure);

      /* INHERITED INTERFACE OF FLEXIBLE BODY */
       void BuildElements() override =0;
      const fmatvec::Vec& evalqRotationElement(int i) { if(updEle) BuildElements(); return qRotationElement[i]; }
      const fmatvec::Vec& evaluRotationElement(int i) { if(updEle) BuildElements(); return uRotationElement[i]; }
       void GlobalVectorContribution(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec) override =0;
       void GlobalMatrixContribution(int n, const fmatvec::Mat& locMat, fmatvec::Mat& gloMat) override =0;
       void GlobalMatrixContribution(int n, const fmatvec::SymMat& locMat, fmatvec::SymMat& gloMat) override =0;
       void exportPositionVelocity(const std::string & filenamePos, const std::string & filenameVel = std::string(), const int & deg = 3, const bool & writePsFile = false) override =0;
       void importPositionVelocity(const std::string & filenamePos, const std::string & filenameVel = std::string()) override =0;
      /***************************************************/

      /* INHERITED INTERFACE OF OBJECTINTERFACE */
       void updateh(int i = 0) override;

      /* INHERITED INTERFACE OF ELEMENT */
      /***************************************************/

      /* GETTER / SETTER */
      virtual void setNumberElements(int n)=0;
      void setEGModuls(double E_, double G_) { E = E_; G = G_; }
      void setDensity(double rho_) { rho = rho_; }
      void setCrossSectionalArea(double A_) { A = A_; }

      virtual void setMomentsInertia(double I1_, double I2_, double I0_) { }
      virtual void setMomentsInertia(double I1_) { }

      virtual void setCurlRadius(double R1_, double R2_) { }
      virtual void setCurlRadius(double R1_) { }
      virtual void setMaterialDamping(double cEps0D_, double cEps1D_, double cEps2D_) { }
      virtual void setMaterialDamping(double cEps0D_, double cEps1D_) { }

      /*!
       * \brief automatically creates its neutral contour
       */
      virtual Contour1sNeutralCosserat* createNeutralPhase(const std::string & contourName = "Neutral");

      /*!
       * \brief interface function to transform the Jacobian if the generalized coordinates have been changed
       *
       * default: no transformation!
       *
       * \todo: make real concept for reduced bodies in MBSim
       */
      virtual fmatvec::Mat3xV transformJacobian(fmatvec::Mat3xV J) { return J; }

      virtual int getNumberOfElementDOF() const { throwError("(FlexibleBody1sCosserat::getNumberOfElementDOF): Not implemented!"); }
      virtual int getNumberElements() const { return Elements; }
      virtual int getqSizeFull() const { return getqSize(); }

      /***************************************************/

//      /**
//       * \brief compute state (positions, angles, velocities, differentiated angles) at Lagrangian coordinate in local FE coordinates
//       * \param Lagrangian coordinate
//       */
//      virtual fmatvec::Vec computeState(double s)=0;
//
//      /**
//       * \brief compute angles at Lagrangian coordinate in local FE coordinates
//       * \param Lagrangian coordinate
//       */
//      virtual fmatvec::Vec3 computeAngles(double sGlobal, const fmatvec::Vec & vec)=0;

      /**
       * \brief initialise beam only for giving information with respect to state, number elements, length, (not for simulation)
       */
      virtual void initInfo()=0;

      /**
       * \brief detect current finite element (translation)
       * \param global parametrisation
       * \param local parametrisation
       * \param finite element number
       */
      virtual void BuildElementTranslation(const double& sGlobal, double& sLocal, int& currentElementTranslation) = 0;

    protected:

      /**
       * \brief stl-vector of finite elements for rotation grid
       */
      std::vector<DiscretizationInterface*> rotationDiscretization;

      /**
       * \brief stl-vector of finite element positions for rotation grid
       */
      std::vector<fmatvec::Vec> qRotationElement;

      /**
       * \brief stl-vector of finite element wise velocities for rotation grid
       */
      std::vector<fmatvec::Vec> uRotationElement;

      /**
       * \brief angle parametrisation
       */
      CardanPtr angle;

      /**
       * \brief number of translational elements
       */
      int Elements;

      /**
       * \brief number of rotational elements =Elements (for a closed structure) or =Elements+1 (for an open structure)
       */
      int rotationalElements;

      /**
       * \brief length of entire beam and finite elements
       */
      double l0;

      /**
       * \brief elastic modules
       */
      double E, G;

      /**
       * \brief area of cross-section
       */
      double A;

      /**
       * \brief area moments of inertia
       * I0: around torsional axis
       * I1: in t-b-plane
       * I2: in t-n-plane
       */
      double I1;

      /**
       * \brief density
       */
      double rho;

      /**
       * \brief radius of undeformed shape
       * R1: in t-b-plane
       * R2: in t-n-plane
       */
      double R1;

      /**
       * \brief strain damping
       */
      double cEps0D, cEps1D;

      /**
       * \brief initialised FLAG
       */
      bool initialised;

      /**
       * \brief boundary conditions for rotation grid
       * first and last finite difference rotation beam element refer to values not directly given by dof in open structure
       * they have to be estimated by the following values calculated in computeBoundaryCondition()
       */
      fmatvec::Vec bound_ang_start;
      fmatvec::Vec bound_ang_end;
      fmatvec::Vec bound_ang_vel_start;
      fmatvec::Vec bound_ang_vel_end;

      /*!
       * \brief contour for the spine extrusion
       */
      Contour1sNeutralCosserat* ncc;

      FlexibleBody1sCosserat(); // standard constructor
      FlexibleBody1sCosserat(const FlexibleBody1sCosserat&); // copy constructor
      FlexibleBody1sCosserat& operator=(const FlexibleBody1sCosserat&); // assignment operator

      /**
       * \brief initialize translational part of mass matrix and calculate Cholesky decomposition
       */
      virtual void initM()=0;

      /**
       * \brief compute boundary conditions for rotation grid
       * first and last finite difference rotation beam element refer to values not directly given by dof in open structure
       * they have to be estimated in the following function
       */
      virtual void computeBoundaryCondition()=0;

      /**
       * \brief insert 'local' information in global vectors for rotation grid
       * \param number of finite element
       * \param local vector
       * \param global vector
       */
      virtual void GlobalVectorContributionRotation(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec)=0;
  };
}

#endif /* FLEXIBLE_BODY_1S_COSSERAT_H_ */
