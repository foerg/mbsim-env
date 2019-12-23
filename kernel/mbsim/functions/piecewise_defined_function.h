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

#ifndef _PIECEWISE_DEFINED_FUNCTIONS_H_
#define _PIECEWISE_DEFINED_FUNCTIONS_H_

#include "mbsim/functions/function.h"
#include "mbsim/utils/utils.h"

namespace MBSim {

  template<typename Sig> class LimitedFunction; 

  template <typename Ret, typename Arg>
  struct LimitedFunction<Ret(Arg)> {
    LimitedFunction(Function<Ret(Arg)> *function_, double limit_) : function(function_), limit(limit_) { }
    Function<Ret(Arg)> *function;
    double limit;
  };

  template<typename Sig> class PiecewiseDefinedFunction; 

  template<typename Ret, typename Arg>
  class PiecewiseDefinedFunction<Ret(Arg)> : public Function<Ret(Arg)> {
    using B = fmatvec::Function<Ret(Arg)>; 
    public:
      PiecewiseDefinedFunction()  { a.push_back(0); }
      ~PiecewiseDefinedFunction() override {
        for(unsigned int i=0; i<function.size(); i++)
          delete function[i];
      }
      void addLimitedFunction(const LimitedFunction<Ret(Arg)> &limitedFunction) {
        function.push_back(limitedFunction.function);
        limitedFunction.function->setParent(this);
        a.push_back(limitedFunction.limit);
      }
      int getArgSize() const override { return 1; }
      std::pair<int, int> getRetSize() const override { return function.size()?function[0]->getRetSize():std::make_pair(0,1); }
      Ret operator()(const Arg &x) override {
        for(unsigned int i=0; i<function.size(); i++)
          if(ToDouble<Arg>::cast(x)<=a[i+1])
            return y0[i] + (*function[i])(x-FromDouble<Arg>::cast(x0[i]));
        this->throwError("(PiecewiseDefinedFunction::operator()): x out of range! x= "+fmatvec::toString(x)+", upper bound= "+fmatvec::toString(a[function.size()]));
      }
      typename B::DRetDArg parDer(const Arg &x) override {
        for(unsigned int i=0; i<function.size(); i++)
          if(ToDouble<Arg>::cast(x)<=a[i+1])
            return function[i]->parDer(x-FromDouble<Arg>::cast(x0[i]));
        this->throwError("(PiecewiseDefinedFunction::parDer): x out of range! x= "+fmatvec::toString(x)+", upper bound= "+fmatvec::toString(a[function.size()]));
      }
      typename B::DRetDArg parDerDirDer(const Arg &xDir, const Arg &x) override {
        for(unsigned int i=0; i<function.size(); i++)
          if(ToDouble<Arg>::cast(x)<=a[i+1])
            return function[i]->parDerDirDer(xDir,x-FromDouble<Arg>::cast(x0[i]));
        this->throwError("(PiecewiseDefinedFunction::parDerDirDer): x out of range! x= "+fmatvec::toString(x)+", upper bound= "+fmatvec::toString(a[function.size()]));
      }

      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"limitedFunctions");
        xercesc::DOMElement *ee=e->getFirstElementChild();
        while(ee && MBXMLUtils::E(ee)->getTagName()==MBSIM%"LimitedFunction") {
          addLimitedFunction(LimitedFunction<Ret(Arg)>(ObjectFactory::createAndInit<Function<Ret(Arg)> >(MBXMLUtils::E(ee)->getFirstElementChildNamed(MBSIM%"function")->getFirstElementChild()),MBXMLUtils::E(MBXMLUtils::E(ee)->getFirstElementChildNamed(MBSIM%"limit"))->getText<double>()));
          ee=ee->getNextElementSibling();
        }
        e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"shiftAbscissa");
        if(e) shiftAbscissa=MBXMLUtils::E(e)->getText<bool>();
        e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"shiftOrdinate");
        if(e) shiftOrdinate=MBXMLUtils::E(e)->getText<bool>();
      }
      void init(Element::InitStage stage, const InitConfigSet &config) override {
        Function<Ret(Arg)>::init(stage, config);
        for(auto it=function.begin(); it!=function.end(); it++)
          (*it)->init(stage, config);
        if(stage==Element::preInit) {
          if(shiftAbscissa) {
            for(unsigned int i=1; i<a.size(); i++)
              a[i] += a[i-1];
            x0 = a;
          }
          else
            x0.resize(a.size());
          y0.resize(a.size(),0.0*(*function[0])(Arg(1)));
          if(shiftOrdinate) {
            for(unsigned int i=1; i<a.size(); i++)
              y0[i] = (*this)(FromDouble<Arg>::cast(a[i]));
          }
        }
      }
    private:
      std::vector<Function<Ret(Arg)> *> function;
      std::vector<double> a, x0;
      std::vector<Ret> y0;
      bool shiftAbscissa{false}, shiftOrdinate{false};
  };

}

#endif
