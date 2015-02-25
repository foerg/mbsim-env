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
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include "mbsim/spring_damper.h"
#include "mbsim/utils/eps.h"
#include "mbsim/objectfactory.h"
#include "mbsim/frame.h"
#include "mbsim/rigid_body.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/coilspring.h>
#include <openmbvcppinterface/arrow.h>
#include "openmbvcppinterface/group.h"
#include "openmbvcppinterface/objectfactory.h"
#endif

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace boost;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(SpringDamper, MBSIM%"SpringDamper")

  SpringDamper::SpringDamper(const string &name) : LinkMechanics(name), func(NULL)
  {}

  SpringDamper::~SpringDamper() {
    delete func;
  }

  void SpringDamper::updateh(double t, int j) {
    la(0)=(*func)(g(0),gd(0));
    if(dist<=epsroot() && abs(la(0))>epsroot())
      msg(Warn)<<"The SpringDamper force is not 0 and the force direction can not calculated!\nUsing force=0 at t="<<t<<endl;
    WF[0]=n*la;
    WF[1]=-WF[0];
    for(unsigned int i=0; i<2; i++)
      h[j][i]+=frame[i]->getJacobianOfTranslation(j).T()*WF[i];
  }

  void SpringDamper::updateg(double) {
    Vec3 WrP0P1=frame[1]->getPosition() - frame[0]->getPosition();
    dist=nrm2(WrP0P1);
    if(dist>epsroot())
      n=WrP0P1/dist;
    else
      n.init(0);
    g(0)=dist;
  } 

  void SpringDamper::updategd(double) {
    Vec3 Wvrel=frame[1]->getVelocity() - frame[0]->getVelocity();
    gd(0)=Wvrel.T()*n;
  }

  void SpringDamper::connect(Frame *frame0, Frame* frame1) {
    LinkMechanics::connect(frame0);
    LinkMechanics::connect(frame1);
  }

  void SpringDamper::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_ref1!="" && saved_ref2!="")
        connect(getByPath<Frame>(saved_ref1), getByPath<Frame>(saved_ref2));
      if(not(frame.size()))
        THROW_MBSIMERROR("No connection given!");
      LinkMechanics::init(stage);
    }
    else if(stage==resize) {
      LinkMechanics::init(stage);
      g.resize(1);
      gd.resize(1);
      la.resize(1);
    }
    else if(stage==plotting) {
      updatePlotFeatures();
      if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled) {
          if(coilspringOpenMBV) {
            coilspringOpenMBV->setName(name);
            parent->getOpenMBVGrp()->addObject(coilspringOpenMBV);
          }
        }
#endif
        LinkMechanics::init(stage);
      }
    }
    else
      LinkMechanics::init(stage);
    func->init(stage);
  }

  void SpringDamper::plot(double t,double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled) {
        if (coilspringOpenMBV) {
          Vec3 WrOToPoint;
          Vec3 WrOFromPoint;

          WrOFromPoint = frame[0]->getPosition();
          WrOToPoint   = frame[1]->getPosition();
          vector<double> data;
          data.push_back(t); 
          data.push_back(WrOFromPoint(0));
          data.push_back(WrOFromPoint(1));
          data.push_back(WrOFromPoint(2));
          data.push_back(WrOToPoint(0));
          data.push_back(WrOToPoint(1));
          data.push_back(WrOToPoint(2));
          data.push_back(la(0));
          coilspringOpenMBV->append(data);
        }
      }
#endif
      LinkMechanics::plot(t,dt);
    }
  }

  void SpringDamper::initializeUsingXML(DOMElement *element) {
    LinkMechanics::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"forceFunction");
    Function<double(double,double)> *f=ObjectFactory::createAndInit<Function<double(double,double)> >(e->getFirstElementChild());
    setForceFunction(f);
    e=E(element)->getFirstElementChildNamed(MBSIM%"connect");
    saved_ref1=E(e)->getAttribute("ref1");
    saved_ref2=E(e)->getAttribute("ref2");
#ifdef HAVE_OPENMBVCPPINTERFACE
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVCoilSpring");
    if(e) {
      OpenMBVCoilSpring ombv;
      coilspringOpenMBV=ombv.createOpenMBV(e);
    }
    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      std::vector<bool> which; which.resize(2, true);
      LinkMechanics::setOpenMBVForceArrow(ombv.createOpenMBV(e), which);
    }
#endif
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(DirectionalSpringDamper, MBSIM%"DirectionalSpringDamper")

  DirectionalSpringDamper::DirectionalSpringDamper(const string &name) : LinkMechanics(name), func(NULL), refFrame(NULL)
  {}

  DirectionalSpringDamper::~DirectionalSpringDamper() {
    delete func;
  }

  void DirectionalSpringDamper::updateh(double t, int j) {
    Mat3x3 tWrP0P1 = tilde(WrP0P1);

    C.setJacobianOfTranslation(frame[0]->getJacobianOfTranslation(j) - tWrP0P1*frame[0]->getJacobianOfRotation(j),j);
    C.setJacobianOfRotation(frame[0]->getJacobianOfRotation(j),j);
    C.setGyroscopicAccelerationOfTranslation(frame[0]->getGyroscopicAccelerationOfTranslation(j) - tWrP0P1*frame[0]->getGyroscopicAccelerationOfRotation(j) + crossProduct(frame[0]->getAngularVelocity(),crossProduct(frame[0]->getAngularVelocity(),WrP0P1)),j);
    C.setGyroscopicAccelerationOfRotation(frame[0]->getGyroscopicAccelerationOfRotation(j),j);

    la(0)=(*func)(g(0),gd(0));
    WF[0]=WforceDir*la; // projected force in direction of WforceDir
    WF[1]=-WF[0];
    h[j][0]+=C.getJacobianOfTranslation(j).T()*WF[0];
    h[j][1]+=frame[1]->getJacobianOfTranslation(j).T()*WF[1];
  }

  void DirectionalSpringDamper::updateg(double) {
    WrP0P1=frame[1]->getPosition() - frame[0]->getPosition();

    WforceDir=refFrame->getOrientation()*forceDir; // force direction in world system
    g(0)=WrP0P1.T()*WforceDir;

    C.setOrientation(frame[0]->getOrientation());
    C.setPosition(frame[1]->getPosition() - WforceDir*g(0));
  } 

  void DirectionalSpringDamper::updategd(double) {
    C.setAngularVelocity(frame[0]->getAngularVelocity());
    C.setVelocity(frame[0]->getVelocity() + crossProduct(frame[0]->getAngularVelocity(),WrP0P1));
    Vec3 WvP0P1 = frame[1]->getVelocity()-C.getVelocity();
    gd(0)=WvP0P1.T()*WforceDir;
  }

  void DirectionalSpringDamper::connect(Frame *frame0, Frame* frame1) {
    LinkMechanics::connect(frame0);
    LinkMechanics::connect(frame1);
  }

  void DirectionalSpringDamper::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_ref1!="" && saved_ref2!="")
        connect(getByPath<Frame>(saved_ref1), getByPath<Frame>(saved_ref2));
      if(not(frame.size()))
        THROW_MBSIMERROR("No connection given!");
      LinkMechanics::init(stage);
    }
    else if(stage==resize) {
      LinkMechanics::init(stage);
      g.resize(1);
      gd.resize(1);
      la.resize(1);
    }
    else if(stage==plotting) {
      updatePlotFeatures();
      if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled) {
          if(coilspringOpenMBV) {
            coilspringOpenMBV->setName(name);
            parent->getOpenMBVGrp()->addObject(coilspringOpenMBV);
          }
        }
#endif
        LinkMechanics::init(stage);
      }
    }
    else if(stage==unknownStage) {
      refFrame=frame[0];
      C.getJacobianOfTranslation(0).resize(frame[0]->getJacobianOfTranslation(0).cols());
      C.getJacobianOfRotation(0).resize(frame[0]->getJacobianOfRotation(0).cols());
      C.getJacobianOfTranslation(1).resize(frame[0]->getJacobianOfTranslation(1).cols());
      C.getJacobianOfRotation(1).resize(frame[0]->getJacobianOfRotation(1).cols());
      LinkMechanics::init(stage);
    }
    else
      LinkMechanics::init(stage);
    func->init(stage);
  }

  void DirectionalSpringDamper::plot(double t,double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled) {
        if (coilspringOpenMBV) {
          Vec3 WrOToPoint;
          Vec3 WrOFromPoint;

          WrOFromPoint = C.getPosition();
          WrOToPoint   = frame[1]->getPosition();
          vector<double> data;
          data.push_back(t); 
          data.push_back(WrOFromPoint(0));
          data.push_back(WrOFromPoint(1));
          data.push_back(WrOFromPoint(2));
          data.push_back(WrOToPoint(0));
          data.push_back(WrOToPoint(1));
          data.push_back(WrOToPoint(2));
          data.push_back(la(0));
          coilspringOpenMBV->append(data);
        }
      }
#endif
      LinkMechanics::plot(t,dt);
    }
  }

  void DirectionalSpringDamper::initializeUsingXML(DOMElement *element) {
    LinkMechanics::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"forceDirection");
    setForceDirection(getVec(e,3));
    e=E(element)->getFirstElementChildNamed(MBSIM%"forceFunction");
    Function<double(double,double)> *f=ObjectFactory::createAndInit<Function<double(double,double)> >(e->getFirstElementChild());
    setForceFunction(f);
    e=E(element)->getFirstElementChildNamed(MBSIM%"connect");
    saved_ref1=E(e)->getAttribute("ref1");
    saved_ref2=E(e)->getAttribute("ref2");
#ifdef HAVE_OPENMBVCPPINTERFACE
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVCoilSpring");
    if(e) {
      OpenMBVCoilSpring ombv;
      coilspringOpenMBV=ombv.createOpenMBV(e);
    }
    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      std::vector<bool> which; which.resize(2, true);
      LinkMechanics::setOpenMBVForceArrow(ombv.createOpenMBV(e), which);
    }
#endif
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(GeneralizedSpringDamper, MBSIM%"GeneralizedSpringDamper")

  GeneralizedSpringDamper::GeneralizedSpringDamper(const string &name) : LinkMechanics(name), func(NULL), body(2)
  {
    WF.resize(2);
    WM.resize(2);
    h[0].resize(2);
    h[1].resize(2);
    body[0] = 0;
    body[1] = 0;
  }

  GeneralizedSpringDamper::~GeneralizedSpringDamper() {
    delete func;
  }

  void GeneralizedSpringDamper::updatehRef(const Vec &hParent, int j) {
    Index I = Index(body[1]->gethInd(j),body[1]->gethInd(j)+body[1]->gethSize(j)-1);
    h[j][1]>>hParent(I);
    if(body[0]) {
      Index I = Index(body[0]->gethInd(j),body[0]->gethInd(j)+body[0]->gethSize(j)-1);
      h[j][0]>>hParent(I);
    }
    else {
      Index I = Index(body[1]->getFrameOfReference()->gethInd(j),body[1]->getFrameOfReference()->gethInd(j)+body[1]->getFrameOfReference()->getJacobianOfTranslation(j).cols()-1);
      h[j][0]>>hParent(I);
    }
  } 

  void GeneralizedSpringDamper::updateh(double t, int j) {
    la(0) = (*func)(g(0),gd(0));
    if(j==0) {
      if(body[0]) h[j][0]+=body[0]->getJRel(j).T()*la;
      h[j][1]-=body[1]->getJRel(j).T()*la;
    }
    else {
      WF[1] = body[1]->getFrameOfReference()->getOrientation()*body[1]->getPJT()*la;
      WM[1] = body[1]->getFrameOfReference()->getOrientation()*body[1]->getPJR()*la;
      h[j][1]-=body[1]->getFrameForKinematics()->getJacobianOfTranslation(j).T()*WF[1] + body[1]->getFrameForKinematics()->getJacobianOfRotation(j).T()*WM[1];
      if(body[0]) {
        WF[0] = -body[0]->getFrameOfReference()->getOrientation()*body[0]->getPJT()*la;
        WM[0] = -body[0]->getFrameOfReference()->getOrientation()*body[0]->getPJR()*la;
        h[j][0]-=body[0]->getFrameForKinematics()->getJacobianOfTranslation(j).T()*WF[0] + body[0]->getFrameForKinematics()->getJacobianOfRotation(j).T()*WM[0];
      } else {
        WF[0] = -WF[1];
        WM[0] = -WM[1];
        h[j][0]-=body[1]->getFrameOfReference()->getJacobianOfTranslation(j).T()*WF[0]+body[1]->getFrameOfReference()->getJacobianOfRotation(j).T()*WM[0];
      }
    }
  }

  void GeneralizedSpringDamper::updateg(double) {
    g=body[0]?(body[1]->getqRel()-body[0]->getqRel()):body[1]->getqRel();
  } 

  void GeneralizedSpringDamper::updategd(double) {
    gd=body[0]?(body[1]->getuRel()-body[0]->getuRel()):body[1]->getuRel();
  }

  void GeneralizedSpringDamper::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_body1!="")
        setRigidBodyFirstSide(getByPath<RigidBody>(saved_body1));
      if(saved_body2!="")
        setRigidBodySecondSide(getByPath<RigidBody>(saved_body2));
      if(body[1]==NULL)
        THROW_MBSIMERROR("rigid body on second side must be given!");
      if(body[0]) LinkMechanics::connect(body[0]->getFrameForKinematics());
      LinkMechanics::connect(body[1]->getFrameForKinematics());
      LinkMechanics::init(stage);
    }
    else if(stage==resize) {
      LinkMechanics::init(stage);
      g.resize(1);
      gd.resize(1);
      la.resize(1);
    }
    else if(stage==plotting) {
      updatePlotFeatures();
      plotColumns.push_back("la(0)");
      if(getPlotFeature(plotRecursive)==enabled) {
  #ifdef HAVE_OPENMBVCPPINTERFACE
        if(coilspringOpenMBV) {
          coilspringOpenMBV->setName(name);
          parent->getOpenMBVGrp()->addObject(coilspringOpenMBV);
        }
  #endif
        LinkMechanics::init(stage);
      }
    }
    else if(stage==unknownStage) {
      if(body[0] and body[0]->getuRelSize()!=1)
        THROW_MBSIMERROR("rigid body on first side to must have of 1 dof!");
      if(body[1]->getuRelSize()!=1)
        THROW_MBSIMERROR("rigid body on second side must have 1 dof!");
      LinkMechanics::init(stage);
    }
    else
      LinkMechanics::init(stage);
    func->init(stage);
  }

  void GeneralizedSpringDamper::plot(double t,double dt) {
    plotVector.push_back(la(0));
    if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
      if (coilspringOpenMBV) {
        Vec WrOToPoint;
        Vec WrOFromPoint;

        WrOFromPoint = body[0]?body[0]->getFrameForKinematics()->getPosition():body[1]->getFrameOfReference()->getPosition();
        WrOToPoint   = body[1]->getFrameForKinematics()->getPosition();
        vector<double> data;
        data.push_back(t); 
        data.push_back(WrOFromPoint(0));
        data.push_back(WrOFromPoint(1));
        data.push_back(WrOFromPoint(2));
        data.push_back(WrOToPoint(0));
        data.push_back(WrOToPoint(1));
        data.push_back(WrOToPoint(2));
        data.push_back(la(0));
        coilspringOpenMBV->append(data);
      }
#endif
      LinkMechanics::plot(t,dt);
    }
  }

  void GeneralizedSpringDamper::initializeUsingXML(DOMElement *element) {
    LinkMechanics::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"generalizedForceFunction");
    Function<double(double,double)> *f=ObjectFactory::createAndInit<Function<double(double,double)> >(e->getFirstElementChild());
    setGeneralizedForceFunction(f);
    e=E(element)->getFirstElementChildNamed(MBSIM%"rigidBodyFirstSide");
    if(e) saved_body1=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIM%"rigidBodySecondSide");
    saved_body2=E(e)->getAttribute("ref");
#ifdef HAVE_OPENMBVCPPINTERFACE
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVCoilSpring");
    if(e) {
      OpenMBVCoilSpring ombv;
      coilspringOpenMBV=ombv.createOpenMBV(e);
    }
    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      std::vector<bool> which; which.resize(2, true);
      LinkMechanics::setOpenMBVForceArrow(ombv.createOpenMBV(e), which);
    }
    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVMoment");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
      std::vector<bool> which; which.resize(2, true);
      LinkMechanics::setOpenMBVMomentArrow(ombv.createOpenMBV(e), which);
    }
#endif
  }

}
