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
 * Contact: thschindler@users.berlios.de
 */

#include<config.h>
#define FMATVEC_NO_INITIALIZATION
#define FMATVEC_NO_BOUNDS_CHECK
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_translation.h"
#include "mbsim/utils/eps.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  FiniteElement1s33CosseratTranslation::FiniteElement1s33CosseratTranslation(double l0_, double rho_,double A_, double E_, double G_, double I1_, double I2_, double I0_, const Vec& g_) : l0(l0_), rho(rho_), A(A_), E(E_), G(G_), I1(I1_), I2(I2_), I0(I0_), g(g_), cEps0D(0.), sigma1(1.), sigma2(1.), M(9,INIT,0.), h(9,INIT,0.), X(12,INIT,0.) {}

  FiniteElement1s33CosseratTranslation::~FiniteElement1s33CosseratTranslation() {}

  void FiniteElement1s33CosseratTranslation::setMaterialDamping(double cEps0D_) {
    cEps0D = cEps0D_;
  }

  void FiniteElement1s33CosseratTranslation::computeM(const Vec& qG) {
    /* Cardan angles */
    double sbeta = sin(qG(4));
    double sgamma = sin(qG(5));
    double cbeta = cos(qG(4));
    double cgamma = cos(qG(5));

    /* transient mass matrix is just standard rigid body rotational part */
    M(3,3) = rho*l0*(I0*cbeta*cbeta*cgamma*cgamma + I1*cbeta*cbeta*sgamma*sgamma + I2*sbeta*sbeta);
    M(3,4) = rho*l0*(I0*cbeta*cgamma*sgamma - I1*cbeta*sgamma*cgamma);
    M(3,5) = rho*l0*I2*sbeta;
    M(4,4) = rho*l0*(I0*sgamma*sgamma + I1*cgamma*cgamma);
    M(5,5) = rho*l0*I2;
  }

  void FiniteElement1s33CosseratTranslation::computeh(const Vec& qG, const Vec& qGt) {
    /* Cardan angles */
    double salpha = sin(qG(3));
    double sbeta = sin(qG(4));
    double sgamma = sin(qG(5));
    double calpha = cos(qG(3));
    double cbeta = cos(qG(4));
    double cgamma = cos(qG(5));
    
    //const double &alphat = qGt(3);
    //const double &betat = qGt(4);
    //const double &gammat = qGt(5);

    Vec tangent(3);
    tangent(0) = cbeta*cgamma;
    tangent(1) = calpha*sgamma+salpha*sbeta*cgamma;
    tangent(2) = salpha*sgamma-calpha*sbeta*cgamma;

    SqrMat dtangentdphi(3);
    dtangentdphi(0,0) = 0.;
    dtangentdphi(0,1) = -sbeta*cgamma;
    dtangentdphi(0,2) = -cbeta*sgamma;
    dtangentdphi(1,0) = -salpha*sgamma+calpha*sbeta*cgamma; 
    dtangentdphi(1,1) = salpha*cbeta*cgamma;
    dtangentdphi(1,2) = calpha*cgamma-salpha*sbeta*sgamma;
    dtangentdphi(2,0) = calpha*sgamma+salpha*sbeta*cgamma;
    dtangentdphi(2,1) = -calpha*cbeta*cgamma;
    dtangentdphi(2,2) = salpha*cgamma+calpha*sbeta*sgamma;

    Vec tangentt = dtangentdphi*qGt(3,5);
    
    Vec normal(3);
    normal(0) = -sgamma*cbeta;
    normal(1) = cgamma*calpha-salpha*sbeta*sgamma;
    normal(2) = salpha*cgamma+calpha*sbeta*sgamma;

    SqrMat dnormaldphi(3);
    dnormaldphi(0,0)= 0.;
    dnormaldphi(0,1)= sbeta*sgamma;
    dnormaldphi(0,2)= -cgamma*cbeta;
    dnormaldphi(1,0)= -salpha*cgamma-calpha*sbeta*sgamma;
    dnormaldphi(1,1)= -salpha*cbeta*sgamma;
    dnormaldphi(1,2)= -sgamma*calpha-salpha*sbeta*cgamma;
    dnormaldphi(2,0)= calpha*cgamma-salpha*sbeta*sgamma;
    dnormaldphi(2,1)= calpha*cbeta*sgamma;
    dnormaldphi(2,2)= -salpha*sgamma+calpha*sbeta*cgamma;

    Vec normaltt = dnormaldphi*qGt(3,5);

    Vec binormal(3);
    binormal(0) = sbeta;
    binormal(1) = -cbeta*salpha;
    binormal(2) = cbeta*calpha;
    
    SqrMat dbinormaldphi(3);
    dbinormaldphi(0,0) = 0.;
    dbinormaldphi(0,1) = cbeta;
    dbinormaldphi(0,2) = 0.;
    dbinormaldphi(1,0) = -cbeta*calpha;
    dbinormaldphi(1,1) = sbeta*salpha;
    dbinormaldphi(1,2) = 0.;
    dbinormaldphi(2,0) = -salpha*cbeta;
    dbinormaldphi(2,1) = -sbeta*calpha;
    dbinormaldphi(2,2) = 0.;

    Vec binormaltt = dbinormaldphi*qGt(3,5);

    /* position and velocity difference */
    Vec deltax = qG(6,8)-qG(0,2);
    Vec deltaxt = qGt(6,8)-qGt(0,2);

    /* differentiation of 'gravitational energy' with respect to qG */
    Vec dVgdqG(9,INIT,0.);
    dVgdqG(0,2) = -0.5*rho*A*l0*g;
    dVgdqG(6,8) = -0.5*rho*A*l0*g;

    /* differentiation of 'strain energy' with respect to qG */
    Vec dSETdqG(9); // strain tangential
    dSETdqG(0,2) = -tangent.copy();
    dSETdqG(3) = deltax.T()*dtangentdphi.col(0);
    dSETdqG(4) = deltax.T()*dtangentdphi.col(1);
    dSETdqG(5) = deltax.T()*dtangentdphi.col(2);
    dSETdqG(6,8) = tangent.copy();
    dSETdqG *= E*A*(tangent.T()*deltax-l0)/l0;

    Vec dSENdqG(9); // strain normal
    dSENdqG(0,2) = -normal.copy();
    dSENdqG(3) = deltax.T()*dnormaldphi.col(0);
    dSENdqG(4) = deltax.T()*dnormaldphi.col(1);
    dSENdqG(5) = deltax.T()*dnormaldphi.col(2);
    dSENdqG(6,8) = normal.copy();
    dSENdqG *= G*sigma1*A*normal.T()*deltax/l0;

    Vec dSEBdqG(9); // strain binormal
    dSEBdqG(0,2) = -binormal.copy(); 
    dSEBdqG(3) = deltax.T()*dbinormaldphi.col(0);
    dSEBdqG(4) = deltax.T()*dbinormaldphi.col(1);
    dSEBdqG(5) = deltax.T()*dbinormaldphi.col(2);
    dSEBdqG(6,8) = binormal.copy();
    dSEBdqG *= G*sigma2*A*binormal.T()*deltax/l0;

    Vec dVeldqG = dSETdqG + dSENdqG + dSEBdqG;

    /* differentation of 'kinetic energy' with respect to phi
     * remark: translational part is zero
     */
    Vec dTRdphi(3,INIT,0.); // TODO : check -> LIKE THIS IT IS AGAIN WRONG
    //dTRdphi(0) = 0.;
    //dTRdphi(1) = -(alphat*cgamma*cgamma*cbeta*I0*sbeta*alphat+cgamma*sbeta*I0*sgamma*betat+cbeta*I1*sbeta*alphat-1.0*cbeta*I1*sbeta*alphat*cgamma*cgamma-1.0*sgamma*sbeta*I1*cgamma*betat-1.0*sbeta*I2*cbeta*alphat-1.0*cbeta*I2*gammat);
    //dTRdphi(2) = (-1.0*sgamma*cbeta*cbeta*I0*cgamma*alphat*alphat-1.0*alphat*cbeta*I0*betat+2.0*alphat*cgamma*cgamma*cbeta*I0*betat+cgamma*cbeta*cbeta*I1*sgamma*alphat*alphat-2.0*alphat*cgamma*cgamma*cbeta*I1*betat+alphat*cbeta*I1*betat+cgamma*I0*sgamma*betat*betat-1.0*sgamma*I1*cgamma*betat*betat);
    //dTRdphi*=rho*l0;

    /* differentiation of 'kinetic energy' with respect to phit, phi
     * remark: translational part is zero
     */
    SqrMat dTRdphitphi(3,INIT,0.); // TODO : check -> LIKE THIS IT IS AGAIN WRONG
    //dTRdphitphi(0,0) = 0.0;
    //dTRdphitphi(0,1) = 0.5*(-2.0*cgamma*cgamma*cbeta*I0*sbeta-2.0*sgamma*sgamma*cbeta*I1*sbeta+2.0*sbeta*I2*cbeta)*alphat+0.5*(-cgamma*sbeta*I0*(cgamma*cbeta*alphat+sgamma*betat)-cgamma*cgamma*cbeta*I0*sbeta*alphat+sgamma*sbeta*I1*(-sgamma*cbeta*alphat+cgamma*betat)-sgamma*sgamma*cbeta*I1*sbeta*alphat+cbeta*I2*(sbeta*alphat+gammat)+sbeta*I2*cbeta*alphat)+0.5*(-sgamma*I0*cgamma*sbeta+cgamma*I1*sgamma*sbeta)*betat+0.5*I2*cbeta*gammat;
    //dTRdphitphi(0,2) = 0.5*(-2.0*cgamma*cbeta*cbeta*I0*sgamma+2.0*sgamma*cbeta*cbeta*I1*cgamma)*alphat+0.5*(-sgamma*cbeta*I0*(cgamma*cbeta*alphat+sgamma*betat)+cgamma*cbeta*I0*(-sgamma*cbeta*alphat+cgamma*betat)-cgamma*cbeta*I1*(-sgamma*cbeta*alphat+cgamma*betat)-sgamma*cbeta*I1*(-cgamma*cbeta*alphat-sgamma*betat))+0.5*(cgamma*cgamma*I0*cbeta-sgamma*sgamma*I0*cbeta+sgamma*sgamma*I1*cbeta-cgamma*cgamma*I1*cbeta)*betat;
    //dTRdphitphi(1,0) = 0.0;
    //dTRdphitphi(1,1) = 0.5*(-sgamma*I0*cgamma*sbeta+cgamma*I1*sgamma*sbeta)*alphat+0.5*(-sgamma*I0*cgamma*sbeta*alphat+cgamma*I1*sgamma*sbeta*alphat);
    //dTRdphitphi(1,2) = 0.5*(cgamma*cgamma*I0*cbeta-sgamma*sgamma*I0*cbeta+sgamma*sgamma*I1*cbeta-cgamma*cgamma*I1*cbeta)*alphat+0.5*(2.0*sgamma*I0*cgamma-2.0*cgamma*I1*sgamma)*betat+0.5*(cgamma*I0*(cgamma*cbeta*alphat+sgamma*betat)+sgamma*I0*(-sgamma*cbeta*alphat+cgamma*betat)-sgamma*I1*(-sgamma*cbeta*alphat+cgamma*betat)+cgamma*I1*(-cgamma*cbeta*alphat-sgamma*betat));
    //dTRdphitphi(2,0) = 0.0;
    //dTRdphitphi(2,1) = 0.1E1*cbeta*I2*alphat;
    //dTRdphitphi(2,2) = 0.0;
    //dTRdphitphi*=rho*l0;

    /* differentiation of 'strain dissipation' with respect to qG
     * attention: for ring structures damping like this seems not ro be appropriate
     * -> use setMassProportionalDamping
     */
    Vec dSDdqGt(9); // elongation TODO remaining terms
    dSDdqGt(0,2) = -tangent.copy();
    dSDdqGt(3) = deltax.T()*dtangentdphi.col(0);
    dSDdqGt(4) = deltax.T()*dtangentdphi.col(1);
    dSDdqGt(5) = deltax.T()*dtangentdphi.col(2);
    dSDdqGt(6,8) = tangent.copy();
    dSDdqGt *= 2.*cEps0D*(deltax.T()*tangentt + deltaxt.T()*tangent)/l0;

    /* generalized forces */
    h = -dVgdqG-dVeldqG-dSDdqGt;
    h(3,5) += dTRdphi-dTRdphitphi*qGt(3,5);
  }

  double FiniteElement1s33CosseratTranslation::computeKineticEnergy(const fmatvec::Vec& qG, const fmatvec::Vec& qGt) {
    /* translational kinetic energy */
    double TT = 0.25*rho*A*l0*(pow(nrm2(qGt(0,2)),2.)+pow(nrm2(qGt(6,8)),2.)); 
    
    /* Cardan angles */
    double sbeta = sin(qG(4));
    double sgamma = sin(qG(5));
    double cbeta = cos(qG(4));
    double cgamma = cos(qG(5));

    /* inertia tensor */
    SymMat Itilde(3,INIT,0.); 
    Itilde(0,0) = (I0*cbeta*cbeta*cgamma*cgamma + I1*cbeta*cbeta*sgamma*sgamma + I2*sbeta*sbeta);
    Itilde(0,1) = (I0*cbeta*cgamma*sgamma - I1*cbeta*sgamma*cgamma);
    Itilde(0,2) = I2*sbeta;
    Itilde(1,1) = (I0*sgamma*sgamma + I1*cgamma*cgamma);
    Itilde(2,2) = I2;
    
    /* rotational kinetic energy */
    double TR = 0.5*rho*l0*qGt.T()(3,5)*Itilde*qGt(3,5); 

    return TT + TR;
  }

  double FiniteElement1s33CosseratTranslation::computeGravitationalEnergy(const fmatvec::Vec& qG) {
   return -0.5*rho*A*l0*g.T()*(qG(0,2)+qG(6,8)); 
  }

  double FiniteElement1s33CosseratTranslation::computeElasticEnergy(const fmatvec::Vec& qG) {
    /* Cardan angles */
    double salpha = sin(qG(3));
    double sbeta = sin(qG(4));
    double sgamma = sin(qG(5));
    double calpha = cos(qG(3));
    double cbeta = cos(qG(4));
    double cgamma = cos(qG(5));

    Vec tangent(3);
    tangent(0) = cbeta*cgamma;
    tangent(1) = calpha*sgamma+salpha*sbeta*cgamma;
    tangent(2) = salpha*sgamma-calpha*sbeta*cgamma;

    Vec normal(3);
    normal(0) = -sgamma*cbeta;
    normal(1) = cgamma*calpha-salpha*sbeta*sgamma;
    normal(2) = salpha*cgamma+calpha*sbeta*sgamma;

    Vec binormal(3);
    binormal(0) = sbeta;
    binormal(1) = -cbeta*salpha;
    binormal(2) = cbeta*calpha;

    /* position difference */
    Vec deltax = qG(6,8)-qG(0,2);
   
    /* strain energy */
    return 0.5*(E*A*pow(tangent.T()*deltax-l0,2.) + G*sigma1*A*pow(normal.T()*deltax,2.) + G*sigma2*A*pow(binormal.T()*deltax,2.))/l0;
  }

  const Vec& FiniteElement1s33CosseratTranslation::computeState(const Vec& qG, const Vec& qGt,double s) {
    X(0,2) = qG(0,2) + s*(qG(6,8)-qG(0,2))/l0; // position
    X(6,8) = qGt(0,2) + s*((qGt(6,8)-qGt(0,2))/l0); // velocity

    X(3,5) = qG(3,5); // angles TODO in angle element or better in FlexibleBody
    X(9,11) = qGt(3,5); // time differentiated angels TODO in angle element or better in FlexibleBody

    return X;
  }
      
  void FiniteElement1s33CosseratTranslation::initM() {
    /* constant mass matrix is just standard rigid body translational part */
    M(0,0) = 0.5*rho*A*l0;
    M(1,1) = 0.5*rho*A*l0;
    M(2,2) = 0.5*rho*A*l0;
    M(6,6) = 0.5*rho*A*l0;
    M(7,7) = 0.5*rho*A*l0;
    M(8,8) = 0.5*rho*A*l0;
  }

}

