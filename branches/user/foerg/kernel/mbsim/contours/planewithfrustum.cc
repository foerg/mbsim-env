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
 * Contact: markus.ms.schneider@gmail.com
 */

#include<config.h>
#include "mbsim/contours/planewithfrustum.h"
#include "mbsim/utils/utils.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/frustum.h>
#endif

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace boost;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(PlaneWithFrustum, MBSIM%"PlaneWithFrustum")

#ifdef HAVE_OPENMBVCPPINTERFACE
  void PlaneWithFrustum::enableOpenMBV() {
    openMBVRigidBody=OpenMBV::ObjectFactory::create<OpenMBV::Frustum>();
    if (hFrustum<0) {
      static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setBaseRadius(rFrustumOnPlane);
      static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setInnerBaseRadius(rFrustumOnPlane);
      static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setTopRadius(rFrustumOnTop);
      static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setInnerTopRadius(rFrustumOnTop);
    }
    else {
      static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setBaseRadius(rFrustumOnPlane);
      static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setTopRadius(rFrustumOnTop);
    }
    static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setHeight(hFrustum);
    static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setInitialRotation(0, M_PI/2., 0);
    static_pointer_cast<OpenMBV::Frustum>(openMBVRigidBody)->setInitialTranslation(hFrustum, 0, 0);
  }
#endif

  void PlaneWithFrustum::initializeUsingXML(DOMElement *element) {
    RigidContour::initializeUsingXML(element);
    DOMElement* e;
    e=E(element)->getFirstElementChildNamed(MBSIM%"baseRadius");
    rFrustumOnPlane=getDouble(e);
    e=E(element)->getFirstElementChildNamed(MBSIM%"topRadius");
    rFrustumOnTop=getDouble(e);
    e=E(element)->getFirstElementChildNamed(MBSIM%"height");
    hFrustum=getDouble(e);
    e=E(element)->getFirstElementChildNamed(MBSIM%"roundingRadius");
    rho=getDouble(e);
    e=e->getNextElementSibling();
#ifdef HAVE_OPENMBVCPPINTERFACE
    if(E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBV"))
      enableOpenMBV();
#endif
    checkInput();
  }

  DOMElement* PlaneWithFrustum::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = Contour::writeXMLFile(parent);
//    addElementText(ele0,MBSIM%"baseRadius",rFrustumOnPlane);
//    addElementText(ele0,MBSIM%"topRadius",rFrustumOnTop);
//    addElementText(ele0,MBSIM%"height",hFrustum);
//    addElementText(ele0,MBSIM%"roundingRadius",rho);
//#ifdef HAVE_OPENMBVCPPINTERFACE
//    if(openMBVRigidBody)
//      ele0->LinkEndChild(new DOMElement(MBSIM%"enableOpenMBV"));
//#endif
    return ele0;
  }

}

