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
 * Contact: mfoerg@users.berlios.de
 */

#include<config.h>
#include "mbsim/contours/circle_hollow.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/frustum.h>
#endif

using namespace std;
using namespace fmatvec;

namespace MBSim {

#ifdef HAVE_OPENMBVCPPINTERFACE
  void CircleHollow::enableOpenMBV(bool enable) {
    if(enable) {
      openMBVRigidBody=new OpenMBV::Frustum;
      ((OpenMBV::Frustum*)openMBVRigidBody)->setInnerBaseRadius(r);
      ((OpenMBV::Frustum*)openMBVRigidBody)->setInnerTopRadius(r);
      ((OpenMBV::Frustum*)openMBVRigidBody)->setBaseRadius(1.1*r);
      ((OpenMBV::Frustum*)openMBVRigidBody)->setTopRadius(1.1*r);

      ((OpenMBV::Frustum*)openMBVRigidBody)->setHeight(0);
    }
    else openMBVRigidBody=0;
  }
#endif
}

