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
 * Contact: martin.o.foerg@gmail.com
 */

#ifndef _TRANSLATION_ALONG_FIXED_AXIS_H_
#define _TRANSLATION_ALONG_FIXED_AXIS_H_

#include "mbsim/functions/function.h"
#include "mbsim/utils/utils.h"

namespace MBSim {

  template<class Arg>
  class TranslationAlongFixedAxis : public Function<fmatvec::Vec3(Arg)> {
    using B = fmatvec::Function<fmatvec::Vec3(Arg)>; 
    private:
      fmatvec::Vec3 a, zero;
    public:
      TranslationAlongFixedAxis() = default;
      TranslationAlongFixedAxis(const fmatvec::Vec3 &a_) : a(a_) { }
      int getArgSize() const override { return 1; }
      fmatvec::Vec3 operator()(const Arg &arg) override { return a*arg; }
      typename B::DRetDArg parDer(const Arg &arg) override { return a; }
      typename B::DRetDArg parDerDirDer(const Arg &arg1Dir, const Arg &arg1) override { return zero; }
      bool constParDer() const override { return true; }
      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"axisOfTranslation");
        a=FromMatStr<fmatvec::Vec3>::cast((MBXMLUtils::X()%MBXMLUtils::E(e)->getFirstTextChild()->getData()).c_str());
      }
  };

}

#endif
