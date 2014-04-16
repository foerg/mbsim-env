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
#include "mbsimHydraulics/pressure_loss.h"
#include "mbsimHydraulics/rigid_line.h"
#include "mbsimHydraulics/leakage_line.h"
#include "mbsimHydraulics/dimensionless_line.h"
#include "mbsimHydraulics/environment.h"
#include "mbsim/functions/tabular_functions.h"
#include "mbsim/utils/nonlinear_algebra.h"
#include "mbsim/utils/eps.h"
#include "mbsimHydraulics/defines.h"
#include "mbsim/objectfactory.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimHydraulics {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, SerialResistanceLinePressureLoss,  MBSIMHYDRAULICS%"SerialResistanceLinePressureLoss")

  double SerialResistanceLinePressureLoss::operator()(const double& Q) {
    double pl=0;
    for (unsigned int i=0; i<slp.size(); i++)
      pl+=(*slp[i])(Q);
    return pl;
  }

  void SerialResistanceLinePressureLoss::initializeUsingXML(DOMElement * element) {
    LinePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=element->getFirstElementChild();
    while (e) {
      LinePressureLoss *p=MBSim::ObjectFactory<FunctionBase>::createAndInit<LinePressureLoss>(e);
      addLinePressureLoss(p);
      e=e->getNextElementSibling();
    }
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, ParallelResistanceLinePressureLoss,  MBSIMHYDRAULICS%"ParallelResistanceLinePressureLoss")

  double ParallelResistanceLinePressureLoss::operator()(const double& Q) {
    return (*pl)(Q/number);
  }

  void ParallelResistanceLinePressureLoss::initializeUsingXML(DOMElement * element) {
    LinePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"number");
    int n=Element::getInt(e);
    e=e->getNextElementSibling();
    LinePressureLoss *p=MBSim::ObjectFactory<FunctionBase>::createAndInit<LinePressureLoss>(e);
    setLinePressureLoss(p, n);
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, ZetaLinePressureLoss,  MBSIMHYDRAULICS%"ZetaLinePressureLoss")

  double ZetaLinePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      double d=((const RigidLine*)(line))->getDiameter();
      double A=M_PI*d*d/4.;
      c*=rho/2./A/A;
      initialized=true;
    }
    return c*Q*fabs(Q);
  }

  void ZetaLinePressureLoss::initializeUsingXML(DOMElement * element) {
    LinePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"zeta");
    setZeta(Element::getDouble(e));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, ZetaPosNegLinePressureLoss,  MBSIMHYDRAULICS%"ZetaPosNegLinePressureLoss")

  double ZetaPosNegLinePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      double d=((const RigidLine*)(line))->getDiameter();
      double A=M_PI*d*d/4.;
      cPos*=rho/2./A/A;
      cNeg*=rho/2./A/A;
      initialized=true;
    }
    return (Q>=0?cPos:cNeg)*Q*fabs(Q);
  }

  void ZetaPosNegLinePressureLoss::initializeUsingXML(DOMElement * element) {
    LinePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"zetaPos");
    setZetaPos(Element::getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"zetaNeg");
    setZetaNeg(Element::getDouble(e));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, LaminarTubeFlowLinePressureLoss,  MBSIMHYDRAULICS%"LaminarTubeFlowLinePressureLoss")

  double LaminarTubeFlowLinePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double eta=HydraulicEnvironment::getInstance()->getDynamicViscosity();
      double d=((const RigidLine*)(line))->getDiameter();
      double l=((const RigidLine*)(line))->getLength();
      double area=M_PI*d*d/4.;
      c=32.*eta*l/d/d/area;
      initialized=true;
    }
    return c*Q;
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, TurbulentTubeFlowLinePressureLoss,  MBSIMHYDRAULICS%"TurbulentTubeFlowLinePressureLoss")

  void TurbulentTubeFlowLinePressureLoss::setHydraulicDiameter(double dHyd_, double dHydNeg_) {
    assert(dHyd_>=0);
    assert(dHydNeg_>=0);
    dHyd=dHyd_; 
    dHydNeg=((fabs(dHydNeg_)<epsroot())?dHyd_:dHydNeg_); 
  }
  
  double TurbulentTubeFlowLinePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      double l=((const RigidLine*)(line))->getLength();
      double d=((const RigidLine*)(line))->getDiameter();
      double area=M_PI*d*d/4.;
      c=l/d*rho/2./area/area;
      double nu=HydraulicEnvironment::getInstance()->getKinematicViscosity();
      double areaRef=M_PI*dRef*dRef/4.;
      ReynoldsFactor=dHyd/areaRef/nu;
      ReynoldsFactorNeg=-dHydNeg/areaRef/nu;
      class Lambda : public Function<double(double)> {
        public:
          Lambda(double Re_, double k_, double d_) : Re(Re_), k(k_), d(d_) {}
          double operator()(const double &lambda) {
            return -2.*log10(2.51/Re/sqrt(lambda)+k/3.71/d)-1./sqrt(lambda);
          }
        private:
          double Re, k, d;
      };
      vector<double> re, la;
      re.push_back(2320.);
      Lambda fLambda(re.back(), k, d);
      RegulaFalsi solver(&fLambda);
      solver.setTolerance(epsroot());
      la.push_back(solver.solve(1e-4, 1e-1));
      do {
        re.push_back(re.back()*1.1);
        Lambda l(re.back(), k, d);
        RegulaFalsi solver(&l);
        solver.setTolerance(epsroot());
        la.push_back(solver.solve(1e-4, 1e-1));
      } while (fabs(la.back()-la[la.size()-2])>1e-6);
      Vec ReValues(re.size(), INIT, 0);
      Vec lambdaValues(re.size(), INIT, 0);
      for (int i=0; i<int(re.size()); i++) {
        ReValues(i)=re[i];
        lambdaValues(i)=la[i];
      }
      lambdaTabular = new TabularFunction<double(double)>(ReValues, lambdaValues);
      initialized=true;
    }
    const double Re=Q*((Q>0)?ReynoldsFactor:ReynoldsFactorNeg);
    if (Re<1404.) // laminar
      return 64./ReynoldsFactor*c*Q;
    else {
      double lambda;
      if (Re<2320.) // transition
        lambda=64./Re+((*lambdaTabular)(2320.)-64./Re)/(2320.-1404.)*(Re-1404.);
      else // turbulent
        lambda=(*lambdaTabular)(Re);
      return lambda*c*fabs(Q)*Q;
    }
  }

  void TurbulentTubeFlowLinePressureLoss::initializeUsingXML(DOMElement * element) {
    PressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e = E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"referenceDiameter");
    double dR=Element::getDouble(e);
    setReferenceDiameter(dR);
    e = E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"hydraulicDiameter");
    double dH=Element::getDouble(e);
    e = E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"negativeHydraulicDiameter");
    double dHNeg=0;
    if (e)
     dHNeg=Element::getDouble(e);
    setHydraulicDiameter(dH, dHNeg);
    e = E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"surfaceRoughness");
    double kS=Element::getDouble(e);
    setSurfaceRoughness(kS);
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, CurveFittedLinePressureLoss,  MBSIMHYDRAULICS%"CurveFittedLinePressureLoss")

  double CurveFittedLinePressureLoss::operator()(const double &Q) {
    if (!initialized) {
      double nu=HydraulicEnvironment::getInstance()->getKinematicViscosity();
      double areaRef=M_PI*dRef*dRef/4.;
      ReynoldsFactor=dHyd/areaRef/nu;
      initialized=true;
    }
    const double Re=ReynoldsFactor*Q; 
    return Re*((Re>0)?aPos+bPos*Re:aNeg-bNeg*Re); 
  }

  void CurveFittedLinePressureLoss::initializeUsingXML(DOMElement * element) {
    PressureLoss::initializeUsingXML(element);
    dRef=Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"referenceDiameter"));
    dHyd=Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"hydraulicDiameter"));
    aPos=Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"aPositive"));
    bPos=Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"bPositive"));
    aNeg=Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"aNegative"));
    bNeg=Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"bNegative"));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, TabularLinePressureLoss,  MBSIMHYDRAULICS%"TabularLinePressureLoss")

  double TabularLinePressureLoss::operator()(const double& Q) {
    return ((*zetaTabular)(Q));
  }

  void TabularLinePressureLoss::initializeUsingXML(DOMElement * element) {
    LinePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"function");
    zetaTabular=MBSim::ObjectFactory<FunctionBase>::createAndInit<Function<double(double)> >(e->getFirstElementChild());
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, RelativeAreaZetaClosablePressureLoss,  MBSIMHYDRAULICS%"RelativeAreaZetaClosablePressureLoss")

  double RelativeAreaZetaClosablePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      double d=((const RigidLine*)(line))->getDiameter();
      double A=M_PI*d*d/4.;
      c*=rho/2./A/A;
      if (cNeg<0)
        cNeg=c;
      else
        cNeg*=rho/2./A/A;
      initialized=true;
    }
    const double areaRel=((const ClosableRigidLine*)(line))->getRegularizedValue();
    if (Q<0)
      return cNeg*Q*fabs(Q)/areaRel/areaRel;
    else
      return c*Q*fabs(Q)/areaRel/areaRel;
  }

  void RelativeAreaZetaClosablePressureLoss::initializeUsingXML(DOMElement * element) {
    ClosablePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"zeta");
    setZeta(Element::getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"zetaNegative");
    if (e)
      setZetaNegative(Element::getDouble(e));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, GapHeightClosablePressureLoss,  MBSIMHYDRAULICS%"GapHeightClosablePressureLoss")

  double GapHeightClosablePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double eta=HydraulicEnvironment::getInstance()->getDynamicViscosity();
      c=12.*eta*l/b;
      initialized=true;
    }
    const double h=((const ClosableRigidLine*)(line))->getRegularizedValue();
    return c/h/h/h*Q;
  }

  void GapHeightClosablePressureLoss::initializeUsingXML(DOMElement * element) {
    ClosablePressureLoss::initializeUsingXML(element);
    setLength(Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"length")));
    setWidth(Element::getDouble(E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"width")));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, ReynoldsClosablePressureLoss,  MBSIMHYDRAULICS%"ReynoldsClosablePressureLoss")

  double ReynoldsClosablePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      nu=HydraulicEnvironment::getInstance()->getKinematicViscosity();
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      const double x0=1404.;
      const double x1=2320.;
      const double y0=64./1404.;
      const double y1=.3164/pow(2320, .25);
      lambdaSlope=(y1-y0)/(x1-x0);
      lambdaOffset=y0-lambdaSlope*x0;
      zetaFactor=rho/2.*((const RigidLine*)(line))->getLength();
      initialized=true;
    }
    const double diameter=((const ClosableRigidLine*)(line))->getRegularizedValue();
    const double area=M_PI*diameter*diameter/4.;
    const double Re=fabs(Q)*diameter/area/nu;
    double lambda=0;
    
    const double ReCritical = .1;
    if (Re<ReCritical) {
      const double lambdaCritical = .3164 * pow(ReCritical, -.25);
      const double lambdaSlope = .3164 * (-.25) * pow(ReCritical, -.25-1.);
      const double lambdaOffset = lambdaCritical - lambdaSlope * ReCritical;
      lambda = lambdaSlope * Re + lambdaOffset;
    }
    else
      lambda=.3164 * pow(Re, -.25);
    
    return zetaFactor * lambda/diameter * Q * fabs(Q) / area /area;
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, RelativeAlphaClosablePressureLoss,  MBSIMHYDRAULICS%"RelativeAlphaClosablePressureLoss")

  double RelativeAlphaClosablePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double d=((const RigidLine*)(line))->getDiameter();
      double area=M_PI*d*d/4.;
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      alpha2=alpha*alpha;
      c=rho/2./area/area;
      initialized=true;
    }
    double alphaRel=((const ClosableRigidLine*)(line))->getRegularizedValue();
    if (alphaRel>1.)
      alphaRel=1.;
    return (1./(alphaRel * alphaRel * alpha2) - 1.)*c*Q*fabs(Q);
  }

  void RelativeAlphaClosablePressureLoss::initializeUsingXML(DOMElement * element) {
    ClosablePressureLoss::initializeUsingXML(element);
    DOMElement * e = E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"alpha");
    setAlpha(Element::getDouble(e));
  }


  void CheckvalveClosablePressureLoss::initializeUsingXML(DOMElement * element) {
    ClosablePressureLoss::initializeUsingXML(element);
    DOMElement * e = E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"ballRadius");
    setBallRadius(Element::getDouble(e));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, GammaCheckvalveClosablePressureLoss,  MBSIMHYDRAULICS%"GammaCheckvalveClosablePressureLoss")

  double GammaCheckvalveClosablePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      siga=sin(gamma);
      coga=cos(gamma);
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      c=rho/2./alpha/alpha/M_PI/M_PI;
      initialized=true;
    }
    const double x=((const ClosableRigidLine*)(line))->getRegularizedValue();
    const double fx=x*(2.*rBall*siga/coga + x*siga/coga/coga );
    return c/fx/fx*fabs(Q)*Q;
  }

  void GammaCheckvalveClosablePressureLoss::initializeUsingXML(DOMElement * element) {
    CheckvalveClosablePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"alpha");
    setAlpha(Element::getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"gamma");
    setGamma(Element::getDouble(e));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, IdelchickCheckvalveClosablePressureLoss,  MBSIMHYDRAULICS%"IdelchickCheckvalveClosablePressureLoss")

  double IdelchickCheckvalveClosablePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double d=((const RigidLine*)(line))->getDiameter();
      double area=M_PI*d*d*.25;
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      d0=d;
      c=rho/2./area/area;
      initialized=true;
    }
    const double xOpen=((const ClosableRigidLine*)(line))->getRegularizedValue();
    const double hdivd0=xOpen/d0;
    const double beta2=.8/hdivd0;
    const double beta3=.14/hdivd0/hdivd0;
    return c*Q*fabs(Q)*(2.7-beta2+beta3); 
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, ConeCheckvalveClosablePressureLoss,  MBSIMHYDRAULICS%"ConeCheckvalveClosablePressureLoss")

  double ConeCheckvalveClosablePressureLoss::operator()(const double& Q) {
    if (!initialized) {
      double d=((const RigidLine*)(line))->getDiameter();
      double rOpen=d/2.;
      double rBall2=rBall*rBall;
      double rOpen2=rOpen*rOpen;
      double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      c=rho/2./alpha/alpha/M_PI/M_PI;
      numer[0]=rBall2;
      numer[1]=2.*sqrt(rBall2-rOpen2);
      denom[0]=4.*(rBall2-rOpen2);
      denom[1]=4.*sqrt(rBall2-rOpen2);
      initialized=true;
    }
    const double xOpen=((const ClosableRigidLine*)(line))->getRegularizedValue();
    const double xOpen2=xOpen*xOpen;
    const double xOpen4=xOpen2*xOpen2;
    return c*(xOpen2+xOpen*numer[1]+numer[0])/(xOpen2+xOpen*denom[1]+denom[0])/xOpen4 * Q * fabs(Q);
  }

  void ConeCheckvalveClosablePressureLoss::initializeUsingXML(DOMElement * element) {
    CheckvalveClosablePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"alpha");
    setAlpha(Element::getDouble(e));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, PlaneLeakagePressureLoss,  MBSIMHYDRAULICS%"PlaneLeakagePressureLoss")

  double PlaneLeakagePressureLoss::operator()(const double& pVorQ) {
    if (!initialized) {
      if (((HLine*)(line))->getJacobian().rows())
        stateless=false;
      double h;
      double w;
      if (stateless) {
        h=((const PlaneLeakage0DOF*)(line))->getGapHeight();
        w=((const PlaneLeakage0DOF*)(line))->getGapWidth();
      }
      else {
        h=((const PlaneLeakageLine*)(line))->getGapHeight();
        w=((const PlaneLeakageLine*)(line))->getGapWidth();
      }
      double eta=HydraulicEnvironment::getInstance()->getDynamicViscosity();
      // vgl. E. Becker: Technische Strömungslehre, (6. 13)
      pVfac = -w*h*h*h/12./eta;
      xdfac = w*h/2.;
      initialized=true;
    }
    if (stateless) {
      const double dp=pVorQ;
      const double gl=((const PlaneLeakage0DOF*)(line))->getGapLength();
      const double s1v=((const PlaneLeakage0DOF*)(line))->getSurface1Velocity();
      const double s2v=((const PlaneLeakage0DOF*)(line))->getSurface2Velocity();
      return xdfac*(s1v+s2v) + pVfac/gl*dp;
    }
    else {
      const double Q=pVorQ;
      const double gl=((const PlaneLeakageLine*)(line))->getGapLength();
      const double s1v=((const PlaneLeakageLine*)(line))->getSurface1Velocity();
      const double s2v=((const PlaneLeakageLine*)(line))->getSurface2Velocity();
      return -(Q-xdfac*(s1v+s2v))*gl/pVfac;
    }
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, EccentricCircularLeakagePressureLoss,  MBSIMHYDRAULICS%"EccentricCircularLeakagePressureLoss")

  double EccentricCircularLeakagePressureLoss::operator()(const double& pVorQ) {
    if (!initialized) {
      if (((HLine*)(line))->getJacobian().rows())
        stateless=false;
      double rI;
      double rO;
      double hGap;
      if (stateless) {
        rI=((const CircularLeakage0DOF*)(line))->getInnerRadius();
        rO=((const CircularLeakage0DOF*)(line))->getOuterRadius();
        hGap=((const CircularLeakage0DOF*)(line))->getGapHeight();
      }
      else {
        rI=((const CircularLeakageLine*)(line))->getInnerRadius();
        rO=((const CircularLeakageLine*)(line))->getOuterRadius();
        hGap=((const CircularLeakageLine*)(line))->getGapHeight();
      }
      double area=M_PI*(rO*rO-rI*rI);
      area=M_PI*2.*rI*hGap;
      double eta=HydraulicEnvironment::getInstance()->getDynamicViscosity();
      pVfac=-area*hGap*hGap*(1.+1.5*ecc*ecc*ecc)/12./eta;
      xdfac=area/2.;
      initialized=true;
    }
    if (stateless) {
      const double dp=pVorQ;
      const double gl=((const CircularLeakage0DOF*)(line))->getGapLength();
      const double s1v=((const CircularLeakage0DOF*)(line))->getSurface1Velocity();
      const double s2v=((const CircularLeakage0DOF*)(line))->getSurface2Velocity();
      return xdfac*(s1v+s2v) + pVfac/gl*dp;
    }
    else {
      const double Q=pVorQ;
      const double gl=((const CircularLeakageLine*)(line))->getGapLength();
      const double s1v=((const CircularLeakageLine*)(line))->getSurface1Velocity();
      const double s2v=((const CircularLeakageLine*)(line))->getSurface2Velocity();
      return - (Q-xdfac*(s1v+s2v))*gl/pVfac;
    }
  }

  void EccentricCircularLeakagePressureLoss::initializeUsingXML(DOMElement * element) {
    CircularLeakagePressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"eccentricity");
    setEccentricity(Element::getDouble(e));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, RealCircularLeakagePressureLoss,  MBSIMHYDRAULICS%"RealCircularLeakagePressureLoss")

  double RealCircularLeakagePressureLoss::operator()(const double& pVorQ) {
    if (!initialized) {
      if (((HLine*)(line))->getJacobian().rows())
        stateless=false;
      double rI;
      double rA;
      if (stateless) {
        rI=((const CircularLeakage0DOF*)(line))->getInnerRadius();
        rA=((const CircularLeakage0DOF*)(line))->getOuterRadius();
      }
      else {
        rI=((const CircularLeakageLine*)(line))->getInnerRadius();
        rA=((const CircularLeakageLine*)(line))->getOuterRadius();
      }
      double eta=HydraulicEnvironment::getInstance()->getDynamicViscosity();
      vIfac=-(-rI*rI+2.*rI*rI*log(rI)-2.*rI*rI*log(rA)+rA*rA)*M_PI/(log(rI)-log(rA))/2.;
      vOfac=(rA*rA-2.*rA*rA*log(rA)+2.*rA*rA*log(rI)-rI*rI)*M_PI/(log(rI)-log(rA))/2.;
      pVfac=(1.+log(rI)-log(rA))*M_PI/eta/(log(rI)-log(rA))*rA*rA*rA*rA/8.-rA*rA*M_PI/eta/(log(rI)-log(rA))*rI*rI/4.+(log(rA)-log(rI)+1.)*M_PI/eta/(log(rI)-log(rA))*rI*rI*rI*rI/8.;
      initialized=true;
    }
    // vgl. Spurk Stroemungslehre S.162, (6.65)
    if (stateless) {
      const double dp=pVorQ;
      const double gl=((const CircularLeakage0DOF*)(line))->getGapLength();
      const double vI=((const CircularLeakage0DOF*)(line))->getSurface1Velocity();
      const double vO=((const CircularLeakage0DOF*)(line))->getSurface2Velocity();
      return vIfac*vI + vOfac*vO - pVfac/gl*dp;
    }
    else {
      const double Q=pVorQ;
      const double gl=((const CircularLeakageLine*)(line))->getGapLength();
      const double vI=((const CircularLeakageLine*)(line))->getSurface1Velocity();
      const double vO=((const CircularLeakageLine*)(line))->getSurface2Velocity();
      return (Q-vOfac*vO-vIfac*vI)*gl/pVfac;
    }
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FunctionBase, UnidirectionalZetaPressureLoss,  MBSIMHYDRAULICS%"UnidirectionalZetaPressureLoss")

  double UnidirectionalZetaPressureLoss::operator()(const double& Q) {
    if (!initialized) {
      const double rho=HydraulicEnvironment::getInstance()->getSpecificMass();
      const double d=((const RigidLine*)(line))->getDiameter();
      const double A=M_PI*d*d/4.;
      c*=rho/2./A/A;
      initialized=true;
    }
    const double pressureLoss=c*Q*fabs(Q);
    const double pressureLossMin=((const UnidirectionalRigidLine*)(line))->getMinimalPressureDrop();
    return pressureLoss<pressureLossMin?pressureLoss:0;
  }

  void UnidirectionalZetaPressureLoss::initializeUsingXML(DOMElement * element) {
    PressureLoss::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"zeta");
    setZeta(Element::getDouble(e));
  }


  //  //  void PositiveFlowLimittingPressureLoss::update(const double& Q) {
  //  //    QLimit=checkSizeSignal->getSignal()(0);
  //  //    setClosed(Q>QLimit);
  //  //  }
  //  //
  //  //  double RegularizedPositiveFlowLimittingPressureLoss::operator()(const double& Q, const void *) {
  //  //    if (isClosed()) {
  //  //      zeta = zeta1 * ((Q>QLimit+offset) ? 1. : (Q-QLimit)/offset * zeta1);
  //  //      pLoss = 1.e4;
  //  //    }
  //  //    else {
  //  //      zeta = 0;
  //  //      pLoss = 0;
  //  //    }
  //  //    return pLoss;
  //  //  }
  //  //
  //  //  void NegativeFlowLimittingPressureLoss::update(const double& Q) {
  //  //    setClosed(Q<checkSizeSignal->getSignal()(0));
  //  //  }

}

