/*
 * neutral_nurbs_2s.cc
 *
 *  Created on: 03.12.2013
 *      Author: zwang
 */

#include <config.h>

#include "nurbs_2s.h"

using namespace MBSim;
using namespace fmatvec;

namespace MBSimFlexibleBody {


  NeutralNurbs2s::NeutralNurbs2s(Element* parent_,  const MatVI & nodes, double nodeOffset_, int degU_, int degV_, bool openStructure_):
      surface(), parent(parent_), nodes(nodes), nodeOffset(nodeOffset_), numOfNodesU(nodes.rows()), numOfNodesV(nodes.cols()), Nodelist(nodes.rows(), nodes.cols()), degU(degU_), degV(degV_), openStructure(openStructure_), updSurface(true) {

  }

  NeutralNurbs2s:: ~NeutralNurbs2s()= default;

  void NeutralNurbs2s::resetUpToDate() {
    updSurface = true;
  }

  void NeutralNurbs2s::computeCurve(bool update){
    buildNodelist();

//    if (update)
//      surface.update(Nodelist);
//    else {
//      if (openStructure) {
//        surface.globalInterp(Nodelist, degV, degU, true);
//      }
//      else {
//        surface.globalInterpClosedU(Nodelist, degV, degU, true);
//      }
//    }
    if (openStructure) {
      surface.globalInterp(Nodelist, uk, vl, degU, degV);
    }else{
      surface.globalInterpClosedU(Nodelist, uk, vl, degU, degV);
    }
    updSurface = false;
  }
}
