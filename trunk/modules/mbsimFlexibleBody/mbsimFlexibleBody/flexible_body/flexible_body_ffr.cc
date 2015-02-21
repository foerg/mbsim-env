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

#include <config.h>
#include "flexible_body_ffr.h"
#include "mbsimFlexibleBody/fixed_nodal_frame.h"
#include "mbsim/contour.h"
#include "mbsim/dynamic_system_solver.h"
#include "mbsim/joint.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/objectfactory.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematic_functions.h"
#include "mbsim/contours/compound_contour.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/rigidbody.h>
#include <openmbvcppinterface/invisiblebody.h>
#include <openmbvcppinterface/objectfactory.h>
#include <openmbvcppinterface/group.h>
#endif

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimFlexibleBody {

  template <class Row, class AT>
  inline Matrix<Symmetric, Row, Row, AT> ApAT(const SquareMatrix<Row, AT> &A) {
    Matrix<Symmetric, Row, Row, AT> S(A.size(), A.size(), NONINIT);
    for (int i = 0; i < A.cols(); i++)
      for (int j = i; j < A.cols(); j++)
        S.ej(i, j) = A.e(i, j) + A.e(j, i);
    return S;
  }

  Range<Var,Var> i02(0,2);

//  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FlexibleBodyFFR, MBSIMFLEXIBLEBODY%"FlexibleBodyFFR")

  FlexibleBodyFFR::FlexibleBodyFFR(const string &name) : Body(name), m(0), ne(0), coordinateTransformation(true), APK(EYE), fTR(0), fPrPK(0), fAPK(0), frameForJacobianOfRotation(0), translationDependentRotation(false), constJT(false), constJR(false), constjT(false), constjR(false) {

    K=new FixedNodalFrame("K");
    Body::addFrame(K);
#ifdef HAVE_OPENMBVCPPINTERFACE
    openMBVFrame=K;
    FWeight = 0;
    FArrow = 0;
    MArrow = 0;
#endif

    updateJacobians_[0] = &FlexibleBodyFFR::updateJacobians0;
    updateJacobians_[1] = &FlexibleBodyFFR::updateJacobians1;
  }

  FlexibleBodyFFR::~FlexibleBodyFFR() {
    if(fPrPK) { delete fPrPK; fPrPK=0; }
    if(fAPK) { delete fAPK; fAPK=0; }
    if(fTR) { delete fTR; fTR=0; }
  }

  void FlexibleBodyFFR::updateh(double t, int index) {
    h[index] += KJ[index].T()*(h_ - M_*Ki[index]);
  }

  void FlexibleBodyFFR::updatehInverseKinetics(double t, int j) {
    h[j] -= KJ[1].T()*(M_*(KJ[0]*udall[0] + Ki[0]));
  }

  void FlexibleBodyFFR::updateStateDerivativeDependentVariables(double t) {
    for(unsigned int i=0; i<frame.size(); i++)
      ((FixedNodalFrame*)frame[i])->updateStateDerivativeDependentVariables(udall[0]);
    //for(unsigned int i=0; i<RBC.size(); i++)
    //  RBC[i]->updateStateDerivativeDependentVariables(udall[0],t);
  }

  void FlexibleBodyFFR::calcqSize() {
    Body::calcqSize();
    qSize = nq;
  }

  void FlexibleBodyFFR::calcuSize(int j) {
    Body::calcuSize(j);
    if(j==0)
      uSize[j] = nu[j];
    else
      uSize[j] = 6 + ne;
  }

  void FlexibleBodyFFR::determineSID() {
    C2.resize(ne,NONINIT);
    C6.resize(ne);
    Gr.getM0().resize(ne);
    Gr.getM1().resize(ne);
    Ge.getM0().resize(ne);
    Oe.getM0().resize(ne,NONINIT);
    Oe.getM1().resize(6);
    std::vector<std::vector<fmatvec::SqrMatV> > Kom(3);
    mCM.getM0() = m*c0;
    mmi.getM0() = I0;
    mCM.getM1() = C1;
    for(int i=0; i<3; i++) {
      Kom[i].resize(3);
      for(int j=0; j<3; j++) {
        Kom[i][j].resize(ne,NONINIT);
        if(i!=j)
          Kom[i][j] = C3[i][j];
      }
    }
    Kom[0][0] = -C3[1][1]-C3[2][2];
    Kom[1][1] = -C3[2][2]-C3[0][0];
    Kom[2][2] = -C3[0][0]-C3[1][1];
    for(int i=0; i<ne; i++)
      C6[i].resize(ne);
    for(int i=0; i<ne; i++) {
      for(int j=0; j<3; j++)
        Oe.getM0().e(i,j) = C4[i](j,j);
      Oe.getM0()(i,3) = C4[i].e(0,1)+C4[i].e(1,0);
      Oe.getM0()(i,4) = C4[i].e(1,2)+C4[i].e(2,1); 
      Oe.getM0()(i,5) = C4[i].e(2,0)+C4[i].e(0,2);
      Gr.getM0()[i] = -2.*C4[i];
      Gr.getM1()[i].resize(ne);
      C2.e(0,i) = C4[i].e(2,1)-C4[i].e(1,2);
      C2.e(1,i) = C4[i].e(0,2)-C4[i].e(2,0);
      C2.e(2,i) = C4[i].e(1,0)-C4[i].e(0,1);
      for(int j=i; j<ne; j++) {
        C6[i][j].e(0,0) = -C3[1][1].e(i,j) - C3[2][2].e(i,j);
        C6[i][j].e(1,1) = -C3[0][0].e(i,j) - C3[2][2].e(i,j);
        C6[i][j].e(2,2) = -C3[0][0].e(i,j) - C3[1][1].e(i,j);

        C6[i][j].e(0,1) = C3[1][0].e(i,j);
        C6[i][j].e(0,2) = C3[2][0].e(i,j);
        C6[i][j].e(1,2) = C3[2][1].e(i,j);
        C6[i][j].e(1,0) = C3[0][1].e(i,j);
        C6[i][j].e(2,0) = C3[0][2].e(i,j);
        C6[i][j].e(2,1) = C3[1][2].e(i,j);
        C6[j][i] = C6[i][j].T();
      }
      for(int j=0; j<ne; j++)
        Gr.getM1()[i][j] = -2.*C6[i][j];
    }
    Ct.getM0() = C1.T();
    for(unsigned int i=0; i<K0t.size(); i++)
      Ct.getM1().push_back(K0t[i]);

    Cr.getM0() = C2.T();

    std::vector<fmatvec::SqrMatV> Kr(3);
    for(int i=0; i<3; i++)
      Kr[i].resize(ne,NONINIT);
    Kr[0] = -C3[1][2] + C3[1][2].T();
    Kr[1] = -C3[2][0] + C3[2][0].T();
    Kr[2] = -C3[0][1] + C3[0][1].T();

    for(unsigned int i=0; i<Kr.size(); i++)
      Cr.getM1().push_back(Kr[i]);
    for(unsigned int i=0; i<K0r.size(); i++)
      Cr.getM1()[i] += K0r[i];

    Me.getM0().resize(ne,NONINIT);
    mmi.getM1().resize(ne);
    mmi.getM2().resize(ne);
    for(int i=0; i<ne; i++) {
      mmi.getM1()[i] = -ApAT(C4[i]);
      mmi.getM2()[i].resize(ne);
      for(int j=0; j<ne; j++)
        mmi.getM2()[i][j] = -C6[i][j];
      for(int j=i; j<ne; j++)
        Me.getM0().ej(i,j) = C3[0][0].e(i,j) + C3[1][1].e(i,j) + C3[2][2].e(i,j);
    }

    Ge.getM0().resize(3);
    for(int i=0; i<3; i++)
      Ge.getM0()[i].resize() = 2.*Kr[i];

    for(int i=0; i<3; i++)
      Oe.getM1()[i].resize() = Kom[i][i];
    Oe.getM1()[3].resize() = Kom[0][1] + Kom[0][1].T();
    Oe.getM1()[4].resize() = Kom[1][2] + Kom[1][2].T();
    Oe.getM1()[5].resize() = Kom[2][0] + Kom[2][0].T();
    for(unsigned int i=0; i<K0om.size(); i++)
      Oe.getM1()[i] += K0om[i];

    if(not(De.getM0().size()))
      De.getM0() = beta.e(0)*Me.getM0() + beta.e(1)*Ke.getM0();

    if(C7.size()) {
      Ke.getM1().resize(C7.size());
      if(C8.size()) {
        Ke.getM2().resize(C8.size());
        for(unsigned int i=0; i<C8.size(); i++)
          Ke.getM2()[i].resize(C8.size());
      }
      for(unsigned int i=0; i<C7.size(); i++) {
        Ke.getM1()[i].resize() = (C7[i].T() + 0.5*C7[i]);
        for(unsigned int j=0; j<C8.size(); j++)
          Ke.getM2()[i][j].resize() = 0.5*C8[i][j];
      }
    }

    ksigma.setM0(ke0);
    ksigma.setM1(Ke0);
  }

  void FlexibleBodyFFR::prefillMassMatrix() {
    M_.resize(6+ne,NONINIT);
    for(int i=0; i<3; i++) {
      M_.ej(i,i)=m;
      for(int j=i+1; j<3; j++)
        M_.ej(i,j)=0;
    }
    for(int i=0; i<ne; i++) {
      for(int j=0; j<3; j++)
        M_.e(i+6,j) = Ct.getM0().e(i,j);
      for(int j=i; j<ne; j++)
        M_.ej(i+6,j+6) = Me.getM0().ej(i,j);
    }
  }

  void FlexibleBodyFFR::init(InitStage stage) {
    if(stage==preInit) {
     ne = C1.cols();
     for(unsigned int i=0; i<frame.size(); i++)
       static_cast<FixedNodalFrame*>(frame[i])->setNumberOfModeShapes(ne);

      Mat3xV T(uSize[0]);
      Vec3 t;
      for(unsigned int k=0; k<contour.size(); k++) {
        if(!(contour[k]->getFrameOfReference()))
          contour[k]->setFrameOfReference(K);
        CompoundContour *c = dynamic_cast<CompoundContour*>(contour[k]);
        if(c) RBC.push_back(c);
      }

      Body::init(stage);

      int nqT=0, nqR=0, nuT=0, nuR=0;
      if(fPrPK) {
        nqT = fPrPK->getArg1Size();
        nuT = fPrPK->getArg1Size(); // TODO fTT->getArg1Size()
      }
      if(fAPK) {
        nqR = fAPK->getArg1Size();
        nuR = fAPK->getArg1Size(); // TODO fTR->getArg1Size()
      }
 
      if(translationDependentRotation) {
        assert(nqT == nqR);
        assert(nuT == nuR);
        nq = nqT + ne;
        nu[0] = nuT + ne;
        iqT = Range<Var,Var>(0,nqT+nqR-1);
        iqR = Range<Var,Var>(0,nqT+nqR-1);
        iqE = Range<Var,Var>(nqT+nqR,nq-1);
        iuT = Range<Var,Var>(0,nuT+nuR-1);
        iuR = Range<Var,Var>(0,nuT+nuR-1);
        iuE = Range<Var,Var>(nuT+nuR,nu[0]-1);
      }
      else {
        nq = nqT + nqR + ne;
        nu[0] = nuT + nuR + ne;
        iqT = Range<Var,Var>(0,nqT-1);
        iqR = Range<Var,Var>(nqT,nqT+nqR-1);
        iqE = Range<Var,Var>(nqT+nqR,nq-1);
        iuT = Range<Var,Var>(0,nuT-1);
        iuR = Range<Var,Var>(nuT,nqT+nqR-1);
        iuE = Range<Var,Var>(nuT+nuR,nu[0]-1);
      }

      nu[1] = 6 + ne;
    }
    else if(stage==relativeFrameContourLocation) {

      //RBF.push_back(C);
      for(unsigned int k=1; k<frame.size(); k++) {
        if(!((FixedNodalFrame*) frame[k])->getFrameOfReference())
          ((FixedNodalFrame*) frame[k])->setFrameOfReference(K);
      }
      for(unsigned int k=1; k<frame.size(); k++) {
        FixedNodalFrame *P = (FixedNodalFrame*)frame[k];
        const FixedNodalFrame *R = P;
        do {
          R = static_cast<const FixedNodalFrame*>(R->getFrameOfReference());
          P->setRelativePosition(R->getRelativePosition() + R->getRelativeOrientation()*P->getRelativePosition());
          P->setRelativeOrientation(R->getRelativeOrientation()*P->getRelativeOrientation());
        } while(R!=K);
        P->setFrameOfReference(K);
        if(P!=K)
          RBF.push_back(P);
      }
    }
    else if(stage==resize) {
      Body::init(stage);

  
      KJ[0].resize(6+ne,hSize[0]);
      KJ[1].resize(6+ne,hSize[1]);
      K->getJacobianOfDeformation().resize(ne,hSize[0]);
      K->getJacobianOfDeformation(1).resize(ne,hSize[1]);

      for(int i=0; i<ne; i++) {
        KJ[0](6+i,hSize[0]-ne+i) = 1;
        KJ[1](6+i,hSize[1]-ne+i) = 1;
        K->getJacobianOfDeformation()(i,hSize[0]-ne+i) = 1;
        K->getJacobianOfDeformation(1)(i,hSize[1]-ne+i) = 1;
      }

      Ki[0].resize(6+ne);
      Ki[1].resize(6+ne);

      PJT[0].resize(nu[0]);
      PJR[0].resize(nu[0]);

      PJT[1].resize(nu[1]);
      PJR[1].resize(nu[1]);
      for(int i=0; i<3; i++)
	PJT[1](i,i) = 1;
      for(int i=3; i<6; i++)
	PJR[1](i-3,i) = 1;

      qRel.resize(nq);
      uRel.resize(nu[0]);
      q.resize(qSize);
      u.resize(uSize[0]);

      TRel.resize(nq,nu[0],Eye());

      WJTrel.resize(nu[0]);
      WJRrel.resize(nu[0]);

      updateM_ = &FlexibleBodyFFR::updateMNotConst;
      facLLM_ = &FlexibleBodyFFR::facLLMNotConst;
    }
    else if(stage==unknownStage) {
      Body::init(stage);

      determineSID();
      prefillMassMatrix();

      K->getJacobianOfTranslation(1) = PJT[1];
      K->getJacobianOfRotation(1) = PJR[1];

      bool cb = false;
      StateDependentFunction<RotMat3> *Atmp = dynamic_cast<StateDependentFunction<RotMat3>*>(fAPK);
      if(Atmp and coordinateTransformation and dynamic_cast<RotationAboutAxesXYZ<VecV>*>(Atmp->getFunction())) {
        fTR = new RotationAboutAxesXYZMapping<VecV>;
        fTR->setParent(this);
        constJR = true;
        constjR = true;
        PJRR = SqrMat3(EYE);
        PJR[0].set(i02,iuR,PJRR);
      }
      else if(Atmp and dynamic_cast<RotationAboutAxesXYZ2<VecV>*>(Atmp->getFunction())) {
        cb = true;
        if(coordinateTransformation) {
          fTR = new RotationAboutAxesXYZMapping2<VecV>;
          fTR->setParent(this);
          constJR = true;
          constjR = true;
          PJRR = SqrMat3(EYE);
          PJR[0].set(i02,iuR,PJRR);
        }
      }
      else if(Atmp and coordinateTransformation and dynamic_cast<RotationAboutAxesZXZ<VecV>*>(Atmp->getFunction())) {
        fTR = new RotationAboutAxesZXZMapping<VecV>;
        fTR->setParent(this);
        constJR = true;
        constjR = true;
        PJRR = SqrMat3(EYE);
        PJR[0].set(i02,iuR,PJRR);
      }
      else if(Atmp and coordinateTransformation and dynamic_cast<RotationAboutAxesZYX<VecV>*>(Atmp->getFunction())) {
        fTR = new RotationAboutAxesZYXMapping<VecV>;
        fTR->setParent(this);
        constJR = true;
        constjR = true;
        PJRR = SqrMat3(EYE);
        PJR[0].set(i02,iuR,PJRR);
      }

      if(fPrPK) {
        if(fPrPK->constParDer1()) {
          constJT = true;
          PJTT = fPrPK->parDer1(qTRel,0);
          PJT[0].set(i02,iuT,PJTT);
        }
        if(fPrPK->constParDer2()) {
          constjT = true;
          PjhT = fPrPK->parDer2(qTRel,0);
        }
      }
      if(fAPK) {
        if(fAPK->constParDer1()) {
          constJR = true;
          PJRR = fTR?fAPK->parDer1(qRRel,0)*(*fTR)(qRRel):fAPK->parDer1(qRRel,0);
          PJR[0].set(i02,iuR,PJRR);
        }
        if(fAPK->constParDer2()) {
          constjR = true;
          PjhR = fAPK->parDer2(qRRel,0);
        }
      }

      if(cb) {
        frameForJacobianOfRotation = K;
        // TODO: do not invert generalized mass matrix in case of special
        // parametrisation
//        if(K == C && dynamic_cast<DynamicSystem*>(R->getParent())) {
//          if(fPrPK) {
//            fPrPK->updateJacobian(qRel(iqT),0);
//            PJT[0].set(i02,iuT,fPrPK->getJacobian());
//          }
//          if(fAPK) {
//            fAPK->updateJacobian(qRel(iqR),0);
//            PJR[0].set(i02,iuR,fAPK->getJacobian());
//          }
//          updateM_ = &FlexibleBodyFFR::updateMConst;
//          Mbuf = m*JTJ(PJT[0]) + JTMJ(SThetaS,PJR[0]);
//          LLM[0] = facLL(Mbuf);
//          facLLM_ = &FlexibleBodyFFR::facLLMConst;
//        }
      }
      else
        frameForJacobianOfRotation = R;

      T.init(Eye());
    }
    else if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
        if(getPlotFeature(notMinimalState)==enabled) {
          for(int i=0; i<nq; i++)
            plotColumns.push_back("qRel("+numtostr(i)+")");
          for(int i=0; i<nu[0]; i++)
            plotColumns.push_back("uRel("+numtostr(i)+")");
        }
        Body::init(stage);
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled) {
          if(getPlotFeature(openMBV)==enabled && FWeight) {
            FWeight->setName("Weight");
            openMBVGrp->addObject(FWeight);
          }
        }
#endif
      }
    }
    else
      Body::init(stage);
    if(fTR) fTR->init(stage);
    if(fPrPK) fPrPK->init(stage);
    if(fAPK) fAPK->init(stage);
  }

  void FlexibleBodyFFR::initz() {
    Object::initz();
    qRel>>q;
    uRel>>u;
    TRel>>T;
  }

  void FlexibleBodyFFR::setUpInverseKinetics() {
    InverseKineticsJoint *joint = new InverseKineticsJoint(string("Joint_")+R->getParent()->getName()+"_"+name);
    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(joint);
    joint->setForceDirection(Mat3xV(3,EYE));
    joint->setMomentDirection(Mat3xV(3,EYE));
    joint->connect(R,K);
    joint->setBody(this);
    if(FArrow)
      joint->setOpenMBVForce(FArrow);
    if(MArrow)
      joint->setOpenMBVMoment(MArrow);
  }

  void FlexibleBodyFFR::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      if(getPlotFeature(notMinimalState)==enabled) {
        for(int i=0; i<nq; i++)
          plotVector.push_back(qRel.e(i));
        for(int i=0; i<nu[0]; i++)
          plotVector.push_back(uRel.e(i));
      }

#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled) {
        if(FWeight) {
          vector<double> data;
          data.push_back(t);
          Vec3 WrOP=K->getPosition();
          Vec3 WG = m*MBSimEnvironment::getInstance()->getAccelerationOfGravity();
          data.push_back(WrOP(0));
          data.push_back(WrOP(1));
          data.push_back(WrOP(2));
          data.push_back(WG(0));
          data.push_back(WG(1));
          data.push_back(WG(2));
          data.push_back(1.0);
          FWeight->append(data);
        }
        if(openMBVBody) {
          vector<double> data;
          data.push_back(t);
          Vec3 WrOS=openMBVFrame->getPosition();
          Vec3 cardan=AIK2Cardan(openMBVFrame->getOrientation());
          data.push_back(WrOS(0));
          data.push_back(WrOS(1));
          data.push_back(WrOS(2));
          data.push_back(cardan(0));
          data.push_back(cardan(1));
          data.push_back(cardan(2));
          data.push_back(0);
          ((OpenMBV::RigidBody*)openMBVBody)->append(data);
        }
      }
#endif
      Body::plot(t,dt);
    }
  }

  void FlexibleBodyFFR::updateqd(double t) {
    qd(iqT) = uRel(iuT);
    if(fTR)
      qd(iqR) = (*fTR)(qRel(iuR))*uRel(iuR);
    else
      qd(iqR) = uRel(iuR);
    qd(iqE) = uRel(iuE);
  }

  void FlexibleBodyFFR::updatedq(double t, double dt) {
    qd(iqT) = uRel(iuT)*dt;
    if(fTR)
      qd(iqR) = (*fTR)(qRel(iuR))*uRel(iuR)*dt;
    else
      qd(iqR) = uRel(iuR)*dt;
    qd(iqE) = uRel(iuE)*dt;
  }

  void FlexibleBodyFFR::updateT(double t) {
    if(fTR) TRel(iqR,iuR) = (*fTR)(qRel(iuR));
  }

  void FlexibleBodyFFR::updateKinematicsForSelectedFrame(double t) {

    if(fPrPK) {
      qTRel = qRel(iqT);
      uTRel = uRel(iuT);
      PrPK = (*fPrPK)(qTRel,t);
      if(!constJT) {
        PJTT = fPrPK->parDer1(qTRel,t);
        PJT[0].set(i02,iuT,PJTT);
//        cout << "JT" << endl;
      }
      if(!constjT) {
        PjhT = fPrPK->parDer2(qTRel,t);
//        cout << "jT" << endl;
      }
      WvPKrel = R->getOrientation()*(PJTT*uTRel + PjhT);
    }

    if(fAPK) {
      qRRel = qRel(iqR);
      uRRel = uRel(iuR);
      APK = (*fAPK)(qRRel,t);
      if(!constJR) {
        PJRR = fTR?fAPK->parDer1(qRRel,t)*(*fTR)(qRRel):fAPK->parDer1(qRRel,t);
        PJR[0].set(i02,iuR,PJRR);
//        cout << "JR" << endl;
      }
      if(!constjR) {
        PjhR = fAPK->parDer2(qRRel,t);
//        cout << "jr" << endl;
      }
      WomPK = frameForJacobianOfRotation->getOrientation()*(PJRR*uRRel + PjhR);
    }

    K->setOrientation(R->getOrientation()*APK);
    WrPK = R->getOrientation()*PrPK;

    K->setAngularVelocity(R->getAngularVelocity() + WomPK);
    K->setPosition(WrPK + R->getPosition());
    K->setVelocity(R->getVelocity() + WvPKrel + crossProduct(R->getAngularVelocity(),WrPK));

    Vec3 mc = mCM.getM0() + mCM.getM1()*q(iqE);
    SqrMat3 mtc = tilde(mc);

    SymMat3 I1;
    SqrMat3 I2;
    for (int i=0; i<ne; i++) {
      I1 += mmi.getM1()[i]*q(iqE).e(i);
      for (int j=0; j<ne; j++)
        I2 += mmi.getM2()[i][j]*(q(iqE).e(i)*q(iqE).e(j));
    }
    SymMat3 I = mmi.getM0() + I1 + SymMat3(I2);
    for(int i=0; i<3; i++) {
      for(int j=0; j<3; j++) {
        M_.e(i+3,j+3) = I.e(i,j);
        M_.e(i+3,j) = mtc.e(i,j);
      }
    }

    MatVx3 Cr1(ne,NONINIT);
    for(int i=0; i<3; i++)
      Cr1.set(i,Cr.getM1()[i]*q(iqE));
    for(int i=0; i<ne; i++)
      for(int j=0; j<3; j++)
        M_.e(i+6,j+3) = Cr.getM0().e(i,j) + Cr1.e(i,j);

    MatVx3 Ct_ = Ct.getM0();
    if(Ct.getM1().size()) {
      MatVx3 Ct1(ne,NONINIT);
      for(int i=0; i<3; i++)
        Ct1.set(i,Ct.getM1()[i]*q(iqE)); 
      Ct_ += Ct1;
      for(int i=0; i<ne; i++)
        for(int j=0; j<3; j++)
          M_.e(i+6,j) = Ct_.e(i,j);
    }

    fmatvec::Matrix<fmatvec::General,fmatvec::Var,fmatvec::Fixed<6>,double> Oe_(ne,NONINIT), Oe1(ne,NONINIT);
    for(int i=0; i<6; i++)
      Oe1.set(i,Oe.getM1()[i]*q(iqE));

    Oe_ = Oe.getM0() + Oe1;

    std::vector<fmatvec::SqrMat3> Gr1(ne);
    fmatvec::SqrMat3 hom21;
    for(int i=0; i<ne; i++) {
      Gr1[i].init(0);
      for(int j=0; j<ne; j++)
        Gr1[i] += Gr.getM1()[j][i]*q(iqE).e(j);
      hom21 += (Gr.getM0()[i]+Gr1[i])*u(iuE).e(i);
    }
    fmatvec::MatVx3 Ge_(ne,NONINIT);
    for(int i=0; i<3; i++)
      Ge_.set(i,Ge.getM0()[i]*u(iuE));

    Vec3 om = K->getOrientation().T()*K->getAngularVelocity();
    Vector<Fixed<6>,double> omq;
    for(int i=0; i<3; i++)
      omq.e(i) = pow(om.e(i),2);
    omq.e(3) = om.e(0)*om.e(1); 
    omq.e(4) = om.e(1)*om.e(2); 
    omq.e(5) = om.e(0)*om.e(2);

    VecV hom(6+ne), hg(6+ne), he(6+ne);

    hom.set(Index(0,2),-crossProduct(om,crossProduct(om,mc))-2.*crossProduct(om,Ct_.T()*u(iuE)));
    hom.set(Index(3,5),-(hom21*om) - crossProduct(om,I*om));
    hom.set(Index(6,6+ne-1),-(Ge_*om) - Oe_*omq);

    Vec Kg = K->getOrientation().T()*(MBSimEnvironment::getInstance()->getAccelerationOfGravity());
    hg.set(Index(0,2),m*Kg);
    hg.set(Index(3,5),mtc*Kg);
    hg.set(Index(6,hg.size()-1),Ct_*Kg);

    SqrMatV Ke_ = SqrMatV(Ke.getM0());
    for(unsigned int i=0; i<Ke.getM1().size(); i++) {
      Ke_ += Ke.getM1()[i]*q(iqE).e(i);
      for(unsigned int j=0; j<Ke.getM2().size(); j++)
        Ke_ += Ke.getM2()[i][j]*(q(iqE).e(i)*q(iqE).e(j));
    }

    VecV ke = Ke_*q(iqE) + De.getM0()*qd(iqE);

    if(ksigma.getM0().size())
      ke += ksigma.getM0();
    if(ksigma.getM1().size())
      ke += ksigma.getM1()*q(iqE);

    he.set(Index(6,hg.size()-1),ke);

    h_ = hom + hg - he;

//    cout << "M_" << endl;
//    cout << M_ << endl;
//    cout << "h_" << endl;
//    cout << h_ << endl;
  }

  void FlexibleBodyFFR::updateJacobiansForSelectedFrame0(double t) {
    K->getJacobianOfTranslation().init(0);
    K->getJacobianOfRotation().init(0);

    uTRel = uRel(iuT);
    uRRel = uRel(iuR);
    VecV qdTRel = uTRel;
    VecV qdRRel = fTR ? (*fTR)(qRRel)*uRRel : uRRel;
    if(fPrPK) {
      if(not(constJT and constjT)) {
//        cout << "jbT" << endl;
        PjbT = (fPrPK->parDer1DirDer1(qdTRel,qTRel,t)+fPrPK->parDer1ParDer2(qTRel,t))*uTRel + fPrPK->parDer2DirDer1(qdTRel,qTRel,t) + fPrPK->parDer2ParDer2(qTRel,t);
      }
    }
    if(fAPK) {
      if(not(constJR and constjR)) {
//        cout << "jbR" << endl;
        if(fTR) {
          Mat3xV JRd = (fAPK->parDer1DirDer1(qdRRel,qRRel,t)+fAPK->parDer1ParDer2(qRRel,t));
          MatV TRd = fTR->dirDer(qdRRel,qRRel);
          PjbR = JRd*qdRRel + fAPK->parDer1(qRRel,t)*TRd*uRRel + fAPK->parDer2DirDer1(qdRRel,qRRel,t) + fAPK->parDer2ParDer2(qRRel,t);
        }
        else
          PjbR = (fAPK->parDer1DirDer1(qdRRel,qRRel,t)+fAPK->parDer1ParDer2(qRRel,t))*uRRel + fAPK->parDer2DirDer1(qdRRel,qRRel,t) + fAPK->parDer2ParDer2(qRRel,t);
      }
    }

    SqrMat3 tWrPK = tilde(WrPK);
    K->setGyroscopicAccelerationOfTranslation(R->getGyroscopicAccelerationOfTranslation() - tWrPK*R->getGyroscopicAccelerationOfRotation() + R->getOrientation()*PjbT + crossProduct(R->getAngularVelocity(), 2.*WvPKrel+crossProduct(R->getAngularVelocity(),WrPK)));
    K->setGyroscopicAccelerationOfRotation(R->getGyroscopicAccelerationOfRotation() + frameForJacobianOfRotation->getOrientation()*PjbR + crossProduct(R->getAngularVelocity(), WomPK));

    K->getJacobianOfTranslation().set(i02,Index(0,R->getJacobianOfTranslation().cols()-1), R->getJacobianOfTranslation() - tWrPK*R->getJacobianOfRotation());
    K->getJacobianOfRotation().set(i02,Index(0,R->getJacobianOfRotation().cols()-1), R->getJacobianOfRotation());

    K->getJacobianOfTranslation().add(i02,Index(gethSize(0)-getuSize(0),gethSize(0)-1), R->getOrientation()*PJT[0]);
    K->getJacobianOfRotation().add(i02,Index(gethSize(0)-getuSize(0),gethSize(0)-1), frameForJacobianOfRotation->getOrientation()*PJR[0]);

    KJ[0].set(Index(0,2),Index(0,KJ[0].cols()-1),K->getOrientation().T()*K->getJacobianOfTranslation());
    KJ[0].set(Index(3,5),Index(0,KJ[0].cols()-1),K->getOrientation().T()*K->getJacobianOfRotation());
    Ki[0].set(Index(0,2),K->getOrientation().T()*K->getGyroscopicAccelerationOfTranslation());
    Ki[0].set(Index(3,5),K->getOrientation().T()*K->getGyroscopicAccelerationOfRotation());
  }

  void FlexibleBodyFFR::updateKinematicsForRemainingFramesAndContours(double t) {
    for(unsigned int i=0; i<RBF.size(); i++)
      RBF[i]->updateStateDependentVariables();
    //for(unsigned int i=0; i<RBC.size(); i++)
    //  RBC[i]->updateStateDependentVariables(t);
  }

  void FlexibleBodyFFR::updateJacobiansForRemainingFramesAndContours(double t, int j) {
    for(unsigned int i=0; i<RBF.size(); i++)
      RBF[i]->updateJacobians(j);
    //for(unsigned int i=0; i<RBC.size(); i++)
    //  RBC[i]->updateJacobians(t,j);
  }

  void FlexibleBodyFFR::updateJacobiansForRemainingFramesAndContours1(double t) {
    KJ[1].set(Index(0,2),Index(0,KJ[1].cols()-1),K->getOrientation().T()*K->getJacobianOfTranslation(1));
    KJ[1].set(Index(3,5),Index(0,KJ[1].cols()-1),K->getOrientation().T()*K->getJacobianOfRotation(1));
    KJ[1].set(Index(6,KJ[1].rows()-1),Index(0,KJ[1].cols()-1),K->getJacobianOfDeformation(1));
    for(unsigned int i=0; i<RBF.size(); i++) {
      RBF[i]->updateRelativePosition();
      RBF[i]->updateJacobians(1);
    }
    for(unsigned int i=0; i<RBC.size(); i++)
      RBC[i]->updateJacobians(t,1);
  }

  void FlexibleBodyFFR::updateqRef(const Vec& ref) {
    Object::updateqRef(ref);
    qRel>>q;
    for(unsigned int i=0; i<frame.size(); i++)
      static_cast<FixedNodalFrame*>(frame[i])->updateqRef(q(nq-ne,nq-1));
  }

  void FlexibleBodyFFR::updateuRef(const Vec& ref) {
    Object::updateuRef(ref);
    uRel>>u;
    for(unsigned int i=0; i<frame.size(); i++)
      static_cast<FixedNodalFrame*>(frame[i])->updateqdRef(u(nu[0]-ne,nu[0]-1));
  }

  void FlexibleBodyFFR::addFrame(FixedNodalFrame *frame_) {
    Body::addFrame(frame_);
  }

#ifdef HAVE_OPENMBVCPPINTERFACE
  void FlexibleBodyFFR::setOpenMBVRigidBody(OpenMBV::RigidBody* body) {
    openMBVBody=body;
  }
#endif

  void FlexibleBodyFFR::updateMConst(double t, int i) {
    M[i] += Mbuf;
  }

  void FlexibleBodyFFR::updateMNotConst(double t, int index) {
    M[index] += JTMJ(M_,KJ[index]); 
//    cout << M[index] << endl;
//    cout << inv(M[index]) << endl;
//    cout << facLL(M[index]) << endl;
//    cout << "ende" << endl;
  }

  void FlexibleBodyFFR::initializeUsingXML(DOMElement *element) {
    DOMElement *e;
    Body::initializeUsingXML(element);

    // frames
    e=E(element)->getFirstElementChildNamed(MBSIM%"frames")->getFirstElementChild();
    while(e) {
      FixedNodalFrame *f=new FixedNodalFrame(E(e)->getAttribute("name"));
      addFrame(f);
      f->initializeUsingXML(e);
      e=e->getNextElementSibling();
    }

    // contours
    e=E(element)->getFirstElementChildNamed(MBSIM%"contours")->getFirstElementChild();
    while(e) {
      Contour *c=ObjectFactory::createAndInit<Contour>(e);
      addContour(c);
      e=e->getNextElementSibling();
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"mass");
    setMass(getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIM%"inertiaTensor");
    setI0(getSymMat3(e));
    e=E(element)->getFirstElementChildNamed(MBSIM%"generalTranslation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<Vec3(VecV,double)> *trans=ObjectFactory::createAndInit<MBSim::Function<Vec3(VecV,double)> >(e->getFirstElementChild());
      setGeneralTranslation(trans);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"timeDependentTranslation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<Vec3(double)> *trans=ObjectFactory::createAndInit<MBSim::Function<Vec3(double)> >(e->getFirstElementChild());
      setTimeDependentTranslation(trans);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"stateDependentTranslation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<Vec3(VecV)> *trans=ObjectFactory::createAndInit<MBSim::Function<Vec3(VecV)> >(e->getFirstElementChild());
      setStateDependentTranslation(trans);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"generalRotation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<RotMat3(VecV,double)> *rot=ObjectFactory::createAndInit<MBSim::Function<RotMat3(VecV,double)> >(e->getFirstElementChild());
      setGeneralRotation(rot);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"timeDependentRotation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<RotMat3(double)> *rot=ObjectFactory::createAndInit<MBSim::Function<RotMat3(double)> >(e->getFirstElementChild());
      setTimeDependentRotation(rot);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"stateDependentRotation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<RotMat3(VecV)> *rot=ObjectFactory::createAndInit<MBSim::Function<RotMat3(VecV)> >(e->getFirstElementChild());
      setStateDependentRotation(rot);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"translationDependentRotation");
    if(e) translationDependentRotation = getBool(e);
    e=E(element)->getFirstElementChildNamed(MBSIM%"coordinateTransformationForRotation");
    if(e) coordinateTransformation = getBool(e);

#ifdef HAVE_OPENMBVCPPINTERFACE
    e=E(element)->getFirstElementChildNamed(MBSIM%"openMBVRigidBody");
    if(e) {
      OpenMBV::RigidBody *rb=OpenMBV::ObjectFactory::create<OpenMBV::RigidBody>(e->getFirstElementChild());
      setOpenMBVRigidBody(rb);
      rb->initializeUsingXML(e->getFirstElementChild());
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"openMBVFrameOfReference");
    if(e) setOpenMBVFrameOfReference(getByPath<Frame>(E(e)->getAttribute("ref"))); // must be on of "Frame[X]" which allready exists

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVFrameC");
    if(e) {
      if(!openMBVBody) setOpenMBVRigidBody(new OpenMBV::InvisibleBody);
      OpenMBVFrame ombv;
      K->setOpenMBVFrame(ombv.createOpenMBV(e));
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVWeight");
    if(e) {
      if(!openMBVBody) setOpenMBVRigidBody(new OpenMBV::InvisibleBody);
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FWeight=ombv.createOpenMBV(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVJointForce");
    if (e) {
      if(!openMBVBody) setOpenMBVRigidBody(new OpenMBV::InvisibleBody);
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FArrow=ombv.createOpenMBV(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVJointMoment");
    if (e) {
      if(!openMBVBody) setOpenMBVRigidBody(new OpenMBV::InvisibleBody);
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
      MArrow=ombv.createOpenMBV(e);
    }
#endif
  }

  DOMElement* FlexibleBodyFFR::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = Body::writeXMLFile(parent);

//    DOMElement * ele1 = new DOMElement( MBSIM%"frameForKinematics" );
//    string str = string("Frame[") + getFrameForKinematics()->getName() + "]";
//    ele1->SetAttribute("ref", str);
//    ele0->LinkEndChild(ele1);
//
//    addElementText(ele0,MBSIM%"mass",getMass());
//    if(frameForInertiaTensor)
//      THROW_MBSIMERROR("Inertia tensor with respect to frame " + frameForInertiaTensor->getPath() + " not supported in XML. Provide inertia tensor with respect to frame C.");
//    addElementText(ele0,MBSIM%"inertiaTensor",getInertiaTensor());
//
//    ele1 = new DOMElement( MBSIM%"translation" );
//    if(getTranslation()) 
//      getTranslation()->writeXMLFile(ele1);
//    ele0->LinkEndChild(ele1);
//
//    ele1 = new DOMElement( MBSIM%"rotation" );
//    if(getRotation()) 
//      getRotation()->writeXMLFile(ele1);
//    ele0->LinkEndChild(ele1);
//
//    ele1 = new DOMElement( MBSIM%"frames" );
//    for(vector<Frame*>::iterator i = frame.begin()+1; i != frame.end(); ++i) 
//      (*i)->writeXMLFile(ele1);
//    ele0->LinkEndChild( ele1 );
//
//    ele1 = new DOMElement( MBSIM%"contours" );
//    for(vector<Contour*>::iterator i = contour.begin(); i != contour.end(); ++i) 
//      (*i)->writeXMLFile(ele1);
//    ele0->LinkEndChild( ele1 );
//
//#ifdef HAVE_OPENMBVCPPINTERFACE
//    if(getOpenMBVBody()) {
//      ele1 = new DOMElement( MBSIM%"openMBVRigidBody" );
//      getOpenMBVBody()->writeXMLFile(ele1);
//
//      if(getOpenMBVFrameOfReference()) {
//        DOMElement * ele2 = new DOMElement( MBSIM%"frameOfReference" );
//        string str = string("Frame[") + getOpenMBVFrameOfReference()->getName() + "]";
//        ele2->SetAttribute("ref", str);
//        ele1->LinkEndChild(ele2);
//      }
//      ele0->LinkEndChild(ele1);
//    }
//
//    if(C->getOpenMBVFrame()) {
//      ele1 = new DOMElement( MBSIM%"enableOpenMBVFrameC" );
//      addElementText(ele1,MBSIM%"size",C->getOpenMBVFrame()->getSize());
//      addElementText(ele1,MBSIM%"offset",C->getOpenMBVFrame()->getOffset());
//      ele0->LinkEndChild(ele1);
//    }
//
//    if(FWeight) {
//      ele1 = new DOMElement( MBSIM%"openMBVWeightArrow" );
//      FWeight->writeXMLFile(ele1);
//      ele0->LinkEndChild(ele1);
//    }
//
//    if(FArrow) {
//      ele1 = new DOMElement( MBSIM%"openMBVJointForceArrow" );
//      FArrow->writeXMLFile(ele1);
//      ele0->LinkEndChild(ele1);
//    }
//
//    if(MArrow) {
//      ele1 = new DOMElement( MBSIM%"openMBVJointMomentArrow" );
//      MArrow->writeXMLFile(ele1);
//      ele0->LinkEndChild(ele1);
//    }
//#endif

    return ele0;
  }

}
