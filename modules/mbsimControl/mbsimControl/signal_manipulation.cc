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

#include <config.h>
#include "mbsimControl/signal_manipulation.h"
#include "mbsim/utils/eps.h"
#include "mbsim/utils/index.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimControl {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMCONTROL, SignalTimeDiscretization)

  void SignalTimeDiscretization::initializeUsingXML(DOMElement *element) {
    Signal::initializeUsingXML(element);
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"inputSignal");
    signalString=E(e)->getAttribute("ref");
  }

  void SignalTimeDiscretization::init(InitStage stage) {
    if (stage==resolveXMLPath) {
      if (signalString!="")
        setInputSignal(getByPath<Signal>(signalString));
      Signal::init(stage);
    }
    else
      Signal::init(stage);
  }

  void SignalTimeDiscretization::updateSignal() {
    if (fabs(tOld-getTime())>epsroot()) {
      Signal::s=s->evalSignal();
      tOld=getTime();
    }
    upds = false;
  }

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMCONTROL, PIDController)

  void PIDController::initializeUsingXML(DOMElement * element) {
    Signal::initializeUsingXML(element);
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"inputSignal");
    sString=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"derivativeOfInputSignal");
    sdString=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"P");
    double p=Element::getDouble(e);
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"I");
    double i=Element::getDouble(e);
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"D");
    double d=Element::getDouble(e);
    setPID(p, i, d);
  }

  void PIDController::updatedx() {
    if(xSize) dx=s->evalSignal()*getStepSize();
  }

  void PIDController::updatexd() {
    if(xSize) xd=s->evalSignal();
  }

  void PIDController::init(InitStage stage) {
    if (stage==resolveXMLPath) {
      if (sString!="")
        setInputSignal(getByPath<Signal>(sString));
      if (sdString!="")
        setDerivativeOfInputSignal(getByPath<Signal>(sdString));
      Signal::init(stage);
    }
    else if (stage==unknownStage) {
      Signal::init(stage);
      x.resize(xSize, INIT, 0);
    }
    else
      Signal::init(stage);
  }

  void PIDController::updateSignal() {
    (this->*updateSignalMethod)();
    upds = false;
  }

  void PIDController::updateSignalPID() {
    Signal::s = P*s->evalSignal() + D*sd->evalSignal() + I*x;
  }

  void PIDController::updateSignalPD() {
    Signal::s = P*s->evalSignal() + D*sd->evalSignal();
  }

  void PIDController::setPID(double PP, double II, double DD) {
    if ((fabs(II)<epsroot()))
      updateSignalMethod=&PIDController::updateSignalPD;
    else
      updateSignalMethod=&PIDController::updateSignalPID;
    P = PP; I = II; D = DD;
  }

}
