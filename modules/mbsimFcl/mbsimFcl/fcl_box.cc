/* Copyright (C) 2004-2018 MBSim Development Team
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
#include "fcl_box.h"
#include "mbsimFcl/namespace.h"
#include "fcl/geometry/shape/box.h"
#include <openmbvcppinterface/cuboid.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace fcl;

namespace MBSimFcl {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMFCL, FclBox)

  void FclBox::init(InitStage stage, const InitConfigSet &config) {
    if(stage==preInit)
      cg = shared_ptr<CollisionGeometry<double>>(new Box<double>(lx,ly,lz));
    else if (stage == plotting) {
      if(plotFeature[openMBV] && openMBVRigidBody)
        static_pointer_cast<OpenMBV::Cuboid>(openMBVRigidBody)->setLength(lx,ly,lz);
    }
    FclContour::init(stage, config);
  }

  void FclBox::initializeUsingXML(DOMElement *element) {
    FclContour::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIMFCL%"length");
    setLength(E(e)->getText<Vec3>());
    e=E(element)->getFirstElementChildNamed(MBSIMFCL%"enableOpenMBV");
    if(e) {
      OpenMBVColoredBody ombv;
      ombv.initializeUsingXML(e);
      openMBVRigidBody=ombv.createOpenMBV<OpenMBV::Cuboid>();
    }
  }

}
