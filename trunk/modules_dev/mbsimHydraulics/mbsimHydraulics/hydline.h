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
 * Contact: schneidm@users.berlios.de
 */

#ifndef  _HYDLINE_H_
#define  _HYDLINE_H_

#include "object_hydraulics.h"

namespace MBSim {

  class HydNode;
  class PressureLoss;
  class PressureLossVar;
  class HydlineClosedBilateral;
  class HydlineClosedUnilateral;

  class HydLineAbstract : public ObjectHydraulics {
    public:
      HydLineAbstract(const std::string &name);
      ~HydLineAbstract();
      virtual std::string getType() const { return "HydLineAbstract"; }

      void setFromNode(HydNode * nFrom_);
      void setToNode(HydNode * nTo_);
      void setLength(double l_) {l=l_; }
      void setDiameter(double d_) {d=d_; }
      HydNode * getFromNode() { return nFrom; }
      HydNode * getToNode() {return nTo; }
      double getDiameter() {return d; }

      virtual fmatvec::Vec getQIn(double t) = 0;
      virtual fmatvec::Vec getQOut(double t) = 0;
      virtual fmatvec::Vec getInflowFactor() = 0;
      virtual fmatvec::Vec getOutflowFactor() = 0;

      void init(InitStage stage);

    protected:
      HydNode * nFrom;
      HydNode * nTo;
      double d, l;
      double Area, rho;
  };

  class HydLine : public HydLineAbstract {
    public:
      HydLine(const std::string &name) : HydLineAbstract(name), pLossSum(0), pdVar(NULL) {}
      ~HydLine() {};
      virtual std::string getType() const { return "HydLine"; }

      void addPressureLoss(PressureLoss * dp);

      virtual fmatvec::Vec getQIn(double t) {return u; }
      virtual fmatvec::Vec getQOut(double t) {return -u; }
      virtual fmatvec::Vec getInflowFactor() {return fmatvec::Vec(1, fmatvec::INIT, -1.); }
      virtual fmatvec::Vec getOutflowFactor() {return fmatvec::Vec(1, fmatvec::INIT, 1.); }
      PressureLossVar * getPressureLossVar() {return pdVar; }

      void init(InitStage stage);
      void calcqSize() {qSize=0; }
      void calcuSize(int j) {uSize[j]=1; }

      void updateh(double t);
      void updateM(double t) {M(0,0)=MFac; }

      void plot(double t, double dt);

    private:
      double MFac, pLossSum;

    protected:
      std::vector<PressureLoss*> pd;
      PressureLossVar * pdVar;
  };

  class HydLineValve : public HydLine {
    public:
      HydLineValve(const std::string &name) : HydLine(name) {};
      virtual std::string getType() const { return "HydLineValve"; }
  };

  class HydLineValveBilateral : public HydLineValve {
    public:
      HydLineValveBilateral(const std::string &name) : HydLineValve(name) {}
      virtual std::string getType() const { return "HydLineValveBilateral"; }

      void init(InitStage stage);

    private:
      HydlineClosedBilateral * closed;
  };

  class HydLineCheckvalveUnilateral : public HydLineValve {
    public:
      HydLineCheckvalveUnilateral(const std::string &name) : HydLineValve(name) {}
      virtual std::string getType() const { return "HydLineCheckvalveUnilateral"; }

      void init(InitStage stage);

    private:
      HydlineClosedUnilateral * closed;
  };

}

#endif   /* ----- #ifndef _HYDLINE_H_  ----- */

