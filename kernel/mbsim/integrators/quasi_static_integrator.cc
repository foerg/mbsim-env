/* Copyright (C) 2004-2014 MBSim Development Team
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

#include<config.h>
#include<mbsim/dynamic_system_solver.h>
#include "quasi_static_integrator.h"
#include <mbsim/utils/nonlinear_algebra.h>
#include <mbsim/numerics/nonlinear_algebra/multi_dimensional_newton_method.h>
#include <ctime>

#ifndef NO_ISO_14882
using namespace std;
#endif

using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, QuasiStaticIntegrator)

  QuasiStaticIntegrator::QuasiStaticIntegrator()  {
  }

  void QuasiStaticIntegrator::preIntegrate() {
    debugInit();
    // initialisation
    assert(dtPlot >= dt);

    t = tStart;

    int nq = system->getqSize(); // size of positions, velocities, state
    int nu = system->getuSize();
    int nx = system->getxSize();
    int n = nq + nu + nx;

    RangeV Iq(0, nq - 1);
    RangeV Iu(nq, nq + nu - 1);
    RangeV Ix(nq + nu, n - 1);
    z.resize(n);
    q &= z(Iq);
    u &= z(Iu);
    x &= z(Ix);

    if(z0.size()) {
      if(z0.size() != system->getzSize())
        throwError("(QuasiStaticIntegrator::integrate): size of z0 does not match, must be " + to_string(system->getzSize()));
      z = z0;
    }
    else
      z = system->evalz0();

    // Perform a projection of generalized positions and velocities at time t=0
    if(system->getInitialProjection()) {
      system->projectGeneralizedPositions(2,true);
      system->projectGeneralizedVelocities(2);
    }

    stepPlot = (int) (dtPlot / dt + 0.5);
    if (fabs(stepPlot * dt - dtPlot) > dt * dt) {
      msg(Warn) << "Due to the plot-Step settings it is not possible to plot exactly at the correct times." << endl;
    }

    s0 = clock();
  }

  void QuasiStaticIntegrator::subIntegrate(double tStop) {
//    static_cast<DynamicSystem&>(system).plot();

    /* FIND EQUILIBRIUM*/
    hgFun fun_hg(system);

    VecV qla(q.size() + system->getla(false).size());
    VecV la(system->getla(false).size());
    RangeV qInd = RangeV(0, q.size() - 1);
    RangeV laInd = RangeV(q.size(), qla.size() - 1);

    VecV qlaStart(qla.size());
    VecV qlaOld(qla.size());
    VecV qlaOldOld(qla.size());

    qla.set(qInd, q);
    qla.set(laInd, system->getla(false));

    /* use MultiDimNewtonMethod*/
//    MultiDimNewtonMethod newton(&fun_hg);
//    newton.setLinearAlgebra(1);
//    newton.setTolerance(gTol);
    /* use  MultiDimensionalNewtonMethod */
    NewtonJacobianFunction * jac = new NumericalNewtonJacobianFunction();
    MultiDimensionalNewtonMethod newton;
    map<RangeV, double> tolerances;
    tolerances.insert(pair<RangeV, double>(qInd, hTol));
    tolerances.insert(pair<RangeV, double>(laInd, gTol));
    LocalResidualCriteriaFunction cfunc(tolerances);
    GlobalResidualCriteriaFunction cfuncGlob(gTol);
    StandardDampingFunction dfunc(30, 0.2);
    newton.setFunction(&fun_hg);
    newton.setJacobianFunction(jac);
    newton.setCriteriaFunction(&cfunc);
    newton.setDampingFunction(&dfunc);
    newton.setLinearAlgebra(1); // as system is possible underdetermined
    newton.setJacobianUpdateFreq(updateJacobianEvery);

//    system->setUpdatela(false);
//    static_cast<DynamicSystem&>(system).plot();

    while (t < tStop) { // time loop
      integrationSteps++;

      fun_hg.setT(t);

      qlaStart = qla;
      if (integrationSteps > 1) {
        if (maxExtraPolate == 1 and integrationSteps > extraPolateAfter) {
          qlaStart = 2. * qla - qlaOld;
        }
        else if (maxExtraPolate == 2) {
          // TODO: right now maximal second order extrapolation is supported
          qlaStart = 3. * qla - 3. * qlaOld + qlaOldOld;
        }
        qlaOldOld = qlaOld;
        qlaOld = qla;
      }

      qla = newton.solve(qlaStart); // use the qla from the previous timestep as the initial guess.

//      qla = newton.solve(qla);

      if (newton.getInfo() != 0)
        throwError("ERROR (QuasiStaticIntegrator::subIntegrate): No convergence of Newton method for the new time step");
      iter = newton.getNumberOfIterations();

      for (int i = 0; i < q.size(); i++)
        q(i) = qla(i);

      for (int i = 0; i < la.size(); i++)
        la(i) = qla(q.size() + i);

      system->setla(la);

      // todo: check whether the plot make the openMBV unstable.
      if ((step * stepPlot - integrationSteps) < 0) {
        /* WRITE OUTPUT */
        step++;
        system->setUpdatela(false);
        system->plot();
        double s1 = clock();
        time += (s1 - s0) / CLOCKS_PER_SEC;
        s0 = s1;
        if (msgAct(Status))
          msg(Status) << "   t = " << t << ",\tdt = " << dt << ",\titer = " << setw(5) << setiosflags(ios::left) << iter << flush;
        tPlot += dtPlot;
      }

      /* UPDATE SYSTEM FOR NEXT STEP*/
      t += dt;   // step 0: update time, go into new time step.

//      x += system->deltax(z, t, dt);  // todo: framework is not ready for x.
    }
    delete jac;
  }

  void QuasiStaticIntegrator::postIntegrate() {
    msg(Info) << endl << endl << "******************************" << endl;
    msg(Info) << "INTEGRATION SUMMARY: " << endl;
    msg(Info) << "End time [s]: " << tEnd << endl;
    msg(Info) << "Integration time [s]: " << time << endl;
    msg(Info) << "Integration steps: " << integrationSteps << endl;
    msg(Info) << "Maximum number of iterations: " << maxIter << endl;
    msg(Info) << "Average number of iterations: " << double(sumIter) / integrationSteps << endl;
    msg(Info) << "******************************" << endl;
    msg(Info).flush();
  }

  void QuasiStaticIntegrator::integrate() {
    preIntegrate();
    subIntegrate(tEnd);
    postIntegrate();
  }

  void QuasiStaticIntegrator::initializeUsingXML(DOMElement *element) {
    Integrator::initializeUsingXML(element);
    DOMElement *e;
    e = E(element)->getFirstElementChildNamed(MBSIM % "stepSize");
    setStepSize(E(e)->getText<double>());
  }

  fmatvec::Vec hgFun::operator()(const fmatvec::Vec& qla) {
    int qSize = sys->getqSize();
    int laSize = sys->getlaSize();
    int qlaSize = qSize + laSize;

    // set the q of system to be the input q
    sys->setq(qla(RangeV(0, qSize - 1)));
    sys->setla(qla(RangeV(qSize, qlaSize - 1)));

    // get the new h vector
    Vec hg(qla.size());

    // need new h, W, g, input la (not from the system, but from the input value),
    sys->setTime(t);
    sys->resetUpToDate();
    hg(RangeV(0, qSize - 1)) = sys->evalh() + sys->evalW() * qla(RangeV(qSize, qlaSize - 1));
    hg(RangeV(qSize, qlaSize - 1)) = sys->evalg();

//    msg(Info) << "t = "  << t << "\n";
//    msg(Info) << "sys.geth() = "  <<  sys->geth().T() << "\n";
//    msg(Info) << "la= " << sys->getla().T() << endl;
//    msg(Info) << "W= " << sys->getW().T() << endl;
//    msg(Info) << "w * la = "  << Vec(sys->getW() * sys->getla()).T()  << "\n \n";
//    msg(Info) << "hg = "  << hg.T()  << "\n \n";

    return hg;
  }

  fmatvec::SqrMat jacFun::operator()(const fmatvec::Vec& q) {
    // backup the original q of the dynamical system sys.
    Vec qOld;
    qOld = sys->getq();

    // set the q of system to be the input q
    sys->setq(q);

    SqrMat jac;
    jac = sys->dhdq(t);

    // recover the old sys state
    sys->setq(qOld);
//    sys->update(z, t);

    return jac;
  }
}

