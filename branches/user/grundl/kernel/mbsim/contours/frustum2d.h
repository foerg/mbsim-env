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
 * Contact: mfeorg@users.berlios.de
 */

#ifndef _FRUSTUM2D_H_
#define _FRUSTUM2D_H_

#include "mbsim/contour.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <mbsim/utils/openmbv_utils.h>
#endif

namespace MBSim {

  /**
   * \brief planar slice of a frustum
   * \author Martin Foerg
   * \date 2009-04-20 some comments (Thorsten Schindler)
   * \date 2009-07-16 new file (Bastian Esefeld)
   */
  class Frustum2D : public RigidContour {
    public:
      /**
       * \brief constructor
       * \param name of frustum
       */
      Frustum2D(const std::string &name) : RigidContour(name), h(0) {}

      /* INHERITED INTERFACE OF ELEMENT */
      std::string getType() const { return "Frustum2D"; }
      virtual void init(InitStage stage);
      /***************************************************/

      /* GETTER / SETTER */
      void setRadii(const fmatvec::Vec2 &r_) { r = r_; }
      const fmatvec::Vec2& getRadii() const { return r; } 
      void setHeight(double h_) { h = h_; }
      double getHeight() const { return h; } 
      /***************************************************/

#ifdef HAVE_OPENMBVCPPINTERFACE
      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, tag, (optional (diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0))) { 
        OpenMBVFrustum ombv(1,1,1,diffuseColor,transparency);
        openMBVRigidBody=ombv.createOpenMBV(); 
      }
#endif

    private:
      /**
       * \brief radii of frustum in dirction of axis
       */
      fmatvec::Vec2 r;

      /**
       * \brief height of frustum
       */
      double h;

  };
  
}

#endif /* _FRUSTUM2D_H_ */
