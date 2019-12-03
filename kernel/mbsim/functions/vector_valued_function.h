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

#ifndef _VECTOR_VALUED_FUNCTIONS_H_
#define _VECTOR_VALUED_FUNCTIONS_H_

#include "mbsim/functions/function.h"

namespace MBSim {

  template <typename Ret, typename Arg>
    typename fmatvec::Function<Ret(double)>::DRetDArg parDer_(std::vector<Function<double(double)> *> component, const double &x) {
      typename fmatvec::Function<Ret(double)>::DRetDArg y(component.size(),fmatvec::NONINIT);
        for (unsigned int i=0; i<component.size(); i++)
          y(i)=component[i]->parDer(x);
      return y;
    }
  template <typename Ret, typename Arg>
    typename fmatvec::Function<Ret(Arg)>::DRetDArg parDer_(std::vector<Function<double(Arg)> *> component, const Arg &x) {
      typename fmatvec::Function<Ret(Arg)>::DRetDArg y(component.size(),x.size(),fmatvec::NONINIT);
        for (unsigned int i=0; i<component.size(); i++) {
          auto row=component[i]->parDer(x);
          for (int j=0; j<x.size(); j++)
            y(i,j)=row(j);
        }
      return y;
    }
  template <typename Ret, typename Arg>
    typename fmatvec::Function<Ret(double)>::DRetDArg parDerDirDer_(std::vector<Function<double(double)> *> component, const double &xDir, const double &x) {
      typename fmatvec::Function<Ret(double)>::DRetDArg y(component.size(),fmatvec::NONINIT);
        for (unsigned int i=0; i<component.size(); i++)
          y(i)=component[i]->parDerDirDer(xDir,x);
      return y;
    }

  template <typename Ret, typename Arg>
    typename fmatvec::Function<Ret(Arg)>::DRetDArg parDerDirDer_(std::vector<Function<double(Arg)> *> component, const Arg &xDir, const Arg &x) {
      typename fmatvec::Function<Ret(Arg)>::DRetDArg y(component.size(),x.size(),fmatvec::NONINIT);
        for (unsigned int i=0; i<component.size(); i++) {
          auto row=component[i]->parDerDirDer(xDir,x);
          for (int j=0; j<x.size(); j++)
            y(i,j)=row(j);
        }
      return y;
    }
  template<typename Sig> class VectorValuedFunction;

  // VectorValuedFunction with a vector as argument (no 2nd derivative defined)
  template<typename Ret, typename Arg>
  class VectorValuedFunction<Ret(Arg)> : public Function<Ret(Arg)> {
    using B = fmatvec::Function<Ret(Arg)>; 

    public:
      VectorValuedFunction() = default;
      VectorValuedFunction(std::vector<Function<double(Arg)> *> component_) : component(std::move(component_)) {
        for(auto it=component.begin(); it!=component.end(); ++it)
          (*it)->setParent(this);
      }
      ~VectorValuedFunction() override {
        for (unsigned int i=0; i<component.size(); i++)
          delete component[i];
      }
      void addComponent(Function<double(Arg)> *function) {
        component.push_back(function);
        function->setParent(this);
      }
      int getArgSize() const override { return 1; }
      std::pair<int, int> getRetSize() const override { return std::make_pair(component.size(),1); }
      Ret operator()(const Arg &x) override {
        Ret y(component.size(),fmatvec::NONINIT);
        for (unsigned int i=0; i<component.size(); i++)
          y(i)=(*component[i])(x);
        return y;
      }
      typename B::DRetDArg parDer(const Arg &x) override {
        return parDer_<Ret,Arg>(component,x);
      }
      typename B::DRetDArg parDerDirDer(const Arg &xDir, const Arg &x) override {
       return parDerDirDer_<Ret,Arg>(component,xDir,x);
      }
      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"components")->getFirstElementChild();
        while (e) {
          addComponent(ObjectFactory::createAndInit<Function<double(Arg)> >(e));
          e=e->getNextElementSibling();
        }
      }
      void init(Element::InitStage stage, const InitConfigSet &config) override {
        Function<Ret(Arg)>::init(stage, config);
        for(auto it=component.begin(); it!=component.end(); ++it)
          (*it)->init(stage, config);
      }
    private:
      std::vector<Function<double(Arg)> *> component;
  };

}

#endif
