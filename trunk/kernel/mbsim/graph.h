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


#ifndef _GRAPH_H_
#define _GRAPH_H_

#include "mbsim/dynamic_system.h"

namespace H5 {
  class Group;
}

namespace MBSim {


  /**
   * \brief class for tree-structured mechanical systems with recursive and flat memory mechanism
   * \author Martin Foerg
   * \date 2009-03-26 some comments (Thorsten Schindler)
   */
  class Graph : public DynamicSystem {
    public:
      /**
       * \brief constructor
       * \param name of tree
       */
      Graph(const std::string &name);

      /**
       * \brief destructor
       */
      virtual ~Graph();

      /* INHERITED INTERFACE OF OBJECTINTERFACE */
      virtual void updateStateDependentVariables(double t);
      virtual void updatedu(double t, double dt);
      virtual void updatezd(double t);
      virtual void updateud(double t, int i=0);
      virtual void sethSize(int h, int j=0) {(this->*sethSize_[j])(h);}
      virtual void calcqSize();
      virtual void calcuSize(int j=0) {(this->*calcuSize_[j])();}
      virtual void setqInd(int qInd);
      virtual void setuInd(int uInd, int j=0) {(this->*setuInd_[j])(uInd);}
      virtual void sethInd(int hInd, int j=0) {(this->*sethInd_[j])(hInd);}

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "Graph"; }
      /***************************************************/

      /* INHERITED INTERFACE OF SUBSYSTEM */
      virtual void updateJacobians(double t, int j=0);
      void facLLM(int i=0); 

      void (Graph::*calcuSize_[2])(); 
      void (Graph::*sethSize_[2])(int h); 
      void (Graph::*setuInd_[2])(int uInd); 
      void (Graph::*sethInd_[2])(int hInd); 
      void calcuSize0();
      void calcuSize1();
      void sethSize0(int h);
      void sethSize1(int h);
      void setuInd0(int uInd);
      void setuInd1(int uInd);
      void sethInd0(int hInd);
      void sethInd1(int hInd);

      /***************************************************/

      /**
       * \brief add new object to graph at level
       * \param level
       * \param object
       */
      void addObject(int level, Object* object); 

      void printGraph();

    protected:
      /**
       * \brief none
       */
      std::vector< std::vector<Object*> > obj;
  };

}

#endif

