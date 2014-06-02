/* Copyright (C) 2004-2009 MBSim Development Team
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

#ifndef _CONTOUR_PDATA_H_
#define _CONTOUR_PDATA_H_

#include "fmatvec/fmatvec.h"
#include "mbsim/frame.h"
#include <vector>

namespace MBSim {

  class Point;

  /**
   * \brief struct for data-management for single point on a contour to describe contact kinematics
   * \author Roland Zander
   * \date 2009-03-19 some comments (Thorsten Schindler)
   * \date 2009-04-02 Wn / Wt / WrOC deleted (Thorsten Schindler)
   * \date 2009-04-05 added specific constructors for arguments double and Vec (Schindler / Zander)
   * \date 2012-03-14 added ContourParameterType for staggered grid and modified constructor for argument int (Cebulla)
   */
  class ContourPointData {
    public:

      enum ContourParameterType { node, staggeredNode, continuum, extInterpol };

      /**
       * \brief constructor
       */
      ContourPointData() : type(continuum), ID(0) {}
      ContourPointData(const double       &alpha_) : type(continuum), ID(0), alpha() {
        alpha(0) = alpha_;
      }
      ContourPointData(const fmatvec::Vec2 &alpha_) : type(continuum), ID(0), alpha(alpha_) {}
      ContourPointData(const int  &id_, const ContourParameterType type_ = node) : type(type_), ID(id_) {}

      /**
       * \brief destructor
       */
      virtual ~ContourPointData() {}

      /* GETTER / SETTER */
      ContourParameterType& getContourParameterType() { return type; }
      const ContourParameterType& getContourParameterType() const { return type; }
      int& getNodeNumber() { return ID; }
      const int& getNodeNumber() const { return ID; }
      fmatvec::Vec2& getLagrangeParameterPosition() { return alpha; }
      const fmatvec::Vec2& getLagrangeParameterPosition() const { return alpha; }
      fmatvec::Vec2& getLagrangeParameterVelocity() { return alphap; }
      const fmatvec::Vec2& getLagrangeParameterVelocity() const { return alphap; }
      fmatvec::VecV& getInterpolationWeights() { return iWeights; }
      const fmatvec::VecV& getInterpolationWeights() const { return iWeights; }
      Frame& getFrameOfReference() { return cosy; }
      const Frame& getFrameOfReference() const { return cosy; }
      /***************************************************/

    protected:
      /** 
       * \brief type of data representation: node, continuum, interpolation (extinterpol) 
       */
      ContourParameterType type;

      /** 
       * \brief ID of node or other discret interface within body -> FiniteElements
       */
      int ID;

      /**
       * \brief contour parameter(s)
       */
      fmatvec::Vec2 alpha;

      /**
       * \brief contour parameter(s) velocities
       */
      fmatvec::Vec2 alphap;

      /** 
       * \brief interpolation weights
       */
      fmatvec::VecV iWeights;

      /**
       * \brief list of nodes used in interpolation
       *
       * the (body specific) ID can be accessed using ->iPoint[NNumber]->getID();
       */
      std::vector<Point*> iPoints;

      /**
       * \brief accompanying frame
       */
      Frame cosy;
  };

}

#endif /* _CONTOUR_PDATA_H_ */

