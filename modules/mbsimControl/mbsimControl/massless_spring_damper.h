/* Copyright (C) 2011 Markus Schneider

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
 *
 * Contact:
 *   schneidm@amm.mw.tu-muenchen.de
 *
 */ 

#ifndef _MASSLESS_SPRING_DAMPER_
#define _MASSLESS_SPRING_DAMPER_

#include "mbsimControl/signal_.h"

namespace MBSimControl {

  /*!
   * \brief Massless Spring Damper (PT1)
   * \author Markus Schneider
   */
  class MasslessSpringDamper : public Signal {

    public:
      MasslessSpringDamper(const std::string& name="");
      void initializeUsingXML(xercesc::DOMElement * element) override;

      void calcxSize() override {xSize=1; }

      void init(InitStage stage, const MBSim::InitConfigSet &config) override;

      void updateSignal() override { s = x; upds = false; }

      void updatexd() override;

      void plot() override;

      void setSpringStiffness(double c_) {c=c_; }
      void setBasicSpringForce(double F0_) {F0=F0_; }
      void setDampingCoefficient(double d_) {dPos=d_; }
      void setNegativeDampingCoefficient(double d_) {dNeg=d_; }
      void setFrictionForce(double FFric_) {FFricPos=FFric_; }
      void setNegativeFrictionForce(double FFric_) {FFricNeg=FFric_; }
      void setMinimumPositionValue(double xMin_) {xMin=xMin_; }
      void setMaximumPositionValue(double xMax_) {xMax=xMax_; }

      void setInputSignal(Signal * inputSignal_) { inputSignal=inputSignal_; }
      int getSignalSize() const override { return inputSignal->getSignalSize(); }

    private:
      double c, F0, dPos, dNeg, FFricPos, FFricNeg, xMin, xMax;
      double xdLocal;
      Signal * inputSignal;
      std::string inputSignalString;
  };
}

#endif
