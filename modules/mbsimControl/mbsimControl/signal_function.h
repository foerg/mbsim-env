/* Copyright (C) MBSim Development Team
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

#ifndef _SIGNAL_FUNCTION_H_
#define _SIGNAL_FUNCTION_H_

#include "mbsim/functions/function.h"
#include "mbsimControl/signal_.h"
#include "mbsim/element.h"
#include "mbsim/functions/function.h"
#include "mbsim/utils/utils.h"

namespace MBSimControl {

  //! A function which get its return value from a signal
  template<typename Sig>
  class SignalFunction;

  //! A function with one argument which get its return value from a signal
  template<typename Ret, typename Arg>
  class SignalFunction<Ret(Arg)> : public MBSim::Function<Ret(Arg)> {
    public:
      SignalFunction(Signal *ret_=nullptr) : ret(ret_) {}

      void setReturnSignal(Signal *ret_);

      std::pair<int, int> getRetSize() const override { return std::make_pair(ret->getSignalSize(),1); }

      Ret operator()(const Arg& a) override {
        return MBSim::FromVecV<Ret>::cast(ret->evalSignal());
      }

      void init(MBSim::Element::InitStage stage, const MBSim::InitConfigSet &config) override;

      void initializeUsingXML(xercesc::DOMElement *element) override;

      MBSim::Element* getDependency() const override { return ret; }

    protected:
      std::string retString;
      Signal *ret;
  };

  template<typename Ret, typename Arg>
  void SignalFunction<Ret(Arg)>::setReturnSignal(Signal *ret_) {
    ret=ret_;
  }

  template<typename Ret, typename Arg>
  void SignalFunction<Ret(Arg)>::initializeUsingXML(xercesc::DOMElement *element) {
    MBSim::Function<Ret(Arg)>::initializeUsingXML(element);
    xercesc::DOMElement *e;
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMCONTROL%"returnSignal");
    retString=MBXMLUtils::E(e)->getAttribute("ref");
  }

  template<typename Ret, typename Arg>
  void SignalFunction<Ret(Arg)>::init(MBSim::Element::InitStage stage, const MBSim::InitConfigSet &config) {
    if(stage==MBSim::Element::resolveStringRef) {
      if(not retString.empty())
        setReturnSignal(this->template getByPath<Signal>(retString));
      if(not ret)
        MBSim::Element::throwError("Signal is not given!");
      MBSim::Function<Ret(Arg)>::init(stage, config);
    }
    else
      MBSim::Function<Ret(Arg)>::init(stage, config);
  }

  //! A function with two arguments which get its return value from a signal
  template<typename Ret, typename Arg1, typename Arg2>
  class SignalFunction<Ret(Arg1,Arg2)> : public MBSim::Function<Ret(Arg1,Arg2)> {
    public:
      SignalFunction(Signal *ret_=nullptr) : ret(ret_) {}

      void setReturnSignal(Signal *ret_);

      std::pair<int, int> getRetSize() const override { return std::make_pair(ret->getSignalSize(),1); }

      Ret operator()(const Arg1& a1, const Arg2& a2) override {
        return MBSim::FromVecV<Ret>::cast(ret->evalSignal());
      }

      void init(MBSim::Element::InitStage stage, const MBSim::InitConfigSet &config) override;

      void initializeUsingXML(xercesc::DOMElement *element) override;

      MBSim::Element* getDependency() const override { return ret; }

    protected:
      std::string retString;
      Signal *ret;
  };

  template<typename Ret, typename Arg1, typename Arg2>
  void SignalFunction<Ret(Arg1, Arg2)>::setReturnSignal(Signal *ret_) {
    ret=ret_;
  }

  template<typename Ret, typename Arg1, typename Arg2>
  void SignalFunction<Ret(Arg1, Arg2)>::initializeUsingXML(xercesc::DOMElement *element) {
    MBSim::Function<Ret(Arg1,Arg2)>::initializeUsingXML(element);
    xercesc::DOMElement *e;
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMCONTROL%"returnSignal");
    retString=MBXMLUtils::E(e)->getAttribute("ref");
  }

  template<typename Ret, typename Arg1, typename Arg2>
  void SignalFunction<Ret(Arg1, Arg2)>::init(MBSim::Element::InitStage stage, const MBSim::InitConfigSet &config) {
    if(stage==MBSim::Element::resolveStringRef) {
      if(not retString.empty())
        setReturnSignal(this->template getByPath<Signal>(retString));
      if(not ret)
        MBSim::Element::throwError("Signal is not given!");
      MBSim::Function<Ret(Arg1,Arg2)>::init(stage, config);
    }
    else
      MBSim::Function<Ret(Arg1,Arg2)>::init(stage, config);
  }

}

#endif
