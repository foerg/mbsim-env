/* Copyright (C) 2004-2014 MBSim Development Team
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
#include "mbsim/links/floating_frame_link.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  FloatingFrameLink::FloatingFrameLink(const std::string &name) : FrameLink(name), C("F") {
    C.setParent(this);
  }

  void FloatingFrameLink::resetUpToDate() {
    FrameLink::resetUpToDate();
    C.resetUpToDate();  
  }

  void FloatingFrameLink::calcSize() {
    ng = forceDir.cols() + momentDir.cols();
    ngd = ng;
    nla = ng;
    updSize = false;
  }

  void FloatingFrameLink::calclaSize(int j) {
    laSize = forceDir.cols() + momentDir.cols();
  }

  void FloatingFrameLink::calcgSize(int j) {
    gSize = forceDir.cols() + momentDir.cols();
  }

  void FloatingFrameLink::calcgdSize(int j) {
    gdSize = forceDir.cols() + momentDir.cols();
  }

  void FloatingFrameLink::calcrFactorSize(int j) {
    rFactorSize = isSetValued() ? forceDir.cols() + momentDir.cols() : 0;
  }

  void FloatingFrameLink::calccorrSize(int j) {
    corrSize = forceDir.cols() + momentDir.cols();
  }

  void FloatingFrameLink::updateW(int j) {
    W[j][0] += C.evalJacobianOfTranslation(j).T() * evalRF(0) + C.evalJacobianOfRotation(j).T() * evalRM(0);
    W[j][1] += frame[1]->evalJacobianOfTranslation(j).T() * evalRF(1) + frame[1]->evalJacobianOfRotation(j).T() * evalRM(1);
  }

  void FloatingFrameLink::updateh(int j) {
    h[j][0] += C.evalJacobianOfTranslation(j).T() * evalForce(0) + C.evalJacobianOfRotation(j).T() * evalMoment(0);
    h[j][1] += frame[1]->evalJacobianOfTranslation(j).T() * evalForce(1) + frame[1]->evalJacobianOfRotation(j).T() * evalMoment(1);
  }

  void FloatingFrameLink::updatePositions(Frame *frame_) {
    frame_->setPosition(frame[1]->evalPosition());
    frame_->setOrientation(frame[0]->evalOrientation());
  }

  void FloatingFrameLink::updateVelocities() {
    WvP0P1 = frame[1]->evalVelocity() - C.evalVelocity();
    WomK0K1 = frame[1]->getAngularVelocity() - C.getAngularVelocity();
    updVel = false;
  }

  void FloatingFrameLink::updateGeneralizedPositions() {
    rrel.set(iF, evalGlobalForceDirection().T() * evalGlobalRelativePosition());
    rrel.set(iM, evalGeneralizedRelativePositionOfRotation());
    updrrel = false;
  }

  void FloatingFrameLink::updateGeneralizedVelocities() {
    vrel.set(iF, evalGlobalForceDirection().T() * evalGlobalRelativeVelocity());
    vrel.set(iM, evalGlobalMomentDirection().T() * evalGlobalRelativeAngularVelocity());
    updvrel = false;
  }

  void FloatingFrameLink::updateForceDirections() {
    DF = frame[refFrame]->evalOrientation() * forceDir;
    DM = frame[refFrame]->getOrientation() * momentDir;
    updDF = false;
  }

  void FloatingFrameLink::updateg() {
    g.set(iF, evalGeneralizedRelativePosition()(iF));
    g.set(iM, getGeneralizedRelativePosition()(iM));;
  }

  void FloatingFrameLink::updategd() {
    gd.set(iF, evalGeneralizedRelativeVelocity()(iF));
    gd.set(iM, getGeneralizedRelativeVelocity()(iM));
  }

  void FloatingFrameLink::init(InitStage stage, const InitConfigSet &config) {
    if(stage==preInit) {
      if(refFrame==unknown)
        throwError("(FloatingFrameLink::init): frame of reference unknown");
      iF = RangeV(0, forceDir.cols() - 1);
      iM = RangeV(forceDir.cols(), getGeneralizedRelativePositionSize() - 1);
      lambdaF.resize(forceDir.cols());
      lambdaM.resize(momentDir.cols());
      DF.resize(forceDir.cols(),NONINIT);
      DM.resize(momentDir.cols(),NONINIT);
    }
    else if(stage==unknownStage) {
      C.setFrameOfReference(frame[0]);
      P[0] = &C;
      P[1] = frame[1];
      C.sethSize(frame[0]->gethSize());
      C.sethSize(frame[0]->gethSize(1),1);
      C.init(stage, config);
    }
    FrameLink::init(stage, config);
  }

  void FloatingFrameLink::initializeUsingXML(DOMElement *element) {
    FrameLink::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"frameOfReference");
    if(e) {
      string refFrameStr=string(X()%E(e)->getFirstTextChild()->getData()).substr(1,string(X()%E(e)->getFirstTextChild()->getData()).length()-2);
      if(refFrameStr=="firstFrame") refFrame=firstFrame;
      else if(refFrameStr=="secondFrame") refFrame=secondFrame;
      else refFrame=unknown;
    }
  }

}
