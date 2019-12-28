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
#include "mbsimFlexibleBody/flexible_body/flexible_body_1s.h"
#include "mbsimFlexibleBody/frames/frame_1s.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/mbsim_event.h"

using namespace fmatvec;
using namespace std;
using namespace MBSim;

namespace MBSimFlexibleBody {

  FlexibleBody1s::FlexibleBody1s(const std::string &name, bool openStructure_) : FlexibleBodyContinuum<double>(name), L(0), openStructure(openStructure_) {
    P.setParent(this);
  }

  void FlexibleBody1s::init(InitStage stage, const InitConfigSet &config) {
    if(stage==plotting) {
      if(openMBVBody) ((OpenMBV::SpineExtrusion*)openMBVBody.get())->setInitialRotation((vector<double>)AIK2Cardan(R->evalOrientation()));
      FlexibleBodyContinuum<double>::init(stage, config);
    }
    else
      FlexibleBodyContinuum<double>::init(stage, config);
  }

  void FlexibleBody1s::plot() {
    if(plotFeature[openMBV] and openMBVBody) {
      vector<double> data;
      data.push_back(getTime());
      double ds = openStructure ? L/(((OpenMBV::SpineExtrusion*)openMBVBody.get())->getNumberOfSpinePoints()-1) : L/(((OpenMBV::SpineExtrusion*)openMBVBody.get())->getNumberOfSpinePoints()-2);
      for(int i=0; i<((OpenMBV::SpineExtrusion*)openMBVBody.get())->getNumberOfSpinePoints(); i++) {
        Vec3 pos = getPosition(ds*i);
        data.push_back(pos(0)); // global x-position
        data.push_back(pos(1)); // global y-position
        data.push_back(pos(2)); // global z-position
        data.push_back(getAngles(ds*i)(0)); // local twist
      }
      ((OpenMBV::SpineExtrusion*)openMBVBody.get())->append(data);
    }
    FlexibleBodyContinuum<double>::plot();
  }

  void FlexibleBody1s::addFrame(Frame1s *frame) { 
    Body::addFrame(frame); 
  }

  void FlexibleBody1s::updatePositions(Frame1s *frame) {
    throwError("(FlexibleBody1s::updatePositions): Not implemented.");
  }

  void FlexibleBody1s::updateVelocities(Frame1s *frame) {
    throwError("(FlexibleBody1s::updateVelocities): Not implemented.");
  }

  void FlexibleBody1s::updateAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s::updateAccelerations): Not implemented.");
  }

  void FlexibleBody1s::updateJacobians(Frame1s *frame, int j) {
    throwError("(FlexibleBody1s::updateJacobians): Not implemented.");
  }

  void FlexibleBody1s::updateGyroscopicAccelerations(Frame1s *frame) {
    throwError("(FlexibleBody1s::updateGyroscopicAccelerations): Not implemented.");
  }

  Vec3 FlexibleBody1s::getPosition(double s) {
    P.resetUpToDate();
    P.setParameter(s);
    return P.evalPosition();
  }

  SqrMat3 FlexibleBody1s::getOrientation(double s) {
    P.resetUpToDate();
    P.setParameter(s);
    return P.evalOrientation();
  }

}
