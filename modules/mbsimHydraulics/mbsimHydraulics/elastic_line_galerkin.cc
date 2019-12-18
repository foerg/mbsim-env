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
#include "mbsimHydraulics/elastic_line_galerkin.h"
#include "environment.h"
#include "mbsim/utils/ansatz_functions.h"
#include "mbsim/utils/utils.h"
#include "mbsim/frames/frame.h"

#include "mbsim/dynamic_system_solver.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimHydraulics {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMHYDRAULICS, ElasticLineGalerkin)

  ElasticLineGalerkin::ElasticLineGalerkin(const string &name) : HLine(name),  WInt(), wA(), wE(), lambda(0), MatIntWWT(), MatIntWSWST(), K(), D(), N(), Omega(), phi(), ansatz(nullptr), plotVecW(), plotVecWS(),  relPlotPoints() {
  }

  void ElasticLineGalerkin::calcSize() {
    Object::nq = mdim;
    Object::nu = mdim;
    updSize = false;
  }

  void ElasticLineGalerkin::setAnsatzFunction(AnsatzTypes method_, int nAnsatz_) {
    if (l<=1e-4)
      throwError("set length first");
    switch (method_) {
      case BSplineOrd4:
        ansatz = new ansatz_function_BSplineOrd4(nAnsatz_, l);
        break;
      case BSplineOrd3:
        ansatz = new ansatz_function_BSplineOrd3(nAnsatz_, l);
        break;
      case Polynom:
        ansatz = new ansatz_function_polynom(nAnsatz_, l);
        break;
      case Harmonic:
        ansatz = new ansatz_function_harmonic(nAnsatz_, l);
        break;
    }

    mdim=ansatz->dim();
    WInt.assign(ansatz->VecIntW());
    wA.assign(ansatz->VecW0());
    wE.assign(ansatz->VecWL());
    MatIntWWT.assign(ansatz->MatIntWWT());
    MatIntWSWST.assign(ansatz->MatIntWSWST());
  }

  void ElasticLineGalerkin::init(InitStage stage, const InitConfigSet &config) {
    if (stage==preInit) {
      Area=M_PI*d*d/4.;
      if (direction.size()>0)
        g=trans(((DynamicSystem*)parent)->getFrame("I")->getOrientation(false)*MBSimEnvironment::getInstance()->getAccelerationOfGravity())*direction;
      else
        g=0;
      double E0=HydraulicEnvironment::getInstance()->getBasicBulkModulus();
      double kappa=HydraulicEnvironment::getInstance()->getKappa();
      double pinf=HydraulicEnvironment::getInstance()->getEnvironmentPressure();
      OilBulkModulus bulkModulus(name, E0, pinf, kappa, fracAir);
      E=bulkModulus(p0);
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      k=rho*g*delta_h/l;
      MFac.assign(Area*rho*MatIntWWT);
      K.assign(Area*E*MatIntWSWST);
      double nu=HydraulicEnvironment::getInstance()->getKinematicViscosity();
      phi.resize(mdim);
      
      Jacobian.resize(mdim, mdim, INIT, 0);
      for (int i=0; i<mdim; i++)
        Jacobian(i, i)=1.;
      
      if (eigvec(K, MFac, phi, lambda))
        throwError("Fehler bei Eigenvektorberechnung!");
      Omega.resize(mdim, INIT, 0);
      for (int i=1; i<mdim; i++) // analytische Loesung unabhaengig vom Ansatztyp --> omega(0)=0
        Omega(i,i)=sqrt(lambda(i));
      N.resize(mdim, INIT, 0);
      if (Flow2D) {
        for (int i=0; i<mdim; i++) {
          double kH=sqrt(Omega(i,i)/nu)*sqrt(Area/M_PI);
          N(i,i)=(kH<5.)?1.+0.0024756*pow(kH, 3.0253322):0.44732718+0.175*kH;
        }
      }
      Mat DTmp(mdim, mdim, INIT, 0);
      if (!Flow2D)
        DTmp=8*rho*M_PI*nu*MatIntWWT*SymMat(mdim, EYE);
      else
        DTmp=8*rho*M_PI*nu*MatIntWWT*phi*N*inv(phi);
      if (DLehr>0)
        DTmp += 2.*DLehr*inv(trans(phi))*Omega*trans(phi)*MFac;
      D.resize(mdim, INIT, 0);
      for (int i=0; i<mdim; i++)
        for (int j=0; j<mdim; j++)
          D(i,j)=DTmp(i,j);
    }
    else if (stage==plotting) {
      if(plotFeature[plotRecursive]) {
        plotdim=relPlotPoints.size();
        plotVecW.resize(mdim, plotdim);
        plotVecWS.resize(mdim, plotdim);
        for (int i=0; i<plotdim; i++) {
          plotVecW.col(i)=ansatz->VecW(relPlotPoints(i));
          plotVecWS.col(i)=ansatz->VecWS(relPlotPoints(i));
        }
        delete ansatz;
        if(plotFeature[volumeFlow]) {
          for (int i=0; i<plotdim; i++)
            plotColumns.push_back("Q(x="+toString(relPlotPoints(i)*l, -1)+") [l/min]");
        }
        if(plotFeature[pressure]) {
          for (int i=0; i<plotdim; i++)
            plotColumns.push_back("p(x="+toString(relPlotPoints(i)*l, -1)+") [bar]");
        }
      }
    }
    else if (stage==unknownStage) {
      u0.assign(inv(MatIntWWT)*WInt*Q0);
      //plotParameters();
    }
    HLine::init(stage, config);

  }

  void ElasticLineGalerkin::updateQ() {
    QIn(0)=Area*trans(wA)*u;
    QOut(0)=-Area*trans(wE)*u;
    updQ = false;
  }

  void ElasticLineGalerkin::updateT() {
    T=SqrMat(mdim, EYE);
  }

  void ElasticLineGalerkin::updateM() {
    M=MFac;
  }

  void ElasticLineGalerkin::updateh(int j) {
    h[j] = (-k*WInt + p0*(wE-wA))*Area - D*u - K*q;
  }

  void ElasticLineGalerkin::plot() {
    if(plotFeature[plotRecursive]) {
      if(plotFeature[volumeFlow]) {
        for (int i=0; i<plotdim; i++)
          plotVector.push_back(Area*trans(u)*plotVecW.col(i)*6e4);
      }
      if(plotFeature[pressure]) {
        for (int i=0; i<plotdim; i++)
          plotVector.push_back((-E*trans(q)*plotVecWS.col(i)+p0)*1e-5);
      }
    }
    HLine::plot();
  }

  void ElasticLineGalerkin::plotParameters() {
    msg(Info) << "mdim=" << mdim << endl;
    msg(Info) << "g=" << g << endl;
    msg(Info) << "E=" << E << endl;
    msg(Info) << "k=" << k << endl;
    msg(Info) << "WInt=" << WInt << endl;
    msg(Info) << "wA=" << wA << endl;
    msg(Info) << "wE=" << wE << endl;
    msg(Info) << "MatIntWWT=" << MatIntWWT << endl;
    msg(Info) << "MatIntWSWST=" << MatIntWSWST << endl;
    msg(Info) << "M=" << MFac << endl;
    msg(Info) << "K=" << K << endl;
    msg(Info) << "D=" << D << endl;
    msg(Info) << "lambda=" << lambda << endl;
    msg(Info) << "Omega=" << Omega << endl;
    msg(Info) << "phi=" << phi << endl;
    msg(Info) << "N=" << N << endl;
    msg(Info) << "plotVecW=" << plotVecW << endl;
    msg(Info) << "plotVecWS=" << plotVecWS << endl;
  }

  void ElasticLineGalerkin::initializeUsingXML(DOMElement * element) {
    HLine::initializeUsingXML(element);
    DOMElement * e;
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"initialPressure");
    setp0(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"fracAir");
    setFracAir(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"heightDifference");
    setdh(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"dLehr");
    setDLehr(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"diameter");
    setDiameter(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"length");
    setLength(MBXMLUtils::E(e)->getText<double>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"AnsatzFunction");
    DOMElement * ee = e->getFirstElementChild();
    if (MBXMLUtils::E(ee)->getTagName()==MBSIMHYDRAULICS%"BSplineOrder3")
      setAnsatzFunction(BSplineOrd3, MBXMLUtils::E(ee->getNextElementSibling())->getText<int>());
    else if (MBXMLUtils::E(ee)->getTagName()==MBSIMHYDRAULICS%"BSplineOrder4")
      setAnsatzFunction(BSplineOrd4, MBXMLUtils::E(ee->getNextElementSibling())->getText<int>());
    else if (MBXMLUtils::E(ee)->getTagName()==MBSIMHYDRAULICS%"Polynom")
      setAnsatzFunction(Polynom, MBXMLUtils::E(ee->getNextElementSibling())->getText<int>());
    else if (MBXMLUtils::E(ee)->getTagName()==MBSIMHYDRAULICS%"Harmonic")
      setAnsatzFunction(Harmonic, MBXMLUtils::E(ee->getNextElementSibling())->getText<int>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"flow2d");
    if (e)
      setFlow2D(true);
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"relativePlotPoints");
    setRelativePlotPoints(MBXMLUtils::E(e)->getText<Vec>());
  }

}
