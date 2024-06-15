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
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include "mbsim/objects/body.h"
#include "mbsim/frames/frame.h"
#include "mbsim/contours/contour.h"
#include "mbsim/dynamic_system.h"
#include "openmbvcppinterface/group.h"
#include <openmbvcppinterface/body.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  Body::Body(const string &name) : Object(name), R(nullptr), updPos(true), updVel(true), updPJ(true), saved_frameOfReference("") { }

  Body::~Body() {
    for(auto & i : frame) 
      delete i;
    for(auto & i : contour) 
      delete i;
  }

  void Body::sethSize(int hSize_, int j) {
    Object::sethSize(hSize_, j);

    for(auto & i : frame)
      i->sethSize(hSize[j],j);
    for(auto & i : contour) 
      i->sethSize(hSize[j],j);
  }

  void Body::sethInd(int hInd_, int j) {
    Object::sethInd(hInd_, j);

    for(auto & i : frame) 
      i->sethInd(hInd[j],j);
    for(auto & i : contour) 
      i->sethInd(hInd[j],j);
  }  

  void Body::plot() {
    Object::plot();

    for(auto & j : frame)
      j->plot();
    for(auto & j : contour)
      j->plot();
  }

  void Body::setDynamicSystemSolver(DynamicSystemSolver* sys) {
    Object::setDynamicSystemSolver(sys);

    for(auto & i : frame)
      i->setDynamicSystemSolver(sys);
    for(auto & i : contour)
      i->setDynamicSystemSolver(sys);
  }

  void Body::init(InitStage stage, const InitConfigSet &config) {
    if(stage==resolveStringRef) {
      if(not saved_frameOfReference.empty())
        setFrameOfReference(getByPath<Frame>(saved_frameOfReference));
      if(not R)
        R = static_cast<DynamicSystem*>(parent)->getFrameI();
      else if(R->getParent()==this)
        throwError("(Body::init): frame of reference must not be part of " + name);
      addDependency(dynamic_cast<Body*>(R->getParent()));
    }
    else if(stage==plotting) {
      if(plotFeature[openMBV]) {
        openMBVGrp=OpenMBV::ObjectFactory::create<OpenMBV::Group>();
        openMBVGrp->setName(name);
        openMBVGrp->setExpand(false);
        parent->getObjectsOpenMBVGrp()->addObject(openMBVGrp);
	framesOpenMBVGrp = OpenMBV::ObjectFactory::create<OpenMBV::Group>();
	framesOpenMBVGrp->setName("frames");
	openMBVGrp->addObject(framesOpenMBVGrp);
	contoursOpenMBVGrp = OpenMBV::ObjectFactory::create<OpenMBV::Group>();
	contoursOpenMBVGrp->setName("contours");
	openMBVGrp->addObject(contoursOpenMBVGrp);
        if(openMBVBody) {
          openMBVBody->setName(name);
          openMBVGrp->addObject(openMBVBody);
        }
      }
    }
    Object::init(stage, config);

    for(auto & i : frame) 
      i->init(stage, config);
    for(auto & i : contour) 
      i->init(stage, config);
  }

  void Body::createPlotGroup() {
    Object::createPlotGroup();
    framesPlotGroup = plotGroup->createChildObject<H5::Group>("frames")();
    contoursPlotGroup = plotGroup->createChildObject<H5::Group>("contours")();
  }

  void Body::addContour(Contour* contour_) {
    if(contour_->getName().empty())
      throwError("A empty object name is not allowed!");
    if(getContour(contour_->getName(),false)) { //Contourname exists already
      throwError("(Body::addContour): The body can only comprise one contour by the name \""+contour_->getName()+"\"!");
      assert(getContour(contour_->getName(),false)==nullptr);
    }
    contour.push_back(contour_);
    contour_->setParent(this);
  }

  void Body::addFrame(Frame* frame_) {
    if(frame_->getName().empty())
      throwError("A empty object name is not allowed!");
    if(getFrame(frame_->getName(),false)) { //Contourname exists already
      throwError("(Body::addFrame): The body can only comprise one frame by the name \""+frame_->getName()+"\"!");
      assert(getFrame(frame_->getName(),false)==nullptr);
    }
    frame.push_back(frame_);
    frame_->setParent(this);
  }

  Contour* Body::getContour(const string &name_, bool check) const {
    unsigned int i;
    for(i=0; i<contour.size(); i++) {
      if(contour[i]->getName() == name_)
        return contour[i];
    }
    if(check) {
      if(!(i<contour.size()))
        throwError("(Body::getContour): The body comprises no contour \""+name_+"\"!"); 
      assert(i<contour.size());
    }
    return nullptr;
  }

  Frame* Body::getFrame(const string &name_, bool check) const {
    unsigned int i;
    for(i=0; i<frame.size(); i++) {
      if(frame[i]->getName() == name_)
        return frame[i];
    }             
    if(check) {
      if(!(i<frame.size()))
        throwError("(Body::getFrame): The body comprises no frame \""+name_+"\"!"); 
      assert(i<frame.size());
    }
    return nullptr;
  }

  int Body::frameIndex(const Frame *frame_) const {
    for(unsigned int i=0; i<frame.size(); i++) {
      if(frame_==frame[i])
        return i;
    }
    return -1;
  }

  int Body::contourIndex(const Contour *contour_) const {
    for(unsigned int i=0; i<contour.size(); i++) {
      if(contour_==contour[i])
        return i;
    }
    return -1;
  }

  void Body::initializeUsingXML(DOMElement *element) {
    Object::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"generalizedInitialPosition");
    if (e)
      setGeneralizedInitialPosition(E(e)->getText<Vec>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"generalizedInitialVelocity");
    if (e)
      setGeneralizedInitialVelocity(E(e)->getText<Vec>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"frameOfReference");
    if(e) saved_frameOfReference=E(e)->getAttribute("ref");
  }

  Element * Body::getChildByContainerAndName(const std::string &container, const std::string &name) const {
    if (container=="Frame")
      return getFrame(name);
    else if (container=="Contour")
      return getContour(name);
    else
      throwError("Unknown container '"+container+"'.");
  }

  void Body::resetUpToDate() {
    Object::resetUpToDate();
    updPos = true;
    updVel = true;
    updPJ = true;
    for(auto & i : frame)
      i->resetUpToDate();
    for(auto & i : contour)
      i->resetUpToDate();
  }
  void Body::resetPositionsUpToDate() {
    updPos = true;
    for(auto & i : frame)
      i->resetPositionsUpToDate();
  }
  void Body::resetVelocitiesUpToDate() {
    updVel = true;
    for(auto & i : frame)
      i->resetVelocitiesUpToDate();
  }
  void Body::resetJacobiansUpToDate() {
    for(auto & i : frame)
      i->resetJacobiansUpToDate();
  }
  void Body::resetGyroscopicAccelerationsUpToDate() {
    for(auto & i : frame)
      i->resetGyroscopicAccelerationsUpToDate();
  }

}
