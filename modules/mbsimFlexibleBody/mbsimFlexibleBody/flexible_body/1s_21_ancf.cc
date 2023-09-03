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

#include <config.h>
#include "mbsimFlexibleBody/flexible_body/1s_21_ancf.h"
#include "mbsimFlexibleBody/flexible_body/fe/1s_21_ancf.h"
#include "mbsimFlexibleBody/frames/frame_1s.h"
#include "mbsimFlexibleBody/frames/node_frame.h"
#include "mbsim/utils/utils.h"
#include "mbsim/utils/eps.h"
#include "mbsim/environment.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/mbsim_event.h"
#include "mbsim/dynamic_system_solver.h"

using namespace fmatvec;
using namespace std;
using namespace MBSim;

namespace MBSimFlexibleBody {


  FlexibleBody1s21ANCF::FlexibleBody1s21ANCF(const string &name, bool openStructure) : FlexibleBody1s(name,openStructure), Elements(0), l0(0), E(0), A(0), I(0), rho(0), rc(0.), deps(0.), dkappa(0.), initialised(false), v0(0.), Euler(false), sOld(-1e12) { }

  void FlexibleBody1s21ANCF::GlobalVectorContribution(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec) {
    int j = 4 * n;

    if(n < Elements - 1 || openStructure==true) {
      gloVec.add(RangeV(j,j+7), locVec);
    }
    else { // ring closure at finite element (end,1)
      gloVec.add(RangeV(j,j+3), locVec(RangeV(0,3)));
      gloVec.add(RangeV(0,  3), locVec(RangeV(4,7)));
    }
  }

  void FlexibleBody1s21ANCF::GlobalMatrixContribution(int n, const fmatvec::Mat& locMat, fmatvec::Mat& gloMat) {
    int j = 4 * n;

    if(n < Elements - 1 || openStructure==true) {
      gloMat.add(RangeV(j,j+7),RangeV(j,j+7), locMat);
    }
    else { // ring closure at finite element (end,1)
      gloMat.add(RangeV(j,j+3),RangeV(j,j+3), locMat(RangeV(0,3),RangeV(0,3)));
      gloMat.add(RangeV(j,j+3),RangeV(0,3), locMat(RangeV(0,3),RangeV(4,7)));
      gloMat.add(RangeV(0,3),RangeV(j,j+3), locMat(RangeV(4,7),RangeV(0,3)));
      gloMat.add(RangeV(0,3),RangeV(0,3), locMat(RangeV(4,7),RangeV(4,7)));
    }
  }

  void FlexibleBody1s21ANCF::GlobalMatrixContribution(int n, const fmatvec::SymMat& locMat, fmatvec::SymMat& gloMat) {
    int j = 4 * n;

    if(n < Elements - 1 || openStructure==true) {
      gloMat.add(RangeV(j,j+7), locMat);
    }
    else { // ring closure at finite element (end,1) with angle difference 2*M_PI
      gloMat.add(RangeV(j,j+3), locMat(RangeV(0,3)));
      gloMat.add(RangeV(j,j+3),RangeV(0,3), locMat(RangeV(0,3),RangeV(4,7)));
      gloMat.add(RangeV(0,3), locMat(RangeV(4,7)));
    }
  }

  void FlexibleBody1s21ANCF::updatePositions(Frame1s *frame) {
    double sLocal;
    int currentElement;
    BuildElement(frame->getParameter(), sLocal, currentElement); // Lagrange parameter of affected FE
    frame->setPosition(R->evalPosition() + R->evalOrientation() *  static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->getPosition(getqElement(currentElement),sLocal));
    frame->setOrientation(R->evalOrientation() *  static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->getOrientation(getqElement(currentElement),sLocal));
  }

  void FlexibleBody1s21ANCF::updateVelocities(Frame1s *frame) {
    double sLocal;
    int currentElement;
    BuildElement(frame->getParameter(), sLocal, currentElement); // Lagrange parameter of affected FE
    frame->setVelocity(R->evalOrientation() *  static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->getVelocity(getqElement(currentElement),getuElement(currentElement),sLocal));
    frame->setAngularVelocity(R->evalOrientation() *  static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->getAngularVelocity(getqElement(currentElement),getuElement(currentElement),sLocal));
  }

  void FlexibleBody1s21ANCF::updateAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s21ANCF::updateAccelerations): Not implemented.");
  }

  void FlexibleBody1s21ANCF::updateJacobians(Frame1s *frame, int j) {
    RangeV All(0, 3 - 1);
    Mat Jacobian(qSize, 3, INIT, 0.);

    double sLocal;
    int currentElement;
    BuildElement(frame->getParameter(),sLocal,currentElement);
    Mat Jtmp = static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->JGeneralized(getqElement(currentElement),sLocal);
    if(currentElement<Elements-1 || openStructure) {
      Jacobian.set(RangeV(4*currentElement,4*currentElement+7),All, Jtmp);
    }
    else { // ringstructure
      Jacobian.set(RangeV(4*currentElement,4*currentElement+3),All, Jtmp(RangeV(0,3),All));
      Jacobian.set(RangeV(               0,                 3),All, Jtmp(RangeV(4,7),All));
    }

    frame->setJacobianOfTranslation(R->evalOrientation()(RangeV(0,2),RangeV(0,1))*Jacobian(RangeV(0,qSize-1),RangeV(0,1)).T(),j);
    frame->setJacobianOfRotation(R->evalOrientation()(RangeV(0,2),RangeV(2,2))*Jacobian(RangeV(0,qSize-1),RangeV(2,2)).T(),j);
  }

  void FlexibleBody1s21ANCF::updateGyroscopicAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s21ANCF::updateGyroscopicAccelerations): Not implemented.");
  }

  void FlexibleBody1s21ANCF::updatePositions(NodeFrame *frame) {
    Vec3 tmp(NONINIT);
    int node = frame->getNodeNumber();
    tmp(0) = q(4*node+0);
    tmp(1) = q(4*node+1);
    tmp(2) = 0.;
    frame->setPosition(R->evalPosition() + R->evalOrientation() * tmp);
    tmp(0) = q(4*node+2);
    tmp(1) = q(4*node+3);
    tmp(2) = 0.;
    tmp /= nrm2(tmp);
    frame->getOrientation(false).set(0, R->getOrientation() * tmp);
    tmp(0) = q(4*node+2);
    tmp(1) = -q(4*node+3);
    tmp(2) = 0.;
    tmp /= nrm2(tmp);
    std::swap(tmp(0),tmp(1));
    frame->getOrientation(false).set(1, R->getOrientation() * tmp);
    frame->getOrientation(false).set(2, R->getOrientation().col(2));
  }

  void FlexibleBody1s21ANCF::updateVelocities(NodeFrame *frame) {
    Vec3 tmp(NONINIT);
    int node = frame->getNodeNumber();
    tmp(0) = u(4*node+0);
    tmp(1) = u(4*node+1);
    tmp(2) = 0.;
    if(Euler) {
      tmp(0) += v0*q(4*node+2);
      tmp(1) += v0*q(4*node+3);
    }
    frame->setVelocity(R->evalOrientation() * tmp);
    if(Euler) {
      double der2_1; // curvature first component
      double der2_2; // curvature second component
      if(4*(node+1)+1 < qSize) {
        der2_1 = 6./(l0*l0)*(q(4*(node+1))-q(4*(node)))-2./l0*(q(4*(node+1)+2)+2.*q(4*(node)+2));
        der2_2 = 6./(l0*l0)*(q(4*(node+1)+1)-q(4*(node)+1))-2./l0*(q(4*(node+1)+3)+2.*q(4*(node)+3));
      }
      else {
        der2_1 = 6./(l0*l0)*(q(4*(node-1))-q(4*(node)))+2./l0*(q(4*(node-1)+2)+2.*q(4*(node)+2));
        der2_2 = 6./(l0*l0)*(q(4*(node-1)+1)-q(4*(node)+1))+2./l0*(q(4*(node-1)+3)+2.*q(4*(node)+3));
      }
      tmp(0) = 0.; tmp(1) = 0.; tmp(2) = (-q(4*node+3)*(u(4*node+2) + v0*der2_1)+q(4*node+2)*(u(4*node+3) + v0*der2_2))/sqrt(q(4*node+2)*q(4*node+2)+q(4*node+3)*q(4*node+3));
    } else {
      tmp(0) = 0.;
      tmp(1) = 0.;
      tmp(2) = (-q(4*node+3)*u(4*node+2)+q(4*node+2)*u(4*node+3))/sqrt(q(4*node+2)*q(4*node+2)+q(4*node+3)*q(4*node+3));
    }
    frame->setAngularVelocity(R->evalOrientation() * tmp);
  }

  void FlexibleBody1s21ANCF::updateAccelerations(NodeFrame *frame) {
    throwError("(FlexibleBody1s21ANCF::updateAccelerations): Not implemented.");
  }

  void FlexibleBody1s21ANCF::updateJacobians(NodeFrame *frame, int j) {
    RangeV All(0, 3 - 1);
    Mat Jacobian(qSize, 3, INIT, 0.);
    int node = frame->getNodeNumber();

    Jacobian(4*node,0) = 1.;
    Jacobian(4*node+1,1) = 1.;
    Jacobian(4*node+2,2) = -q(4*node+3);
    Jacobian(4*node+3,2) = q(4*node+2);
    Jacobian.set(RangeV(4*node+2,4*node+3),2, Jacobian(RangeV(4*node+2,4*node+3),2)/sqrt(q(4*node+2)*q(4*node+2)+q(4*node+3)*q(4*node+3)));

    frame->setJacobianOfTranslation(R->evalOrientation()(RangeV(0, 2), RangeV(0, 1)) * Jacobian(RangeV(0, qSize - 1), RangeV(0, 1)).T(),j);
    frame->setJacobianOfRotation(R->evalOrientation()(RangeV(0, 2), RangeV(2, 2)) * Jacobian(RangeV(0, qSize - 1), RangeV(2, 2)).T(),j);
  }

  void FlexibleBody1s21ANCF::updateGyroscopicAccelerations(NodeFrame *frame) {
    throwError("(FlexibleBody1s21ANCF::updateGyroscopicAccelerations): Not implemented.");
  }

  void FlexibleBody1s21ANCF::init(InitStage stage, const InitConfigSet &config) {
    if(stage==unknownStage) {
      FlexibleBody1s::init(stage, config);

      initialised = true;

      l0 = L/Elements;
      Vec g = R->getOrientation()(RangeV(0,2),RangeV(0,1)).T()*ds->getMBSimEnvironment()->getAccelerationOfGravity();

      for(int i=0;i<Elements;i++) {
        qElement.emplace_back(8,INIT,0.);
        uElement.emplace_back(8,INIT,0.);
        discretization.push_back(new FiniteElement1s21ANCF(l0, A*rho, E*A, E*I, g, Euler, v0));
        if(fabs(rc) > epsroot)
          static_cast<FiniteElement1s21ANCF*>(discretization[i])->setCurlRadius(rc);
        static_cast<FiniteElement1s21ANCF*>(discretization[i])->setMaterialDamping(deps,dkappa);
      }
      initM();
    }
    else if(stage==plotting) {
      for(int i=0;i<qRel.size()/4;i++) {
        addToPlot("vel_abs node ("+toString(i)+")");
      }
      FlexibleBody1s::init(stage, config);
    }
    else
      FlexibleBody1s::init(stage, config);
  }

  void FlexibleBody1s21ANCF::plot() {
    if(Euler) {
      for(int i=0;i<qRel.size()/4;i++) {
	Element::plot(sqrt(pow(u(4*i+0) + v0*q(4*i+2),2.)+pow(u(4*i+1) + v0*q(4*i+3),2.)));
      }
    }
    else {
      for(int i=0;i<qRel.size()/4;i++) {
	Element::plot(sqrt(pow(u(4*i+0),2.)+pow(u(4*i+1),2.)));
      }
    }
    FlexibleBody1s::plot();
  }

  void FlexibleBody1s21ANCF::setNumberElements(int n) {
    Elements = n;
    if(openStructure) {
      qSize = 4 * (n+1);
    } else {
      qSize = 4 *  n   ;
    }
    uSize[0] = qSize;
    uSize[1] = qSize;
    q0.resize(qSize);
    u0.resize(uSize[0]);
  }

  void FlexibleBody1s21ANCF::BuildElements() {
    for(int i=0;i<Elements;i++) {
      int n = 4 * i ;

      if(i<Elements-1 || openStructure==true) {
        qElement[i] = q(RangeV(n,n+7));
        uElement[i] = u(RangeV(n,n+7));
      }
      else { // last finite element and ring closure
        qElement[i].set(RangeV(0,3), q(RangeV(n,n+3)));
        uElement[i].set(RangeV(0,3), u(RangeV(n,n+3)));
        qElement[i].set(RangeV(4,7), q(RangeV(0,3)));
        uElement[i].set(RangeV(4,7), u(RangeV(0,3)));
      }
    }
    updEle = false;
  }

  void FlexibleBody1s21ANCF::BuildElement(const double& sGlobal, double& sLocal, int& currentElement) {
    double remainder = fmod(sGlobal,L);
    if(openStructure && sGlobal >= L) remainder += L; // remainder \in (-eps,L+eps)
    if(!openStructure && sGlobal < 0.) remainder += L; // remainder \in [0,L)

    currentElement = int(remainder/l0);
    sLocal = remainder - (currentElement) * l0; // Lagrange/Euler parameter of the affected FE with sLocal==0 in the middle of the FE and sGlobal==0 at the beginning of the beam

    // contact solver computes too large sGlobal at the end of the entire beam is not considered only for open structure
    // for closed structure even sGlobal < L (but sGlobal ~ L) values could lead - due to numerical problems - to a wrong currentElement computation
    if(currentElement >= Elements) {
      currentElement =  Elements-1;
      sLocal += l0;
    }
  }

  void FlexibleBody1s21ANCF::initM() {
    for (auto & i : discretization)
      static_cast<FiniteElement1s21ANCF*>(i)->initM(); // compute attributes of finite element
    for (int i = 0; i < (int) discretization.size(); i++)
      GlobalMatrixContribution(i, discretization[i]->getM(), M); // assemble
    for (int i = 0; i < (int) discretization.size(); i++) {
      int j = 4 * i;
      LLM.set(RangeV(j, j + 3), facLL(M(RangeV(j, j + 3))));
      if (openStructure && i == (int) discretization.size() - 1)
        LLM.set(RangeV(j + 4, j + 7), facLL(M(RangeV(j + 4, j + 7))));
    }
  }

  void FlexibleBody1s21ANCF::initInfo() {
    FlexibleBody1s::init(unknownStage, InitConfigSet());
    l0 = L/Elements;
    Vec g = Vec("[0.;0.;0.]");
    for(int i=0;i<Elements;i++) {
      discretization.push_back(new FiniteElement1s21ANCF(l0, A*rho, E*A, E*I, g, Euler, v0));
      qElement.emplace_back(discretization[0]->getqSize(),INIT,0.);
      uElement.emplace_back(discretization[0]->getuSize(),INIT,0.);
    }
  }

  void FlexibleBody1s21ANCF::initRelaxed(double alpha) {
    if(!initialised) {
      if(Elements==0)
        throwError("(FlexibleBody1s21ANCF::initRelaxed): Set number of finite elements!");
      Vec q0Dummy(q0.size(),INIT,0.);
      if(openStructure) {
        Vec direction(2);
        direction(0) = cos(alpha);
        direction(1) = sin(alpha);

        for(int i=0;i<=Elements;i++) {
          q0Dummy.set(RangeV(4*i+0,4*i+1), direction*double(L/Elements*i));
          q0Dummy(4*i+2) = direction(0);
          q0Dummy(4*i+3) = direction(1);
        }
      }
      else {
        double R = L/(2*M_PI);

        for(int i=0;i<Elements;i++) {
          double alpha_ = i*(2*M_PI)/Elements;
          q0Dummy(4*i+0) = R*cos(alpha_);
          q0Dummy(4*i+1) = R*sin(alpha_);
          q0Dummy(4*i+2) = -sin(alpha_);
          q0Dummy(4*i+3) = cos(alpha_);
        }
      }
      setq0(q0Dummy);
      setu0(Vec(q0Dummy.size(),INIT,0.));
    }
  }

  void FlexibleBody1s21ANCF::setCurlRadius(double rc_) {
    rc = rc_;
    if(initialised)
      for(int i = 0; i < Elements; i++)
        static_cast<FiniteElement1s21ANCF*>(discretization[i])->setCurlRadius(rc);
  }

  void FlexibleBody1s21ANCF::setMaterialDamping(double deps_, double dkappa_) {
    deps = deps_;
    dkappa = dkappa_;
    if(initialised)
      for(int i = 0; i < Elements; i++)
        static_cast<FiniteElement1s21ANCF*>(discretization[i])->setMaterialDamping(deps,dkappa);
  }

  void FlexibleBody1s21ANCF::setEulerPerspective(bool Euler_, double v0_) {
    if(openStructure) {
      throwError("(FlexibleBody1s21ANCF::setEulerPerspective): implemented only for closed structures!");
    }
    else {
      Euler = Euler_;
      v0 = v0_;
    }
  }

}

