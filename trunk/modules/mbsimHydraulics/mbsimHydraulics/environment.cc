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
#include <iostream>
#include "mbsimHydraulics/environment.h"
#include "mbsim/element.h"

using namespace std;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimHydraulics {

  const MBXMLUtils::NamespaceURI MBSIMHYDRAULICS("http://mbsim.berlios.de/MBSimHydraulics");

  MBSIM_OBJECTFACTORY_REGISTERXMLNAMEASSINGLETON(Environment, HydraulicEnvironment, MBSIMHYDRAULICS%"HydraulicEnvironment")

  HydraulicEnvironment *HydraulicEnvironment::instance=NULL;
      
  void HydraulicEnvironment::initializeUsingXML(DOMElement *element) {
    Environment::initializeUsingXML(element);
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"environmentPressure");
    setEnvironmentPressure(MBSim::Element::getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"specificMass");
    if (E(e->getFirstElementChild())->getTagName()==MBSIMHYDRAULICS%"constantSpecificMass")
      setConstantSpecificMass(MBSim::Element::getDouble(e->getFirstElementChild()));
    else if (E(e->getFirstElementChild())->getTagName()==MBSIMHYDRAULICS%"volumeDependingOnTemperature")
      setVolumeDependingOnTemperature(
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"dVdT")), 
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"basicSpecificMass")), 
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"basicTemperature"))
          );
    else if (E(e->getFirstElementChild())->getTagName()==MBSIMHYDRAULICS%"specificMassDependingOnTemperature")
      setSpecificMassDependingOnTemperature(
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"dRhodT")), 
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"basicSpecificMass")), 
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"basicTemperature"))
          );
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"kinematicViscosity");
    if (E(e->getFirstElementChild())->getTagName()==MBSIMHYDRAULICS%"constantKinematicViscosity")
      setConstantKinematicViscosity(MBSim::Element::getDouble(e->getFirstElementChild()));
    else if (E(e->getFirstElementChild())->getTagName()==MBSIMHYDRAULICS%"walterUbbeohdeKinematicViscosity") {
      setWalterUbbelohdeKinematicViscosity(
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"temperature1")), 
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"kinematicViscosity1")), 
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"temperature2")), 
          MBSim::Element::getDouble(E(e->getFirstElementChild())->getFirstElementChildNamed(MBSIMHYDRAULICS%"kinematicViscosity2"))
          );
    }
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"basicBulkModulus");
    setBasicBulkModulus(MBSim::Element::getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"kappa");
    setKappa(MBSim::Element::getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIMHYDRAULICS%"fluidTemperature");
    setTemperature(MBSim::Element::getDouble(e));
    initializeFluidData();
  }

  void HydraulicEnvironment::initializeFluidData() {
    rho=(this->*calcRho)(T);
    nu=(this->*calcNu)(T);
    cout << endl;
    cout << "===============================================" << endl;
    cout << "initializing hydraulic environment at T=" << T-273.16 << " [degC]" << endl;
    cout << "            with kinematic viscosity nu=" << nu*1e6 << " [mm^2/s]" << endl;
    cout << "                      specific mass rho=" << rho << " [kg/m^3]" << endl;
    cout << "                  dynamic viscosity eta=" << getDynamicViscosity()*1e3 << " [mPa*s]" << endl;
    cout << "                                  kappa=" << kappa << " [-]" << endl;
    cout << "                      boundary pressure=" << pinf*1e-5 << " [bar]" << endl;
    cout << "===============================================\n\n" << endl;
    cout << endl;
    assert(pinf>0);
    assert(rho>0);
    assert(E0>0);
    assert(kappa>0);
    assert(nu>0);
  }

  void HydraulicEnvironment::setConstantSpecificMass(double rho_) {
    rhoConstant=rho_;
    calcRho = &HydraulicEnvironment::calcConstantSpecificMass;
  }

  void HydraulicEnvironment::setVolumeDependingOnTemperature(double dVdT_, double rho0_, double T0_) {
    dVdT=dVdT_; 
    rho0=rho0_; 
    T0=T0_; 
    calcRho = &HydraulicEnvironment::calcVolumeDependingOnTemperature;
  }

  void HydraulicEnvironment::setSpecificMassDependingOnTemperature(double dRhodT_, double rho0_, double T0_) {
    dRhodT=dRhodT_; 
    rho0=rho0_; 
    T0=T0_; 
    calcRho = &HydraulicEnvironment::calcSpecificMassDependingOnTemperature;
  }

  void HydraulicEnvironment::setConstantKinematicViscosity(double nu_) {
    nuConstant=nu_;
    calcNu = &HydraulicEnvironment::calcConstantKinematicViscosity;
  }

  void HydraulicEnvironment::setWalterUbbelohdeKinematicViscosity(double T1, double nu1, double T2_, double nu2) {
    Tm=T1;
    Wm=log10(log10(nu1*1e6+0.8));  //Umrechnung in cSt
    T2=T2_;
    double W2=log10(log10(nu2*1e6+0.8));
    m=(Wm-W2)/(log10(T2)-log10(Tm));
    calcNu = &HydraulicEnvironment::calcWalterUbbelohdeKinematicViscosity;
  }

  double HydraulicEnvironment::calcWalterUbbelohdeKinematicViscosity(double T) {
    double Tx=T;
    double Wx=m*(log10(Tm)-log10(Tx))+Wm;
    return (pow(10,pow(10,Wx))-0.8)*1e-6; //Umrechnung zu m^2/s
  }

  double OilBulkModulus::operator()(const double &p) {
    if(p<=0.1) {
      cout << "OilBulkModulus of \"" << ownerName << "\": pressure near zero! Continuing anyway, using p=0.1 Pa" << endl;
      return factor[0]/(1.+factor[1]*pow(.1, factor[2]));
    }
    else {
      // Umdruck zur Vorlesung
      // Grundlagen der Oelhydraulik
      // W.Backe
      // H.Murrenhoff
      // 10. Auflage 1994
      // Formel (3-11), S. 103
      const double EBacke= factor[0]/(1.+factor[1]*pow(p, factor[2]));

      // Formel nach HYSIM
      // const double EHysim= HYSIM[0]*pow(1.+HYSIM[1]/p, HYSIM[3]) / (1. + HYSIM[2]/pow(p, 1+HYSIM[3]));

      // std::cerr << " " << p << " " << EBacke << " " << EHysim << std::endl;

      return EBacke;
    }
  }

}
