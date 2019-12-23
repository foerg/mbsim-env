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

#ifndef _LINEAR_TRANSLATION_H_
#define _LINEAR_TRANSLATION_H_

#include "mbsim/functions/function.h"
#include "mbsim/utils/utils.h"

namespace MBSim {

  template<class Arg>
  class LinearTranslation : public Function<fmatvec::Vec3(Arg)> {
    using B = fmatvec::Function<fmatvec::Vec3(Arg)>; 
    private:
      typename B::DRetDArg A;
      fmatvec::Vec3 b;
      fmatvec::Vec3 zeros(const typename B::DRetDArg &x) { return fmatvec::Vec3(x.rows()); }
    public:
      LinearTranslation() = default;
      LinearTranslation(const typename B::DRetDArg &A_) : A(A_) { }
      LinearTranslation(const typename B::DRetDArg &A_, const fmatvec::Vec3 &b_) : A(A_), b(b_) { }
      void setTranslationVectors(const typename B::DRetDArg &A_) { A <<= A_; }
      void setOffset(const fmatvec::Vec3 &b_) { b <<= b_; }
      int getArgSize() const override { return A.cols(); }
      fmatvec::Vec3 operator()(const Arg &arg) override { return A*arg+b; }
      typename B::DRetDArg parDer(const Arg &arg) override { return A; }
      typename B::DRetDArg parDerDirDer(const Arg &arg1Dir, const Arg &arg1) override { return typename B::DRetDArg(A.rows(),A.cols()); }
      bool constParDer() const override { return true; }
      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"translationVectors");
        setTranslationVectors(MBXMLUtils::E(e)->getText<typename B::DRetDArg>());
        e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"offset");
        if(e) setOffset(MBXMLUtils::E(e)->getText<fmatvec::Vec3>());
      }
  };

  template<>
  inline fmatvec::Vec3 LinearTranslation<double>::parDerDirDer(const double &arg1Dir, const double &arg1) { return fmatvec::Vec3(); }

}

#endif
