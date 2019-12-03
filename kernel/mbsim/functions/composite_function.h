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

#ifndef _NESTED_FUNCTION_H_
#define _NESTED_FUNCTION_H_

#include "mbsim/functions/function.h"

namespace MBSim {

  template<typename Sig> class CompositeFunction; 

  template<typename Ret, typename Argo, typename Argi> 
  class CompositeFunction<Ret(Argo(Argi))> : public Function<Ret(Argi)> {
    using B = Function<Ret(Argi)>; 
    public:
      CompositeFunction(Function<Ret(Argo)> *fo_=nullptr, Function<Argo(Argi)> *fi_=nullptr) : fo(fo_), fi(fi_) {
        if(fo)
          fo->setParent(this);
        if(fi)
          fi->setParent(this);
      }
      ~CompositeFunction() override {
        delete fo;
        delete fi;
      }
      int getArgSize() const override {
        return fi->getArgSize();
      }
      std::pair<int, int> getRetSize() const override {
        return fo->getRetSize();
      }
      Ret operator()(const Argi &arg) override {
        return (*fo)((*fi)(arg));
      }
      typename B::DRetDArg parDer(const Argi &arg) override {
        return fo->parDer((*fi)(arg))*fi->parDer(arg);
      }
      typename B::DRetDArg parDerDirDer(const Argi &argDir, const Argi &arg) override {
        return fo->parDerDirDer(fi->parDer(arg)*argDir,(*fi)(arg))*fi->parDer(arg) + fo->parDer((*fi)(arg))*fi->parDerDirDer(argDir,arg);
      }
      void setOuterFunction(Function<Ret(Argo)> *fo_) {
        fo = fo_;
        fo->setParent(this);
        fo->setName("Outer");
      }
      void setInnerFunction(Function<Argo(Argi)> *fi_) {
        fi = fi_;
        fi->setParent(this);
        fi->setName("Inner");
      }
      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"outerFunction");
        setOuterFunction(ObjectFactory::createAndInit<Function<Ret(Argo)> >(e->getFirstElementChild()));
        e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"innerFunction");
        setInnerFunction(ObjectFactory::createAndInit<Function<Argo(Argi)> >(e->getFirstElementChild()));
      }
      void init(Element::InitStage stage, const InitConfigSet &config) override {
        Function<Ret(Argi)>::init(stage, config);
        fo->init(stage, config);
        fi->init(stage, config);
      }
    private:
      Function<Ret(Argo)> *fo;
      Function<Argo(Argi)> *fi;
  };
}

#endif
