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
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include <mbsim/dynamic_system_solver.h>
#include "explicit_euler_integrator.h"
#include <ctime>

#ifndef NO_ISO_14882
using namespace std;
#endif

using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, ExplicitEulerIntegrator)

  void ExplicitEulerIntegrator::preIntegrate() {
    debugInit();
    assert(dtPlot >= dt);

    system->setTime(tStart);

    if(z0.size()) {
      if(z0.size() != system->getzSize()+system->getisSize())
        throwError("(ExplicitEulerIntegrator::integrate): size of z0 does not match, must be " + to_string(system->getzSize()+system->getisSize()));
      system->setState(z0(RangeV(0,system->getzSize()-1)));
      system->setInternalState(z0(RangeV(system->getzSize(),z0.size()-1)));
    }
    else
      system->evalz0();

    // Perform a projection of generalized positions and velocities at time t=0
    if(system->getInitialProjection()) {
      system->projectGeneralizedPositions(2,true);
      system->projectGeneralizedVelocities(2);
    }

    tPlot = 0.;
    
    stepPlot =(int) (dtPlot/dt + 0.5);
    assert(fabs(stepPlot*dt - dtPlot) < dt*dt);
    
    step = 0;
    integrationSteps = 0;
    
    s0 = clock();
    time = 0;
  }

  void ExplicitEulerIntegrator::subIntegrate(double tStop) {
    while(system->getTime()<tStop) { // time loop
      integrationSteps++;
      if((step*stepPlot - integrationSteps) < 0) {
        step++;
        system->resetUpToDate();
        system->plot();
        double s1 = clock();
        time += (s1-s0)/CLOCKS_PER_SEC;
        s0 = s1;
        if(msgAct(Status)) msg(Status) << "   t = " << system->getTime() << ",\tdt = "<< dt << flush;
        tPlot += dtPlot;
      }

      system->resetUpToDate();
      system->getState() += system->evalzd()*dt;
      system->getTime() += dt;

      system->updateInternalState();
    }
  }

  void ExplicitEulerIntegrator::postIntegrate() {
  }

  void ExplicitEulerIntegrator::integrate() {
    preIntegrate();
    subIntegrate(tEnd);
    postIntegrate();
  }

  void ExplicitEulerIntegrator::initializeUsingXML(DOMElement *element) {
    Integrator::initializeUsingXML(element);
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIM%"stepSize");
    if(e) setStepSize(E(e)->getText<double>());
  }

}
