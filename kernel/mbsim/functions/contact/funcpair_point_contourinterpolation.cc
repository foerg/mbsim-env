/* Copyright (C) 2004-2010 MBSim Development Team
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

#include <config.h>
#include "mbsim/functions/contact/funcpair_point_contourinterpolation.h"
#include "mbsim/frames/frame.h"
#include "mbsim/contours/point.h"
#include "mbsim/contours/contour_interpolation.h"

using namespace fmatvec;

namespace MBSim {

  Vec2 FuncPairPointContourInterpolation::operator()(const Vec2 &alpha) {
    throw std::runtime_error("Check the codel here which does not compile but was not used till now!!!");
    //return (contour->evalWu(alpha)).T() * (contour->evalPosition(alpha) - point->getFrame()->evalPosition());
  }

  Vec3 FuncPairPointContourInterpolation::evalWrD(const Vec2 &alpha) {
    return contour->evalPosition(alpha) - point->getFrame()->evalPosition();
  }

}
