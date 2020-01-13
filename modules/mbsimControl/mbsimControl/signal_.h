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

#ifndef _SIGNALMBSIM_H_
#define _SIGNALMBSIM_H_

#include "mbsim/links/link.h"
#include "mbsimControl/namespace.h"

namespace MBSim {
  class DynamicSystem;
}

namespace MBSimControl {

  extern const MBSim::PlotFeatureEnum signal;

  /*!
   * \brief Signal
   * \author Markus Schneider
   */
  class Signal : public MBSim::Link {

    public:
      Signal(const std::string &name) : Link(name), upds(true) { }

      void init(InitStage stage, const MBSim::InitConfigSet &config) override;

      void updateg() override { }
      void updategd() override { }
      virtual void updateSignal() { }

      void updateWRef(fmatvec::Mat& ref, int i=0) override { }
      void updateVRef(fmatvec::Mat& ref, int i=0) override { }
      void updatehRef(fmatvec::Vec &hRef, int i=0) override { }
      virtual void updatedhdqRef(fmatvec::Mat& ref, int i=0) { }
      virtual void updatedhduRef(fmatvec::SqrMat& ref, int i=0) { }
      virtual void updatedhdtRef(fmatvec::Vec& ref, int i=0) { }
      void updaterRef(fmatvec::Vec &ref, int i=0) override { }
      bool isActive() const override { return false; }
      bool gActiveChanged() override { return false; }
      bool isSingleValued() const override { return true; }

      void plot() override;
      
      const fmatvec::VecV& getSignal(bool check=true) { assert((not check) or (not upds)); return s; }
      const fmatvec::VecV& evalSignal() { if(upds) updateSignal(); return s; }
      virtual int getSignalSize() const { return 0; }

      void resetUpToDate() override { upds = true; }

    protected:
      fmatvec::VecV s;
      bool upds;
  };

}

#endif /* _SIGNALMBSIM_H_ */
