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

#ifndef _FUNCTION_SENSOR_H_
#define _FUNCTION_SENSOR_H_

#include "mbsimControl/sensor.h"
#include <fmatvec/function.h>

namespace MBSimControl {

  /*!
   * \brief FunctionSensor
   * \author Markus Schneider
   */
  class FunctionSensor : public Sensor {
    public:
      FunctionSensor(const std::string &name="") : Sensor(name), function(NULL), y() {}
      FunctionSensor(const std::string &name, fmatvec::Function<fmatvec::VecV(double)>* function_);
      ~FunctionSensor() { delete function; }
      std::string getType() const { return "FunctionSensor"; }
      void setFunction(fmatvec::Function<fmatvec::VecV(double)>* function_);
      fmatvec::Vec getSignal() {return y.copy(); }
      void updateg(double t);
      void initializeUsingXML(xercesc::DOMElement *element);
    private:
      fmatvec::Function<fmatvec::VecV(double)> * function;
      fmatvec::Vec y;
  };

  /*!
   * \brief Function_SSEvaluation
   * \author Markus Schneider
   */
  class Function_SSEvaluation : public Signal {
    public:
      Function_SSEvaluation(const std::string &name="") : Signal(name), signal(NULL), fun(NULL), signalString("") {}
      ~Function_SSEvaluation() { delete fun; }
      void initializeUsingXML(xercesc::DOMElement *element);
      void init(MBSim::InitStage stage);
      void setSignal(Signal * s) {signal=s; }
      void setFunction(fmatvec::Function<double(double)>* fun_) {fun=fun_; }
      fmatvec::Vec getSignal();
    private:
      Signal * signal;
      fmatvec::Function<double(double)>* fun;
      std::string signalString;
  };

  /*!
   * \brief Function_SSSEvaluation
   * \author Markus Schneider
   */
  class Function_SSSEvaluation : public Signal {
    public:
      Function_SSSEvaluation(const std::string &name="") : Signal(name), signal1(NULL), signal2(NULL), fun(NULL), signal1String(""), signal2String("") {}
      ~Function_SSSEvaluation() { delete fun; }
      void initializeUsingXML(xercesc::DOMElement *element);
      void init(MBSim::InitStage stage);
      void setSignals(Signal * s1, Signal * s2) {signal1=s1; signal2=s2; }
      void setFunction(fmatvec::Function<double(double,double)>* fun_) {fun=fun_; }
      fmatvec::Vec getSignal();
    private:
      Signal * signal1;
      Signal * signal2;
      fmatvec::Function<double(double,double)>* fun;
      std::string signal1String;
      std::string signal2String;
  };

}

#endif /* _FUNCTION_SENSOR_H_ */

