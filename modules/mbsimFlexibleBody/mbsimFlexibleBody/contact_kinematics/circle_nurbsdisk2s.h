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

#ifndef _CONTACT_KINEMATICS_CIRCLE_NURBSDISK2S_H_
#define _CONTACT_KINEMATICS_CIRCLE_NURBSDISK2S_H_

#include "mbsim/contact_kinematics/contact_kinematics.h"
#include "mbsim/contours/circle.h"
#include "mbsimFlexibleBody/contours/nurbs_disk_2s.h"

namespace MBSimFlexibleBody {

  /*!
   * \brief contact kinematics for contact between circle and nurbsdisk2s
   * \author Kilian Grundl
   * \date 2009-10-06 initial commit (Thorsten Schindler)
   */
  class ContactKinematicsCircleNurbsDisk2s : public MBSim::ContactKinematics {
    public:
      /*! 
       * \brief constructor 
       * \default no debugging
       * \default no warnings
       * \default global search 
       */
      ContactKinematicsCircleNurbsDisk2s();

      /*! 
       * \brief destructor
       */
      ~ContactKinematicsCircleNurbsDisk2s() override;

      /* INHERITED INTERFACE OF CONTACTKINEAMTICS */
      void assignContours(const std::vector<MBSim::Contour*> &contour) override;
      void updateg(MBSim::SingleContact &contact, int i=0) override;
      /***************************************************/

      /* GETTER / SETTER */
      void setLocalSearch(bool LOCALSEARCH_) { LOCALSEARCH=LOCALSEARCH_; }
      /***************************************************/

      void calcisSize() override { isSize = 1; }

    private:
      /** 
       * \brief contour index
       */
      int icircle{0}, inurbsdisk{0};

      /** 
       * \brief contour classes 
       */
      NurbsDisk2s *nurbsdisk{0}; 
      MBSim::Circle *circle{0};

      /**
       * \brief local contact search?
       */
      bool LOCALSEARCH{false};
  };

}

#endif /* _CONTACT_KINEMATICS_CIRLCE_NURBSDISK2S_H_ */
