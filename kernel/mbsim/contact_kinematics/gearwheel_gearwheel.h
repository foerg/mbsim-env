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

#ifndef _CONTACT_KINEMATICS_GEARWHEEL_GEARWHEEL_H_
#define _CONTACT_KINEMATICS_GEARWHEEL_GEARWHEEL_H_

#include "contact_kinematics.h"

namespace MBSim {

  class GearWheel;

  /**
   * \brief pairing gear wheel to gear wheel
   * \author Martin Foerg
   */
  class ContactKinematicsGearWheelGearWheel : public ContactKinematics {
    public:
      /**
       * \brief constructor
       */
      ContactKinematicsGearWheelGearWheel() = default;

      /**
       * \brief destructor
       */
      ~ContactKinematicsGearWheelGearWheel() override = default;
      
      /* INHERITED INTERFACE */
      void assignContours(const std::vector<Contour*> &contour) override;
      void updateg(SingleContact &contact, int i=0) override;
      void updatewb(SingleContact &contact, int i=0) override;
      /***************************************************/

    private:
      /**
       * \brief contour index
       */
      int igearwheel[2];
      double m;
      double al0, al;
      int z[2];
      double d0[2];
      double db[2];
      double rb[2];
      double sb[2];
      double beta[2];
      double a0, a;
      int side{-1};
      int k[2][2]{{0,0},{0,0}};
      double be[2][2], ga[2];

      /**
       * \brief contour classes
       */
      GearWheel *gearwheel[2];
  };

}

#endif
