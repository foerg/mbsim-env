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
 * Contact: markus.ms.schneider@gmail.com
 */

#ifndef _SIGNALMBSIM_H_
#define _SIGNALMBSIM_H_

#include "mbsim/link.h"

namespace MBSim {
  class DynamicSystem;
}

namespace MBSimControl {

  const MBXMLUtils::NamespaceURI MBSIMCONTROL("http://mbsim.berlios.de/MBSimControl");

  /*!
   * \brief Signal
   * \author Markus Schneider
   */
  class Signal : public MBSim::Link {

    public:
      Signal(const std::string &name) : Link(name) {}

      void init(InitStage stage);

      /* INHERITED INTERFACE OF LINKINTERFACE */
      virtual void updateg(double t) {getSignal(); }; // at least one signal-update
      virtual void updategd(double t) {};
      /***************************************************/

      /* INHERITED INTERFACE OF LINK */
      virtual void updateWRef(const fmatvec::Mat& ref, int i=0) {}
      virtual void updateVRef(const fmatvec::Mat& ref, int i=0) {}
      virtual void updatehRef(const fmatvec::Vec &hRef, int i=0) {}
      virtual void updatedhdqRef(const fmatvec::Mat& ref, int i=0) {}
      virtual void updatedhduRef(const fmatvec::SqrMat& ref, int i=0) {}
      virtual void updatedhdtRef(const fmatvec::Vec& ref, int i=0) {}
      virtual void updaterRef(const fmatvec::Vec &ref, int i=0) {}
      virtual bool isActive() const { return false; }
      virtual bool gActiveChanged() { return false; }
      virtual bool isSingleValued() const { return true; }
      /***************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      std::string getType() const { return "Signal"; }
      virtual void plot(double t, double dt = 1);
      /***************************************************/
      
      virtual const fmatvec::VecV& getSignal() { return s; }
      virtual int getSignalSize() { return s.size(); }

    protected:
      fmatvec::VecV s;
  };

}

#endif /* _SIGNALMBSIM_H_ */

