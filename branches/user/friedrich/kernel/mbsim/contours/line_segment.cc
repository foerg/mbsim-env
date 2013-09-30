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
 * Contact: foerg@users.berlios.de
 */

#include<config.h>
#include "mbsim/contours/line_segment.h"
#include "mbsim/utils/utils.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/grid.h>
#endif

using namespace std;
using namespace MBXMLUtils;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Element, LineSegment, MBSIMNS"LineSegment")

  LineSegment::LineSegment(const std::string& name, double l, double t, Frame *R) : RigidContour(name,R), length(l), thickness(t) {
  }

  LineSegment::LineSegment(const std::string& name, double l, Frame *R) : RigidContour(name,R), length(l), thickness(0.01) {
  }

  LineSegment::LineSegment(const std::string& name, Frame *R) : RigidContour(name,R), length(1), thickness(0.01) {
  }

  void LineSegment::initializeUsingXML(TiXmlElement *element) {
    RigidContour::initializeUsingXML(element);
    TiXmlElement* e;
    e=element->FirstChildElement(MBSIMNS"length");
    setLength(getDouble(e));
#ifdef HAVE_OPENMBVCPPINTERFACE
    e=element->FirstChildElement(MBSIMNS"enableOpenMBV");
    if(e) {
      double s=getDouble(e->FirstChildElement(MBSIMNS"size"));
      int n=getInt(e->FirstChildElement(MBSIMNS"number"));
      enableOpenMBV(true, s, n);
    }
#endif
  }

  TiXmlElement* LineSegment::writeXMLFile(TiXmlNode *parent) {
    TiXmlElement *ele0 = Contour::writeXMLFile(parent);
    addElementText(ele0,MBSIMNS"length",length);
#ifdef HAVE_OPENMBVCPPINTERFACE
    if(openMBVRigidBody) {
      TiXmlElement *ele1 = new TiXmlElement(MBSIMNS"enableOpenMBV");
      addElementText(ele1,MBSIMNS"size",static_cast<OpenMBV::Grid*>(openMBVRigidBody)->getXSize());
      addElementText(ele1,MBSIMNS"number",static_cast<OpenMBV::Grid*>(openMBVRigidBody)->getXNumber());
      ele0->LinkEndChild(ele1);
    }
#endif
    return ele0;
  }

#ifdef HAVE_OPENMBVCPPINTERFACE
  void LineSegment::enableOpenMBV(bool enable, double size, int number) {
    if(enable) {
      openMBVRigidBody=new OpenMBV::Grid;
      ((OpenMBV::Grid*)openMBVRigidBody)->setXSize(size);
      ((OpenMBV::Grid*)openMBVRigidBody)->setYSize(0.01);
      ((OpenMBV::Grid*)openMBVRigidBody)->setXNumber(number);
      ((OpenMBV::Grid*)openMBVRigidBody)->setYNumber(1);
      ((OpenMBV::Grid*)openMBVRigidBody)->setInitialRotation(0.,M_PI/2.,0.);
    }
    else openMBVRigidBody=0;
  }
#endif
}

