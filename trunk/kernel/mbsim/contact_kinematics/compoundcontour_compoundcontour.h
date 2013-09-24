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

#ifndef _CONTACT_KINEMATICS_COMPOUNDCONTOUR_COMPOUNDCONTOUR_H_
#define _CONTACT_KINEMATICS_COMPOUNDCONTOUR_COMPOUNDCONTOUR_H_

#include "contact_kinematics.h"

namespace MBSim {

  class CompoundContour;
  class Contour;

  /**
   * \brief contact paring between set of contours and set of contours
   * \author Martin Foerg
   * \date 2009-07-28 pure virtual updates (Thorsten Schindler)
   */
  class ContactKinematicsCompoundContourCompoundContour : public ContactKinematics {
    public:
      /* INHERITED INTERFACE */
      virtual void assignContours(const std::vector<Contour*> &contour);

      void updateg(fmatvec::Vec &g, ContourPointData *cpData, int index = 0) { }
      void updatewb(fmatvec::Vec &wb, const fmatvec::Vec &g, ContourPointData* cpData) { }

      ContactKinematics* getContactKinematics(int i=0) const { return contactKinematics[i]; }
      /***************************************************/

    private:
      /**
       * \brief contour classes
       */
      CompoundContour *contour0;
      CompoundContour *contour1;

      /**
       * \brief contact kinematics possibilities between compound contours
       */
      std::vector<ContactKinematics*> contactKinematics;
  };

}

#endif /* _CONTACT_KINEMATICS_COMPOUNDCONTOUR_COMPOUNDCONTOUR_H_ */

