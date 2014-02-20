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
 * Contact: thorsten.schindler@mytum.de
 *          rzander@users.berlios.de
 */

#include <config.h>
#include "finite_element_1s_21_ancf.h"
#include <fstream>

using namespace std;
using namespace fmatvec;

namespace MBSimFlexibleBody {

  inline double Power(double base, double exponent) {
    return pow(base,exponent);
  }

  FiniteElement1s21ANCF::FiniteElement1s21ANCF() {}

  FiniteElement1s21ANCF::FiniteElement1s21ANCF(double sl0, double sArho, double sEA, double sEI, Vec sg)
    :l0(sl0), Arho(sArho), EA(sEA), EI(sEI), wss0(0.0), depsilon(0.0), g(sg),
    M(8,INIT,0.), h(8,INIT,0.), Damp(8,INIT,0.0), Dhq(8,INIT,0.), Dhqp(8,INIT,0.)
  {
    l0h2 = l0*l0;
  }

  FiniteElement1s21ANCF::~FiniteElement1s21ANCF() {
  }

  /*void FiniteElement1s21ANCF::setCurleRadius(double R)
    {
    if (R == 0.0) { throw runtime_error("CurleRadius muss ungleich 0 sein!\n"); }
    wss0 = 1/R;
    }
    void FiniteElement1s21ANCF::setMaterialDamping(double depsilons)//, const double& dws)
    {
    depsilon  = depsilons;
    Damp(3,3) = -depsilon;
    }
    void FiniteElement1s21ANCF::setLehrDamping(double D)
    {
  // Longitudinaleifenfrequenz
  double weps = sqrt(12.*EA/(Arho*l0h2));
  // Biegeeigenfrequenz
  //    double wbL  = sqrt(1260.*128.*EI/(197.*Arho*l0h4));

  depsilon  = 1.0 * D * weps * (     Arho*l0h3/12.  );
  Damp(3,3) = -depsilon;
  //    dw       = 0.0 * D * wbL  * (197.*Arho*l0h3/1260.);

  }*/

  void FiniteElement1s21ANCF::computeM(const Vec& qElement) {
    M(0,0) = (13*Arho*l0)/35.;
    M(0,1) = 0;
    M(0,2) = (11*Arho*Power(l0,2))/210.;
    M(0,3) = 0;
    M(0,4) = (9*Arho*l0)/70.;
    M(0,5) = 0;
    M(0,6) = (-13*Arho*Power(l0,2))/420.;
    M(0,7) = 0;
    //M(1,0) = M(0,1);//Symmetrie
    M(1,1) = (13*Arho*l0)/35.;
    M(1,2) = 0;
    M(1,3) = (11*Arho*Power(l0,2))/210.;
    M(1,4) = 0;
    M(1,5) = (9*Arho*l0)/70.;
    M(1,6) = 0;
    M(1,7) = (-13*Arho*Power(l0,2))/420.;
    //M(2,0) = M(0,2);//Symmetrie
    //M(2,1) = M(1,2);//Symmetrie
    M(2,2) = (Arho*Power(l0,3))/105.;
    M(2,3) = 0;
    M(2,4) = (13*Arho*Power(l0,2))/420.;
    M(2,5) = 0;
    M(2,6) = -(Arho*Power(l0,3))/140.;
    M(2,7) = 0;
    //M(3,0) = M(0,3);//Symmetrie
    //M(3,1) = M(1,3);//Symmetrie
    //M(3,2) = M(2,3);//Symmetrie
    M(3,3) = (Arho*Power(l0,3))/105.;
    M(3,4) = 0;
    M(3,5) = (13*Arho*Power(l0,2))/420.;
    M(3,6) = 0;
    M(3,7) = -(Arho*Power(l0,3))/140.;
    //M(4,0) = M(0,4);//Symmetrie
    //M(4,1) = M(1,4);//Symmetrie
    //M(4,2) = M(2,4);//Symmetrie
    //M(4,3) = M(3,4);//Symmetrie
    M(4,4) = (13*Arho*l0)/35.;
    M(4,5) = 0;
    M(4,6) = (-11*Arho*Power(l0,2))/210.;
    M(4,7) = 0;
    //M(5,0) = M(0,5);//Symmetrie
    //M(5,1) = M(1,5);//Symmetrie
    //M(5,2) = M(2,5);//Symmetrie
    //M(5,3) = M(3,5);//Symmetrie
    //M(5,4) = M(4,5);//Symmetrie
    M(5,5) = (13*Arho*l0)/35.;
    M(5,6) = 0;
    M(5,7) = (-11*Arho*Power(l0,2))/210.;
    //M(6,0) = M(0,6);//Symmetrie
    //M(6,1) = M(1,6);//Symmetrie
    //M(6,2) = M(2,6);//Symmetrie
    //M(6,3) = M(3,6);//Symmetrie
    //M(6,4) = M(4,6);//Symmetrie
    //M(6,5) = M(5,6);//Symmetrie
    M(6,6) = (Arho*Power(l0,3))/105.;
    M(6,7) = 0;
    //M(7,0) = M(0,7);//Symmetrie
    //M(7,1) = M(1,7);//Symmetrie
    //M(7,2) = M(2,7);//Symmetrie
    //M(7,3) = M(3,7);//Symmetrie
    //M(7,4) = M(4,7);//Symmetrie
    //M(7,5) = M(5,7);//Symmetrie
    //M(7,6) = M(6,7);//Symmetrie
    M(7,7) = (Arho*Power(l0,3))/105.;
  }

  void FiniteElement1s21ANCF::computeh(const Vec& qElement, const Vec& qpElement) {
    //---  Koordinaten, Geschwindigkeiten
    const double & x1 = qElement(0);    const double & y1 = qElement(1);
    const double &dx1 = qElement(2);    const double &dy1 = qElement(3);
    const double & x2 = qElement(4);    const double & y2 = qElement(5);
    const double &dx2 = qElement(6);    const double &dy2 = qElement(7);

    //---  Gravitation
    static double gx = g(0);
    static double gy = g(1);

    double lh1     =      (Power(x1 - x2,2) + Power(y1 - y2,2))  ;
    double sqrtlh1 = sqrt(lh1);
    double lh2     = lh1*lh1;

    //h-Vektor
    h(0) = (Arho*gx*l0)/2. - ((120*Power(dy1,2)*EI*(x1 - x2) + dy1*(120*dy2*EI*(x1 - x2) - (120*dx1*EI + 60*dx2*EI - 4*dx1*EA*l0h2 + dx2*EA*l0h2 - 6*EA*l0*x1 + 6*EA*l0*x2)*(y1 - y2)) + (EA*(l0h2*((30 + 4*Power(dx1,2) - 2*dx1*dx2 + 4*Power(dx2,2))*x1 - 2*(15 + 2*Power(dx1,2) - dx1*dx2 + 2*Power(dx2,2))*x2 - (dx1 - 4*dx2)*dy2*(y1 - y2))*sqrtlh1 + 72*(x1 - x2)*Power(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2),1.5) - 3*l0*(30*Power(x1,3) - 30*Power(x2,3) - 3*Power(x1,2)*(30*x2 + (dx1 + dx2)*sqrtlh1) + 2*x1*(45*Power(x2,2) + 3*(dx1 + dx2)*x2*sqrtlh1 + (15*y1 - dy2*sqrtlh1 - 15*y2)*(y1 - y2)) - 3*(dx1 + dx2)*Power(x2,2)*sqrtlh1 - 2*x2*(15*y1 - dy2*sqrtlh1 - 15*y2)*(y1 - y2) - (dx1 + dx2)*sqrtlh1*Power(y1 - y2,2))) + 60*dy2*EI*(2*dy2*(x1 - x2) - (dx1 + 2*dx2)*(y1 - y2))*sqrtlh1)/sqrtlh1)*(lh1) - 2*(x1 - x2)*(15*EA*l0h2*Power(x1,2) + 2*Power(dx1,2)*EA*l0h2*Power(x1,2) - dx1*dx2*EA*l0h2*Power(x1,2) + 2*Power(dx2,2)*EA*l0h2*Power(x1,2) + 3*dx1*EA*l0*Power(x1,3) + 3*dx2*EA*l0*Power(x1,3) + 18*EA*Power(x1,4) - 30*EA*l0h2*x1*x2 - 4*Power(dx1,2)*EA*l0h2*x1*x2 + 2*dx1*dx2*EA*l0h2*x1*x2 - 4*Power(dx2,2)*EA*l0h2*x1*x2 - 9*dx1*EA*l0*Power(x1,2)*x2 - 9*dx2*EA*l0*Power(x1,2)*x2 - 72*EA*Power(x1,3)*x2 + 15*EA*l0h2*Power(x2,2) + 2*Power(dx1,2)*EA*l0h2*Power(x2,2) - dx1*dx2*EA*l0h2*Power(x2,2) + 2*Power(dx2,2)*EA*l0h2*Power(x2,2) + 9*dx1*EA*l0*x1*Power(x2,2) + 9*dx2*EA*l0*x1*Power(x2,2) + 108*EA*Power(x1,2)*Power(x2,2) - 3*dx1*EA*l0*Power(x2,3) - 3*dx2*EA*l0*Power(x2,3) - 72*EA*x1*Power(x2,3) + 18*EA*Power(x2,4) + 60*Power(dx1,2)*EI*Power(y1,2) + 60*dx1*dx2*EI*Power(y1,2) + 60*Power(dx2,2)*EI*Power(y1,2) + 15*EA*l0h2*Power(y1,2) + 3*dx1*EA*l0*x1*Power(y1,2) + 3*dx2*EA*l0*x1*Power(y1,2) + 36*EA*Power(x1,2)*Power(y1,2) - 3*dx1*EA*l0*x2*Power(y1,2) - 3*dx2*EA*l0*x2*Power(y1,2) - 72*EA*x1*x2*Power(y1,2) + 36*EA*Power(x2,2)*Power(y1,2) + 18*EA*Power(y1,4) + dy1*(dy2*(60*EI*Power(x1 - x2,2) - EA*l0h2*Power(y1 - y2,2)) + (-4*dx1*(30*EI - EA*l0h2)*(x1 - x2) - dx2*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2)) - 30*EA*l0*Power(x1,2)*sqrtlh1 + 60*EA*l0*x1*x2*sqrtlh1 - 30*EA*l0*Power(x2,2)*sqrtlh1 - 30*EA*l0*Power(y1,2)*sqrtlh1 + 2*Power(dy1,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + 2*Power(dy2,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + dy2*(-4*dx2*(30*EI - EA*l0h2)*(x1 - x2) - dx1*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2) - 120*Power(dx1,2)*EI*y1*y2 - 120*dx1*dx2*EI*y1*y2 - 120*Power(dx2,2)*EI*y1*y2 - 30*EA*l0h2*y1*y2 - 6*dx1*EA*l0*x1*y1*y2 - 6*dx2*EA*l0*x1*y1*y2 - 72*EA*Power(x1,2)*y1*y2 + 6*dx1*EA*l0*x2*y1*y2 + 6*dx2*EA*l0*x2*y1*y2 + 144*EA*x1*x2*y1*y2 - 72*EA*Power(x2,2)*y1*y2 - 72*EA*Power(y1,3)*y2 + 60*EA*l0*y1*sqrtlh1*y2 + 60*Power(dx1,2)*EI*Power(y2,2) + 60*dx1*dx2*EI*Power(y2,2) + 60*Power(dx2,2)*EI*Power(y2,2) + 15*EA*l0h2*Power(y2,2) + 3*dx1*EA*l0*x1*Power(y2,2) + 3*dx2*EA*l0*x1*Power(y2,2) + 36*EA*Power(x1,2)*Power(y2,2) - 3*dx1*EA*l0*x2*Power(y2,2) - 3*dx2*EA*l0*x2*Power(y2,2) - 72*EA*x1*x2*Power(y2,2) + 36*EA*Power(x2,2)*Power(y2,2) + 108*EA*Power(y1,2)*Power(y2,2) - 30*EA*l0*sqrtlh1*Power(y2,2) - 72*EA*y1*Power(y2,3) + 18*EA*Power(y2,4)))/(30.*l0*lh2);
    h(1) = (Arho*gy*l0)/2. - (-2*(y1 - y2)*(15*EA*l0h2*Power(x1,2) + 2*Power(dx1,2)*EA*l0h2*Power(x1,2) - dx1*dx2*EA*l0h2*Power(x1,2) + 2*Power(dx2,2)*EA*l0h2*Power(x1,2) + 3*dx1*EA*l0*Power(x1,3) + 3*dx2*EA*l0*Power(x1,3) + 18*EA*Power(x1,4) - 30*EA*l0h2*x1*x2 - 4*Power(dx1,2)*EA*l0h2*x1*x2 + 2*dx1*dx2*EA*l0h2*x1*x2 - 4*Power(dx2,2)*EA*l0h2*x1*x2 - 9*dx1*EA*l0*Power(x1,2)*x2 - 9*dx2*EA*l0*Power(x1,2)*x2 - 72*EA*Power(x1,3)*x2 + 15*EA*l0h2*Power(x2,2) + 2*Power(dx1,2)*EA*l0h2*Power(x2,2) - dx1*dx2*EA*l0h2*Power(x2,2) + 2*Power(dx2,2)*EA*l0h2*Power(x2,2) + 9*dx1*EA*l0*x1*Power(x2,2) + 9*dx2*EA*l0*x1*Power(x2,2) + 108*EA*Power(x1,2)*Power(x2,2) - 3*dx1*EA*l0*Power(x2,3) - 3*dx2*EA*l0*Power(x2,3) - 72*EA*x1*Power(x2,3) + 18*EA*Power(x2,4) + 60*Power(dx1,2)*EI*Power(y1,2) + 60*dx1*dx2*EI*Power(y1,2) + 60*Power(dx2,2)*EI*Power(y1,2) + 15*EA*l0h2*Power(y1,2) + 3*dx1*EA*l0*x1*Power(y1,2) + 3*dx2*EA*l0*x1*Power(y1,2) + 36*EA*Power(x1,2)*Power(y1,2) - 3*dx1*EA*l0*x2*Power(y1,2) - 3*dx2*EA*l0*x2*Power(y1,2) - 72*EA*x1*x2*Power(y1,2) + 36*EA*Power(x2,2)*Power(y1,2) + 18*EA*Power(y1,4) + dy1*(dy2*(60*EI*Power(x1 - x2,2) - EA*l0h2*Power(y1 - y2,2)) + (-4*dx1*(30*EI - EA*l0h2)*(x1 - x2) - dx2*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2)) - 30*EA*l0*Power(x1,2)*sqrtlh1 + 60*EA*l0*x1*x2*sqrtlh1 - 30*EA*l0*Power(x2,2)*sqrtlh1 - 30*EA*l0*Power(y1,2)*sqrtlh1 + 2*Power(dy1,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + 2*Power(dy2,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + dy2*(-4*dx2*(30*EI - EA*l0h2)*(x1 - x2) - dx1*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2) - 120*Power(dx1,2)*EI*y1*y2 - 120*dx1*dx2*EI*y1*y2 - 120*Power(dx2,2)*EI*y1*y2 - 30*EA*l0h2*y1*y2 - 6*dx1*EA*l0*x1*y1*y2 - 6*dx2*EA*l0*x1*y1*y2 - 72*EA*Power(x1,2)*y1*y2 + 6*dx1*EA*l0*x2*y1*y2 + 6*dx2*EA*l0*x2*y1*y2 + 144*EA*x1*x2*y1*y2 - 72*EA*Power(x2,2)*y1*y2 - 72*EA*Power(y1,3)*y2 + 60*EA*l0*y1*sqrtlh1*y2 + 60*Power(dx1,2)*EI*Power(y2,2) + 60*dx1*dx2*EI*Power(y2,2) + 60*Power(dx2,2)*EI*Power(y2,2) + 15*EA*l0h2*Power(y2,2) + 3*dx1*EA*l0*x1*Power(y2,2) + 3*dx2*EA*l0*x1*Power(y2,2) + 36*EA*Power(x1,2)*Power(y2,2) - 3*dx1*EA*l0*x2*Power(y2,2) - 3*dx2*EA*l0*x2*Power(y2,2) - 72*EA*x1*x2*Power(y2,2) + 36*EA*Power(x2,2)*Power(y2,2) + 108*EA*Power(y1,2)*Power(y2,2) - 30*EA*l0*sqrtlh1*Power(y2,2) - 72*EA*y1*Power(y2,3) + 18*EA*Power(y2,4)) + (lh1)*(dx1*(-4*dy1*(30*EI - EA*l0h2)*(x1 - x2) - dy2*(60*EI + EA*l0h2)*(x1 - x2) + 6*(20*dx2*EI + EA*l0*(x1 - x2))*(y1 - y2)) + 120*Power(dx1,2)*EI*(y1 - y2) + (-60*dx2*EI*sqrtlh1*(dy1*x1 + 2*dy2*x1 - dy1*x2 - 2*dy2*x2 - 2*dx2*y1 + 2*dx2*y2) + EA*(l0h2*(-(dx2*(dy1 - 4*dy2)*(x1 - x2)) + 2*(15 + 2*Power(dy1,2) - dy1*dy2 + 2*Power(dy2,2))*(y1 - y2))*sqrtlh1 + 72*Power(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2),1.5)*(y1 - y2) - 3*l0*(Power(x1,2)*(30*y1 - dy1*sqrtlh1 - dy2*sqrtlh1 - 30*y2) + Power(x2,2)*(30*y1 - dy1*sqrtlh1 - dy2*sqrtlh1 - 30*y2) + 2*dx2*x2*sqrtlh1*(y1 - y2) + 3*(10*y1 - dy1*sqrtlh1 - dy2*sqrtlh1 - 10*y2)*Power(y1 - y2,2) + 2*x1*(dx2*sqrtlh1*(-y1 + y2) + x2*(-30*y1 + dy1*sqrtlh1 + dy2*sqrtlh1 + 30*y2)))))/sqrtlh1))/(30.*l0*lh2);
    h(2) = (Arho*gx*l0h2)/12. - ((x1 - x2)*(EA*l0*(3*Power(x1,2) - 6*x1*x2 + 3*Power(x2,2) + (4*dy1*l0 - dy2*l0 + 3*y1 - 3*y2)*(y1 - y2)) - 60*(2*dy1 + dy2)*EI*(y1 - y2)) - dx2*(EA*l0h2*Power(x1 - x2,2) - 60*EI*Power(y1 - y2,2)) + 4*dx1*(EA*l0h2*Power(x1 - x2,2) + 30*EI*Power(y1 - y2,2)))/(30.*l0*(lh1));
    h(3) = (Arho*gy*l0h2)/12. - (dy2*(60*EI*Power(x1 - x2,2) - EA*l0h2*Power(y1 - y2,2)) + 4*dy1*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + (-4*dx1*(30*EI - EA*l0h2)*(x1 - x2) - dx2*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2))/(30.*l0*(lh1));
    h(4) = (Arho*gx*l0)/2. - ((-120*Power(dy1,2)*EI*(x1 - x2) + dy1*(-120*dy2*EI*(x1 - x2) + (120*dx1*EI + 60*dx2*EI - 4*dx1*EA*l0h2 + dx2*EA*l0h2 - 6*EA*l0*x1 + 6*EA*l0*x2)*(y1 - y2)) + (-(EA*(l0h2*((30 + 4*Power(dx1,2) - 2*dx1*dx2 + 4*Power(dx2,2))*x1 - 2*(15 + 2*Power(dx1,2) - dx1*dx2 + 2*Power(dx2,2))*x2 - (dx1 - 4*dx2)*dy2*(y1 - y2))*sqrtlh1 + 72*(x1 - x2)*Power(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2),1.5) - 3*l0*(30*Power(x1,3) - 30*Power(x2,3) - 3*Power(x1,2)*(30*x2 + (dx1 + dx2)*sqrtlh1) + 2*x1*(45*Power(x2,2) + 3*(dx1 + dx2)*x2*sqrtlh1 + (15*y1 - dy2*sqrtlh1 - 15*y2)*(y1 - y2)) - 3*(dx1 + dx2)*Power(x2,2)*sqrtlh1 - 2*x2*(15*y1 - dy2*sqrtlh1 - 15*y2)*(y1 - y2) - (dx1 + dx2)*sqrtlh1*Power(y1 - y2,2)))) - 60*dy2*EI*(2*dy2*(x1 - x2) - (dx1 + 2*dx2)*(y1 - y2))*sqrtlh1)/sqrtlh1)*(lh1) + 2*(x1 - x2)*(15*EA*l0h2*Power(x1,2) + 2*Power(dx1,2)*EA*l0h2*Power(x1,2) - dx1*dx2*EA*l0h2*Power(x1,2) + 2*Power(dx2,2)*EA*l0h2*Power(x1,2) + 3*dx1*EA*l0*Power(x1,3) + 3*dx2*EA*l0*Power(x1,3) + 18*EA*Power(x1,4) - 30*EA*l0h2*x1*x2 - 4*Power(dx1,2)*EA*l0h2*x1*x2 + 2*dx1*dx2*EA*l0h2*x1*x2 - 4*Power(dx2,2)*EA*l0h2*x1*x2 - 9*dx1*EA*l0*Power(x1,2)*x2 - 9*dx2*EA*l0*Power(x1,2)*x2 - 72*EA*Power(x1,3)*x2 + 15*EA*l0h2*Power(x2,2) + 2*Power(dx1,2)*EA*l0h2*Power(x2,2) - dx1*dx2*EA*l0h2*Power(x2,2) + 2*Power(dx2,2)*EA*l0h2*Power(x2,2) + 9*dx1*EA*l0*x1*Power(x2,2) + 9*dx2*EA*l0*x1*Power(x2,2) + 108*EA*Power(x1,2)*Power(x2,2) - 3*dx1*EA*l0*Power(x2,3) - 3*dx2*EA*l0*Power(x2,3) - 72*EA*x1*Power(x2,3) + 18*EA*Power(x2,4) + 60*Power(dx1,2)*EI*Power(y1,2) + 60*dx1*dx2*EI*Power(y1,2) + 60*Power(dx2,2)*EI*Power(y1,2) + 15*EA*l0h2*Power(y1,2) + 3*dx1*EA*l0*x1*Power(y1,2) + 3*dx2*EA*l0*x1*Power(y1,2) + 36*EA*Power(x1,2)*Power(y1,2) - 3*dx1*EA*l0*x2*Power(y1,2) - 3*dx2*EA*l0*x2*Power(y1,2) - 72*EA*x1*x2*Power(y1,2) + 36*EA*Power(x2,2)*Power(y1,2) + 18*EA*Power(y1,4) + dy1*(dy2*(60*EI*Power(x1 - x2,2) - EA*l0h2*Power(y1 - y2,2)) + (-4*dx1*(30*EI - EA*l0h2)*(x1 - x2) - dx2*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2)) - 30*EA*l0*Power(x1,2)*sqrtlh1 + 60*EA*l0*x1*x2*sqrtlh1 - 30*EA*l0*Power(x2,2)*sqrtlh1 - 30*EA*l0*Power(y1,2)*sqrtlh1 + 2*Power(dy1,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + 2*Power(dy2,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + dy2*(-4*dx2*(30*EI - EA*l0h2)*(x1 - x2) - dx1*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2) - 120*Power(dx1,2)*EI*y1*y2 - 120*dx1*dx2*EI*y1*y2 - 120*Power(dx2,2)*EI*y1*y2 - 30*EA*l0h2*y1*y2 - 6*dx1*EA*l0*x1*y1*y2 - 6*dx2*EA*l0*x1*y1*y2 - 72*EA*Power(x1,2)*y1*y2 + 6*dx1*EA*l0*x2*y1*y2 + 6*dx2*EA*l0*x2*y1*y2 + 144*EA*x1*x2*y1*y2 - 72*EA*Power(x2,2)*y1*y2 - 72*EA*Power(y1,3)*y2 + 60*EA*l0*y1*sqrtlh1*y2 + 60*Power(dx1,2)*EI*Power(y2,2) + 60*dx1*dx2*EI*Power(y2,2) + 60*Power(dx2,2)*EI*Power(y2,2) + 15*EA*l0h2*Power(y2,2) + 3*dx1*EA*l0*x1*Power(y2,2) + 3*dx2*EA*l0*x1*Power(y2,2) + 36*EA*Power(x1,2)*Power(y2,2) - 3*dx1*EA*l0*x2*Power(y2,2) - 3*dx2*EA*l0*x2*Power(y2,2) - 72*EA*x1*x2*Power(y2,2) + 36*EA*Power(x2,2)*Power(y2,2) + 108*EA*Power(y1,2)*Power(y2,2) - 30*EA*l0*sqrtlh1*Power(y2,2) - 72*EA*y1*Power(y2,3) + 18*EA*Power(y2,4)))/(30.*l0*lh2);
    h(5) = (Arho*gy*l0)/2. - (2*(y1 - y2)*(15*EA*l0h2*Power(x1,2) + 2*Power(dx1,2)*EA*l0h2*Power(x1,2) - dx1*dx2*EA*l0h2*Power(x1,2) + 2*Power(dx2,2)*EA*l0h2*Power(x1,2) + 3*dx1*EA*l0*Power(x1,3) + 3*dx2*EA*l0*Power(x1,3) + 18*EA*Power(x1,4) - 30*EA*l0h2*x1*x2 - 4*Power(dx1,2)*EA*l0h2*x1*x2 + 2*dx1*dx2*EA*l0h2*x1*x2 - 4*Power(dx2,2)*EA*l0h2*x1*x2 - 9*dx1*EA*l0*Power(x1,2)*x2 - 9*dx2*EA*l0*Power(x1,2)*x2 - 72*EA*Power(x1,3)*x2 + 15*EA*l0h2*Power(x2,2) + 2*Power(dx1,2)*EA*l0h2*Power(x2,2) - dx1*dx2*EA*l0h2*Power(x2,2) + 2*Power(dx2,2)*EA*l0h2*Power(x2,2) + 9*dx1*EA*l0*x1*Power(x2,2) + 9*dx2*EA*l0*x1*Power(x2,2) + 108*EA*Power(x1,2)*Power(x2,2) - 3*dx1*EA*l0*Power(x2,3) - 3*dx2*EA*l0*Power(x2,3) - 72*EA*x1*Power(x2,3) + 18*EA*Power(x2,4) + 60*Power(dx1,2)*EI*Power(y1,2) + 60*dx1*dx2*EI*Power(y1,2) + 60*Power(dx2,2)*EI*Power(y1,2) + 15*EA*l0h2*Power(y1,2) + 3*dx1*EA*l0*x1*Power(y1,2) + 3*dx2*EA*l0*x1*Power(y1,2) + 36*EA*Power(x1,2)*Power(y1,2) - 3*dx1*EA*l0*x2*Power(y1,2) - 3*dx2*EA*l0*x2*Power(y1,2) - 72*EA*x1*x2*Power(y1,2) + 36*EA*Power(x2,2)*Power(y1,2) + 18*EA*Power(y1,4) + dy1*(dy2*(60*EI*Power(x1 - x2,2) - EA*l0h2*Power(y1 - y2,2)) + (-4*dx1*(30*EI - EA*l0h2)*(x1 - x2) - dx2*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2)) - 30*EA*l0*Power(x1,2)*sqrtlh1 + 60*EA*l0*x1*x2*sqrtlh1 - 30*EA*l0*Power(x2,2)*sqrtlh1 - 30*EA*l0*Power(y1,2)*sqrtlh1 + 2*Power(dy1,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + 2*Power(dy2,2)*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + dy2*(-4*dx2*(30*EI - EA*l0h2)*(x1 - x2) - dx1*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2) - 120*Power(dx1,2)*EI*y1*y2 - 120*dx1*dx2*EI*y1*y2 - 120*Power(dx2,2)*EI*y1*y2 - 30*EA*l0h2*y1*y2 - 6*dx1*EA*l0*x1*y1*y2 - 6*dx2*EA*l0*x1*y1*y2 - 72*EA*Power(x1,2)*y1*y2 + 6*dx1*EA*l0*x2*y1*y2 + 6*dx2*EA*l0*x2*y1*y2 + 144*EA*x1*x2*y1*y2 - 72*EA*Power(x2,2)*y1*y2 - 72*EA*Power(y1,3)*y2 + 60*EA*l0*y1*sqrtlh1*y2 + 60*Power(dx1,2)*EI*Power(y2,2) + 60*dx1*dx2*EI*Power(y2,2) + 60*Power(dx2,2)*EI*Power(y2,2) + 15*EA*l0h2*Power(y2,2) + 3*dx1*EA*l0*x1*Power(y2,2) + 3*dx2*EA*l0*x1*Power(y2,2) + 36*EA*Power(x1,2)*Power(y2,2) - 3*dx1*EA*l0*x2*Power(y2,2) - 3*dx2*EA*l0*x2*Power(y2,2) - 72*EA*x1*x2*Power(y2,2) + 36*EA*Power(x2,2)*Power(y2,2) + 108*EA*Power(y1,2)*Power(y2,2) - 30*EA*l0*sqrtlh1*Power(y2,2) - 72*EA*y1*Power(y2,3) + 18*EA*Power(y2,4)) + (lh1)*(dx1*(4*dy1*(30*EI - EA*l0h2)*(x1 - x2) + dy2*(60*EI + EA*l0h2)*(x1 - x2) - 6*(20*dx2*EI + EA*l0*(x1 - x2))*(y1 - y2)) - 120*Power(dx1,2)*EI*(y1 - y2) + (60*dx2*EI*sqrtlh1*(dy1*x1 + 2*dy2*x1 - dy1*x2 - 2*dy2*x2 - 2*dx2*y1 + 2*dx2*y2) - EA*(l0h2*(-(dx2*(dy1 - 4*dy2)*(x1 - x2)) + 2*(15 + 2*Power(dy1,2) - dy1*dy2 + 2*Power(dy2,2))*(y1 - y2))*sqrtlh1 + 72*Power(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2),1.5)*(y1 - y2) - 3*l0*(Power(x1,2)*(30*y1 - dy1*sqrtlh1 - dy2*sqrtlh1 - 30*y2) + Power(x2,2)*(30*y1 - dy1*sqrtlh1 - dy2*sqrtlh1 - 30*y2) + 2*dx2*x2*sqrtlh1*(y1 - y2) + 3*(10*y1 - dy1*sqrtlh1 - dy2*sqrtlh1 - 10*y2)*Power(y1 - y2,2) + 2*x1*(dx2*sqrtlh1*(-y1 + y2) + x2*(-30*y1 + dy1*sqrtlh1 + dy2*sqrtlh1 + 30*y2)))))/sqrtlh1))/(30.*l0*lh2);
    h(6) = -(Arho*gx*l0h2)/12. - ((x1 - x2)*(EA*l0*(3*Power(x1,2) - 6*x1*x2 + 3*Power(x2,2) + (-(dy1*l0) + 4*dy2*l0 + 3*y1 - 3*y2)*(y1 - y2)) - 60*(dy1 + 2*dy2)*EI*(y1 - y2)) - dx1*(EA*l0h2*Power(x1 - x2,2) - 60*EI*Power(y1 - y2,2)) + 4*dx2*(EA*l0h2*Power(x1 - x2,2) + 30*EI*Power(y1 - y2,2)))/(30.*l0*(lh1));
    h(7) = -(Arho*gy*l0h2)/12. - (dy1*(60*EI*Power(x1 - x2,2) - EA*l0h2*Power(y1 - y2,2)) + 4*dy2*(30*EI*Power(x1 - x2,2) + EA*l0h2*Power(y1 - y2,2)) + (-4*dx2*(30*EI - EA*l0h2)*(x1 - x2) - dx1*(60*EI + EA*l0h2)*(x1 - x2) + 3*EA*l0*(Power(x1,2) - 2*x1*x2 + Power(x2,2) + Power(y1 - y2,2)))*(y1 - y2))/(30.*l0*(lh1));

    // // Daempfung------------------------------------------------------------------
    // //    hdLokal.init(0.0); beim Initialisieren
    //     hdLokal(3) = - depsilon * epsp; // eps

    //     // Impliziten Integratoren
    //     if(implicit)
    //     {
    // 	static Mat Dhz(2*8,8,fmatvec::INIT,0.0);
    // 	Dhz   = hFullJacobi(qElement,qpElement,qLokal,qpLokal,Jeg,Jegp,MLokal,hZwischen);
    // 	Dhq   = static_cast<SqrMat>(Dhz(0,0, 8-1,8-1));
    // 	Dhqp  = static_cast<SqrMat>(Dhz(8,0,2*8-1,8-1));
    // 	Dhqp += trans(Jeg)*Damp*Jeg;
    //     }

    //     cout << "qElement = " << trans(qElement);
    //     cout << "h        = " << trans(h)        << endl;
    //     throw 1;

  }

  // Balkenort ermitteln aus globalen Lagen 
  Vec FiniteElement1s21ANCF::LocateBalken(Vec& qElement, double& s) {
    Mat S = JGeneralized(qElement,s).T();
    return S*qElement;
  }

  Vec FiniteElement1s21ANCF::StateBalken(Vec& qElement, Vec& qpElement, double& s) {
    Mat S = JGeneralized(qElement,s).T();
    Vec X(6);
    X(Index(0,2)) = S* qElement;
    X(Index(3,5)) = S*qpElement;

    return X;
  }

  //  Vec FiniteElement1s21ANCF::Tangente(Vec& qElement, double& s) {
  //    double xi = s/l0;
  //
  //    double & x1 = qElement(0);    double & y1 = qElement(1);
  //    double &dx1 = qElement(2);    double &dy1 = qElement(3);
  //    double & x2 = qElement(4);    double & y2 = qElement(5);
  //    double &dx2 = qElement(6);    double &dy2 = qElement(7);
  //
  //    Vec t(2);
  //    //Tangente                                                                                  // wird gemeinsam unten normiert...
  //    t(0) = (dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)));///(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2));
  //    t(1) = (dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)));///(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2));
  //
  //    t /= nrm2(t);
  //
  //    return t;
  //  }

  Mat FiniteElement1s21ANCF::JGeneralized(const Vec& qElement, const double& s) {
    Mat J(8,3,INIT,0.0);
    double xi = s/l0;

    const double & x1 = qElement(0);    const double & y1 = qElement(1);
    const double &dx1 = qElement(2);    const double &dy1 = qElement(3);
    const double & x2 = qElement(4);    const double & y2 = qElement(5);
    const double &dx2 = qElement(6);    const double &dy2 = qElement(7);

    //Jacobimatrix
    J(0,0) = 1 - 3*Power(xi,2) + 2*Power(xi,3);
    J(0,1) = 0;
    J(0,2) = (6*(-1 + xi)*xi*(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2))))/(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2));
    J(1,0) = 0;
    J(1,1) = 1 - 3*Power(xi,2) + 2*Power(xi,3);
    J(1,2) = (6*(-1 + xi)*xi*(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)))*(-Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) - Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2)))/Power(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2),2);
    J(2,0) = l0*(xi - 2*Power(xi,2) + Power(xi,3));
    J(2,1) = 0;
    J(2,2) = (l0*(-1 + xi)*(-1 + 3*xi)*(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2))))/(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2));
    J(3,0) = 0;
    J(3,1) = l0*(xi - 2*Power(xi,2) + Power(xi,3));
    J(3,2) = (l0*(-1 + xi)*(-1 + 3*xi)*(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)))*(-Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) - Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2)))/Power(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2),2);
    J(4,0) = 3*Power(xi,2) - 2*Power(xi,3);
    J(4,1) = 0;
    J(4,2) = (6*(-1 + xi)*xi*(-Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) - Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2))*(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2))))/Power(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2),2);
    J(5,0) = 0;
    J(5,1) = 3*Power(xi,2) - 2*Power(xi,3);
    J(5,2) = (6*(-1 + xi)*xi*(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi))))/(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2));
    J(6,0) = l0*(-Power(xi,2) + Power(xi,3));
    J(6,1) = 0;
    J(6,2) = (l0*xi*(-2 + 3*xi)*(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2))))/(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2));
    J(7,0) = 0;
    J(7,1) = l0*(-Power(xi,2) + Power(xi,3));
    J(7,2) = (l0*xi*(-2 + 3*xi)*(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)))*(-Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) - Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2)))/Power(Power(dx1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(6*(x1 - x2)*(-1 + xi) + dx2*l0*(-2 + 3*xi)),2) + Power(dy1*l0*(-1 + xi)*(-1 + 3*xi) + xi*(dy2*l0*(-2 + 3*xi) + 6*(-1 + xi)*(y1 - y2)),2),2);

    return J;
  }
}

