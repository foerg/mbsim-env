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

#ifndef _SIGNAL_MANIPULATION_H_
#define _SIGNAL_MANIPULATION_H_

#include "mbsimControl/signal_.h"

namespace MBSim {
  template <class Ret, class Arg> class Function1;
  template <class Ret, class Arg1, class Arg2> class Function2;
}

namespace MBSimControl {

  /*!
   * \brief SignalAddition
   * \author Markus Schneider
   */
  class SignalAddition : public Signal {
    public:
      SignalAddition(const std::string &name="") : Signal(name) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void addSignal(Signal * signal, double factor=1.);
      fmatvec::Vec getSignal();
    private:
      std::vector<Signal *> signals;
      std::vector<double> factors;
      std::vector<std::string> signalString;
      std::vector<double> factorsTmp;
  };

  /*!
   * \brief SignalOffset
   * \author Markus Schneider
   */
  class SignalOffset : public Signal {
    public:
      SignalOffset(const std::string &name="") : Signal(name), signal(0), offset(0, fmatvec::NONINIT), signalString("") {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void setSignal(Signal * s) {signal=s; }
      void setOffset(fmatvec::Vec o) {offset=o; }
      fmatvec::Vec getSignal();
    private:
      Signal * signal;
      fmatvec::Vec offset;
      std::string signalString;
  };

  /*!
   * \brief SignalMultiplication
   * \author Markus Schneider
   */
  class SignalMultiplication : public Signal {
    public:
      SignalMultiplication(const std::string &name="") : Signal(name) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void addSignal(Signal * signal, double exp);
      fmatvec::Vec getSignal();
    private:
      std::vector<Signal *> signals;
      std::vector<double> exponents;
      std::vector<std::string> signalString;
      std::vector<double> exponentsTmp;
  };


  /*!
   * \brief SignalMux
   * \author Markus Schneider
   */
  class SignalMux : public Signal {  
    public:
      SignalMux(const std::string &name="") : Signal(name) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void addSignal(Signal * signal) {signals.push_back(signal); }
      fmatvec::Vec getSignal();
    private:
      std::vector<Signal *> signals;
      std::vector<std::string> signalString;
  };


  /*!
   * \brief SignalDemux
   * \author Markus Schneider
   */
  class SignalDemux : public Signal {  
    public:
      SignalDemux(const std::string &name="") : Signal(name), totalSignalSize(0) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void addSignal(Signal * signal, fmatvec::VecInt index) {signals.push_back(signal); indizes.push_back(index); }
      fmatvec::Vec getSignal();
    private:
      std::vector<Signal *> signals;
      std::vector<fmatvec::VecInt > indizes;
      std::vector<fmatvec::Vec> indizesTmp;
      std::vector<std::string> signalString;
      int totalSignalSize;
  };


  /*!
   * \brief SignalLimitation
   * \author Markus Schneider
   */
  class SignalLimitation : public Signal {  
    public:
      SignalLimitation(const std::string &name="") : Signal(name), s(NULL), minValue(), maxValue(), signalString("") {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void setMinimalValue(fmatvec::Vec minValue_) {minValue=minValue_; }
      void setMaximalValue(fmatvec::Vec maxValue_) {maxValue=maxValue_; }
      void setSignal(Signal * signal_) {s=signal_; }
      fmatvec::Vec getSignal();
    private:
      Signal * s;
      fmatvec::Vec minValue, maxValue;
      std::string signalString;
  };


  /*!
   * \brief SignalTimeDiscretization
   * \author Markus Schneider
   */
  class SignalTimeDiscretization : public Signal {  
    public:
      SignalTimeDiscretization(const std::string &name="") : Signal(name), s(NULL), y(), tOld(-99e99), signalString("") {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void setSignal(Signal * signal_) {s=signal_; }
      void updateg(double t);
      fmatvec::Vec getSignal();
    private:
      Signal * s;
      fmatvec::Vec y;
      double tOld;
      std::string signalString;
  };


  /*!
   * \brief SignalOperation according <cmath>
   * \author Markus Schneider
   */
  class SignalOperation : public Signal {  
    public:
      SignalOperation(const std::string &name="") : Signal(name), s(NULL), s2(NULL), signalString(""), signal2String(""), op(0), s2values(0, fmatvec::NONINIT) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void setSignal(Signal * signal_) {s=signal_; }
      void setSecondSignal(Signal * signal_) {s2=signal_; }
      void setSecondSignalValues(fmatvec::Vec s2_) {s2values=s2_; }
      void setOperator(unsigned int op_) {op=op_; };
      fmatvec::Vec getSignal();
    private:
      Signal * s;
      Signal * s2;
      std::string signalString;
      std::string signal2String;
      unsigned int op;
      fmatvec::Vec s2values;
  };


  /*!
   * \brief SpecialSignalOperation with advanced functionality
   * \author Markus Schneider
   */
  class SpecialSignalOperation : public Signal {  
    public:
      SpecialSignalOperation(const std::string &name="") : Signal(name), s(NULL), s2(NULL), signalString(""), signal2String(""), op(0), s2values(0, fmatvec::NONINIT) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void setSignal(Signal * signal_) {s=signal_; }
      void setSecondSignal(Signal * signal_) {s2=signal_; }
      void setSecondSignalValues(fmatvec::Vec s2_) {s2values=s2_; }
      void setOperator(unsigned int op_) {op=op_; };
      fmatvec::Vec getSignal();
    private:
      Signal * s;
      Signal * s2;
      std::string signalString;
      std::string signal2String;
      unsigned int op;
      fmatvec::Vec s2values;
  };

  /*!
   * \brief PID controller
   * \author Martin Foerg
   */
  class PIDController : public Signal {

    public:   
      PIDController(const std::string& name="") : Signal(name), s(NULL), sd(NULL) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement * element);
      
      void calcxSize() {xSize=getSignalMethod==&PIDController::getSignalPD?0:1;}
      
      void init(MBSim::InitStage stage);

      void updatedx(double t, double dt);
      void updatexd(double t);
      
      void plot(double t,double dt);
     
      void setPID(double P_, double I_, double D_);
      void setInputSignal(Signal *inputSignal_) {s=inputSignal_; }
      void setDerivativeOfInputSignal(Signal *inputSignal_) {sd=inputSignal_; }

      fmatvec::Vec getSignal();

    protected:
      double P,I,D;
      Signal *s, *sd;
      std::string sString, sdString;
      fmatvec::Vec (PIDController::*getSignalMethod)();
      fmatvec::Vec getSignalPD();
      fmatvec::Vec getSignalPID();
  };

  /*!
   * \brief UnarySignalOperation
   * \author Martin Foerg
   */
  class UnarySignalOperation : public Signal {  
    public:
      UnarySignalOperation(const std::string &name="") : Signal(name), s(NULL), signalString(""), f(0) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void setSignal(Signal *signal_) {s=signal_; }
      void setFunction(MBSim::Function1<fmatvec::Vec,fmatvec::Vec> *f_) {f=f_; };
      fmatvec::Vec getSignal();
    private:
      Signal *s;
      std::string signalString;
      MBSim::Function1<fmatvec::Vec,fmatvec::Vec> *f;
  };

  /*!
   * \brief BinarySignalOperation
   * \author Martin Foerg
   */
  class BinarySignalOperation : public Signal {  
    public:
      BinarySignalOperation(const std::string &name="") : Signal(name), s1(NULL), s2(NULL), signal1String(""), signal2String(""), f(0) {}
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      void init(MBSim::InitStage stage);
      void setSignal1(Signal *signal_) {s1=signal_; }
      void setSignal2(Signal *signal_) {s2=signal_; }
      void setFunction(MBSim::Function2<fmatvec::Vec,fmatvec::Vec,fmatvec::Vec> *f_) {f=f_; };
      fmatvec::Vec getSignal();
    private:
      Signal *s1, *s2;
      std::string signal1String, signal2String;
      MBSim::Function2<fmatvec::Vec,fmatvec::Vec,fmatvec::Vec> *f;
  };

}

#endif /* _SIGNAL_MANIPULATION_H_ */

