/* Copyright (C) 2004-2019 MBSim Development Team
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

#ifndef _CONTACT_KINEMATICS_CYLINDRICALGEAR_RACK_H_
#define _CONTACT_KINEMATICS_CYLINDRICALGEAR_RACK_H_

#include "contact_kinematics.h"

namespace MBSim {

  class CylindricalGear;
  class Rack;

  /**
   * \brief pairing cylindrical gear to rack
   * \author Martin Foerg
   */
  class ContactKinematicsCylindricalGearRack : public ContactKinematics {
    public:
      /**
       * \brief constructor
       */
      ContactKinematicsCylindricalGearRack() = default;

      /**
       * \brief destructor
       */
      ~ContactKinematicsCylindricalGearRack() override = default;
      
      /* INHERITED INTERFACE */
      void assignContours(const std::vector<Contour*> &contour) override;
      void updateg(SingleContact &contact, int i=0) override;
      void updatewb(SingleContact &contact, int i=0) override;
      /***************************************************/

    private:
      /**
       * \brief contour index
       */
      int igear;
      int irack;
      double m;
      double al0;
      int z[2];
      double beta[2];
      int ksave[2][2];
      int signisave[2]{1,1};
      double s0h2, delh1;

      /**
       * \brief contour classes
       */
      CylindricalGear *gear;
      Rack *rack;
  };

}

#endif
