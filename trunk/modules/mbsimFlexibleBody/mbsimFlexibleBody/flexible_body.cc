/* Copyright (C) 2004-2011 MBSim Development Team
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
 *          rzander@users.berlios.de
 */

#include <config.h>
#include <mbsimFlexibleBody/flexible_body.h>
#include <mbsim/dynamic_system.h>
#include <mbsim/frame.h>
#include <mbsim/contour.h>
#include <fmatvec/function.h>
#include <mbsim/mbsim_event.h>
#include <mbsim/contour_pdata.h>
#include <mbsim/discretization_interface.h>
#include "mbsimFlexibleBody/defines.h"

//#ifdef _OPENMP
//#include <omp.h>
//#endif

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

MBXMLUtils::NamespaceURI MBSIMFLEX("http://mbsim.berlios.de/MBSimFlexibleBody");

namespace MBSimFlexibleBody {

  FlexibleBody::FlexibleBody(const string &name) : Body(name), d_massproportional(0.) {}

  FlexibleBody::~FlexibleBody() {
    for(unsigned int i=0; i<discretization.size(); i++) {
      if(discretization[i]) { delete discretization[i]; discretization[i] = NULL; }
    }
  }

  void FlexibleBody::updateh(double t, int k) {
    for(int i=0;i<(int)discretization.size();i++)
      discretization[i]->computeh(qElement[i],uElement[i]); // compute attributes of finite element
    for(int i=0;i<(int)discretization.size();i++) GlobalVectorContribution(i,discretization[i]->geth(),h[k]); // assemble

    if(d_massproportional>0) { // mass proportional damping
      h[k] -= d_massproportional*(M[k]*u);
    }
  }

  void FlexibleBody::updateM(double t, int k) {
    for(int i=0;i<(int)discretization.size();i++)
      discretization[i]->computeM(qElement[i]); // compute attributes of finite element
    for(int i=0;i<(int)discretization.size();i++) GlobalMatrixContribution(i,discretization[i]->getM(),M[k]); // assemble
  }

  void FlexibleBody::updatedhdz(double t) {
    updateh(t);
    for(int i=0;i<(int)discretization.size();i++)
      discretization[i]->computedhdz(qElement[i],uElement[i]); // compute attributes of finite element
   for(int i=0;i<(int)discretization.size();i++) GlobalMatrixContribution(i,discretization[i]->getdhdq(),dhdq); // assemble
   for(int i=0;i<(int)discretization.size();i++) GlobalMatrixContribution(i,discretization[i]->getdhdu(),dhdu); // assemble
  }

  void FlexibleBody::updateStateDependentVariables(double t) {
    BuildElements();
    for(unsigned int i=0; i<frame.size(); i++) { // frames
      updateKinematicsForFrame(S_Frame[i],all,frame[i]); 
    }
    // TODO contour non native?
  }

  void FlexibleBody::updateJacobians(double t, int k) {
    for(unsigned int i=0; i<frame.size(); i++) { // frames
      updateJacobiansForFrame(S_Frame[i],frame[i]);
    }
    // TODO contour non native?
  }

  void FlexibleBody::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      Body::plot(t,dt);
    }
  }

  void FlexibleBody::init(InitStage stage) {
    if(stage==unknownStage) {
      Body::init(stage);
      T = SqrMat(qSize,fmatvec::EYE);
      for(unsigned int i=0; i<frame.size(); i++) { // frames
        S_Frame[i].getFrameOfReference().getJacobianOfTranslation().resize(uSize[0]);
        S_Frame[i].getFrameOfReference().getJacobianOfRotation().resize(uSize[0]);
      }
    }
    else if(stage==MBSim::plot) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
        Body::init(stage);
      }
    }
    else
      Body::init(stage);
  }

  double FlexibleBody::computeKineticEnergy() {
    double T = 0.;
    for(unsigned int i=0; i<discretization.size(); i++) {
      T += discretization[i]->computeKineticEnergy(qElement[i],uElement[i]);
    }
    return T;
  }

  double FlexibleBody::computePotentialEnergy() {
    double V = 0.;
    for(unsigned int i=0; i<discretization.size(); i++) {
      V += discretization[i]->computeElasticEnergy(qElement[i]) + discretization[i]->computeGravitationalEnergy(qElement[i]);
    }
    return V;
  }

  void FlexibleBody::setFrameOfReference(Frame *frame) { 
    if(dynamic_cast<DynamicSystem*>(frame->getParent())) 
      R = frame; 
    else 
      throw MBSimError("ERROR (FlexibleBody::setFrameOfReference): Only stationary reference frames are implemented at the moment!"); 
  }

  void FlexibleBody::addFrame(const string &name, const ContourPointData &S_) {
    Frame *frame = new Frame(name);
    addFrame(frame,S_);
  }

  void FlexibleBody::addFrame(Frame* frame, const ContourPointData &S_) {
    Body::addFrame(frame);
    S_Frame.push_back(S_);
  }

  void FlexibleBody::addFrame(const std::string &name, const int &id) {
    ContourPointData cp(id);
    addFrame(name,cp);
  }

  void FlexibleBody::addFrame(Frame *frame, const  int &id) {
    ContourPointData cp(id);
    addFrame(frame,cp);
  }

  void FlexibleBody::addContour(Contour *contour_) {
    stringstream frameName;
    frameName << "ContourFrame" << contour.size();
    Frame *contourFrame = new Frame(frameName.str());
//    addFrame(contourFrame,0);
    contour_->setFrameOfReference(contourFrame);
    Body::addContour(contour_);
  }

  void FlexibleBody::initializeUsingXML(DOMElement *element) {
    Body::initializeUsingXML(element);
    
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"massProportionalDamping");
    setMassProportionalDamping(getDouble(e));
  }

}
