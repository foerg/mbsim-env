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
#include "euler_explicit_integrator.h"
#include <time.h>
#include <boost/thread/thread.hpp>

#ifndef NO_ISO_14882
using namespace std;
#endif

using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Integrator, EulerExplicitIntegrator, MBSIMINT%"EulerExplicitIntegrator")

  EulerExplicitIntegrator::EulerExplicitIntegrator() : dt(1e-3) {
  }

  void EulerExplicitIntegrator::preIntegrate(DynamicSystemSolver& system) {
    assert(dtPlot >= dt);

    t = tStart;

    int nq = system.getqSize();
    int nu = system.getuSize();
    int nx = system.getxSize();
    int n = nq + nu + nx;

    Index Iq(0,nq-1);
    Index Iu(nq,nq+nu-1);
    Index Ix(nq+nu,n-1);
    z.resize(n);
    q>>z(Iq);
    u>>z(Iu);
    x>>z(Ix);

    if(z0.size()) z = z0; // define initial state
    else system.initz(z);

    tPlot = 0.;
    integPlot.open((name + ".plt").c_str());
    cout.setf(ios::scientific, ios::floatfield);
    
    stepPlot =(int) (dtPlot/dt + 0.5);
    assert(fabs(stepPlot*dt - dtPlot) < dt*dt);
    
    
    step = 0;
    integrationSteps = 0;
    
    s0 = clock();
    time = 0;

  }

  void EulerExplicitIntegrator::subIntegrate(DynamicSystemSolver& system, double tStop) { 
    while(t<tStop) { // time loop
      integrationSteps++;
      if((step*stepPlot - integrationSteps) < 0) {
        boost::this_thread::interruption_point();
        step++;
        
        system.plot(z,t);
        double s1 = clock();
        time += (s1-s0)/CLOCKS_PER_SEC;
        s0 = s1;
        integPlot<< t << " " << dt << " " << time << endl;
        if(output) cout << "   t = " <<  t << ",\tdt = "<< dt << "\r"<<flush;
        tPlot += dtPlot;
      }

      z += system.zdot(z,t)*dt; 
      
      t += dt;
    }
  }

  void EulerExplicitIntegrator::postIntegrate(DynamicSystemSolver& system) {
    integPlot.close();

    ofstream integSum((name + ".sum").c_str());
    integSum << "Integration time: " << time << endl;
    integSum << "Integration steps: " << integrationSteps << endl;
    integSum.close();

    cout.unsetf(ios::scientific);
    cout << endl;
  }

  void EulerExplicitIntegrator::integrate(DynamicSystemSolver& system) {
    debugInit();
    preIntegrate(system);
    subIntegrate(system, tEnd);
    postIntegrate(system);
  }

  void EulerExplicitIntegrator::initializeUsingXML(DOMElement *element) {
    Integrator::initializeUsingXML(element);
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIMINT%"stepSize");
    setStepSize(Element::getDouble(e));
  }

}
