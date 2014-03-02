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
#include "mbsimControl/defines.h"
#include "mbsimControl/signal_.h"
#include "mbsimControl/extern_signal_source.h"
#include "mbsim/utils/utils.h"
#include "mbsim/dynamic_system.h"
#include "mbsimControl/defines.h"
#include "mbsim/xmlnamespacemapping.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimControl {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Element, ExternSignalSource, MBSIMCONTROLNS"ExternSignalSource")

  void Signal::init(InitStage stage) {
    if (stage==MBSim::plot) {
      updatePlotFeatures();
      if(getPlotFeature(plotRecursive)==enabled) {
        if(getPlotFeature(state)==enabled) {
          Vec y=getSignal();
          for (int i=0; i<y.size(); i++)
            plotColumns.push_back("Signal (" + numtostr(i) + ")");
        }
      }
      Link::init(stage);
    }
    else
      Link::init(stage);
  }


  void Signal::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      if(getPlotFeature(state)==enabled) {
        Vec y=getSignal();
        for (int i=0; i<y.size(); i++)
          plotVector.push_back(y(i));
      }
    }
    Link::plot(t, dt);
  }

}

// register namspace mapping (must just be done in any *.cc file)
MBSIM_REGISTER_XMLNAMESPACEMAPPING(50, MBSIMCONTROLNS_, "")
MBSIM_REGISTER_XMLNAMESPACEMAPPING(40, MBSIMCONTROLNS_, "ctl")
MBSIM_REGISTER_XMLNAMESPACEMAPPING(30, MBSIMCONTROLNS_, "control")
MBSIM_REGISTER_XMLNAMESPACEMAPPING(20, MBSIMCONTROLNS_, "mbsimcontrol")
