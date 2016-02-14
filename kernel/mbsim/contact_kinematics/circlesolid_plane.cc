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
 *          rzander@users.berlios.de
 */

#include <config.h> 
#include "mbsim/contact_kinematics/circlesolid_plane.h"
#include "mbsim/frame.h"
#include "mbsim/contours/plane.h"
#include "mbsim/contours/solid_circle.h"

using namespace fmatvec;
using namespace std;

namespace MBSim {

  void ContactKinematicsSolidCirclePlane::assignContours(const vector<Contour*> &contour) {
    if(dynamic_cast<SolidCircle*>(contour[0])) {
      icircle = 0;
      iplane = 1;
      circlesolid = static_cast<SolidCircle*>(contour[0]);
      plane = static_cast<Plane*>(contour[1]);
    } 
    else {
      icircle = 1;
      iplane = 0;
      circlesolid = static_cast<SolidCircle*>(contour[1]);
      plane = static_cast<Plane*>(contour[0]);
    }
  }

  void ContactKinematicsSolidCirclePlane::updateg(double t, double &g, std::vector<Frame*> &cFrame, int index) {
    cFrame[iplane]->setOrientation(plane->getFrame()->getOrientation(t));
    cFrame[icircle]->getOrientation(false).set(0, -plane->getFrame()->getOrientation().col(0));
    cFrame[icircle]->getOrientation(false).set(1, -plane->getFrame()->getOrientation().col(1));
    cFrame[icircle]->getOrientation(false).set(2, plane->getFrame()->getOrientation().col(2));

    Vec3 Wd;
    Vec3 Wn = cFrame[iplane]->getOrientation(false).col(0);
    Vec3 Wb = circlesolid->getFrame()->getOrientation(t).col(2);
    double t_EC = Wn.T()*Wb;
    if(t_EC>0) {
      Wb *= -1.;
      t_EC *= -1;	
    }
    Vec3 z_EC = Wn - t_EC*Wb;
    double z_EC_nrm2 = nrm2(z_EC);
    if(z_EC_nrm2 <= 1e-8) { // infinite possible contact points
      Wd = circlesolid->getFrame()->getPosition() - plane->getFrame()->getPosition();
    } 
    else { // exactly one possible contact point
      Wd =  (circlesolid->getFrame()->getPosition() - (circlesolid->getRadius()/z_EC_nrm2)*z_EC) - plane->getFrame()->getPosition();
    }
    g = Wn.T()*Wd;
    cFrame[icircle]->setPosition(circlesolid->getFrame()->getPosition() - (circlesolid->getRadius()/z_EC_nrm2)*z_EC);
    cFrame[iplane]->setPosition(cFrame[icircle]->getPosition(false) - Wn*g);
  }

}

