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
 * Contact: martin.o.foerg@gmail.com
 */

#include <config.h>
#include "mbsim/observers/rigid_body_system_observer.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/environment.h"
#include "mbsim/utils/rotarymatrices.h"
#include <openmbvcppinterface/frame.h>
#include <openmbvcppinterface/arrow.h>
#include <openmbvcppinterface/group.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, RigidBodySystemObserver)

  RigidBodySystemObserver::RigidBodySystemObserver(const std::string &name) : Observer(name), frameOfReference(nullptr) {
  }

  void RigidBodySystemObserver::init(InitStage stage, const InitConfigSet &config) {
    if(stage==resolveStringRef) {
      for(const auto & i : saved_body)
        body.push_back(getByPath<RigidBody>(i));
      if(not saved_frameOfReference.empty())
        setFrameOfReference(getByPath<Frame>(saved_frameOfReference));
      if(body.empty())
        throwError("No rigid bodies are given!");
      Observer::init(stage, config);
    }
    else if(stage==plotting) {
      Observer::init(stage, config);
      if(plotFeature[openMBV]) {
        if(ombvPosition) {
          openMBVPosition=ombvPosition->createOpenMBV();
          openMBVPosition->setName("Position");
          getOpenMBVGrp()->addObject(openMBVPosition);
        }
        if(ombvVelocity) {
          openMBVVelocity=ombvVelocity->createOpenMBV();
          openMBVVelocity->setName("Velocity");
          getOpenMBVGrp()->addObject(openMBVVelocity);
        }
        if(ombvAcceleration) {
          openMBVAcceleration=ombvAcceleration->createOpenMBV();
          openMBVAcceleration->setName("Acceleration");
          getOpenMBVGrp()->addObject(openMBVAcceleration);
        }
        if(ombvWeight) {
          openMBVWeight=ombvWeight->createOpenMBV();
          openMBVWeight->setName("Weight");
          getOpenMBVGrp()->addObject(openMBVWeight);
        }
        if(ombvMomentum) {
          openMBVMomentum=ombvMomentum->createOpenMBV();
          openMBVMomentum->setName("Momentum");
          getOpenMBVGrp()->addObject(openMBVMomentum);
        }
        if(ombvAngularMomentum) {
          openMBVAngularMomentum=ombvAngularMomentum->createOpenMBV();
          openMBVAngularMomentum->setName("AngularMomentum");
          getOpenMBVGrp()->addObject(openMBVAngularMomentum);
        }
        if(ombvDerivativeOfMomentum) {
          openMBVDerivativeOfMomentum=ombvDerivativeOfMomentum->createOpenMBV();
          openMBVDerivativeOfMomentum->setName("DerivativeOfMomentum");
          getOpenMBVGrp()->addObject(openMBVDerivativeOfMomentum);
        }
        if(ombvDerivativeOfAngularMomentum) {
          openMBVDerivativeOfAngularMomentum=ombvDerivativeOfAngularMomentum->createOpenMBV();
          openMBVDerivativeOfAngularMomentum->setName("DerivativeOfAngularMomentum");
          getOpenMBVGrp()->addObject(openMBVDerivativeOfAngularMomentum);
        }
      }
    }
    else
      Observer::init(stage, config);
  }

  void RigidBodySystemObserver::plot() {
    if(plotFeature[openMBV]) {
      double m = 0;
      for(auto & i : body) {
        m += i->getMass();
      }
      Vec3 mpos, mvel, macc;
      Vec3 &p = mvel;
      Vec3 &pd = macc;
      Vec3 L, Ld;
      for(auto & i : body) {
        mpos += i->getMass()*i->getFrameC()->evalPosition();
        mvel += i->getMass()*i->getFrameC()->evalVelocity();
        macc += i->getMass()*i->getFrameC()->evalAcceleration();
      }
      Vec3 rOS = mpos/m;
      Vec3 vS = mvel/m;
      Vec3 aS = macc/m;
      Vec3 rOR = frameOfReference?frameOfReference->evalPosition():rOS;
      Vec3 vR = frameOfReference?frameOfReference->evalVelocity():vS;
      Vec3 aR = frameOfReference?frameOfReference->evalAcceleration():aS;
      for(auto & i : body) {
        SqrMat3 AIK = i->getFrameC()->getOrientation();
        Vec3 rRSi = i->getFrameC()->getPosition() - rOR;
        Vec3 vRSi = i->getFrameC()->getVelocity() - vR;
        Vec3 aRSi = i->getFrameC()->getAcceleration() - aR;
        Vec3 omi = i->getFrameC()->getAngularVelocity();
        Vec3 psii = i->getFrameC()->getAngularAcceleration();
        double mi = i->getMass();
        Mat3x3 WThetaS = AIK*i->getInertiaTensor()*AIK.T();
        L += WThetaS*omi + crossProduct(rRSi,mi*vRSi);
        Ld += WThetaS*psii + crossProduct(omi,WThetaS*omi) + crossProduct(rRSi,mi*aRSi);
      }
      Vec3 G = m*MBSimEnvironment::getInstance()->getAccelerationOfGravity();
      if(openMBVPosition && !openMBVPosition->isHDF5Link()) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(rOR(0));
        data.push_back(rOR(1));
        data.push_back(rOR(2));
        data.push_back(ombvPosition->getColorRepresentation()?nrm2(rOR):0.5);
        openMBVPosition->append(data);
      }
      if(openMBVVelocity && !openMBVVelocity->isHDF5Link()) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(rOR(0));
        data.push_back(rOR(1));
        data.push_back(rOR(2));
        data.push_back(vR(0));
        data.push_back(vR(1));
        data.push_back(vR(2));
        data.push_back(ombvVelocity->getColorRepresentation()?nrm2(vR):0.5);
        openMBVVelocity->append(data);
      }
      if(openMBVAcceleration && !openMBVAcceleration->isHDF5Link()) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(rOR(0));
        data.push_back(rOR(1));
        data.push_back(rOR(2));
        data.push_back(aR(0));
        data.push_back(aR(1));
        data.push_back(aR(2));
        data.push_back(ombvAcceleration->getColorRepresentation()?nrm2(aR):0.5);
        openMBVAcceleration->append(data);
      }
      if(openMBVWeight) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(rOS(0));
        data.push_back(rOS(1));
        data.push_back(rOS(2));
        data.push_back(G(0));
        data.push_back(G(1));
        data.push_back(G(2));
        data.push_back(ombvWeight->getColorRepresentation()?nrm2(G):1.0);
        openMBVWeight->append(data);
      }
      if(openMBVMomentum) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(rOS(0));
        data.push_back(rOS(1));
        data.push_back(rOS(2));
        data.push_back(p(0));
        data.push_back(p(1));
        data.push_back(p(2));
        data.push_back(ombvMomentum->getColorRepresentation()?nrm2(p):1.0);
        openMBVMomentum->append(data);
      }
      if(openMBVAngularMomentum) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(rOR(0));
        data.push_back(rOR(1));
        data.push_back(rOR(2));
        data.push_back(L(0));
        data.push_back(L(1));
        data.push_back(L(2));
        data.push_back(ombvAngularMomentum->getColorRepresentation()?nrm2(L):1);
        openMBVAngularMomentum->append(data);
      }
      if(openMBVDerivativeOfMomentum) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(rOS(0));
        data.push_back(rOS(1));
        data.push_back(rOS(2));
        data.push_back(pd(0));
        data.push_back(pd(1));
        data.push_back(pd(2));
        data.push_back(ombvDerivativeOfMomentum->getColorRepresentation()?nrm2(pd):1);
        openMBVDerivativeOfMomentum->append(data);
      }
      if(openMBVDerivativeOfAngularMomentum) {
        vector<double> data;
        data.push_back(getTime());
        data.push_back(rOR(0));
        data.push_back(rOR(1));
        data.push_back(rOR(2));
        data.push_back(Ld(0));
        data.push_back(Ld(1));
        data.push_back(Ld(2));
        data.push_back(ombvDerivativeOfAngularMomentum->getColorRepresentation()?nrm2(Ld):1);
        openMBVDerivativeOfAngularMomentum->append(data);
      }
    }
    Observer::plot();
  }

  void RigidBodySystemObserver::initializeUsingXML(DOMElement *element) {
    Observer::initializeUsingXML(element);

    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"rigidBody");
    while(e && E(e)->getTagName()==MBSIM%"rigidBody") {
      saved_body.push_back(E(e)->getAttribute("ref"));
      e=e->getNextElementSibling();
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"frameOfReference");
    if(e) saved_frameOfReference=E(e)->getAttribute("ref");

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVPosition");
    if(e) {
      ombvPosition = shared_ptr<OpenMBVArrow>(new OpenMBVArrow);
      ombvPosition->initializeUsingXML(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVVelocity");
    if(e) {
      ombvVelocity = shared_ptr<OpenMBVArrow>(new OpenMBVArrow);
      ombvVelocity->initializeUsingXML(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVAcceleration");
    if(e) {
      ombvAcceleration = shared_ptr<OpenMBVArrow>(new OpenMBVArrow);
      ombvAcceleration->initializeUsingXML(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVWeight");
    if(e) {
      ombvWeight = shared_ptr<OpenMBVArrow>(new OpenMBVArrow(1,1,OpenMBVArrow::toHead,OpenMBVArrow::toPoint));
      ombvWeight->initializeUsingXML(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVMomentum");
    if(e) {
      ombvMomentum = shared_ptr<OpenMBVArrow>(new OpenMBVArrow(1,1,OpenMBVArrow::toHead,OpenMBVArrow::toPoint));
      ombvMomentum->initializeUsingXML(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVAngularMomentum");
    if(e) {
      ombvAngularMomentum = shared_ptr<OpenMBVArrow>(new OpenMBVArrow(1,1,OpenMBVArrow::toDoubleHead,OpenMBVArrow::toPoint));
      ombvAngularMomentum->initializeUsingXML(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVDerivatveOfMomentum");
    if(e) {
      ombvDerivativeOfMomentum = shared_ptr<OpenMBVArrow>(new OpenMBVArrow(1,1,OpenMBVArrow::toHead,OpenMBVArrow::toPoint));
      ombvDerivativeOfMomentum->initializeUsingXML(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVDerivativeOfAngularMomentum");
    if(e) {
      ombvDerivativeOfAngularMomentum = shared_ptr<OpenMBVArrow>(new OpenMBVArrow(1,1,OpenMBVArrow::toDoubleHead,OpenMBVArrow::toPoint));
      ombvDerivativeOfAngularMomentum->initializeUsingXML(e);
    }
  }

}
