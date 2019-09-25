/* Copyright (C) 2004-2009 MBSim Development Team
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

#include<config.h>
#include "mbsim/contours/line_segment.h"

using namespace std;
using namespace MBXMLUtils;
using namespace fmatvec;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, LineSegment)

  LineSegment::LineSegment(const std::string& name, double l, double t, Frame *R) : RigidContour(name,R), length(l) {
    thickness = t;
  }

  LineSegment::LineSegment(const std::string& name, double l, Frame *R) : RigidContour(name,R), length(l) {
  }

  LineSegment::LineSegment(const std::string& name, Frame *R) : RigidContour(name,R) {
  }

  void LineSegment::init(InitStage stage, const InitConfigSet &config) {
    if(stage==plotting) {
      if(plotFeature[openMBV] && openMBVRigidBody)
      {
        auto ils=static_pointer_cast<OpenMBV::IndexedLineSet>(openMBVRigidBody);
        auto pos=ils->getVertexPositions();
        pos[0][1] = -length/2;
        pos[1][1] = length/2;
        ils->setVertexPositions(pos);
      }
    }
    RigidContour::init(stage, config);
  }

  void LineSegment::initializeUsingXML(DOMElement *element) {
    RigidContour::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"length");
    setLength(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBV");
    if(e) {
      OpenMBVLine ombv;
      ombv.initializeUsingXML(e);
      openMBVRigidBody=ombv.createOpenMBV(); 
    }
  }

}
