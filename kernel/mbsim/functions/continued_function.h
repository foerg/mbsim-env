/* Copyright (C) 2004-2016 MBSim Development Team
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

#ifndef _CONTINUED_FUNCTION_H_
#define _CONTINUED_FUNCTION_H_

#include "mbsim/functions/function.h"

namespace MBSim {

  template<typename Sig> class ContinuedFunction; 

  template<typename Ret, typename Arg> 
  class ContinuedFunction<Ret(Arg)> : public Function<Ret(Arg)> {
    using B = fmatvec::Function<Ret(Arg)>; 
    public:
      ContinuedFunction() : f(nullptr), rule(nullptr) { }
      ~ContinuedFunction() override { delete f; delete rule; }
      int getArgSize() const override { return f->getArgSize(); }
      std::pair<int, int> getRetSize() const override { return f->getRetSize(); }
      Ret operator()(const Arg &x) override { return (*f)((*rule)(x)); }
      typename B::DRetDArg parDer(const Arg &x) override { return f->parDer((*rule)(x)); }
      typename B::DRetDArg parDerDirDer(const Arg &xDir, const Arg &x) override { return f->parDerDirDer(xDir,(*rule)(x)); }
      void setFunction(Function<Ret(Arg)> *f_) {
        f = f_;
        f->setParent(this);
        f->setName("Function");
      }
      void setContinuationRule(Function<Arg(Arg)> *rule_) { 
        rule = rule_; 
        rule->setParent(this);
        rule->setName("ContinuationRule");
      }
      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"function");
        setFunction(ObjectFactory::createAndInit<Function<Ret(Arg)> >(e->getFirstElementChild()));
        e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"continuationRule");
        setContinuationRule(ObjectFactory::createAndInit<Function<Arg(Arg)> >(e->getFirstElementChild()));
      }
      void init(Element::InitStage stage, const InitConfigSet &config) override {
        Function<Ret(Arg)>::init(stage, config);
        f->init(stage, config);
        InitConfigSet configRule(config);
        configRule.insert(noDer);
        configRule.insert(noDerDer);
        rule->init(stage, configRule);
      }
    private:
      Function<Ret(Arg)> *f;
      Function<Arg(Arg)> *rule;
  };

}

#endif
