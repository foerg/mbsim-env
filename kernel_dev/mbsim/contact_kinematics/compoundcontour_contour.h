/* Copyright (C) 2008  Martin Förg
 
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
 * Contact:
 *   mfoerg@users.berlios.de
 *
 */

#ifndef _CONTACT_KINEMATICS_COMPOUND_CONTOUR_H_
#define _CONTACT_KINEMATICS_COMPOUND_CONTOUR_H_

#include "contact_kinematics.h"

namespace MBSim {

  class Cuboid;
  class Plane; 
  class CompoundContour;
  class Contour;

    class ContactKinematicsCompoundContourContour : public ContactKinematics {

    private:
      int icompound, icontour;
      CompoundContour *compound;
      Contour *contour;
      std::vector<ContactKinematics*> contactKinematics;

    public:
      void updateg(std::vector<Vec> &g, std::vector<ContourPointData*> &cpData);
      void updategd(std::vector<Vec> &g, std::vector<Vec> &gd, std::vector<ContourPointData*> &cpData);
      void updatewb(std::vector<Vec> &wb, std::vector<Vec> &g, std::vector<ContourPointData*> &cpData);

      void assignContours(const std::vector<Contour*> &contour);
    };

}

#endif 
