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

#ifndef _CONTACT_KINEMATICS_SPHERE_FRUSTUM_H_
#define _CONTACT_KINEMATICS_SPHERE_FRUSTUM_H_

#include "contact_kinematics.h"
#include "mbsim/mbsim_event.h"

namespace MBSim {

  class Sphere;
  class Frustum;

  /**
   * \brief pairing sphere to frustum
   * \author Martin Foerg
   * \date 2009-04-02 some comments (Thorsten Schindler)
   * \date 2009-07-13 updateg implemented (Bastian Esefeld)
   * \todo updatewb implementation
   */   
  class ContactKinematicsSphereFrustum : public ContactKinematics {
    public:
      /* INHERITED INTERFACE */
      virtual void assignContours(const std::vector<Contour*> &contour);
      virtual void updateg(double &g, ContourPointData *cpData, int index = 0);
      virtual void updatewb(fmatvec::Vec &wb, double g, ContourPointData *cpData) { throw MBSimError("(ContactKinematicsSphereFrustum:updatewb): Not implemented!"); }
      //virtual void stage1(double &g, std::vector<ContourPointData> &cpData);
      //virtual void stage2(double g, fmatvec::Vec &gd, std::vector<ContourPointData> &cpData);
      /***************************************************/

    protected:
      /**
       * \brief contour index
       */
      int isphere, ifrustum;
      
      /**
       * \brief contour classes
       */
      Sphere *sphere;
      Frustum *frustum;

  };

}

#endif
