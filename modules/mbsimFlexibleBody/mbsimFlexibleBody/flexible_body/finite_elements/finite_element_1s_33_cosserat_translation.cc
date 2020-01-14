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
 * Contact: thorsten.schindler@mytum.de
 */

#include<config.h>
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_translation.h"
#include "mbsimFlexibleBody/utils/cardan.h"
#include "mbsim/utils/eps.h"
#include "mbsim/mbsim_event.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  FiniteElement1s33CosseratTranslation::FiniteElement1s33CosseratTranslation(double l0_, double rho_,double A_, double E_, double G_, double I1_, double I2_, double I0_, const Vec& g_ ,CardanPtr ag_) : l0(l0_), rho(rho_), A(A_), E(E_), G(G_), I1(I1_), I2(I2_), I0(I0_), g(g_), cEps0D(0.), cEps1D(0.), cEps2D(0.), sigma1(1.), sigma2(1.), M(9,INIT,0.), h(9,INIT,0.), X(12,INIT,0.), ag(ag_) {}

  FiniteElement1s33CosseratTranslation::~FiniteElement1s33CosseratTranslation() {}

  void FiniteElement1s33CosseratTranslation::setMaterialDamping(double cEps0D_, double cEps1D_, double cEps2D_) {
    cEps0D = cEps0D_;
    cEps1D = cEps1D_;
    cEps2D = cEps2D_;
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
    const Vec &Phi = qG(RangeV(3,5));
    const Vec &Phit = qGt(RangeV(3,5));

    const double &alphat = qGt(3);
    const double &betat = qGt(4);
    const double &gammat = qGt(5);

    double sbeta = sin(qG(4));
    double sgamma = sin(qG(5));
    double cbeta = cos(qG(4));
    double cgamma = cos(qG(5));

    Vec tangent = ag->computet(Phi);
    SqrMat dtangentdphi = ag->computetq(Phi);
    Vec tangentt = dtangentdphi*Phit;

    Vec normal = ag->computen(Phi);
    SqrMat dnormaldphi = ag->computenq(Phi);
    Vec normaltt = dnormaldphi*Phit;

    Vec binormal = ag->computeb(Phi);
    SqrMat dbinormaldphi = ag->computebq(Phi);
    Vec binormaltt = dbinormaldphi*Phit;

    /* position and velocity difference */
    Vec deltax = qG(RangeV(6,8))-qG(RangeV(0,2));
    Vec deltaxt = qGt(RangeV(6,8))-qGt(RangeV(0,2));

    /* differentiation of 'gravitational energy' with respect to qG */
    Vec dVgdqG(9,INIT,0.);
    dVgdqG.set(RangeV(0,2), -0.5*rho*A*l0*g);
    dVgdqG.set(RangeV(6,8), -0.5*rho*A*l0*g);

    /* differentiation of 'strain energy' with respect to qG */
    Vec dSETdqG(9); // strain tangential
    dSETdqG.set(RangeV(0,2), -tangent);
    dSETdqG(3) = deltax.T()*dtangentdphi.col(0);
    dSETdqG(4) = deltax.T()*dtangentdphi.col(1);
    dSETdqG(5) = deltax.T()*dtangentdphi.col(2);
    dSETdqG.set(RangeV(6,8), tangent);
    dSETdqG *= E*A*(tangent.T()*deltax-l0)/l0;

    Vec dSENdqG(9); // strain normal
    dSENdqG.set(RangeV(0,2), -normal);
    dSENdqG(3) = deltax.T()*dnormaldphi.col(0);
    dSENdqG(4) = deltax.T()*dnormaldphi.col(1);
    dSENdqG(5) = deltax.T()*dnormaldphi.col(2);
    dSENdqG.set(RangeV(6,8), normal);
    dSENdqG *= G*sigma1*A*normal.T()*deltax/l0;

    Vec dSEBdqG(9); // strain binormal
    dSEBdqG.set(RangeV(0,2), -binormal);
    dSEBdqG(3) = deltax.T()*dbinormaldphi.col(0);
    dSEBdqG(4) = deltax.T()*dbinormaldphi.col(1);
    dSEBdqG(5) = deltax.T()*dbinormaldphi.col(2);
    dSEBdqG.set(RangeV(6,8), binormal);
    dSEBdqG *= G*sigma2*A*binormal.T()*deltax/l0;

    Vec dVeldqG = dSETdqG + dSENdqG + dSEBdqG;

    /* differentation of 'kinetic energy' with respect to phi
     * remark: translational part is zero
     */
    Vec dTRdphi(3,INIT,0.);
    dTRdphi(1) = -1.0*rho*l0*alphat*(pow(cgamma,2.0)*cbeta*I0*alphat*sbeta+cgamma*sbeta*I0*sgamma*betat+cbeta*I1*alphat*sbeta-1.0*cbeta*I1*alphat*pow(cgamma,2.0)*sbeta-1.0*sgamma*sbeta*I1*cgamma*betat-1.0*I2*alphat*cbeta*sbeta-1.0*cbeta*I2*gammat);
    dTRdphi(2) = rho*l0*(-1.0*cgamma*pow(cbeta,2.0)*I0*alphat*alphat*sgamma-1.0*alphat*cbeta*I0*betat+2.0*alphat*pow(cgamma,2.0)*cbeta*I0*betat+pow(cbeta,2.0)*I1*alphat*alphat*cgamma*sgamma-2.0*alphat*pow(cgamma,2.0)*cbeta*I1*betat+alphat*cbeta*I1*betat+I0*betat*betat*cgamma*sgamma-1.0*cgamma*I1*betat*betat*sgamma);

    /* differentiation of 'kinetic energy' with respect to phit, phi
     * remark: translational part is zero
     */
    SqrMat dTRdphitphi(3,INIT,0.);
    dTRdphitphi(0,1) = -1.0*rho*l0*(2.0*pow(cgamma,2.0)*cbeta*I0*alphat*sbeta+cgamma*sbeta*I0*sgamma*betat+2.0*cbeta*I1*alphat*sbeta-2.0*cbeta*I1*alphat*pow(cgamma,2.0)*sbeta-1.0*sgamma*sbeta*I1*cgamma*betat-2.0*I2*alphat*cbeta*sbeta-1.0*cbeta*I2*gammat);
    dTRdphitphi(0,2) = 0.1E1*rho*l0*cbeta*(-2.0*alphat*cgamma*cbeta*I0*sgamma-1.0*I0*betat+2.0*I0*betat*pow(cgamma,2.0)+2.0*alphat*sgamma*cbeta*I1*cgamma-2.0*pow(cgamma,2.0)*I1*betat+I1*betat);
    dTRdphitphi(1,1) = -0.1E1*rho*l0*alphat*cgamma*sbeta*sgamma*(I0-1.0*I1);
    dTRdphitphi(1,2) = rho*l0*(-1.0*alphat*cbeta*I0+2.0*alphat*pow(cgamma,2.0)*cbeta*I0-2.0*alphat*pow(cgamma,2.0)*cbeta*I1+alphat*cbeta*I1+2.0*I0*betat*cgamma*sgamma-2.0*cgamma*I1*betat*sgamma);
    dTRdphitphi(2,1) = 0.1E1*rho*l0*alphat*cbeta*I2;

    /* differentiation of 'strain dissipation' with respect to qGt */
    Vec dSDTdqGt(9); 
    dSDTdqGt.set(RangeV(0,2), -tangent);
    dSDTdqGt(3) = -tangent.T()*dtangentdphi.col(0)*l0;
    dSDTdqGt(4) = -tangent.T()*dtangentdphi.col(1)*l0;
    dSDTdqGt(5) = -tangent.T()*dtangentdphi.col(2)*l0;
    dSDTdqGt.set(RangeV(6,8), tangent);
    dSDTdqGt *= 2.*cEps0D*(deltaxt.T()*tangent/l0 - tangent.T()*tangentt);

    Vec dSDNdqGt(9);
    dSDNdqGt.set(RangeV(0,2), -normal);
    dSDNdqGt(3) = -normal.T()*dtangentdphi.col(0)*l0;
    dSDNdqGt(4) = -normal.T()*dtangentdphi.col(1)*l0;
    dSDNdqGt(5) = -normal.T()*dtangentdphi.col(2)*l0;
    dSDNdqGt.set(RangeV(6,8), normal);
    dSDNdqGt *= 2.*cEps1D*(deltaxt.T()*normal/l0 -normal.T()*tangentt);

    Vec dSDBdqGt(9);
    dSDBdqGt.set(RangeV(0,2), -binormal);
    dSDBdqGt(3) = -binormal.T()*dtangentdphi.col(0)*l0;
    dSDBdqGt(4) = -binormal.T()*dtangentdphi.col(1)*l0;
    dSDBdqGt(5) = -binormal.T()*dtangentdphi.col(2)*l0;
    dSDBdqGt.set(RangeV(6,8), binormal);
    dSDBdqGt *= 2.*cEps2D*(deltax.T()*binormal/l0 - binormal.T()*tangentt);

    Vec dSDdqGt = dSDTdqGt + dSDNdqGt + dSDBdqGt;

    /* generalized forces */
    h = -dVgdqG-dVeldqG-dSDdqGt;
    h.add(RangeV(3,5), dTRdphi-dTRdphitphi*Phit);
  }

  double FiniteElement1s33CosseratTranslation::computeKineticEnergy(const Vec& qG, const Vec& qGt) {
    /* translational kinetic energy */
    double TT = 0.25*rho*A*l0*(pow(nrm2(qGt(RangeV(0,2))),2.)+pow(nrm2(qGt(RangeV(6,8))),2.)); 

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
    double TR = 0.5*rho*l0*qGt.T()(RangeV(3,5))*Itilde*qGt(RangeV(3,5));

    return TT + TR;
  }

  double FiniteElement1s33CosseratTranslation::computeGravitationalEnergy(const Vec& qG) {
    return -0.5*rho*A*l0*g.T()*(qG(RangeV(0,2))+qG(RangeV(6,8))); 
  }

  double FiniteElement1s33CosseratTranslation::computeElasticEnergy(const Vec& qG) {
    /* Cardan angles */
    const Vec &Phi = qG(RangeV(3,5));

    Vec tangent = ag->computet(Phi);
    Vec normal = ag->computen(Phi);
    Vec binormal = ag->computeb(Phi);

    /* position difference */
    Vec deltax = qG(RangeV(6,8))-qG(RangeV(0,2));

    /* strain energy */
    return 0.5*(E*A*pow(tangent.T()*deltax-l0,2.) + G*sigma1*A*pow(normal.T()*deltax,2.) + G*sigma2*A*pow(binormal.T()*deltax,2.))/l0;
  }

  const Vec& FiniteElement1s33CosseratTranslation::computeStateTranslation(const Vec& qG, const Vec& qGt,double s) {
    X.set(RangeV(0,2), qG(RangeV(0,2)) + s*(qG(RangeV(6,8))-qG(RangeV(0,2)))/l0); // position
    X.set(RangeV(6,8), qGt(RangeV(0,2)) + s*((qGt(RangeV(6,8))-qGt(RangeV(0,2)))/l0)); // velocity

    X.set(RangeV(3,5), qG(RangeV(3,5))); // angles TODO in angle element or better in FlexibleBody
    X.set(RangeV(9,11), qGt(RangeV(3,5))); // time differentiated angels TODO in angle element or better in FlexibleBody

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

  void FiniteElement1s33CosseratTranslation::computedhdz(const Vec& qG, const Vec& qGt) { 
    throw runtime_error("(FiniteElement1s33CosseratTranslation::computedhdz): Not implemented");
  }

  Vec3 FiniteElement1s33CosseratTranslation::getPosition(const Vec& q, double s) {
    throw runtime_error("(FiniteElement1s33CosseratTranslation::computePosition): Not implemented!");
  }

  SqrMat3 FiniteElement1s33CosseratTranslation::getOrientation(const Vec& q, double s) {
    throw runtime_error("(FiniteElement1s33CosseratTranslation::computeOrientation): Not implemented!");
  }

  Vec3 FiniteElement1s33CosseratTranslation::getVelocity(const Vec& q, const Vec& u, double s) {
    throw runtime_error("(FiniteElement1s33CosseratTranslation::computeVelocity): Not implemented!");
  }

  Vec3 FiniteElement1s33CosseratTranslation::getAngularVelocity(const Vec& q, const Vec& u, double s) {
    throw runtime_error("(FiniteElement1s33CosseratTranslation::computeAngularVelocity): Not implemented!");
  }

  Mat FiniteElement1s33CosseratTranslation::computeJXqG(const Vec& qG, double s) {
    throw runtime_error("(FiniteElement1s33CosseratTranslation::computeJXqG): Not implemented!");
  }

}
