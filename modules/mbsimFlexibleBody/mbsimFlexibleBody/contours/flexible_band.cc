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
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include "mbsimFlexibleBody/contours/flexible_band.h"
#include "mbsim/frames/floating_contour_frame.h"
#include "mbsim/frames/fixed_contour_frame.h"
#include "mbsimFlexibleBody/frames/floating_relative_flexible_contour_frame.h"
#include "mbsimFlexibleBody/flexible_body/flexible_body_1s.h"
#include "mbsim/utils/rotarymatrices.h"

#include <openmbvcppinterface/group.h>
#include <openmbvcppinterface/polygonpoint.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  void FlexibleBand::init(InitStage stage, const InitConfigSet &config) {
    if(stage==plotting) {
      if(plotFeature[openMBV] and openMBVSpineExtrusion) {
        openMBVSpineExtrusion->setName(name);
        openMBVSpineExtrusion->setShilouetteEdge(true);
        openMBVSpineExtrusion->setInitialRotation((vector<double>)AIK2Cardan(static_cast<Body*>(parent)->getFrameOfReference()->evalOrientation()));
        shared_ptr<vector<shared_ptr<OpenMBV::PolygonPoint> > > rectangle = make_shared<vector<shared_ptr<OpenMBV::PolygonPoint> > >(); // clockwise ordering, no doubling for closure
        shared_ptr<OpenMBV::PolygonPoint>  corner1 = OpenMBV::PolygonPoint::create(0, 0.5*width, 1);
        rectangle->push_back(corner1);
        shared_ptr<OpenMBV::PolygonPoint>  corner2 = OpenMBV::PolygonPoint::create(0, -0.5*width, 1);
        rectangle->push_back(corner2);

        openMBVSpineExtrusion->setContour(rectangle);
        parent->getOpenMBVGrp()->addObject(openMBVSpineExtrusion);
      }
    }
    Contour1s::init(stage, config);
  }

  ContourFrame* FlexibleBand::createContourFrame(const string &name) {
    FloatingRelativeFlexibleContourFrame *frame = new FloatingRelativeFlexibleContourFrame(name,contour);
    return frame;
  }

  void FlexibleBand::setRelativePosition(const fmatvec::Vec2 &r) {
    RrRP(0) = r(0);
    RrRP(2) = r(1);
  }

  void FlexibleBand::setRelativeOrientation(double al) {
    ARK = BasicRotAIKx(al);
  }

  bool FlexibleBand::isZetaOutside(const fmatvec::Vec2 &zeta) { return (static_cast<FlexibleBody1s*>(parent)->getOpenStructure() and (zeta(0) < etaNodes[0] or zeta(0) > etaNodes[etaNodes.size()-1])) or zeta(1) < -0.5*width or zeta(1) > 0.5*width; }

  void FlexibleBand::updatePositions(double s) {
    static Vec3 Kt("[0;0;1]");
    FixedContourFrame P;
    P.setContourOfReference(contour);
    P.setEta(s);
    Ws = P.evalOrientation().col(1);
    Wt = P.getOrientation()*(ARK*Kt);
    WrOP = P.getPosition() + P.getOrientation()*RrRP;
    sOld = s;
  }

  void FlexibleBand::resetUpToDate() {
    Contour1s::resetUpToDate();
    sOld = -1e12;
  }

  void FlexibleBand::plot() {
    if(plotFeature[openMBV] and openMBVSpineExtrusion) {
      vector<double> data;
      data.push_back(getTime());
      double L = getEtaNodes()[getEtaNodes().size()-1];
      double ds = static_cast<FlexibleBody1s*>(parent)->getOpenStructure() ? L/(openMBVSpineExtrusion->getNumberOfSpinePoints()-1) : L/(openMBVSpineExtrusion->getNumberOfSpinePoints()-2);
      for(int i=0; i<openMBVSpineExtrusion->getNumberOfSpinePoints(); i++) {
        Vec3 pos = evalPosition(ds*i);
        data.push_back(pos(0)); // global x-position
        data.push_back(pos(1)); // global y-position
        data.push_back(pos(2)); // global z-position
        data.push_back(static_cast<FlexibleBody1s*>(parent)->getAngles(ds*i)(0)); // local twist
      }
      openMBVSpineExtrusion->append(data);
    }
    Contour1s::plot();
  }

}
