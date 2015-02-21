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
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _FLEXIBLE_BODY_FFR_H_
#define _FLEXIBLE_BODY_FFR_H_

#include "mbsim/body.h"
#include "mbsim/functions/auxiliary_functions.h"
#include "mbsimFlexibleBody/utils/taylor.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "mbsim/utils/boost_parameters.h"
#include "mbsim/utils/openmbv_utils.h"
#endif

namespace MBSim {
  class Frame;
  class Contour;
  class CompoundContour;
}

namespace MBSimFlexibleBody {

  class FixedNodalFrame;

  class FlexibleBodyFFR : public MBSim::Body {
    public:

      FlexibleBodyFFR(const std::string &name=""); 
      /**
       * \brief destructor
       */
      virtual ~FlexibleBodyFFR();

      virtual void updatedq(double t, double dt);
      virtual void updateqd(double t); 
      virtual void updateT(double t);
      virtual void updateh(double t, int j=0);
      virtual void updatehInverseKinetics(double t, int j=0);
      virtual void updateStateDerivativeDependentVariables(double t);
      virtual void updateM(double t, int i=0) { (this->*updateM_)(t,i); }
      virtual void updateStateDependentVariables(double t) { 
        updateKinematicsForSelectedFrame(t); 
        updateKinematicsForRemainingFramesAndContours(t); 
      }
      virtual void updateJacobians(double t, int j=0) { (this->*updateJacobians_[j])(t); }
      void updateJacobians0(double t) { 
        updateJacobiansForSelectedFrame0(t); 
        updateJacobiansForRemainingFramesAndContours(t,0); 
      }
      void updateJacobians1(double t) { 
        updateJacobiansForRemainingFramesAndContours1(t); 
      }
      virtual void calcqSize();
      virtual void calcuSize(int j=0);

      /* INHERITED INTERFACE OF OBJECT */
      virtual void updateqRef(const fmatvec::Vec& ref);
      virtual void updateuRef(const fmatvec::Vec& ref);
      virtual void init(InitStage stage);
      virtual void initz();
      virtual void facLLM(int i=0) { (this->*facLLM_)(i); }
      virtual void setUpInverseKinetics();
      /*****************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "FlexibleBodyFFR"; }
      virtual void plot(double t, double dt=1);
      /*****************************************************/

      /* INTERFACE FOR DERIVED CLASSES */
      /**
       * \brief updates kinematics of kinematic Frame starting from reference Frame
       */
      virtual void updateKinematicsForSelectedFrame(double t);
      /**
       * \brief updates JACOBIAN for kinematics starting from reference Frame
       */
      virtual void updateJacobiansForSelectedFrame0(double t); 

      /**
       * \brief updates kinematics for remaining Frames starting with and from cog Frame
       */
      virtual void updateKinematicsForRemainingFramesAndContours(double t);

      /**
       * \brief updates remaining JACOBIANS for kinematics starting with and from cog Frame
       */
      virtual void updateJacobiansForRemainingFramesAndContours(double t, int j=0);
      virtual void updateJacobiansForRemainingFramesAndContours1(double t);

      /* GETTER / SETTER */

      // NOTE: we can not use a overloaded setTranslation here due to restrictions in XML but define them for convinience in c++
      /*!
       * \brief set Kinematic for genral translational motion
       * \param fPrPK translational kinematic description
       */
      void setGeneralTranslation(MBSim::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK_) {
        fPrPK = fPrPK_;
        fPrPK->setParent(this);
        fPrPK->setName("GeneralTranslation");
      }
      /*!
       * \brief set Kinematic for only time dependent translational motion
       * \param fPrPK translational kinematic description
       */
      void setTimeDependentTranslation(MBSim::Function<fmatvec::Vec3(double)> *fPrPK_) {
        setGeneralTranslation(new MBSim::TimeDependentFunction<fmatvec::Vec3>(fPrPK_));
      }
      /*!
       * \brief set Kinematic for only state dependent translational motion
       * \param fPrPK translational kinematic description
       */
      void setStateDependentTranslation(MBSim::Function<fmatvec::Vec3(fmatvec::VecV)> *fPrPK_) {
        setGeneralTranslation(new MBSim::StateDependentFunction<fmatvec::Vec3>(fPrPK_));
      }
      void setTranslation(MBSim::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK_) { setGeneralTranslation(fPrPK_); }
      void setTranslation(MBSim::Function<fmatvec::Vec3(double)> *fPrPK_) { setTimeDependentTranslation(fPrPK_); }
      void setTranslation(MBSim::Function<fmatvec::Vec3(fmatvec::VecV)> *fPrPK_) { setStateDependentTranslation(fPrPK_); }

      // NOTE: we can not use a overloaded setRotation here due to restrictions in XML but define them for convinience in c++
      /*!
       * \brief set Kinematic for general rotational motion
       * \param fAPK rotational kinematic description
       */
      void setGeneralRotation(MBSim::Function<fmatvec::RotMat3(fmatvec::VecV, double)>* fAPK_) {
        fAPK = fAPK_;
        fAPK->setParent(this);
        fAPK->setName("GeneralRotation");
      }
      /*!
       * \brief set Kinematic for only time dependent rotational motion
       * \param fAPK rotational kinematic description
       */
      void setTimeDependentRotation(MBSim::Function<fmatvec::RotMat3(double)>* fAPK_) { setGeneralRotation(new MBSim::TimeDependentFunction<fmatvec::RotMat3>(fAPK_)); }
      /*!
       * \brief set Kinematic for only state dependent rotational motion
       * \param fAPK rotational kinematic description
       */
      void setStateDependentRotation(MBSim::Function<fmatvec::RotMat3(fmatvec::VecV)>* fAPK_) { setGeneralRotation(new MBSim::StateDependentFunction<fmatvec::RotMat3>(fAPK_)); }
      void setRotation(MBSim::Function<fmatvec::RotMat3(fmatvec::VecV, double)>* fAPK_) { setGeneralRotation(fAPK_); }
      void setRotation(MBSim::Function<fmatvec::RotMat3(double)>* fAPK_) { setTimeDependentRotation(fAPK_); }
      void setRotation(MBSim::Function<fmatvec::RotMat3(fmatvec::VecV)>* fAPK_) { setStateDependentRotation(fAPK_); }

      void setTranslationDependentRotation(bool dep) { translationDependentRotation = dep; }
      void setCoordinateTransformationForRotation(bool ct) { coordinateTransformation = ct; }

      /*!
       * \brief get Kinematic for translational motion
       * \return translational kinematic description
       */
      MBSim::Function<fmatvec::Vec3(fmatvec::VecV, double)>* getTranslation() { return fPrPK; }
      /*!
       * \brief get Kinematic for rotational motion
       * \return rotational kinematic description
       */
      MBSim::Function<fmatvec::RotMat3(fmatvec::VecV, double)>* getRotation() { return fAPK; }

      double getMass() const { return m; }
      FixedNodalFrame* getFrameK() { return K; };

      // Interface for basic data
      /**
       * \brief Set the mass.
       */
      void setMass(double m_) { m = m_; }
      void setc0(const fmatvec::Vec3 &c0_) { c0 = c0_; }
      /**
       * \brief Set the inertia tensor of the undeformend state w.r.t. the frame
       * K.
       */
      void setI0(const fmatvec::SymMat3& I0_) { I0 = I0_; }
      void setC1(const fmatvec::Mat3xV &C1_) { C1 = C1_; }
      void setC3(const std::vector<std::vector<fmatvec::SqrMatV> > &C3_) { C3 = C3_; }
      void setC4(const std::vector<fmatvec::SqrMat3> &C4_) { C4 = C4_; }
      void setKe(const fmatvec::SymMatV &Ke_) { Ke.setM0(Ke_); }
      void setDe(const fmatvec::SymMatV &De_) { De.setM0(De_); }
      void setProportionalDamping(const fmatvec::Vec2 &beta_) { beta = beta_; }
      // End of interface

      // Interface for nonlinear stiffness matrices
      void setC7(const std::vector<fmatvec::SqrMatV> &C7_) { C7 = C7_; }
      void setC8(const std::vector<std::vector<fmatvec::SqrMatV> > &C8_) { C8 = C8_; }
      // End of interface

      // Interface for geometric stiffness matrices 
      void setK0t(const std::vector<fmatvec::SqrMatV> &K0t_) { K0t = K0t_; }
      void setK0r(const std::vector<fmatvec::SqrMatV> &K0r_) { K0r = K0r_; }
      void setK0om(const std::vector<fmatvec::SqrMatV> &K0om_) { K0om = K0om_; }
      // End of interface

      // Interface for reference stresses 
      void setke0(const fmatvec::VecV &ke0_) { ke0 = ke0_; }
      void setKe0(const fmatvec::SqrMatV &Ke0_) { Ke0 = Ke0_; }
      // End of interface

      // Interface for standard input data
      void setmCM(const Taylor<fmatvec::Vec3,fmatvec::Mat3xV> &mCM_) { mCM = mCM_; }
      void setmmi(const Taylor<fmatvec::SymMat3,std::vector<fmatvec::SymMat3>,std::vector<std::vector<fmatvec::SqrMat3> > > &mmi_) { mmi = mmi_; }
      void setCt(const Taylor<fmatvec::MatVx3,std::vector<fmatvec::SqrMatV> > &Ct_) { Ct = Ct_; }
      void setCr(const Taylor<fmatvec::MatVx3,std::vector<fmatvec::SqrMatV> > &Cr_) { Cr = Cr_; }
      void setMe(const Taylor<fmatvec::SymMat> &Me_) { Me = Me_; }
      void setGr(const Taylor<std::vector<fmatvec::SqrMat3>,std::vector<std::vector<fmatvec::SqrMat3> > > &Gr_) { Gr = Gr_; }
      void setGe(const Taylor<std::vector<fmatvec::SqrMatV> > &Ge_) { Ge = Ge_; }
      void setOe(const Taylor<fmatvec::Matrix<fmatvec::General,fmatvec::Var,fmatvec::Fixed<6>,double>,std::vector<fmatvec::SqrMatV> > &Oe_) { Oe = Oe_; }
      void setKe(const Taylor<fmatvec::SymMatV,std::vector<fmatvec::SqrMatV>, std::vector<std::vector<fmatvec::SqrMatV> > > &Ke_) { Ke = Ke_; }
      void setDe(const Taylor<fmatvec::SymMatV,std::vector<fmatvec::SqrMatV>, std::vector<std::vector<fmatvec::SqrMatV> > > &De_) { De = De_; }
      void setksigma(const Taylor<fmatvec::VecV,fmatvec::SqrMatV> &ksigma_) { ksigma = ksigma_; }
      // End of interface
      
      /**
       * \brief Read standard input data (SID) from file.
       */
      void readSIDFromFile(const std::string& file) { THROW_MBSIMERROR("Interface not yet implemented."); }

      void addFrame(FixedNodalFrame *frame); 

      using Body::addContour;

#ifdef HAVE_OPENMBVCPPINTERFACE
      void setOpenMBVRigidBody(OpenMBV::RigidBody* body);
      void setOpenMBVFrameOfReference(MBSim::Frame * frame) {openMBVFrame=frame; }
      const MBSim::Frame* getOpenMBVFrameOfReference() const {return openMBVFrame; }

      /** \brief Visualize the weight */
      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBVWeight, MBSim::tag, (optional (scaleLength,(double),1)(scaleSize,(double),1)(referencePoint,(OpenMBV::Arrow::ReferencePoint),OpenMBV::Arrow::toPoint)(diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0))) { 
        MBSim::OpenMBVArrow ombv(diffuseColor,transparency,OpenMBV::Arrow::toHead,referencePoint,scaleLength,scaleSize);
        FWeight=ombv.createOpenMBV();
      }

      /** \brief Visualize the joint force */
      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBVJointForce, MBSim::tag, (optional (scaleLength,(double),1)(scaleSize,(double),1)(referencePoint,(OpenMBV::Arrow::ReferencePoint),OpenMBV::Arrow::toPoint)(diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0))) { 
        MBSim::OpenMBVArrow ombv(diffuseColor,transparency,OpenMBV::Arrow::toHead,referencePoint,scaleLength,scaleSize);
        FArrow=ombv.createOpenMBV();
      }

      /** \brief Visualize the joint moment */
      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBVJointMoment, MBSim::tag, (optional (scaleLength,(double),1)(scaleSize,(double),1)(referencePoint,(OpenMBV::Arrow::ReferencePoint),OpenMBV::Arrow::toPoint)(diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0))) { 
        MBSim::OpenMBVArrow ombv(diffuseColor,transparency,OpenMBV::Arrow::toHead,referencePoint,scaleLength,scaleSize);
        MArrow=ombv.createOpenMBV();
      }
#endif

      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);

      const fmatvec::Mat3xV& getWJTrel() const {return WJTrel;}
      const fmatvec::Mat3xV& getWJRrel() const {return WJRrel;}
      fmatvec::Mat3xV& getWJTrel() {return WJTrel;}
      fmatvec::Mat3xV& getWJRrel() {return WJRrel;}
      fmatvec::Vec& getqRel() {return qRel;}
      fmatvec::Vec& getuRel() {return uRel;}
      fmatvec::Mat& getTRel() {return TRel;}
      fmatvec::Mat3xV& getPJT(int i=0) {return PJT[i];}
      fmatvec::Mat3xV& getPJR(int i=0) {return PJR[i];}

      int getqRelSize() const {return nq;}
      int getuRelSize(int i=0) const {return nu[i];}

      bool transformCoordinates() const {return fTR!=NULL;}

    protected:
      // Basic input data
      double m;
      fmatvec::Vec3 c0;
      fmatvec::SymMat3 I0;
      fmatvec::Mat3xV C1, C2;
      std::vector<std::vector<fmatvec::SqrMatV> > C3;
      std::vector<fmatvec::SqrMat3> C4;
      std::vector<fmatvec::Mat3xV> C5;
      std::vector<std::vector<fmatvec::SqrMat3> > C6;
      std::vector<fmatvec::SqrMatV> C7;
      std::vector<std::vector<fmatvec::SqrMatV> > C8;
      std::vector<fmatvec::SqrMatV> K0t, K0r, K0om;
      fmatvec::Vec2 beta;
      fmatvec::VecV ke0;
      fmatvec::SqrMatV Ke0;
      // End of basic input data

      // Standard input data (SID)
      Taylor<fmatvec::Vec3,fmatvec::Mat3xV> mCM;
      Taylor<fmatvec::SymMat3,std::vector<fmatvec::SymMat3>,std::vector<std::vector<fmatvec::SqrMat3> > > mmi;
      Taylor<fmatvec::MatVx3,std::vector<fmatvec::SqrMatV> > Ct;
      Taylor<fmatvec::MatVx3,std::vector<fmatvec::SqrMatV> > Cr;
      Taylor<fmatvec::SymMat> Me;
      Taylor<std::vector<fmatvec::SqrMat3>,std::vector<std::vector<fmatvec::SqrMat3> > > Gr;
      Taylor<std::vector<fmatvec::SqrMatV> > Ge;
      Taylor<fmatvec::Matrix<fmatvec::General,fmatvec::Var,fmatvec::Fixed<6>,double>,std::vector<fmatvec::SqrMatV> > Oe;
      Taylor<fmatvec::SymMatV,std::vector<fmatvec::SqrMatV>, std::vector<std::vector<fmatvec::SqrMatV> > > Ke, De;
      Taylor<fmatvec::VecV,fmatvec::SqrMatV> ksigma;
      // End of standard input data (SID)

      // Number of mode shapes 
      int ne;

      FixedNodalFrame *K;

      /**
       * \brief TODO
       */
      fmatvec::SymMat Mbuf;

      /**
       * \brief boolean to use body fixed Frame for rotation
       */
      bool coordinateTransformation;

      /**
       * JACOBIAN of translation, rotation and their derivatives in parent system
       */
      fmatvec::Mat3xV PJT[2], PJR[2];

      fmatvec::Vec3 PjhT, PjhR, PjbT, PjbR;

      /**
       * \brief rotation matrix from kinematic Frame to parent Frame
       */
      fmatvec::SqrMat3 APK;

      /**
       * \brief translation from parent to kinematic Frame in parent and world system
       */
      fmatvec::Vec3 PrPK, WrPK;

      /**
       * \brief translational and angular velocity from parent to kinematic Frame in world system
       */
      fmatvec::Vec3 WvPKrel, WomPK;

      MBSim::Function<fmatvec::MatV(fmatvec::VecV)> *fTR;

      /**
       * \brief translation from parent Frame to kinematic Frame in parent system
       */
      MBSim::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK;

      /**
       * \brief rotation from kinematic Frame to parent Frame
       */
      MBSim::Function<fmatvec::RotMat3(fmatvec::VecV, double)> *fAPK;

      /**
       * \brief function pointer to update mass matrix
       */
      void (FlexibleBodyFFR::*updateM_)(double t, int i);

      /**
       * \brief update constant mass matrix
       */
      void updateMConst(double t, int i=0);

      /**
       * \brief update time dependend mass matrix
       */
      void updateMNotConst(double t, int i=0); 

      /**
       * \brief function pointer for Cholesky decomposition of mass matrix
       */
      void (FlexibleBodyFFR::*facLLM_)(int i);

      /**
       * \brief Cholesky decomposition of constant mass matrix
       */
      void facLLMConst(int i=0) {};

      /**
       * \brief Cholesky decomposition of time dependent mass matrix
       */
      void facLLMNotConst(int i=0) { Object::facLLM(i); }

      void (FlexibleBodyFFR::*updateJacobians_[2])(double t); 

      fmatvec::Vec aT, aR;

      fmatvec::Vec qRel, uRel;
      fmatvec::Mat TRel;

      fmatvec::VecV qTRel, qRRel, uTRel, uRRel;
      fmatvec::Mat3xV WJTrel, WJRrel, PJTT, PJRR;

      int nu[2], nq;

      MBSim::Frame *frameForJacobianOfRotation;

      std::vector<FixedNodalFrame*> RBF;
      std::vector<MBSim::CompoundContour*> RBC;

      fmatvec::Range<fmatvec::Var,fmatvec::Var> iqT, iqR, iqE, iuT, iuR, iuE;

      bool translationDependentRotation, constJT, constJR, constjT, constjR;

      fmatvec::SymMatV M_;
      fmatvec::VecV h_;
      fmatvec::MatV KJ[2];
      fmatvec::VecV Ki[2];

      void determineSID();
      void prefillMassMatrix();

    private:
#ifdef HAVE_OPENMBVCPPINTERFACE
      /**
       * \brief Frame of reference for drawing openMBVBody
       */
      MBSim::Frame * openMBVFrame;
      OpenMBV::Arrow *FWeight, *FArrow, *MArrow;
#endif
  };

}

#endif
