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

#ifndef  _CONTROLVALVE43_H_
#define  _CONTROLVALVE43_H_

#include "mbsim/group.h"
#include "mbsim/utils/function.h"

namespace MBSim {

  class ClosableRigidLine;
  class RigidLine;
  class Signal;
  class LinePressureLoss;

  /*! Controlvalve43 */
  class Controlvalve43 : public Group {
    public:
      Controlvalve43(const std::string& name);

      void setFrameOfReference(Frame * ref);
      void setLength(double l_);
      void setDiameter(double d_);
      void setAlpha(double alpha_);
      void setPARelativeAreaFunction(Function1<double, double> * relAreaPA_) {relAreaPA=relAreaPA_; } 
      void setMinimalRelativeArea(double minRelArea_);
      void setOffset(double off) {offset=off; }
      void setRelativePositionSignal(Signal * s) {position = s; }
      void setLinePDirection(fmatvec::Vec dir);
      void setLinePLength(double l);
      void setLinePDiameter(double d);
      void setLinePPressureLoss(LinePressureLoss * lpl);
      void setLineADirection(fmatvec::Vec dir);
      void setLineALength(double l);
      void setLineADiameter(double d);
      void setLineAPressureLoss(LinePressureLoss * lpl);
      void setLineBDirection(fmatvec::Vec dir);
      void setLineBLength(double l);
      void setLineBDiameter(double d);
      void setLineBPressureLoss(LinePressureLoss * lpl);
      void setLineTDirection(fmatvec::Vec dir);
      void setLineTLength(double l);
      void setLineTDiameter(double d);
      void setLineTPressureLoss(LinePressureLoss * lpl);
      void setSetValued(bool setValued=true);

      RigidLine * getLineP() {return lP; }
      RigidLine * getLineA() {return lA; }
      RigidLine * getLineB() {return lB; }
      RigidLine * getLineT() {return lT; }

      void init(InitStage stage);
      void initializeUsingXML(TiXmlElement * element);
    protected:
      ClosableRigidLine * lPA, * lPB, * lAT, * lBT;
      RigidLine * lP, * lA, * lB, *lT;
      double offset;
      Function1<double, double> * relAreaPA;
      Signal * position;
      Signal * checkSizeSignalPA, * checkSizeSignalPB, * checkSizeSignalAT, * checkSizeSignalBT;
    private:
      std::string refFrameString, positionString;
  };
}

#endif   /* ----- #ifndef _CONTROLVALVE43_H_  ----- */

