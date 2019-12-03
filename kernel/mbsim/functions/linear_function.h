/* Copyright (C) 2004-2014 MBSim Development Team
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

#ifndef _LINEAR_FUNCTION_H_
#define _LINEAR_FUNCTION_H_

#include "mbsim/functions/function.h"
#include "mbsim/utils/utils.h"

namespace MBSim {

  template<typename Sig> class LinearFunction; 

  template<typename Ret, typename Arg>
  class LinearFunction<Ret(Arg)> : public Function<Ret(Arg)> {
    using B = fmatvec::Function<Ret(Arg)>; 
    private:
      double a0{0}, a1;
    public:
      LinearFunction(double a1_=0) :  a1(a1_) { }
      LinearFunction(double a0_, double a1_) : a0(a0_), a1(a1_) { }
      int getArgSize() const override { return 1; }
      std::pair<int, int> getRetSize() const override { return std::make_pair(1,1); }
      Ret operator()(const Arg &x) override { return FromDouble<Ret>::cast(a1*ToDouble<Arg>::cast(x)+a0); }
      typename B::DRetDArg parDer(const Arg &x) override { return FromDouble<typename B::DRetDArg>::cast(a1); }
      typename B::DRetDArg parDerDirDer(const Arg &xDir, const Arg &x) override { return FromDouble<typename B::DRetDArg>::cast(0); }
      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"a0");
        if(e) a0=MBXMLUtils::E(e)->getText<double>();
        e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"a1");
        a1=MBXMLUtils::E(e)->getText<double>();
      }
      void seta0(double a0_) { a0 = a0_; }
      void seta1(double a1_) { a1 = a1_; }
  };

}

#endif
