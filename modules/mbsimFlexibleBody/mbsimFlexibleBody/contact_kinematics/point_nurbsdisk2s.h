/* Copyright (C) 2004-2015 MBSim Development Team
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
 * Contact: thorsten.schindler@mytum.de
 */

#ifndef _CONTACT_KINEMATICS_POINT_NURBSDISK2S_H_
#define _CONTACT_KINEMATICS_POINT_NURBSDISK2S_H_

#include "mbsim/contact_kinematics/contact_kinematics.h"

namespace MBSim {
  class Point;
}

namespace MBSimFlexibleBody {

  class NurbsDisk2s;

  /** 
   * \brief pairing point to nurbs disk
   * \author Kilian Grundl
   * \author Raphael Missel
   * \author Thorsten Schindler
   * \date 2009-05-14 initial commit (Missel / Grundl / Schindler)
   * \date 2009-08-16 contour / visualisation (Missel / Grundl / Schindler)
   */
  class ContactKinematicsPointNurbsDisk2s : public MBSim::ContactKinematics {
    public:
      /*! 
       * \brief constructor 
       */
      ContactKinematicsPointNurbsDisk2s();
      
      /*! 
       * \brief destructor
       */
       ~ContactKinematicsPointNurbsDisk2s() override;

      /* INHERITED INTERFACE */
      void assignContours(const std::vector<MBSim::Contour*> &contour) override;
      void updateg(MBSim::SingleContact &contact, int i=0) override;
      /***************************************************/

    private:
      /**
       * \brief contour index
       */
      int ipoint, inurbsdisk;

      /**
       * \brief contour classes
       */
      MBSim::Point *point;
      NurbsDisk2s *nurbsdisk;
  };

}

#endif /* _CONTACT_KINEMATICS_POINT_NURBSDISK2S_H_ */
