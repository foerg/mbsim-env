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

#include <time.h>
#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>

#ifndef NO_ISO_14882
using namespace std;
#endif

namespace bio = boost::iostreams;
using bio::tee_device;
using bio::stream;

using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimIntegrator {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(QuasiStaticIntegrator, MBSIMINT % "QuasiStaticIntegrator")

  QuasiStaticIntegrator::QuasiStaticIntegrator() :
      dt(1e-3), t(0.), tPlot(0.), gTol(1e-10), hTol(1e-10), iter(0), step(0), integrationSteps(0), maxIter(0), sumIter(0), maxExtraPolate(0), extraPolateAfter(2), updateJacobianEvery(1), s0(0.), time(0.), stepPlot(0) {
  }

  void QuasiStaticIntegrator::preIntegrate(DynamicSystemSolver& system) {
    // initialisation
    assert(dtPlot >= dt);

    t = tStart;

    int nq = system.getqSize(); // size of positions, velocities, state
    int nu = system.getuSize();
    int nx = system.getxSize();
    int n = nq + nu + nx;

    Index Iq(0, nq - 1);
    Index Iu(nq, nq + nu - 1);
    Index Ix(nq + nu, n - 1);
    z.resize(n);
    q >> z(Iq);
    u >> z(Iu);
    x >> z(Ix);

    if (z0.size())
      z = z0; // define initial state
    else
      system.initz(z);

    integPlot.open((name + ".plt").c_str());
    cout.setf(ios::scientific, ios::floatfield);

    stepPlot = (int) (dtPlot / dt + 0.5);
    if (fabs(stepPlot * dt - dtPlot) > dt * dt) {
      cout << "WARNING: Due to the plot-Step settings it is not possible to plot exactly at the correct times." << endl;
    }

    s0 = clock();
  }

  void QuasiStaticIntegrator::subIntegrate(DynamicSystemSolver& system, double tStop) {
//    static_cast<DynamicSystem&>(system).plot(t, dt);

    /* FIND EQUILIBRIUM*/
    hgFun fun_hg(&system);

    VecV qla(q.size() + system.getla().size());
    VecV la(system.getla().size());
    Index qInd = Index(0, q.size() - 1);
    Index laInd = Index(q.size(), qla.size() - 1);

    VecV qlaStart(qla.size());
    VecV qlaOld(qla.size());
    VecV qlaOldOld(qla.size());

    qla.set(qInd, q);
    qla.set(laInd, system.getla());

    /* use MultiDimNewtonMethod*/
//    MultiDimNewtonMethod newton(&fun_hg);
//    newton.setLinearAlgebra(1);
//    newton.setTolerance(gTol);
    /* use  MultiDimensionalNewtonMethod */
    NewtonJacobianFunction * jac = new NumericalNewtonJacobianFunction();
    MultiDimensionalNewtonMethod newton;
    map<Index, double> tolerances;
    tolerances.insert(pair<Index, double>(qInd, hTol));
    tolerances.insert(pair<Index, double>(laInd, gTol));
    LocalResidualCriteriaFunction cfunc(tolerances);
    GlobalResidualCriteriaFunction cfuncGlob(gTol);
    StandardDampingFunction dfunc(30, 0.2);
    newton.setFunction(&fun_hg);
    newton.setJacobianFunction(jac);
    newton.setCriteriaFunction(&cfunc);
    newton.setDampingFunction(&dfunc);
    newton.setLinearAlgebra(1); // as system is possible underdetermined
    newton.setJacobianUpdateFreq(updateJacobianEvery);

    static_cast<DynamicSystem&>(system).plot(t, dt);

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
        throw MBSimError("ERROR (QuasiStaticIntegrator::subIntegrate): No convergence of Newton method for the new time step");
      iter = newton.getNumberOfIterations();

      for (int i = 0; i < q.size(); i++)
        q(i) = qla(i);

      for (int i = 0; i < la.size(); i++)
        la(i) = qla(q.size() + i);

      system.setLa(la);

      // todo: check whether the plot make the openMBV unstable.
      if ((step * stepPlot - integrationSteps) < 0) {
        /* WRITE OUTPUT */
        step++;
        static_cast<DynamicSystem&>(system).plot(t, dt);
        double s1 = clock();
        time += (s1 - s0) / CLOCKS_PER_SEC;
        s0 = s1;
        integPlot << t << " " << dt << " " << iter << " " << time << " " << system.getlaSize() << endl;
        if (output)
          cout << "   t = " << t << ",\tdt = " << dt << ",\titer = " << setw(5) << setiosflags(ios::left) << iter << "\r" << flush;
        tPlot += dtPlot;
      }

      /* UPDATE SYSTEM FOR NEXT STEP*/
      t += dt;   // step 0: update time, go into new time step.

//      x += system.deltax(z, t, dt);  // todo: framework is not ready for x.
    }
  }

  void QuasiStaticIntegrator::postIntegrate(DynamicSystemSolver& system) {
    integPlot.close();

    typedef tee_device<ostream, ofstream> TeeDevice;
    typedef stream<TeeDevice> TeeStream;
    ofstream integSum((name + ".sum").c_str());
    TeeDevice ts_tee(cout, integSum);
    TeeStream ts_split(ts_tee);

    ts_split << endl << endl << "******************************" << endl;
    ts_split << "INTEGRATION SUMMARY: " << endl;
    ts_split << "End time [s]: " << tEnd << endl;
    ts_split << "Integration time [s]: " << time << endl;
    ts_split << "Integration steps: " << integrationSteps << endl;
    ts_split << "Maximum number of iterations: " << maxIter << endl;
    ts_split << "Average number of iterations: " << double(sumIter) / integrationSteps << endl;
    ts_split << "******************************" << endl;
    ts_split.flush();
    ts_split.close();

    cout.unsetf(ios::scientific);
    cout << endl;
  }

  void QuasiStaticIntegrator::integrate(DynamicSystemSolver& system) {
    debugInit();
    preIntegrate(system);
    subIntegrate(system, tEnd);
    postIntegrate(system);
  }

  void QuasiStaticIntegrator::initializeUsingXML(DOMElement *element) {
    Integrator::initializeUsingXML(element);
    DOMElement *e;
    e = E(element)->getFirstElementChildNamed(MBSIMINT % "stepSize");
    setStepSize(Element::getDouble(e));
  }

  fmatvec::Vec hgFun::operator()(const fmatvec::Vec& qla) {
    int qSize = sys->getqSize();
    int laSize = sys->getlaSize();
    int qlaSize = qSize + laSize;

    // set the q of system to be the input q
    sys->setq(qla(0, qSize - 1));
    sys->setLa(qla(qSize, qlaSize - 1));

    sys->updateStateDependentVariables(t);
    sys->updateg(t); // for joint, gap distance need also be updated.
//    sys->checkActive(1);   // todo: flag = 1, gap distance level
//    sys->updategd(t);  //todo:  ??? gd is needed for updating h
//    sys->updateT(t);
    sys->updateJacobians(t); // needed for calculating W.
    sys->updateh(t);
//    sys->updateM(t);  // todo: needed?
    if ((qlaSize - qSize) > 0)
      sys->updateW(t);
//    sys->updateV(t);  //not needed, because updateV is only needed for contact calculation
//    sys->updateG(t);  //  todo: not needed, because la is not solved by contact iteration

    // get the new h vector
    Vec hg;
    hg.resize(qla.size());

    // need new h, W, g, input la (not from the system, but from the input value),
    hg(0, qSize - 1) = sys->geth() + sys->getW() * qla(qSize, qlaSize - 1);
    hg(qSize, qlaSize - 1) = sys->getg().copy();

//    cout << "t = "  << t << "\n";
//    cout << "sys.geth() = "  <<  sys->geth().T() << "\n";
//    cout << "la= " << sys->getla().T() << endl;
//    cout << "W= " << sys->getW().T() << endl;
//    cout << "w * la = "  << Vec(sys->getW() * sys->getla()).T()  << "\n \n";
//    cout << "hg = "  << hg.T()  << "\n \n";

    return hg;
  }

  fmatvec::SqrMat jacFun::operator()(const fmatvec::Vec& q) {
    // backup the original q of the dynamical system sys.
    Vec qOld;
    qOld << sys->getq();

    // set the q of system to be the input q
    sys->setq(q);

    SqrMat jac;
    jac << sys->dhdq(t);

    // recover the old sys state
    sys->setq(qOld);
    sys->update(z, t);

    return jac;
  }
}

