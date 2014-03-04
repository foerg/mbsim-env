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
 */

#include<config.h>
#include "mbsimFlexibleBody/contours/flexible_band.h"
#include "mbsimFlexibleBody/flexible_body.h"
#include <vector>

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  FlexibleBand::FlexibleBand(const string& name) :
      Contour1sFlexible(name), Cn(2, INIT, 0.), width(0.), nDist(0.)

  {
  }

  void FlexibleBand::setCn(const Vec& Cn_) {
    assert(Cn_.size() == 2);
    Cn = Cn_ / nrm2(Cn_);
  }

  void FlexibleBand::updateKinematicsForFrame(ContourPointData& cp, FrameFeature ff) {
    if (ff == firstTangent || ff == cosy || ff == position_cosy || ff == velocity_cosy || ff == velocities_cosy)
      Contour1sFlexible::updateKinematicsForFrame(cp, firstTangent);
    if (ff == normal || ff == secondTangent || ff == cosy || ff == position_cosy || ff == velocity_cosy || ff == velocities_cosy) {
//      static_cast<FlexibleBody*>(parent)->updateKinematicsForFrame(cp,normal);
//      static_cast<FlexibleBody*>(parent)->updateKinematicsForFrame(cp,secondTangent);
      Contour1sFlexible::updateKinematicsForFrame(cp, normal);
      Contour1sFlexible::updateKinematicsForFrame(cp, secondTangent);

      Vec WnLocal = cp.getFrameOfReference().getOrientation().col(0);
      Vec WbLocal = cp.getFrameOfReference().getOrientation().col(2);
      if (ff != secondTangent)
        cp.getFrameOfReference().getOrientation().set(0, WnLocal * Cn(0) + WbLocal * Cn(1));
      if (ff != normal)
        cp.getFrameOfReference().getOrientation().set(2, -WnLocal * Cn(1) + WbLocal * Cn(0));
    }
    if (ff == position || ff == position_cosy) {
      Contour1sFlexible::updateKinematicsForFrame(cp, position_cosy);
      cp.getFrameOfReference().getPosition() += cp.getFrameOfReference().getOrientation().col(0) * nDist + cp.getFrameOfReference().getOrientation().col(2) * cp.getLagrangeParameterPosition()(1);
    }
    if (ff == angularVelocity || ff == velocities || ff == velocities_cosy) {
      Contour1sFlexible::updateKinematicsForFrame(cp, angularVelocity);
    }
    if (ff == velocity || ff == velocity_cosy || ff == velocities || ff == velocities_cosy) {
      Contour1sFlexible::updateKinematicsForFrame(cp, velocity);
      Vec3 dist = cp.getFrameOfReference().getOrientation().col(0) * nDist + cp.getFrameOfReference().getOrientation().col(2) * cp.getLagrangeParameterPosition()(1);
      cp.getFrameOfReference().getVelocity() += crossProduct(cp.getFrameOfReference().getAngularVelocity(), dist);
    }
  }

  void FlexibleBand::updateJacobiansForFrame(ContourPointData &cp, int j /*=0*/) {
    Contour1sFlexible::updateJacobiansForFrame(cp);
    Vec WrPC = cp.getFrameOfReference().getOrientation().col(0) * nDist + cp.getFrameOfReference().getOrientation().col(2) * cp.getLagrangeParameterPosition()(1); // vector from neutral line to contour surface point
    SqrMat tWrPC = tilde(WrPC).copy(); // tilde matrix of above vector
    cp.getFrameOfReference().setJacobianOfTranslation(cp.getFrameOfReference().getJacobianOfTranslation() - tWrPC * cp.getFrameOfReference().getJacobianOfRotation()); // Jacobian of translation at contour surface with standard description assuming rigid cross-section
  }

}

