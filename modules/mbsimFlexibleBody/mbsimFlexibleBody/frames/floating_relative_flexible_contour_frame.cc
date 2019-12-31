/* Copyright (C) 2004-2014 MBSim Development Team
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
#include "mbsimFlexibleBody/frames/floating_relative_flexible_contour_frame.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimFlexibleBody {

  FloatingRelativeFlexibleContourFrame::FloatingRelativeFlexibleContourFrame(const std::string &name, Contour *contour) : FloatingRelativeContourFrame(name) {
    P.setContourOfReference(contour);
    setFrameOfReference(&P);
  }

  void FloatingRelativeFlexibleContourFrame::init(InitStage stage, const InitConfigSet &config) {
    if(stage==unknownStage) {
      P.sethSize(gethSize());
      P.sethSize(gethSize(1),1);
      P.init(stage, config);
    }
    FloatingRelativeContourFrame::init(stage, config);
  }

  void FloatingRelativeFlexibleContourFrame::resetUpToDate() {
    FloatingRelativeContourFrame::resetUpToDate();
    P.resetUpToDate();
  }

  void FloatingRelativeFlexibleContourFrame::updatePositions() {
    parent->updatePositions(this);
    static_cast<ContourFrame*>(R)->setZeta(getZeta(false));
    WrRP = getPosition(false) - R->evalPosition();
    updPos = false;
  }

  void FloatingRelativeFlexibleContourFrame::updateVelocities() {
    static_cast<ContourFrame*>(R)->setZeta(evalZeta());
    FloatingRelativeContourFrame::updateVelocities();
  }

  void FloatingRelativeFlexibleContourFrame::updateAccelerations() {
    static_cast<ContourFrame*>(R)->setZeta(evalZeta());
    FloatingRelativeContourFrame::updateAccelerations();
  }

  void FloatingRelativeFlexibleContourFrame::updateJacobians(int j) {
    static_cast<ContourFrame*>(R)->setZeta(evalZeta());
    FloatingRelativeContourFrame::updateJacobians(j);
  }

  void FloatingRelativeFlexibleContourFrame::updateGyroscopicAccelerations() {
    static_cast<ContourFrame*>(R)->setZeta(evalZeta());
    FloatingRelativeContourFrame::updateGyroscopicAccelerations();
  }

}
