/*
 * flexible_body_1s_cosserat.h
 *
 *  Created on: 16.01.2013
 *      Author: rvonzitzewitz
 */

#ifndef FLEXIBLE_BODY_1S_COSSERAT_H_
#define FLEXIBLE_BODY_1S_COSSERAT_H_

#include "mbsimFlexibleBody/flexible_body.h"
#include "mbsimFlexibleBody/pointer.h"
#include <mbsimFlexibleBody/contours/neutral_contour/contour_1s_neutral_cosserat.h>
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_translation.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_rotation.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_21_cosserat_translation.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_21_cosserat_rotation.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/spineextrusion.h>

#endif

#include "mbsimFlexibleBody/contours/nurbs_curve_1s.h"

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

  class FlexibleBody1sCosserat : public FlexibleBodyContinuum<double> {
    public:

      /**
       * \brief constructor
       * \param name of body
       * \param bool to specify open (cantilever) or closed (ring) structure
       */
      FlexibleBody1sCosserat(const std::string &name, bool openStructure);

      /**
       * \brief destructor
       */
      virtual ~FlexibleBody1sCosserat();

      /* INHERITED INTERFACE OF FLEXIBLE BODY */
      virtual void BuildElements()=0;
      virtual void GlobalVectorContribution(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec)=0;
      virtual void GlobalMatrixContribution(int n, const fmatvec::Mat& locMat, fmatvec::Mat& gloMat)=0;
      virtual void GlobalMatrixContribution(int n, const fmatvec::SymMat& locMat, fmatvec::SymMat& gloMat)=0;
      virtual void updateKinematicsForFrame(MBSim::ContourPointData &cp, MBSim::Frame::Frame::Feature ff, MBSim::Frame *frame=0)=0;
      virtual void updateJacobiansForFrame(MBSim::ContourPointData &data, MBSim::Frame *frame = 0)=0;
      virtual void exportPositionVelocity(const std::string & filenamePos, const std::string & filenameVel = std::string(), const int & deg = 3, const bool & writePsFile = false)=0;
      virtual void importPositionVelocity(const std::string & filenamePos, const std::string & filenameVel = std::string())=0;
      /***************************************************/

      /* INHERITED INTERFACE OF OBJECT */
      virtual void init(InitStage stage)=0;
      virtual double computePotentialEnergy()=0;
      virtual void facLLM(int i = 0)=0;
      /***************************************************/

      /* INHERITED INTERFACE OF OBJECTINTERFACE */
      virtual void updateh(double t, int i = 0);

      /* INHERITED INTERFACE OF ELEMENT */
      virtual void plot(double t, double dt = 1)=0;
      virtual std::string getType() const {
        return "FlexibleBody1sCosserat";
      }
      /***************************************************/

      /* GETTER / SETTER */
      virtual void setNumberElements(int n)=0;
      void setLength(double L_);
      void setEGModuls(double E_, double G_);
      void setDensity(double rho_);
      void setCrossSectionalArea(double A_);

      virtual void setMomentsInertia(double I1_, double I2_, double I0_) {
      }
      virtual void setMomentsInertia(double I1_) {
      }

      virtual void setCurlRadius(double R1_, double R2_) {
      }
      virtual void setCurlRadius(double R1_) {
      }
      virtual void setMaterialDamping(double cEps0D_, double cEps1D_, double cEps2D_) {
      }
      virtual void setMaterialDamping(double cEps0D_, double cEps1D_) {
      }

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
      virtual fmatvec::Mat3xV transformJacobian(fmatvec::Mat3xV J) {
        return J;
      }

      virtual int getNumberOfElementDOF() const {
        THROW_MBSIMERROR("(FlexibleBody1sCosserat::getNumberOfElementDOF): Not implemented!");
      }
      virtual int getNumberElements() const {
        return Elements;
      }
      virtual double getLength() const {
        return L;
      }
      virtual int getqSizeFull() const {
        return getqSize();
      }
      virtual bool isOpenStructure() const {
        return openStructure;
      }


      /***************************************************/

      /**
       * \brief compute state (positions, angles, velocities, differentiated angles) at Lagrangian coordinate in local FE coordinates
       * \param Lagrangian coordinate
       */
      virtual fmatvec::Vec computeState(double s)=0;

      /**
       * \brief compute angles at Lagrangian coordinate in local FE coordinates
       * \param Lagrangian coordinate
       */
      virtual fmatvec::Vec3 computeAngles(double sGlobal, const fmatvec::Vec & vec)=0;

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
      std::vector<MBSim::DiscretizationInterface*> rotationDiscretization;

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
      double L, l0;

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
       * \brief open or closed beam structure
       */
      bool openStructure;

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
  inline void FlexibleBody1sCosserat::setLength(double L_) {
    L = L_;
  }
  inline void FlexibleBody1sCosserat::setEGModuls(double E_, double G_) {
    E = E_;
    G = G_;
  }
  inline void FlexibleBody1sCosserat::setDensity(double rho_) {
    rho = rho_;
  }
  inline void FlexibleBody1sCosserat::setCrossSectionalArea(double A_) {
    A = A_;
  }
}

#endif /* FLEXIBLE_BODY_1S_COSSERAT_H_ */
