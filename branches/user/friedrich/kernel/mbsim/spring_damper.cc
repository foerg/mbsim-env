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

#include "config.h"
#include "mbsim/spring_damper.h"
#include "mbsim/utils/eps.h"
#include "mbsim/objectfactory.h"
#include "mbsim/frame.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/coilspring.h>
#include <openmbvcppinterface/arrow.h>
#include "openmbvcppinterface/group.h"
#include "openmbvcppinterface/objectfactory.h"
#endif

using namespace std;
using namespace MBXMLUtils;
using namespace fmatvec;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Element, SpringDamper, MBSIMNS"SpringDamper")

  SpringDamper::SpringDamper(const string &name) : LinkMechanics(name), func(NULL), refFrame(NULL) 
#ifdef HAVE_OPENMBVCPPINTERFACE
    , coilspringOpenMBV(NULL)
#endif
  {}

  void SpringDamper::updateh(double t, int j) {
    la(0)=(*func)(g(0),gd(0));
    if(refFrame==0 && dist<=epsroot() && abs(la(0))>epsroot())
      cerr<<"Warning! The SpringDamper force is not 0 and the force direction can not calculated!\nUsing force=0 at t="<<t<<endl;
    if(refFrame==0) // Point to Point
      WF[0]=n*la;
    else // Directed
      WF[0]=WforceDir*la; // projected force in direction of WforceDir
    WF[1]=-WF[0];
    for(unsigned int i=0; i<2; i++) {
      h[j][i]+=frame[i]->getJacobianOfTranslation(j).T()*WF[i];
    }
  }

  void SpringDamper::updateg(double) {
    Vec3 WrP0P1=frame[1]->getPosition() - frame[0]->getPosition();
    dist=nrm2(WrP0P1);
    if(dist>epsroot())
      n=WrP0P1/dist;
    else
      n.init(0);
    if(refFrame==0) // Point to Point
      g(0)=dist;
    else {
      WforceDir=refFrame->getOrientation()*forceDir; // force direction in world system
      g(0)=WrP0P1.T()*WforceDir;
    }
  } 

  void SpringDamper::updategd(double) {
    Vec3 Wvrel=frame[1]->getVelocity() - frame[0]->getVelocity();
    if(refFrame==0) // Point to Point
      gd(0)=Wvrel.T()*n;
    else {
      gd(0)=Wvrel.T()*WforceDir;
    }
  }

  void SpringDamper::connect(Frame *frame0, Frame* frame1) {
    LinkMechanics::connect(frame0);
    LinkMechanics::connect(frame1);
  }

  void SpringDamper::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_frameOfReference!="")
        setProjectionDirection(getByPath<Frame>(saved_frameOfReference), saved_direction);
      if(saved_ref1!="" && saved_ref2!="")
        connect(getByPath<Frame>(saved_ref1), getByPath<Frame>(saved_ref2));
      LinkMechanics::init(stage);
    }
    else if(stage==resize) {
      LinkMechanics::init(stage);
      g.resize(1);
      gd.resize(1);
      la.resize(1);
    }
    else if(stage==MBSim::plot) {
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

  void SpringDamper::initializeUsingXML(TiXmlElement *element) {
    TiXmlElement *e;
    LinkMechanics::initializeUsingXML(element);
    e=element->FirstChildElement(MBSIMNS"forceFunction");
    Function<double(double,double)> *f=ObjectFactory<FunctionBase>::createAndInit<Function<double(double,double)> >(e->FirstChildElement());
    setForceFunction(f);
    e=element->FirstChildElement(MBSIMNS"projectionDirection");
    if(e) {
      TiXmlElement *ee=e->FirstChildElement(MBSIMNS"frameOfReference");
      saved_frameOfReference=ee->Attribute("ref");
      ee=e->FirstChildElement(MBSIMNS"direction");
      saved_direction=getVec(ee,3);
    }
    e=element->FirstChildElement(MBSIMNS"connect");
    saved_ref1=e->Attribute("ref1");
    saved_ref2=e->Attribute("ref2");
    e=e->NextSiblingElement();
#ifdef HAVE_OPENMBVCPPINTERFACE
    try { // e is of a type derived from CoilSpring or Arrow: if CoilSpring then ...
      if(e) {
        OpenMBV::CoilSpring *coilSpring=OpenMBV::ObjectFactory::create<OpenMBV::CoilSpring>(e);
        setOpenMBVSpring(coilSpring);
        coilSpring->initializeUsingXML(e);
        e=e->NextSiblingElement();
      }
    }
    catch(const runtime_error&) { // if not CoilSpring then check for Arrow ... (if not a error is thrown)
      if(e) {
        OpenMBV::Arrow *arrow=OpenMBV::ObjectFactory::create<OpenMBV::Arrow>(e);
        arrow->initializeUsingXML(e); // first initialize, because setOpenMBVForceArrow calls the copy constructor on arrow
        setOpenMBVForceArrow(arrow);
        e=e->NextSiblingElement();
      }
    }
#endif
  }

}
