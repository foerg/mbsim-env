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
 * Contact: friedrich.at.gc@googlemail.com
 */

#ifndef _EXTERNSIGNALSOURCE_H_
#define _EXTERNSIGNALSOURCE_H_

#include "mbsimControl/signal_.h"

namespace MBSimControl {

  /** Signal which value if given by a external resource. E.g. Cosimulation */
  class ExternSignalSource : public Signal {
    protected:
      fmatvec::VecV source;
      int sourceSize;
    public:
      ExternSignalSource(const std::string &name="") : Signal(name), sourceSize(0) {}
      void setSourceSize(int size) { sourceSize=size; source.resize(sourceSize); }
      std::string getType() const { return "ExternSignalSource"; }
      void updateh(double t, int j=0) { s = source; }
      void setSignal(const fmatvec::VecV& s) { assert(s.size()==source.size()); source=s; }
      void initializeUsingXML(xercesc::DOMElement *element) {
        Signal::initializeUsingXML(element);
        setSourceSize(getInt(MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMCONTROL%"sourceSize")));
      }
  };

}

#endif /* _SENSOR_H_ */
