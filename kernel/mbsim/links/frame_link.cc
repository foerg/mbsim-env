/* Copyright (C) 2004-2016 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include "mbsim/links/frame_link.h"
#include <mbsim/dynamic_system.h>
#include "mbsim/frames/frame.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  FrameLink::FrameLink(const std::string &name) : MechanicalLink(name), frame(2), updPos(true), updVel(true), updDF(true) {
    P.resize(2);
    F.resize(2);
    M.resize(2);
    RF.resize(2);
    RM.resize(2);
    for(unsigned int i=0; i<2; i++) {
      W[i].resize(2);
      V[i].resize(2);
      h[i].resize(2);
      r[i].resize(2);
    }
  }

  void FrameLink::resetUpToDate() {
    MechanicalLink::resetUpToDate();
    updPos = true;
    updVel = true;
    updDF = true;
  }

  void FrameLink::init(InitStage stage, const InitConfigSet &config) {
    if(stage==resolveStringRef) {
      if(not saved_ref1.empty() and not saved_ref2.empty())
        connect(getByPath<Frame>(saved_ref1), getByPath<Frame>(saved_ref2));
      else if(not saved_ref2.empty())
        connect(nullptr, getByPath<Frame>(saved_ref2));
      if(not frame[0])
        frame[0] = static_cast<DynamicSystem*>(parent)->getFrameI();
      if(not frame[1])
        throwError("Not all connections are given!");
    }
    else if(stage==preInit) {
      if(isSetValued()) {
        g.resize(getGeneralizedRelativePositionSize());
        gd.resize(ng);
        RF[0].resize(ng);
        RM[0].resize(ng);
        RF[1].resize(ng);
        RM[1].resize(ng);
        la.resize(ng);
      }
    }
    MechanicalLink::init(stage, config);
  }

  void FrameLink::updateWRef(Mat& WParent, int j) {
    for(unsigned i=0; i<2; i++) {
      RangeV J = RangeV(laInd,laInd+laSize-1);
      RangeV I = RangeV(frame[i]->gethInd(j),frame[i]->gethInd(j)+frame[i]->gethSize(j)-1); // TODO Prüfen ob hSize
      W[j][i].ref(WParent,I,J);
    }
  }

  void FrameLink::updateVRef(Mat& VParent, int j) {
    for(unsigned i=0; i<2; i++) {
      RangeV J = RangeV(laInd,laInd+laSize-1);
      RangeV I = RangeV(frame[i]->gethInd(j),frame[i]->gethInd(j)+frame[i]->gethSize(j)-1);
      V[j][i].ref(VParent,I,J);
    }
  }

  void FrameLink::updatehRef(Vec &hParent, int j) {
    for(unsigned i=0; i<2; i++) {
      RangeV I = RangeV(frame[i]->gethInd(j),frame[i]->gethInd(j)+frame[i]->gethSize(j)-1);
      h[j][i].ref(hParent,I);
    }
  }

  void FrameLink::updatedhdqRef(fmatvec::Mat& dhdqParent, int k) {
    throwError("Internal error");
  }

  void FrameLink::updatedhduRef(fmatvec::SqrMat& dhduParent, int k) {
    throwError("Internal error");
  }

  void FrameLink::updatedhdtRef(fmatvec::Vec& dhdtParent, int j) {
    for(unsigned i=0; i<2; i++) {
      RangeV I = RangeV(frame[i]->gethInd(j),frame[i]->gethInd(j)+frame[i]->gethSize(j)-1);
      dhdt[i].ref(dhdtParent,I);
    }
  }

  void FrameLink::updaterRef(Vec &rParent, int j) {
    for(unsigned i=0; i<2; i++) {
      int hInd =  frame[i]->gethInd(j);
      RangeV I = RangeV(hInd,hInd+frame[i]->gethSize(j)-1);
      r[j][i].ref(rParent,I);
    }
  }

  void FrameLink::initializeUsingXML(DOMElement *element) {
    MechanicalLink::initializeUsingXML(element);
    DOMElement *e = E(element)->getFirstElementChildNamed(MBSIM%"connect");
    saved_ref1 = E(e)->getAttribute("ref1");
    saved_ref2 = E(e)->getAttribute("ref2");
  }

  void FrameLink::connect(Frame *frame0, Frame* frame1) {
    frame[0] = frame0;
    frame[1] = frame1;
  }

  void FrameLink::updatePositions() {
    WrP0P1 = frame[1]->evalPosition() - frame[0]->evalPosition();
    AK0K1 = frame[0]->getOrientation().T()*frame[1]->getOrientation();
    updPos = false;
  }

  void FrameLink::updateForce() {
    F[1] = evalGlobalForceDirection()*evalGeneralizedForce()(iF);
    F[0] = -F[1];
    updF = false;
  }

  void FrameLink::updateMoment() {
    M[1] = evalGlobalMomentDirection()*evalGeneralizedForce()(iM);
    M[0] = -M[1];
    updM = false;
  }

  void FrameLink::updateR() {
    RF[1].set(RangeV(0,2), RangeV(iF), evalGlobalForceDirection());
    RM[1].set(RangeV(0,2), RangeV(iM), evalGlobalMomentDirection());
    RF[0] = -RF[1];
    RM[0] = -RM[1];
    updRMV = false;
  }

}
