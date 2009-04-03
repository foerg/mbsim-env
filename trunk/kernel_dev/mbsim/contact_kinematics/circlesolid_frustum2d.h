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
 *          rzander@users.berlios.de
 */

#ifndef _CONTACT_KINEMATICS_CIRCLESOLID_FRUSTUM2D_H_
#define _CONTACT_KINEMATICS_CIRCLESOLID_FRUSTUM2D_H_

#include "contact_kinematics.h"

namespace MBSim {

  class CircleSolid;
  class Frustum2D;

  /** 
   * \brief pairing circle outer side to planar frustum
   * \author Martin Foerg
   * \date 2009-04-02 some comments (Thorsten Schindler)
   * \todo change stage to new interface TODO
   */
  class ContactKinematicsCircleSolidFrustum2D : public ContactKinematics {
    public:
      /* INHERITED INTERFACE */
      virtual void assignContours(const std::vector<Contour*> &contour);
      virtual void stage1(Vec &g, std::vector<ContourPointData> &cpData);
      virtual void stage2(const Vec &g, Vec &gd, std::vector<ContourPointData> &cpData);
      /***************************************************/

    private:
      /**
       * \brief contour index
       */
      int icircle, ifrustum;
      
      /**
       * \brief contour classes
       */
      CircleSolid *circle;
      Frustum2D *frustum; 

  };

}

#endif

