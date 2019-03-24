/*
 * NeutralVelocity1sNurbs.cpp
 *
 *  Created on: 24.10.2013
 *      Author: zwang
 */
#include <config.h>
#include "neutral_nurbs_velocity_1s.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsimFlexibleBody/flexible_body.h"
#include "mbsimFlexibleBody/frames/node_frame.h"

using namespace MBSim;
using namespace fmatvec;

namespace MBSimFlexibleBody {
//  class FlexibleBodyContinuum<double>;

  NeutralNurbsVelocity1s::NeutralNurbsVelocity1s(Element* parent_, const VecInt & nodes, double nodeOffset_, double uMin_, double uMax_, int degU_, bool openStructure_):
    NeutralNurbs1s(parent_, nodes, nodeOffset_, uMin_, uMax_, degU_, openStructure_){
    // TODO Auto-generated constructor stub
    
  }
  
  NeutralNurbsVelocity1s::~NeutralNurbsVelocity1s() {
    // TODO Auto-generated destructor stub
  }

  void NeutralNurbsVelocity1s::update(ContourFrame *frame) {
    if(updCurve) computeCurve(true);
    frame->setVelocity(curve.pointAt(frame->evalEta()));
  }

  void NeutralNurbsVelocity1s::buildNodelist() {
    for (int i = 0; i < nodes.size(); i++) {
      Nodelist.set(i,static_cast<NodeBasedBody*>(parent)->evalNodalVelocity(static_cast<NodeBasedBody*>(parent)->getNodeIndex(nodes(i))).T());
    }
//    msg(Debug) << "neutralVelocity"<< Nodelist << endl << endl;
  }


} /* namespace MBSimFlexibleBody */
