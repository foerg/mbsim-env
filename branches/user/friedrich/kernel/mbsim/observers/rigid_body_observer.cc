/* Copyright (C) 2004-2011 MBSim Development Team
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
 * Contact: martin.o.foerg@gmail.com
 */

#include <config.h>
#include "mbsim/observers/rigid_body_observer.h"
#include "mbsim/rigid_body.h"
#include "mbsim/frame.h"
#include "mbsim/environment.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/utils/eps.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/frame.h>
#endif

using namespace std;
using namespace fmatvec;

namespace MBSim {

  RigidBodyObserver::RigidBodyObserver(const std::string &name) : Observer(name) {
#ifdef HAVE_OPENMBVCPPINTERFACE
    openMBVAxisOfRotation=0;
#endif
  }

  void RigidBodyObserver::init(InitStage stage) {
    if(stage==MBSim::plot) {
      updatePlotFeatures();

      Observer::init(stage);
      if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled) {
          if(openMBVAxisOfRotation) {
            openMBVAxisOfRotation->setName(name+"_AxisOfRotation");
            getOpenMBVGrp()->addObject(openMBVAxisOfRotation);
          }
        }
#endif
      }
    }
    else
      Observer::init(stage);
  }

  void RigidBodyObserver::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      Vec3 r = body->getFrame("C")->getPosition();
      Vec3 om = body->getFrame("C")->getAngularVelocity();
      Vec3 a;
      double nrmom = nrm2(om);
      if(nrmom > epsroot())
        a = om/nrmom;
      if(getPlotFeature(openMBV)==enabled) {
        if(openMBVAxisOfRotation) {
          vector<double> data;
          data.push_back(t);
          data.push_back(r(0));
          data.push_back(r(1));
          data.push_back(r(2));
          data.push_back(a(0));
          data.push_back(a(1));
          data.push_back(a(2));
          data.push_back(0.5);
          openMBVAxisOfRotation->append(data);
        }
      }
      Observer::plot(t,dt);
    }
  }
}
