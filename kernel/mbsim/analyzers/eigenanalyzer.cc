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

#include <config.h>
#include "eigenanalyzer.h"
#include "mbsim/dynamic_system_solver.h"
#include "fmatvec/linear_algebra_complex.h"
#include "mbsim/utils/eps.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

// TODO: remove the following two functions and provide an uniform concept to
// cast between complex and double
Vector<Ref, complex<double>> toComplex(const Vector<Ref, double> &x) {
  Vector<Ref, complex<double>> y(x.size(),NONINIT);
  for(int i=0; i<x.size(); i++)
    y(i) = complex<double>(x(i),0);
  return y;
}
const Vector<Ref, double> fromComplex(const Vector<Ref, complex<double>> &x) {
  Vector<Ref, double> y(x.size(),NONINIT);
  for(int i=0; i<x.size(); i++)
    y(i) = x(i).real();
  return y;
}

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, Eigenanalyzer)

  void Eigenanalyzer::execute() {
    if(task == eigenmodes) computeEigenmodes();
    else if(task == eigenmotion) computeEigenmotion();
    else throwError("(Eigenanalyzer::init): task unknown");
  }

  Vec Eigenanalyzer::getEigenfrequencies() const {
    fmatvec::Vec freq(f.size(),NONINIT);
    for(int i=0; i<freq.size(); i++)
      freq.e(i) = f[i].first;
    return freq;
  }

  void Eigenanalyzer::computeEigenvalues() {
    if(not(z0.size()))
      zEq <<= system->evalz0();
    else {
      if(z0.size()!=system->getzSize()+system->getisSize())
	throwError(string("(Eigenanalyzer::computeEigenvalues): size of z0 does not match, must be ") + to_string(system->getzSize()));
      zEq <<= z0(RangeV(0,system->getzSize()-1));
      system->setcuris(z0(RangeV(system->getzSize(),z0.size()-1)));
    }

    system->setTime(tStart);

    A_.resize(system->getzSize(),NONINIT);
    Vec zd(system->getzSize(),NONINIT), zdOld(system->getzSize(),NONINIT);
    system->setState(zEq);
    system->resetUpToDate();
    system->computeInitialCondition();
    zEq = system->getState();
    zdOld = system->evalzd();
    for (int i=0; i<system->getzSize(); i++) {
      double ztmp = system->getState()(i);
      system->getState()(i) += epsroot;
      system->resetUpToDate();
      zd = system->evalzd();
      A_.set(i, (zd - zdOld) / epsroot);
      system->getState()(i) = ztmp;
    }
    eigvec(A_,V,w);
    f.clear();
    for (int i=0; i<w.size(); i++) {
      if((abs(imag(w(i))) > macheps) and (i < w.size()-1) and (w(i+1)==conj(w(i)))) {
        f.push_back(pair<double,int>(imag(w(i))/2/M_PI,i));
        i++;
      }
    }
    std::sort(f.begin(), f.end());
    saveEigenanalyis();
  }

  void Eigenanalyzer::computeEigenmodes() {
    computeEigenvalues();
    Vector<Ref, complex<double>> c(w.size());
    Vector<Ref, complex<double>> deltaz(w.size(),NONINIT);
    Vector<Ref, complex<double>> wbuf = w;

    VecV Av(system->getzSize()/2,INIT,A);
    for(int i=0; i<MA.rows(); i++) {
      if(int(MA(i,0))<0 or int(MA(i,0))>=Av.size())
        throwError("(Eigenanalyzer::computeEigenmodes): size of mode amplitude matrix does not match");
      Av(int(MA(i,0))) = MA(i,1);
    }

    double t0 = tStart;
    for(int j=0; j<static_cast<int>(f.size()); j++) {
      if(Av(j) > 0) {
      c(f[j].second) = complex<double>(0,Av(j));
      c(f[j].second+1) = complex<double>(0,-Av(j));
      wbuf(f[j].second).real(0);
      wbuf(f[j].second+1).real(0);
      double T = double(loops)/f[j].first;
      for(double t=t0; t<t0+T+dtPlot; t+=dtPlot) {
        deltaz.init(0);
        for(int i=0; i<wbuf.size(); i++)
          deltaz += c(i)*V.col(i)*exp(wbuf(i)*t); 
        system->setTime(t);
        system->setState(zEq + fromComplex(deltaz));
        system->resetUpToDate();
        system->plot();
        system->checkExitRequest();
        if(system->getIntegratorExitRequest())
          break;
      }
      if(system->getIntegratorExitRequest())
        break;
      t0 += T+dtPlot;
      c(f[j].second) = complex<double>(0,0);
      c(f[j].second+1) = complex<double>(0,0);
      }
    }
  }

  void Eigenanalyzer::computeEigenmotion() {
    computeEigenvalues();
    Vector<Ref, complex<double>> deltaz(w.size(),NONINIT);

    if(deltaz0.size()==0)
      deltaz0.resize(w.size());
    Vector<Ref, complex<double>> c = slvLU(V,toComplex(deltaz0));

    for(double t=tStart; t<tEnd+dtPlot; t+=dtPlot) {
      deltaz.init(0);
      for(int i=0; i<w.size(); i++)
        deltaz += c(i)*V.col(i)*exp(w(i)*t); 
      system->setTime(t);
      system->setState(zEq + fromComplex(deltaz));
      system->resetUpToDate();
      system->plot();
    }
  }

  void Eigenanalyzer::saveEigenanalyis() {
    ofstream os("eigenanalysis.mat");
    if(os.is_open()) {
      os << "# name: " << "lambda" << endl;
      os << "# type: " << "complex matrix" << endl;
      os << "# rows: " << w.size() << endl;
      os << "# columns: " << 1 << endl;
      for(int i=0; i<w.size(); i++)
        os << setw(28) << w.e(i) << endl;
      os << endl;
      os << "# name: " << "V" << endl;
      os << "# type: " << "complex matrix" << endl;
      os << "# rows: " << V.rows() << endl;
      os << "# columns: " << V.cols() << endl;
      for(int i=0; i<V.rows(); i++) {
        for(int j=0; j<V.cols(); j++)
          os << setw(28) << V.e(i,j) << " ";
        os << endl;
      }
      os << endl;
      os << "# name: " << "z" << endl;
      os << "# type: " << "matrix" << endl;
      os << "# rows: " << zEq.size() << endl;
      os << "# columns: " << 1 << endl;
      for(int i=0; i<zEq.size(); i++)
        os << setw(28) << zEq.e(i) << endl;
      os << endl;
      os << "# name: " << "A" << endl;
      os << "# type: " << "matrix" << endl;
      os << "# rows: " << A_.rows() << endl;
      os << "# columns: " << A_.cols() << endl;
      for(int i=0; i<A_.rows(); i++) {
        for(int j=0; j<A_.cols(); j++)
          os << setw(28) << A_.e(i,j) << " ";
        os << endl;
      }
      os << endl;
      os.close();
    }
  }

  void Eigenanalyzer::initializeUsingXML(DOMElement *element) {
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIM%"startTime");
    if(e) setStartTime(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"endTime");
    if(e) setEndTime(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"plotStepSize");
    if(e) setPlotStepSize(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"initialState");
    if(e) setInitialState(E(e)->getText<Vec>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"task");
    if(e) {
      string str=X()%E(e)->getFirstTextChild()->getData();
      str=str.substr(1,str.length()-2);
      if(str=="eigenmodes") task=eigenmodes;
      else if(str=="eigenmotion") task=eigenmotion;
      else task=unknown;
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"initialDeviation");
    if(e) setInitialDeviation(E(e)->getText<Vec>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"amplitude");
    if(e) setAmplitude(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"modeAmplitudeTable");
    if(e) {
      setModeAmplitudeTable(E(e)->getText<Matrix<General,Var,Fixed<2>,double>>());
      for(int i=0; i<MA.rows(); i++)
        MA(i,0)--;
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"loops");
    if(e) setLoops(E(e)->getText<double>());
  }

}
