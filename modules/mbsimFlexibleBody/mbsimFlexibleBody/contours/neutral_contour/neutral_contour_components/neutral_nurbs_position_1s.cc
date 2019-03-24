/*
 * NeutralPosition1sNurbs.cpp
 *
 *  Created on: 24.10.2013
 *      Author: zwang
 */
#include <config.h>
#include "neutral_nurbs_position_1s.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsimFlexibleBody/flexible_body.h"
#include "mbsimFlexibleBody/frames/node_frame.h"

using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {
//  class FlexibleBodyContinuum<double>;

  NeutralNurbsPosition1s::NeutralNurbsPosition1s(Element* parent_, const VecInt & nodes, double nodeOffset_, double uMin_, double uMax_, int degU_, bool openStructure_) :
      NeutralNurbs1s(parent_, nodes, nodeOffset_, uMin_, uMax_, degU_, openStructure_) {
    // TODO Auto-generated constructor stub
    
  }
  
  NeutralNurbsPosition1s::~NeutralNurbsPosition1s() {
    // TODO Auto-generated destructor stub
  }

  Vec3 NeutralNurbsPosition1s::evalPosition(double s) {
    if(updCurve) computeCurve(true);
    return curve.pointAt(s);
  }

  Vec3 NeutralNurbsPosition1s::evalWs(double s) {
    if(updCurve) computeCurve(true);
    Vec3 t = curve.firstDn(s);
    return t / nrm2(t);
  }

  Vec3 NeutralNurbsPosition1s::evalWt(double s) {
    Vec3 t = evalWs(s);
    Vec3 n = crossProduct(t,binormalDir);
    return crossProduct(n,t);
  }

  void NeutralNurbsPosition1s::update(ContourFrame *frame) {
    if(updCurve) computeCurve(true);
    frame->setPosition(curve.pointAt(frame->getEta()));
  }

  // TODO: this Normal and secondTangent is only work for the neutral curve on the xy plane. Need to adapt to different situations.
  void NeutralNurbsPosition1s::updatePositionNormal(ContourFrame *frame) {
    frame->getOrientation(false).set(0, crossProduct(evalWs(frame->getEta()),binormalDir));
  }

  void NeutralNurbsPosition1s::updatePositionFirstTangent(ContourFrame *frame) {
    frame->getOrientation(false).set(1, evalWs(frame->getEta()));
  }

  void NeutralNurbsPosition1s::updatePositionSecondTangent(ContourFrame *frame) {
    frame->getOrientation(false).set(2, evalWt(frame->getEta()));
  }

  void NeutralNurbsPosition1s::buildNodelist() {
    for (int i = 0; i < nodes.size(); i++) {
      Nodelist.set(i,static_cast<NodeBasedBody*>(parent)->evalNodalPosition(static_cast<NodeBasedBody*>(parent)->getNodeIndex(nodes(i))).T());
    }
//    msg(Debug) << "neutralPosition"<< Nodelist << endl << endl;
  }

} /* namespace MBSimFlexibleBody */
