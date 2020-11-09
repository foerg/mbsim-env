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
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _CONSTRAINT_H
#define _CONSTRAINT_H

#include "mbsim/element.h"

namespace MBSim {

  /** 
   * \brief Class for constraints between generalized coordinates of objects
   * \author Martin Foerg
   */
  class Constraint : public Element {
    public:
      Constraint(const std::string &name);
      virtual void updateGeneralizedCoordinates() {}
      virtual void updateGeneralizedJacobians(int j=0) { }
      virtual void updatedx();
      virtual void updatexd() { }
      virtual void calcxSize() { xSize = 0; }
      virtual const fmatvec::Vec& getx() const { return x; }
      virtual fmatvec::Vec& getx() { return x; }
      virtual int getxInd() { return xInd; }
      virtual void setxInd(int xInd_) { xInd = xInd_; };
      virtual int getxSize() const { return xSize; }
      virtual void updatexRef(fmatvec::Vec& xParent);
      virtual void updatexdRef(fmatvec::Vec& xdParent);
      virtual void updatedxRef(fmatvec::Vec& dxParent);
      virtual void initz();
      virtual void writez(H5::GroupBase *group);
      virtual void readz0(H5::GroupBase *group);
      virtual void setUpInverseKinetics() { }
      std::shared_ptr<OpenMBV::Group> getOpenMBVGrp() override {return std::shared_ptr<OpenMBV::Group>();}
      bool getUpdateGeneralizedCoordinates() const { return updGC; }
      bool getUpdateGeneralizedJacobians() const { return updGJ; }
      void resetUpToDate() override { updGC = true; updGJ = true; }

      const fmatvec::Vec& evalxd();

      void createPlotGroup() override;

    protected:
      /** 
       * \brief order one parameters
       */
      fmatvec::Vec x;

      /** 
       * \brief differentiated order one parameters 
       */
      fmatvec::Vec xd;

      fmatvec::Vec dx;

      /**
       * \brief order one initial value
       */
      fmatvec::Vec x0;

      /**
       * \brief size  and local index of order one parameters
       */
      int xSize, xInd;
      bool updGC, updGJ;
  };

}

#endif
