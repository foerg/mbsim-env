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

#ifndef _CONTACT_KINEMATICS_CIRCLE_SPATIALCONTOUR_H_
#define _CONTACT_KINIMATICS_CIRCLE_SPATIALCONTOUR_H_

#include "contact_kinematics.h"

namespace MBSim {

  class Circle;
  class SpatialContour;
  class FuncPairSpatialContourCircle;

  /*! \brief pairing circle outer side to spatial contour
   * \author Markus Friedrich
   */
  class ContactKinematicsCircleSpatialContour : public ContactKinematics {
    public:
      ContactKinematicsCircleSpatialContour() = default;
      ~ContactKinematicsCircleSpatialContour();

      /* INHERITED INTERFACE */
      void calcisSize() override { isSize = 2*maxNumContacts; }
      void assignContours(const std::vector<Contour*> &contour) override;
      void search() override;
      void updateg(SingleContact &contact, int i=0) override;
      void setInitialGuess(const fmatvec::MatV &zeta0_) override;
      /***************************************************/

    private:
      /**
       * \brief contour index
       */
      int icircle, ispatialcontour;

      /**
       * \brief contour classes
       */
      Circle *circle;
      SpatialContour *spatialcontour;

      /**
       * \brief root function
       */
      MBSim::FuncPairSpatialContourCircle *func;
  };

}

#endif /* _CONTACT_KINEMATICS_CIRCLESOLID_SPATIALCONTOUR_H_ */
