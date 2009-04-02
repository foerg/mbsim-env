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
 * Contact: rzander@users.berlios.de
 */

#ifndef _CONTACT_KINEMATICS_CIRCLEHOLLOW_CYLINDERFLEXIBLE_H_
#define _CONTACT_KINEMATICS_CIRCLEHOLLOW_CYLINDERFLEXIBLE_H_

#include "contact_kinematics.h"

namespace MBSim {

  class CircleHollow;
  class CylinderFlexible;
  class FuncPairContour1sCircleHollow;

  /**
   * \brief pairing CircleHollow to CylinderFlexible
   * \author Roland Zander
   * \date 18.03.09
   */
  class ContactKinematicsCircleHollowCylinderFlexible : public ContactKinematics {
    public:
      /**
       * \brief constructor
       */
      ContactKinematicsCircleHollowCylinderFlexible() {}

      /**
       * \brief destructor
       */
      virtual ~ContactKinematicsCircleHollowCylinderFlexible();

      /* INHERITED INTERFACE */
      void assignContours(const std::vector<Contour*> &contour);
      void updateg(Vec &g, ContourPointData *cpData);
    
    private:
      int icircle, icylinder;
      CircleHollow *circle;
      CylinderFlexible *cylinder;
      FuncPairContour1sCircleHollow *func;
  };

}

#endif

