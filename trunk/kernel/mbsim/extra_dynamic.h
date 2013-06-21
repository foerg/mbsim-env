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
 * Contact: mbachmayer@gmx.de
 */

#ifndef _EXTRA_DYNAMIC_H_
#define _EXTRA_DYNAMIC_H_

#include "mbsim/element.h"

namespace MBSim {

  class DynamicSystem;
  class Link;

  /**
   * \brief base class for dynamic systems of the form \f$\dot{x}=f\left(x,u\right)\f$ and \f$y=g\left(x,u\right)\f$
   * \author Mathias Bachmayer
   * \date 2009-04-06 interface extracted (Thorsten Schindler)
   * \date 2009-07-28 splitted interfaces (Thorsten Schindler)
   */
  class ExtraDynamic : public Element {
    public:
      /**
       * \brief constructor
       * \param name of extra dynamic system
       */
      ExtraDynamic(const std::string &name);

      /*!
       * \brief update order one parameter increment
       * \param simulation time
       * \param simulation step size
       */
      virtual void updatedx(double t, double dt) = 0;

      /*!
       * \brief update differentiated order one parameter
       * \param simulation time
       */
      virtual void updatexd(double t) = 0;
      virtual void calcxSize() {};
      virtual const fmatvec::Vec& getx() const { return x; }
      virtual fmatvec::Vec& getx() { return x; }
      virtual void setxInd(int xInd_) { xInd = xInd_; };
      virtual int getxSize() const { return xSize; }
      virtual void updatexRef(const fmatvec::Vec& ref);
      virtual void updatexdRef(const fmatvec::Vec& ref);
      virtual void init(InitStage stage);
      virtual void initz();
      virtual void writez(const H5::Group & group);
      virtual void readz0(const H5::Group & group);
      /***************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "ExtraDynamic"; }
      virtual void closePlot(); 
      virtual void plot(double t, double dt = 1); 
      /***************************************************/

      /* INTERFACE TO BE DEFINED IN DERIVED CLASSES */
      virtual void updateg(double t) = 0;
      /***************************************************/

      /* GETTER / SETTER */
      void setInitialState(const fmatvec::Vec &x0_) { x0 = x0_; }
      virtual Element *getByPathSearch(std::string path);
      /***************************************************/
      
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);

    protected:
      /**
       * \brief order one parameter, differentiated order one parameter, initial order one parameters
       */
      fmatvec::Vec x, xd, x0;

      /**
       * \brief size of order one parameter vector
       */
      int xSize;

      /**
       * \brief index of order one parameters
       */
      int xInd;

      /**
       * \brief system output \f$y=g\left(x\right)\f$
       */
      fmatvec::Vec y;

  };

}

#endif /* _EXTRA_DYNAMIC_H_ */

