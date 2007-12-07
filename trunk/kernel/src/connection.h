/* Copyright (C) 2004-2006  Martin Förg
 
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
 * Contact:
 *   mfoerg@users.berlios.de
 *
 */

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include "link.h"

namespace MBSim {

  /*! \brief Class for connections: Constraints on Ports
   *
   * */
  class Connection: public LinkPort {

    protected:
      Index IT, IR;
      Mat forceDir, momentDir;
      Mat Wf, Wm;
      int KOSYID;
      Vec WrP0P1, WvP0P1, WomP0P1;

    public: 
      Connection(const string &name, bool setValued);

      virtual void connect(Port *port1, Port* port2);

      void calcSize();
      void init();

      void setKOSY(int);
      void setForceDirection(const Mat& fd);
      void setMomentDirection(const Mat& md);
      void updateStage1(double t);
      void updateStage2(double t);
      virtual void updateKinetics(double t) = 0;
      void updatexd(double t);
      void updatedx(double t, double dt);
  };

}

#endif
