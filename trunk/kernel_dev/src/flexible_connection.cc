/* Copyright (C) 2004-2008  Martin Förg
 
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
#include <config.h>
#include "flexible_connection.h"
#include "coordinate_system.h"

namespace MBSim {

  FlexibleConnection::FlexibleConnection(const string &name) : Connection(name,false), ffl(0), fml(0) {
  }

  void FlexibleConnection::updateh(double t) {
    for(int i=0; i<forceDir.cols(); i++) 
      la(i) = (*ffl)(g(i), gd(i));
    
    for(int i=forceDir.cols(); i<forceDir.cols() + momentDir.cols(); i++)
      la(i) = (*fml)(g(i), gd(i));

    WF[1] = Wf*la(IT);
    WM[1] = Wm*la(IR);
    WF[0] = -WF[1];
    WM[0] = -WM[1]+crossProduct(WrP0P1,WF[0]);
    for(unsigned int i=0; i<port.size(); i++)
      h[i] += trans(port[i]->getJacobianOfTranslation())*WF[i] + trans(port[i]->getJacobianOfRotation())*WM[i];
  }

  double FlexibleConnection::computePotentialEnergy() {
    double V = 0.5* (trans(la)*g);
    return V;
  }

}
