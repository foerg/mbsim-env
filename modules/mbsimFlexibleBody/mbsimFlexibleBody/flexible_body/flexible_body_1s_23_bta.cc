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
 * Contact: thorsten.schindler@mytum.de
 */

#include <config.h>
#include "mbsimFlexibleBody/flexible_body/flexible_body_1s_23_bta.h"
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_23_bta.h"
#include "mbsimFlexibleBody/frames/frame_1s.h"
#include "mbsim/environment.h"
#include "mbsim/utils/rotarymatrices.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimFlexibleBody {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMFLEX, FlexibleBody1s23BTA)

  FlexibleBody1s23BTA::FlexibleBody1s23BTA(const string &name) : FlexibleBody1s(name,true) { 
//    cylinderFlexible = new CylinderFlexible("CylinderFlexible");
//    addContour(cylinderFlexible);
  }

  void FlexibleBody1s23BTA::BuildElements() {
    for(int i=0;i<Elements;i++) {
      RangeV activeElement( discretization[i]->getuSize()/2*i , discretization[i]->getuSize()/2*(i+2) -1 );
      qElement[i] = q(activeElement);
      uElement[i] = u(activeElement);
    }
    updEle = false;
  }

  void FlexibleBody1s23BTA::GlobalVectorContribution(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec) {
    RangeV activeElement( discretization[n]->getuSize()/2*n , discretization[n]->getuSize()/2*(n+2) -1 );
    gloVec(activeElement) += locVec;
  }

  void FlexibleBody1s23BTA::GlobalMatrixContribution(int n, const fmatvec::Mat& locMat, fmatvec::Mat& gloMat) {
    RangeV activeElement( discretization[n]->getuSize()/2*n , discretization[n]->getuSize()/2*(n+2) -1 );
    gloMat(activeElement) += locMat;
  }

  void FlexibleBody1s23BTA::GlobalMatrixContribution(int n, const fmatvec::SymMat& locMat, fmatvec::SymMat& gloMat) {
    RangeV activeElement( discretization[n]->getuSize()/2*n , discretization[n]->getuSize()/2*(n+2) -1 );
    gloMat(activeElement) += locMat;
  }

  void FlexibleBody1s23BTA::updatePositions(Frame1s *frame) {
    fmatvec::Vector<Fixed<6>, double> X = getPositions(frame->getParameter());
    X(0) = frame->getParameter();
    frame->setPosition(R->evalPosition() + R->evalOrientation() * X(Range<Fixed<0>,Fixed<2>>()));
    frame->setOrientation(R->getOrientation() * getOrientation(frame->getParameter()));
  }

  void FlexibleBody1s23BTA::updateVelocities(Frame1s *frame) {
    fmatvec::Vector<Fixed<6>, double> Xt = getVelocities(frame->getParameter());
    frame->setVelocity(R->evalOrientation() * Xt(Range<Fixed<0>,Fixed<2>>()));
    frame->setAngularVelocity(R->getOrientation() * Xt(Range<Fixed<3>,Fixed<5>>()));
  }

  void FlexibleBody1s23BTA::updateAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s23BTA::updateAccelerations): Not implemented.");
  }

  void FlexibleBody1s23BTA::updateJacobians(Frame1s *frame, int j) {
    RangeV All(0,5-1);
    Mat Jacobian(qSize, 5, INIT, 0); // boeser Kaefer, Initialisierung notwendig!!! M. Schneider

    double sLocal;
    int currentElement;
    BuildElement(frame->getParameter(), sLocal, currentElement);

    RangeV activeElement( discretization[currentElement]->getuSize()/2*currentElement, discretization[currentElement]->getuSize()/2*(currentElement+2) -1 );
    Jacobian(activeElement,All) = static_cast<FiniteElement1s23BTA*>(discretization[currentElement])->JGeneralized(getqElement(currentElement),sLocal);

    frame->setJacobianOfTranslation(R->evalOrientation()(RangeV(0,2),RangeV(1,2))*Jacobian(RangeV(0,qSize-1),RangeV(0,1)).T());
    frame->setJacobianOfRotation(R->getOrientation()*Jacobian(RangeV(0,qSize-1),RangeV(2,4)).T());
  }

  void FlexibleBody1s23BTA::updateGyroscopicAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s23BTA::updateGyroscopicAccelerations): Not implemented.");
  }

  void FlexibleBody1s23BTA::updatePositions(NodeFrame *frame) {
    throwError("(FlexibleBody1s23BTA::updatePositions): Not implemented.");
  }

  void FlexibleBody1s23BTA::updateVelocities(NodeFrame *frame) {
    throwError("(FlexibleBody1s23BTA::updateVelocities): Not implemented.");
  }

  void FlexibleBody1s23BTA::updateAccelerations(NodeFrame *frame) {
    throwError("(FlexibleBody1s23BTA::updateAccelerations): Not implemented.");
  }

  void FlexibleBody1s23BTA::updateJacobians(NodeFrame *frame, int j) {
    throwError("(FlexibleBody1s23BTA::updateJacobians): Not implemented.");
  }

  void FlexibleBody1s23BTA::updateGyroscopicAccelerations(NodeFrame *frame) {
    throwError("(FlexibleBody1s23BTA::updateGyroscopicAccelerations): Not implemented.");
  }

  void FlexibleBody1s23BTA::init(InitStage stage, const InitConfigSet &config) {
    if(stage==unknownStage) {
      FlexibleBody1s::init(stage, config);

      assert(0<Elements);

      //cylinderFlexible->getFrame()->setOrientation(R->getOrientation());
      //cylinderFlexible->setAlphaStart(0);  cylinderFlexible->setAlphaEnd(L);
      //if(userContourNodes.size()==0) {
      //  Vec contourNodes(Elements+1);
      //  for(int i=0;i<=Elements;i++) contourNodes(i) = L/Elements * i; 
      //  cylinderFlexible->setNodes(contourNodes);
      //}
      //else cylinderFlexible->setNodes(userContourNodes);

      l0 = L/Elements;
      Vec g = R->getOrientation().T()*MBSimEnvironment::getInstance()->getAccelerationOfGravity();
      for(int i=0; i<Elements; i++) {
        discretization.push_back(new FiniteElement1s23BTA(l0, A*rho, E*Iyy, E*Izz, It*rho, G*It, g ));
        qElement.emplace_back(discretization[0]->getqSize(),INIT,0.);
        uElement.emplace_back(discretization[0]->getuSize(),INIT,0.);
        static_cast<FiniteElement1s23BTA*>(discretization[i])->setTorsionalDamping(dTorsional);
      }
    }
    else
      FlexibleBody1s::init(stage, config);
  }

  void FlexibleBody1s23BTA::setNumberElements(int n) {
    Elements = n;
    qSize = 5*( Elements + 1 ); 
    uSize[0] = qSize;
    uSize[1] = qSize; // TODO
    q0.resize(qSize);
    u0.resize(uSize[0]);
  }

  fmatvec::Vector<Fixed<6>, double> FlexibleBody1s23BTA::getPositions(double sGlobal) {
    double sLocal;
    int currentElement;
    BuildElement(sGlobal, sLocal, currentElement); // Lagrange parameter of affected FE
    return static_cast<FiniteElement1s23BTA*>(discretization[currentElement])->getPositions(getqElement(currentElement), sLocal);
  }

  fmatvec::Vector<Fixed<6>, double> FlexibleBody1s23BTA::getVelocities(double sGlobal) {
    double sLocal;
    int currentElement;
    BuildElement(sGlobal, sLocal, currentElement); // Lagrange parameter of affected FE
    return static_cast<FiniteElement1s23BTA*>(discretization[currentElement])->getVelocities(getqElement(currentElement), getuElement(currentElement), sLocal);
  }

  SqrMat3 FlexibleBody1s23BTA::getOrientation(double sGlobal) {
    double sLocal;
    int currentElement;
    BuildElement(sGlobal, sLocal, currentElement); // Lagrange parameter of affected FE
    return static_cast<FiniteElement1s23BTA*>(discretization[currentElement])->getOrientation(getqElement(currentElement),sLocal);
  }

  void FlexibleBody1s23BTA::BuildElement(const double& sGlobal, double& sLocal, int& currentElement) {
    currentElement = int(sGlobal/l0);   
    sLocal = sGlobal - currentElement * l0; // Lagrange-Parameter of the affected FE 

    if(currentElement >= Elements) { // contact solver computes to large sGlobal at the end of the entire beam
      currentElement =  Elements-1;
      sLocal += l0;
    }
  }

  Vec3 FlexibleBody1s23BTA::getAngles(double s) {
    return getPositions(s)(Range<Fixed<3>,Fixed<5>>());
  }

  void FlexibleBody1s23BTA::initializeUsingXML(DOMElement *element) {
    DOMElement *e;
    FlexibleBody::initializeUsingXML(element);

//    // frames
//    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"frames")->getFirstElementChild();
//    while(e && MBXMLUtils::E(e)->getTagName()==MBSIMFLEX%"frameOnFlexibleBody1s") {
//      DOMElement *ec=e->getFirstElementChild();
//      Frame *f=new Frame(MBXMLUtils::E(ec)->getAttribute("name"));
//      f->initializeUsingXML(ec);
//      ec=ec->getNextElementSibling();
//      double pos=MBXMLUtils::E(ec)->getText<double>();
//
//      addFrame(f, pos);
//      e=e->getNextElementSibling();
//    }

    //other properties 

    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"numberOfElements");
    setNumberElements(MBXMLUtils::E(e)->getText<int>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"length");
    setLength(MBXMLUtils::E(e)->getText<double>());

    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"youngsModulus");
    auto E=MBXMLUtils::E(e)->getText<double>();
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"shearModulus");
    auto G=MBXMLUtils::E(e)->getText<double>();
    setElastModuls(E, G);

    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"density");
    setDensity(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"crossSectionArea");
    setCrossSectionalArea(MBXMLUtils::E(e)->getText<double>());

    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"momentOfInertia");
    Vec TempVec2=MBXMLUtils::E(e)->getText<Vec>();
    setMomentsInertia(TempVec2(0),TempVec2(1),TempVec2(2));

//    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"radiusOfContour");
//    setContourRadius(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"torsionalDamping");
    setTorsionalDamping(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"massProportionalDamping");
    setMassProportionalDamping(MBXMLUtils::E(e)->getText<double>());

    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"openMBVBody");
    if(e) {
      std::shared_ptr<OpenMBV::SpineExtrusion> rb=OpenMBV::ObjectFactory::create<OpenMBV::SpineExtrusion>(e->getFirstElementChild());
      setOpenMBVSpineExtrusion(rb);
      rb->initializeUsingXML(e->getFirstElementChild());
      rb->setNumberOfSpinePoints(4*Elements+1);
    }

  }

}
