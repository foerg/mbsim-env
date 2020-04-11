/* Copyright (C) 2004-2012 MBSim Development Team
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
 * Contact: jan.p.clauberg@googlemail.com
 */

#include <config.h>

#include "mbsim/integrators/theta_time_stepping_ssc_integrator.h"
#include "mbsim/dynamic_system_solver.h"
#include "mbsim/utils/eps.h"

#include <cmath>
#include <time.h>

#ifndef NO_ISO_14882
using namespace std;
#endif

using namespace fmatvec;

namespace MBSim {

  ThetaTimeSteppingSSCIntegrator::ThetaTimeSteppingSSCIntegrator() : dt(1e-3), theta(0.5), t(0.), tPlot(0.), iter(0), step(0), integrationSteps(0), maxIter(0), sumIter(0), s0(0.), time(0.), stepPlot(0), driftCompensation(false) {}

  void ThetaTimeSteppingSSCIntegrator::update(const Vec& z, double t) {
    if(system->getq()()!=z()) system->updatezRef(z);

    system->updateStateDependentVariables(t);
    system->updateg();
    system->checkActive(1);
    system->setUpActiveLinks();
    if(system->gActiveChanged()) {
      // system->checkAllgd(); // TODO necessary?
      system->calcgdSize(3); // IH
      system->calclaSize(3); // IH
      system->calcrFactorSize(3); // IH

      system->updateWRef(system->getWParent()(RangeV(0,system->getuSize()-1),Index(0,system->getlaSize()-1)));
      system->updateVRef(system->getVParent()(RangeV(0,system->getuSize()-1),Index(0,system->getlaSize()-1)));
      system->updatelaRef(system->getlaParent()(0,system->getlaSize()-1));
      system->updategdRef(system->getgdParent()(0,system->getgdSize()-1));
      if(system->getImpactSolver() == RootFinding) system->updateresRef(system->getresParent()(0,system->getlaSize()-1));
      system->updaterFactorRef(system->getrFactorParent()(0,system->getrFactorSize()-1));
    }
    system->updategd();

    system->updateT();
    system->updateJacobians(t);
    system->updateM();
    system->updateW();
    system->updateV();
  }
  
  void ThetaTimeSteppingSSCIntegrator::preIntegrate() {
    debugInit();
    // initialisation
    assert(dtPlot >= dt);

    t = tStart;

    int nq = system->getqSize();
    int nu = system->getuSize();
    int nx = system->getxSize();
    int n = nq + nu + nx;

    Index Iq(0,nq-1);
    Index Iu(nq,nq+nu-1);
    Index Ix(nq+nu,n-1);
    z.resize(n);
    q>>z(Iq);
    u>>z(Iu);
    x>>z(Ix);

    if(z0.size()) {
      if(z0.size() != zSize)
        throwError("(ThetaTimeSteppingSSCIntegrator::integrate): size of z0 does not match");
      z = z0;
    }
    else
      z = system->evalz0();

    integPlot.open(name + ".plt");
    
    stepPlot = (int)(dtPlot/dt + 0.5);
    assert(fabs(stepPlot*dt - dtPlot) < dt*dt);

    s0 = clock();
  }

  void ThetaTimeSteppingSSCIntegrator::subIntegrate(double tStop) {
    while(t<=tStop) { // time loop
      integrationSteps++;
      if((step*stepPlot - integrationSteps) < 0) {
        step++;
        if(driftCompensation) system->projectGeneralizedPositions(t,0);
        system->plot2(z,t,dt);
        double s1 = clock();
        time += (s1-s0)/CLOCKS_PER_SEC;
        s0 = s1; 
        integPlot << t << " " << dt << " " <<  iter << " " << time << " "<<system->getlaSize() << endl;
        if(msgAct(Status)) msg(Status) << "   t = " <<  t << ",\tdt = "<< dt << ",\titer = "<< setw(5) << setiosflags(ios::left) << iter << flush;
        tPlot += dtPlot;
      }

      double te = t + dt;
      t += theta*dt;
      update(system,z,t);

      Mat T = system->getT();
      SymMat M = system->getM();
      Vec h = system->geth();
      Mat W = system->getW();
      Mat V = system->getV();
      Mat dhdq = system->dhdq(t);
      Mat dhdu = system->dhdu(t);

      Vector<int> ipiv(M.size());
      SqrMat luMeff = SqrMat(facLU(M - theta*dt*dhdu - theta*theta*dt*dt*dhdq*T,ipiv));
      Vec heff = h+theta*dhdq*T*u*dt;
      system->getG().resize() = SqrMat(W.T()*slvLUFac(luMeff,V,ipiv));
      system->getGs().resize() << system->getG();
      system->getb().resize() = system->getgd() + W.T()*slvLUFac(luMeff,heff,ipiv)*dt; // TODO system->getgd() necessary?

      iter = system->solveImpacts(dt);
      if(iter>maxIter) maxIter = iter;
      sumIter += iter;

      Vec du = slvLUFac(luMeff,heff*dt + V*system->getla(),ipiv);
      
      q += T*(u+theta*du)*dt;
      u += du;
      x += system->deltax(z,t,dt);
      t = te;
    }
  }

  void ThetaTimeSteppingSSCIntegrator::postIntegrate() {
    integPlot.close();

    ofstream integSum(name + ".sum");
    integSum << "Integration time: " << time << endl;
    integSum << "Integration steps: " << integrationSteps << endl;
    integSum << "Maximum number of iterations: " << maxIter << endl;
    integSum << "Average number of iterations: " << double(sumIter)/integrationSteps << endl;
    integSum.close();
  }

  void ThetaTimeSteppingSSCIntegrator::integrate() {
    preIntegrate();
    subIntegrate(tEnd);
    postIntegrate();
  }

  void ThetaTimeSteppingSSCIntegrator::initializeUsingXML(xercesc::DOMElement *element) {
    //Integrator::initializeUsingXML(element);
    //TiXmlElement *e;
    //e=element->FirstChildElement(MBSIMNS"stepSize");
    //setStepSize(E(e)->getText<double>());
    //e=element->FirstChildElement(MBSIMNS"theta");
    //const double theta=E(e)->getText<double>();
    //assert(theta>=0);
    //assert(theta<=1);
    //setTheta(theta);
    //e=element->FirstChildElement(MBSIMNS"driftCompensation");
    //setDriftCompensation(E(e)->getText<bool>());
  }

}
