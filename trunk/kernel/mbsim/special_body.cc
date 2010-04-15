/* Copyright (C) 2004-2009 MBSim Development Team
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
 * Contact: mfoerg@users.berlios.de
 */

#include <config.h>
#include "special_body.h"
#include "mbsim/contour.h"
#include "mbsim/dynamic_system_solver.h"
#include "mbsim/joint.h"
#include "mbsim/constitutive_laws.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/objectfactory.h"
#include <mbsim/environment.h>
#include "constraint.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/invisiblebody.h>
#include <openmbvcppinterface/objectfactory.h>
#endif

using namespace std;
using namespace fmatvec;

namespace MBSim {

  SpecialBody::SpecialBody(const string &name) : Body(name), m(0), SThetaS(3,INIT,0.), WThetaS(3,INIT,0.), iKinematics(-1), iInertia(-1), cb(false), PjT(3,INIT,0.), PjR(3,INIT,0.), PdjT(3,INIT,0.), PdjR(3,INIT,0.), APK(3,INIT,0.), PrPK(3,INIT,0.), WrPK(3,INIT,0.), WvPKrel(3,INIT,0.), WomPK(3,INIT,0.), fT(0), fPrPK(0), fAPK(0), fPJT(0), fPJR(0), fPdJT(0), fPdJR(0), fPjT(0), fPjR(0), fPdjT(0), fPdjR(0), constraint(0) {
    APK(0,0)=1.;
    APK(1,1)=1.;
    APK(2,2)=1.;

    C=new Frame("C");
    Body::addFrame(C);
#ifdef HAVE_OPENMBVCPPINTERFACE
    openMBVFrame=C;
#endif

    SrSF.push_back(Vec(3,INIT,0.));
    WrSF.push_back(Vec(3,INIT,0.));
    ASF.push_back(SqrMat(3,EYE));
  }

  SpecialBody::~SpecialBody() {
    if(fT) { delete fT; fT=0; }
    if(fPrPK) { delete fPrPK; fPrPK=0; }
    if(fAPK) { delete fAPK; fAPK=0; }
    if(fPJT) { delete fPJT; fPJT=0; }
    if(fPJR) { delete fPJR; fPJR=0; }
    if(fPdJT) { delete fPdJT; fPdJT=0; }
    if(fPdJR) { delete fPdJR; fPdJR=0; }
    if(fPjT) { delete fPjT; fPjT=0; }
    if(fPjR) { delete fPjR; fPjR=0; }
    if(fPdjT) { delete fPdjT; fPdjT=0; }
    if(fPdjR) { delete fPdjR; fPdjR=0; }
  }

  void SpecialBody::updateh(double t) {

    Vec WF = m*MBSimEnvironment::getInstance()->getAccelerationOfGravity() - m*frame[0]->getGyroscopicAccelerationOfTranslation();
    Vec WM = crossProduct(WThetaS*frame[0]->getAngularVelocity(),frame[0]->getAngularVelocity()) - WThetaS*frame[0]->getGyroscopicAccelerationOfRotation();

    h += frame[0]->getJacobianOfTranslation().T()*WF + frame[0]->getJacobianOfRotation().T()*WM;
    hObject += frame[0]->getJacobianOfTranslation().T()*WF + frame[0]->getJacobianOfRotation().T()*WM;
  }

  void SpecialBody::updateStateDerivativeDependentVariables(double t) {

    aT = frame[0]->getJacobianOfTranslation()*udall;// + frame[0]->getGyroscopicAccelerationOfTranslation();
    aR = frame[0]->getJacobianOfRotation()*udall; //+ frame[0]->getGyroscopicAccelerationOfRotation(); 
  }

  void SpecialBody::calcqSize() {
    Body::calcqSize();
    if(constraint) {
      qSize = 0;
    }
    else {
      qSize = 0;
      if(fPrPK)
	qSize += fPrPK->getqSize();
      if(fAPK)
	qSize += fAPK->getqSize();
    }
  }

  void SpecialBody::calcuSize(int j) {
    Body::calcuSize(j);
    if(constraint) {
      uSize[0] = 0;
    }
    else {
      if(j==0) {
	uSize[j] = 0;
	if(fPJT==0) {
	  LinearTranslation* fPrPK_ = dynamic_cast<LinearTranslation*>(fPrPK);
	  if(fPrPK_) 
	    uSize[j] += fPrPK->getqSize();
	} else
	  uSize[j] += fPJT->getuSize();
	if(fPJR==0) {
	  //TODO  Euler-Parameter
	  //RotationAxis* fAPK_ = dynamic_cast<RotationAxis*>(fAPK);
	  if(fAPK)
	    uSize[j] += fAPK->getqSize();
	  //CardanAngles* fAPK_ = dynamic_cast<CardanAngles*>(fAPK);
	  //if(fAPK_)
	  //uSize += fAPK->getqSize();
	} else
	  uSize[j] += fPJR->getuSize();
      } else {
	uSize[j] = uSize[0];
	uSize[j] += forceDir.cols();
	uSize[j] += momentDir.cols();
      }
    }
  }

  void SpecialBody::init(InitStage stage) {
    if(stage==preInit) {
      Body::init(stage);
      //for(unsigned int i=0; i<body.size(); i++)
      if(constraint)
	dependency.push_back(constraint);
    }
    else if(stage==relativeFrameContourLocation) {
      // This outer loop is nessesary because the frame hierarchy must not be in the correct order!
      for(size_t k=0; k<saved_refFrameF.size(); k++)
        for(size_t j=0; j<saved_refFrameF.size(); j++) {
          int i = 0;
          if(saved_refFrameF[j]!="") i = frameIndex(getFrame(saved_refFrameF[j]));

          SrSF[j+1]=SrSF[i] + ASF[i]*saved_RrRF[j];
          ASF[j+1]=ASF[i]*saved_ARF[j];
        }
      for(size_t j=0; j<saved_refFrameC.size(); j++) {
        int i = 0;
        if(saved_refFrameC[j]!="") i = frameIndex(getFrame(saved_refFrameC[j]));

        SrSC[j]=SrSF[i] + ASF[i]*saved_RrRC[j];
        ASC[j]=ASF[i]*saved_ARC[j];
      }
    }
    else if(stage==unknownStage) {
      if(iKinematics == -1)
        iKinematics = 0;

      Body::init(stage);

      int nu1, nu2, nq;
      if(constraint) {
	nu1 = 1;
	nu2 = 1;
	nq = 1;
	T.resize(nu1,nu1);
      }
      else {
	nu1 = uSize[0];
	nu2 = uSize[1];
	nq = qSize;
	//T.resize(qSize,nu1);
      }

      PJT.resize(3,nu1);
      PJR.resize(3,nu1);

      PdJT.resize(3,nu1);
      PdJR.resize(3,nu1);

      PJTs.resize(3,nu2);
      PJRs.resize(3,nu2);

      JRel.resize(nu1,hSize[0]);
      for(int i=0; i<uSize[0]; i++)
	JRel(i,hSize[0]-uSize[0]+i) = 1;
      jRel.resize(nu1);
      qRel.resize(nq);
      uRel.resize(nu1);

      WJTrel.resize(3,nu1);
      WjTrel.resize(3);
      WJRrel.resize(3,nu1);
      WjRrel.resize(3);

      updateM_ = &SpecialBody::updateMNotConst;
      facLLM_ = &SpecialBody::facLLMNotConst;

      if(fPJT==0) {
        Mat JT(3,0);
        if(dynamic_cast<LinearTranslation*>(fPrPK)) {
          JT.resize() = dynamic_cast<LinearTranslation*>(fPrPK)->getTranslationVectors();
        } 
        PJT(Index(0,2), Index(0,JT.cols()-1)) = JT;
        PJTs(Index(0,2), Index(0,JT.cols()-1)) = JT;
        PJTs(Index(0,2), Index(JT.cols(),JT.cols()+forceDir.cols()-1)) = forceDir;
      }
      if(fPJR==0) {
        Mat JR(3,0);

	if(dynamic_cast<RotationAboutZAxis*>(fAPK)) 
	  JR.resize() = Vec("[0;0;1]");
	else if(dynamic_cast<RotationAboutFixedAxis*>(fAPK)) 
	  JR.resize() = dynamic_cast<RotationAboutFixedAxis*>(fAPK)->getAxisOfRotation();
	else if(dynamic_cast<RotationAboutAxesYZ*>(fAPK)) {
	  fPJR = new JRotationAboutAxesYZ(nu1);
	  fPdJR = new JdRotationAboutAxesYZ(nu1);
	}
	else if(dynamic_cast<RotationAboutAxesXY*>(fAPK)) {
	  fPJR = new JRotationAboutAxesXY(nu1);
	  fPdJR = new JdRotationAboutAxesXY(nu1);
	}
	else if(dynamic_cast<CardanAngles*>(fAPK)) {
	  JR.resize() << DiagMat(3,INIT,1);
	  if(cb) {
	    fT = new TCardanAngles2(qSize,nu1);
	  }
	  else {
	    fT = new TCardanAngles(qSize,nu1);
	  }
	}

        Mat JRR(3, nu1);
        PJR(Index(0,2), Index(nu1-JR.cols(),nu1-1)) = JR;
        PJRs(Index(0,2), Index(nu2-momentDir.cols()-JR.cols(),nu2-momentDir.cols()-1)) = JR;
        PJRs(Index(0,2), Index(nu2-momentDir.cols(),nu2-1)) = momentDir;

        if(cb) {
          if(iKinematics == 0 && false) {
            updateM_ = &SpecialBody::updateMConst;
            Mbuf = m*JTJ(PJT) + JTMJ(SThetaS,PJR);
            LLM = facLL(Mbuf);
            facLLM_ = &SpecialBody::facLLMConst;
          }
          PJR0 = PJR;
        } 
        else {
        }
      }

      if(iInertia != 0)
        SThetaS = SymMat(ASF[iInertia]*SThetaS*ASF[iInertia].T()) - m*JTJ(tilde(SrSF[iInertia]));

      for(int i=0; i<nu1; i++) 
        T(i,i) = 1;
    }
    else if(stage==MBSim::plot) {
      updatePlotFeatures(parent);

      if(getPlotFeature(plotRecursive)==enabled) {
        if(getPlotFeature(globalPosition)==enabled) {
          plotColumns.push_back("WxOS");
          plotColumns.push_back("WyOS");
          plotColumns.push_back("WzOS");
          plotColumns.push_back("alpha");
          plotColumns.push_back("beta");
          plotColumns.push_back("gamma");
        }
        if(getPlotFeature(globalVelocity)==enabled) {
          plotColumns.push_back("WvxS");
          plotColumns.push_back("WvyS");
          plotColumns.push_back("WvzS");
          plotColumns.push_back("alphap");
          plotColumns.push_back("betap");
          plotColumns.push_back("gammap");
        }
        Body::init(stage);
      }
    }
    else
      Body::init(stage);
  }

  void SpecialBody::resizeJacobians(int j) {
    for(unsigned int i=0; i<frame.size(); i++) 
      frame[i]->resizeJacobians(j);

    for(unsigned int i=0; i<contour.size(); i++)
      contour[i]->resizeJacobians(j);
  }

  void SpecialBody::updateqRef(const fmatvec::Vec& ref) {
    Body::updateqRef(ref);
  }

  void SpecialBody::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      if(getPlotFeature(globalPosition)==enabled) {
        Vec WrOS=frame[0]->getPosition();
        Vec cardan=AIK2Cardan(frame[0]->getOrientation());
        plotVector.push_back(WrOS(0));
        plotVector.push_back(WrOS(1));
        plotVector.push_back(WrOS(2));
        plotVector.push_back(cardan(0));
        plotVector.push_back(cardan(1));
        plotVector.push_back(cardan(2));
      }
      if(getPlotFeature(globalVelocity)==enabled) {
        Vec WvS    =frame[0]->getVelocity();
        Vec WomegaS=frame[0]->getAngularVelocity();
        plotVector.push_back(WvS(0));
        plotVector.push_back(WvS(1));
        plotVector.push_back(WvS(2));
        plotVector.push_back(WomegaS(0));
        plotVector.push_back(WomegaS(1));
        plotVector.push_back(WomegaS(2));
      }

#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled && openMBVBody) {
        vector<double> data;
        data.push_back(t);
        Vec WrOS=openMBVFrame->getPosition();
        Vec cardan=AIK2Cardan(openMBVFrame->getOrientation());
        data.push_back(WrOS(0));
        data.push_back(WrOS(1));
        data.push_back(WrOS(2));
        data.push_back(cardan(0));
        data.push_back(cardan(1));
        data.push_back(cardan(2));
        data.push_back(0);
        ((OpenMBV::RigidBody*)openMBVBody)->append(data);
      }
#endif
      Body::plot(t,dt);
    }
  }

  void SpecialBody::updateKinematicsForSelectedFrame(double t) {
    if(!constraint) {
      qRel = q;
      uRel = u;
    }

    if(fPJT)
      PJT = (*fPJT)(qRel,t);
    if(fPJR)
      PJR = (*fPJR)(qRel,t);

    if(fPjT)
      PjT = (*fPjT)(t);
    if(fPjR)
      PjR = (*fPjR)(t);

    if(fAPK)
      APK = (*fAPK)(qRel,t);
    if(fPrPK)
      PrPK = (*fPrPK)(qRel,t);

    frame[iKinematics]->setOrientation(frameOfReference->getOrientation()*APK);

    if(cb) {
      PJR = frameOfReference->getOrientation().T()*frame[iKinematics]->getOrientation()*PJR0;
    }

    WrPK = frameOfReference->getOrientation()*PrPK;
    WomPK = frameOfReference->getOrientation()*(PJR*uRel + PjR);
    WvPKrel = frameOfReference->getOrientation()*(PJT*uRel + PjT);

    frame[iKinematics]->setAngularVelocity(frameOfReference->getAngularVelocity() + WomPK);
    frame[iKinematics]->setPosition(WrPK + frameOfReference->getPosition());
    frame[iKinematics]->setVelocity(frameOfReference->getVelocity() + WvPKrel + crossProduct(frameOfReference->getAngularVelocity(),WrPK));
  }

  void SpecialBody::updateJacobiansForSelectedFrame(double t) {
    frame[iKinematics]->getJacobianOfTranslation().init(0);
    frame[iKinematics]->getJacobianOfRotation().init(0);

    if(fPdJT)
      PdJT = (*fPdJT)(T*uRel,qRel,t);
    if(fPdJR)
      PdJR = (*fPdJR)(T*uRel,qRel,t);

    if(fPdjT)
      PdjT = (*fPdjT)(t);
    if(fPdjR)
      PdjR = (*fPdjR)(t);

    SqrMat tWrPK = tilde(WrPK);
      frame[iKinematics]->setGyroscopicAccelerationOfTranslation(frameOfReference->getGyroscopicAccelerationOfTranslation() - tWrPK*frameOfReference->getGyroscopicAccelerationOfRotation() + frameOfReference->getOrientation()*(PdJT*uRel + PdjT + PJT*jRel) + crossProduct(frameOfReference->getAngularVelocity(), 2*WvPKrel+crossProduct(frameOfReference->getAngularVelocity(),WrPK)));
      frame[iKinematics]->setGyroscopicAccelerationOfRotation(frameOfReference->getGyroscopicAccelerationOfRotation() + frameOfReference->getOrientation()*(PdJR*uRel + PdjR + PJR*jRel) + crossProduct(frameOfReference->getAngularVelocity(), WomPK));

    frame[iKinematics]->getJacobianOfTranslation()(Index(0,2),Index(0,frameOfReference->getJacobianOfTranslation().cols()-1)) = frameOfReference->getJacobianOfTranslation() - tWrPK*frameOfReference->getJacobianOfRotation();
    frame[iKinematics]->getJacobianOfRotation()(Index(0,2),Index(0,frameOfReference->getJacobianOfRotation().cols()-1)) = frameOfReference->getJacobianOfRotation();

    frame[iKinematics]->getJacobianOfTranslation()(Index(0,2),Index(0,gethSize(0)-1)) += frameOfReference->getOrientation()*PJT*JRel;
    frame[iKinematics]->getJacobianOfRotation()(Index(0,2),Index(0,gethSize(0)-1)) += frameOfReference->getOrientation()*PJR*JRel;
  }

  void SpecialBody::updateKinematicsForRemainingFramesAndContours(double t) {
    if(iKinematics != 0) // only if kinematics frame is not cog-frame, update JACOBIAN of cog
      frame[0]->setOrientation(frame[iKinematics]->getOrientation()*ASF[iKinematics].T());

    for(unsigned int i=1; i<frame.size(); i++) {
      WrSF[i] = frame[0]->getOrientation()*SrSF[i];
    }

    for(unsigned int i=0; i<contour.size(); i++) {
      WrSC[i] = frame[0]->getOrientation()*SrSC[i];
    }

    if(iKinematics != 0) { // only if kinematics frame is not cog-frame, update JACOBIAN of cog
      frame[0]->setPosition(frame[iKinematics]->getPosition() - WrSF[iKinematics]);
      frame[0]->setVelocity(frame[iKinematics]->getVelocity() - crossProduct(frame[iKinematics]->getAngularVelocity(), WrSF[iKinematics]));
      frame[0]->setAngularVelocity(frame[iKinematics]->getAngularVelocity());
    }

    for(unsigned int i=1; i<frame.size(); i++) {
      if(i!=unsigned(iKinematics)) {
        frame[i]->setPosition(frame[0]->getPosition() + WrSF[i]);
        frame[i]->setVelocity(frame[0]->getVelocity() + crossProduct(frame[0]->getAngularVelocity(), WrSF[i]));
        frame[i]->setAngularVelocity(frame[0]->getAngularVelocity());
        frame[i]->setOrientation(frame[0]->getOrientation()*ASF[i]);
      }
    }
    for(unsigned int i=0; i<contour.size(); i++) {
      contour[i]->setReferenceOrientation(frame[0]->getOrientation()*ASC[i]);
      contour[i]->setReferenceAngularVelocity(frame[0]->getAngularVelocity());
      contour[i]->setReferencePosition(frame[0]->getPosition() + WrSC[i]);
      contour[i]->setReferenceVelocity(frame[0]->getVelocity() + crossProduct(frame[0]->getAngularVelocity(), WrSC[i]));
    }
    WThetaS = JTMJ(SThetaS,frame[0]->getOrientation().T());
  }

  void SpecialBody::updateJacobiansForRemainingFramesAndContours(double t) {

    if(iKinematics != 0) { // only if kinematics frame is not cog-frame, update JACOBIAN of cog
      SqrMat tWrSK = tilde(WrSF[iKinematics]);
      frame[0]->setJacobianOfTranslation(frame[iKinematics]->getJacobianOfTranslation() + tWrSK*frame[iKinematics]->getJacobianOfRotation());
      frame[0]->setJacobianOfRotation(frame[iKinematics]->getJacobianOfRotation());
      frame[0]->setGyroscopicAccelerationOfTranslation(frame[iKinematics]->getGyroscopicAccelerationOfTranslation() + tWrSK*frame[iKinematics]->getGyroscopicAccelerationOfRotation() + crossProduct(frame[iKinematics]->getAngularVelocity(),crossProduct(frame[iKinematics]->getAngularVelocity(),-WrSF[iKinematics])));
      frame[0]->setGyroscopicAccelerationOfRotation(frame[iKinematics]->getGyroscopicAccelerationOfRotation());
    }

    for(unsigned int i=1; i<frame.size(); i++) {
      if(i!=unsigned(iKinematics)) {
        SqrMat tWrSK = tilde(WrSF[i]);
        frame[i]->setJacobianOfTranslation(frame[0]->getJacobianOfTranslation() - tWrSK*frame[0]->getJacobianOfRotation());
        frame[i]->setJacobianOfRotation(frame[0]->getJacobianOfRotation());
        frame[i]->setGyroscopicAccelerationOfTranslation(frame[0]->getGyroscopicAccelerationOfTranslation() - tWrSK*frame[0]->getGyroscopicAccelerationOfRotation() + crossProduct(frame[0]->getAngularVelocity(),crossProduct(frame[0]->getAngularVelocity(),WrSF[i])));
        frame[i]->setGyroscopicAccelerationOfRotation(frame[0]->getGyroscopicAccelerationOfRotation());
      }
    }

    for(unsigned int i=0; i<contour.size(); i++) {
      SqrMat tWrSC = tilde(WrSC[i]);
      contour[i]->setReferenceJacobianOfRotation(frame[0]->getJacobianOfRotation());
      contour[i]->setReferenceGyroscopicAccelerationOfRotation(frame[0]->getGyroscopicAccelerationOfRotation());
      contour[i]->setReferenceJacobianOfTranslation(frame[0]->getJacobianOfTranslation() - tWrSC*frame[0]->getJacobianOfRotation());
      contour[i]->setReferenceGyroscopicAccelerationOfTranslation(frame[0]->getGyroscopicAccelerationOfTranslation() - tWrSC*frame[0]->getGyroscopicAccelerationOfRotation() + crossProduct(frame[0]->getAngularVelocity(),crossProduct(frame[0]->getAngularVelocity(),WrSC[i])));
    }
  }

  void SpecialBody::setForceDirection(const Mat &fd) {
    assert(fd.rows() == 3);

    forceDir = fd;

    for(int i=0; i<fd.cols(); i++)
      forceDir.col(i) = forceDir.col(i)/nrm2(fd.col(i));
  }

  void SpecialBody::setMomentDirection(const Mat &md) {
    assert(md.rows() == 3);

    momentDir = md;

    for(int i=0; i<md.cols(); i++)
      momentDir.col(i) = momentDir.col(i)/nrm2(md.col(i));
  }

  void SpecialBody::addFrame(Frame *cosy, const Vec &RrRF, const SqrMat &ARF, const string& refFrameName) {
    Body::addFrame(cosy);

    saved_refFrameF.push_back(refFrameName);
    saved_RrRF.push_back(RrRF.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    saved_ARF.push_back(ARF.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    SrSF.push_back(Vec(3));
    WrSF.push_back(Vec(3));
    ASF.push_back(SqrMat(3));
  }

  void SpecialBody::addFrame(const string &str, const Vec &SrSF, const SqrMat &ASF, const Frame* refFrame) {
    addFrame(new Frame(str),SrSF,ASF,refFrame);
  }

  void SpecialBody::addContour(Contour* contour, const Vec &RrRC, const SqrMat &ARC, const string& refFrameName) {
    Body::addContour(contour);

    saved_refFrameC.push_back(refFrameName);
    saved_RrRC.push_back(RrRC.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    saved_ARC.push_back(ARC.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    SrSC.push_back(Vec(3));
    WrSC.push_back(Vec(3));
    ASC.push_back(SqrMat(3));
  }

  void SpecialBody::updateMConst(double t) {
    M += Mbuf;
  }

  void SpecialBody::updateMNotConst(double t) {
    M += m*JTJ(frame[0]->getJacobianOfTranslation()) + JTMJ(WThetaS,frame[0]->getJacobianOfRotation());
  }

  void SpecialBody::updatePositionAndOrientationOfFrame(double t, unsigned int i) {

    if(fAPK)
      APK = (*fAPK)(qRel,t);
    if(fPrPK)
      PrPK = (*fPrPK)(qRel,t);

    frame[iKinematics]->setOrientation(frameOfReference->getOrientation()*APK);

    WrPK = frameOfReference->getOrientation()*PrPK;

    frame[iKinematics]->setPosition(WrPK + frameOfReference->getPosition());

    if(iKinematics != 0) // only if kinematics frame is not cog-frame, update JACOBIAN of cog
      frame[0]->setOrientation(frame[iKinematics]->getOrientation()*ASF[iKinematics].T());

    WrSF[iKinematics] = frame[0]->getOrientation()*SrSF[iKinematics]; // TODO prüfen ob immer nötig
    if(i>0)
      WrSF[i] = frame[0]->getOrientation()*SrSF[i];


    if(iKinematics != 0) { // only if kinematics frame is not cog-frame, update JACOBIAN of cog
      frame[0]->setPosition(frame[iKinematics]->getPosition() - WrSF[iKinematics]);
    }

    if(i>0) {
      if(i!=unsigned(iKinematics)) {
        frame[i]->setPosition(frame[0]->getPosition() + WrSF[i]);
        frame[i]->setOrientation(frame[0]->getOrientation()*ASF[i]);
      }
    }
  }

  void SpecialBody::updateVelocities(double t, unsigned int i) {

    if(fPJT)
      PJT = (*fPJT)(qRel,t);
    if(fPJR)
      PJR = (*fPJR)(qRel,t);

    if(fPjT)
      PjT = (*fPjT)(t);
    if(fPjR)
      PjR = (*fPjR)(t);

    WJRrel = frameOfReference->getOrientation()*PJR;
    WjRrel = frameOfReference->getOrientation()*PjR;

    WJTrel = frameOfReference->getOrientation()*PJT;
    WjTrel = frameOfReference->getOrientation()*PjT;

    WjRrel += frameOfReference->getAngularVelocity(); // = WOmK = WOmC
    WjTrel += frameOfReference->getVelocity() + crossProduct(frameOfReference->getAngularVelocity(),WrPK);

    if(iKinematics != 0) {
      WjTrel -= crossProduct(WjRrel, WrSF[iKinematics]); // + WJP*urel - crossProduct(WJRrel*urel, WrSF[iKinematics])  = WJC*urel;
      WJTrel += tilde(WrSF[iKinematics])*WJRrel;
    }

    if(i>0) {
      if(i!=unsigned(iKinematics)) {

	WjTrel += crossProduct(WjRrel, WrSF[i]); // WJC*urel + crossProduct(WJRrel*urel,WrSF[i]) = WJQ*urel
	WJTrel -= tilde(WrSF[i])*WJRrel;

      }
    }
  }

  void SpecialBody::updateAcclerations(double t, unsigned int i) {
    frame[iKinematics]->getJacobianOfTranslation().init(0);
    frame[iKinematics]->getJacobianOfRotation().init(0);
     if(fPdJT)
      PdJT = (*fPdJT)(T*uRel,qRel,t);
    if(fPdJR)
      PdJR = (*fPdJR)(T*uRel,qRel,t);

    if(fPdjT)
      PdjT = (*fPdjT)(t);
    if(fPdjR)
      PdjR = (*fPdjR)(t);

    // TODO prüfen ob Optimierungspotential

    SqrMat tWrPK = tilde(WrPK);
    WomPK = frameOfReference->getOrientation()*(PJR*uRel + PjR);
    WvPKrel = frameOfReference->getOrientation()*(PJT*uRel + PjT);

    frame[iKinematics]->setGyroscopicAccelerationOfTranslation(frameOfReference->getGyroscopicAccelerationOfTranslation() - tWrPK*frameOfReference->getGyroscopicAccelerationOfRotation() + frameOfReference->getOrientation()*(PdJT*uRel + PdjT) + crossProduct(frameOfReference->getAngularVelocity(), 2*WvPKrel+crossProduct(frameOfReference->getAngularVelocity(),WrPK)));
    frame[iKinematics]->setGyroscopicAccelerationOfRotation(frameOfReference->getGyroscopicAccelerationOfRotation() + frameOfReference->getOrientation()*(PdJR*uRel + PdjR) + crossProduct(frameOfReference->getAngularVelocity(), WomPK));

    frame[iKinematics]->getJacobianOfTranslation()(Index(0,2),Index(0,frameOfReference->getJacobianOfTranslation().cols()-1)) = frameOfReference->getJacobianOfTranslation() - tWrPK*frameOfReference->getJacobianOfRotation();
    frame[iKinematics]->getJacobianOfRotation()(Index(0,2),Index(0,frameOfReference->getJacobianOfRotation().cols()-1)) = frameOfReference->getJacobianOfRotation();

    frame[iKinematics]->setAngularVelocity(frameOfReference->getAngularVelocity() + WomPK);

    if(iKinematics != 0) { // only if kinematics frame is not cog-frame, update JACOBIAN of cog
      frame[0]->setAngularVelocity(frame[iKinematics]->getAngularVelocity());

      SqrMat tWrSK = tilde(WrSF[iKinematics]);
      frame[0]->setJacobianOfTranslation(frame[iKinematics]->getJacobianOfTranslation() + tWrSK*frame[iKinematics]->getJacobianOfRotation());
      frame[0]->setJacobianOfRotation(frame[iKinematics]->getJacobianOfRotation());
      frame[0]->setGyroscopicAccelerationOfTranslation(frame[iKinematics]->getGyroscopicAccelerationOfTranslation() + tWrSK*frame[iKinematics]->getGyroscopicAccelerationOfRotation() + crossProduct(frame[iKinematics]->getAngularVelocity(),crossProduct(frame[iKinematics]->getAngularVelocity(),-WrSF[iKinematics])));
      frame[0]->setGyroscopicAccelerationOfRotation(frame[iKinematics]->getGyroscopicAccelerationOfRotation());
    }

    if(i>0) {
      if(i!=unsigned(iKinematics)) {
        SqrMat tWrSK = tilde(WrSF[i]);
        frame[i]->setJacobianOfTranslation(frame[0]->getJacobianOfTranslation() - tWrSK*frame[0]->getJacobianOfRotation());
        frame[i]->setJacobianOfRotation(frame[0]->getJacobianOfRotation());
        frame[i]->setGyroscopicAccelerationOfTranslation(frame[0]->getGyroscopicAccelerationOfTranslation() - tWrSK*frame[0]->getGyroscopicAccelerationOfRotation() + crossProduct(frame[0]->getAngularVelocity(),crossProduct(frame[0]->getAngularVelocity(),WrSF[i])));
        frame[i]->setGyroscopicAccelerationOfRotation(frame[0]->getGyroscopicAccelerationOfRotation());
      }
    }

  }

}

