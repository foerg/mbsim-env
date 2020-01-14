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
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_cosserat_rotation.h"
#include "mbsimFlexibleBody/utils/cardan.h"
#include "mbsim/utils/eps.h"
#include "mbsim/mbsim_event.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  FiniteElement1s33CosseratRotation::FiniteElement1s33CosseratRotation(double l0_, double E_, double G_, double I1_, double I2_, double I0_, CardanPtr ag_) : l0(l0_), E(E_), G(G_), I1(I1_), I2(I2_), I0(I0_), k10(0.), k20(0.), h(9,INIT,0.), X(12,INIT,0.), ag(ag_) {}

  FiniteElement1s33CosseratRotation::~FiniteElement1s33CosseratRotation() {}

  void FiniteElement1s33CosseratRotation::setCurlRadius(double R1,double R2) {
    if (fabs(R1)>epsroot) k10 = 1./R1;
    if (fabs(R2)>epsroot) k20 = 1./R2;
  }

  void FiniteElement1s33CosseratRotation::computeh(const Vec& qG, const Vec& qGt) {
    /* angles */
    Vec phi = (qG(RangeV(0,2))+qG(RangeV(6,8)))/2.;
    Vec dphids = (qG(RangeV(6,8))-qG(RangeV(0,2)))/l0;

    Vec tangent = ag->computet(phi);
    Vec normal = ag->computen(phi);
    Vec binormal = ag->computeb(phi);

    SqrMat dtangentdphi = ag->computetq(phi);
    SqrMat dnormaldphi = ag->computenq(phi);
    SqrMat dbinormaldphi = ag->computebq(phi);

    /* differentiation of 'bending and torsion energy' with respect to qG */
    double GI0ktilde0 = G*I0*binormal.T()*dnormaldphi*dphids;
    Vec ktilde0_0 = 0.5*dbinormaldphi.T()*dnormaldphi*dphids;
    Vec ktilde0_1 = dnormaldphi.T()*binormal/l0;
    Vec ktilde0_2 = 0.5*(ag->computenqt(phi,dphids)).T()*binormal;
    Vec dBTtorsiondqG(9,INIT,0.);
    dBTtorsiondqG.set(RangeV(0,2), ktilde0_0 - ktilde0_1 + ktilde0_2);
    dBTtorsiondqG.set(RangeV(6,8), ktilde0_0 + ktilde0_1 + ktilde0_2);
    dBTtorsiondqG *= GI0ktilde0;

    double EI1ktilde1 = E*I1*(tangent.T()*dbinormaldphi*dphids-k10);
    Vec ktilde1_0 = 0.5*dtangentdphi.T()*dbinormaldphi*dphids;
    Vec ktilde1_1 = dbinormaldphi.T()*tangent/l0;
    Vec ktilde1_2 = 0.5*(ag->computebqt(phi,dphids)).T()*tangent;
    Vec dBTbending1dqG(9,INIT,0.);
    dBTbending1dqG.set(RangeV(0,2), ktilde1_0 - ktilde1_1 + ktilde1_2);
    dBTbending1dqG.set(RangeV(6,8), ktilde1_0 + ktilde1_1 + ktilde1_2);
    dBTbending1dqG *= EI1ktilde1;

    double EI2ktilde2 = E*I2*(normal.T()*dtangentdphi*dphids-k20);
    Vec ktilde2_0 = 0.5*dnormaldphi.T()*dtangentdphi*dphids;
    Vec ktilde2_1 = dtangentdphi.T()*normal/l0;
    Vec ktilde2_2 = 0.5*(ag->computetqt(phi,dphids)).T()*normal;
    Vec dBTbending2dqG(9,INIT,0.);
    dBTbending2dqG.set(RangeV(0,2), ktilde2_0 - ktilde2_1 + ktilde2_2);
    dBTbending2dqG.set(RangeV(6,8), ktilde2_0 + ktilde2_1 + ktilde2_2);
    dBTbending2dqG *= EI2ktilde2;

    /* generalized forces */
    h = (dBTtorsiondqG+dBTbending1dqG+dBTbending2dqG)*(-l0);
    //msg(Debug) << h << endl;
    //throw;
  }

  double FiniteElement1s33CosseratRotation::computeElasticEnergy(const fmatvec::Vec& qG) {
    Vec phi = (qG(RangeV(0,2))+qG(RangeV(6,8)))/2.;
    Vec dphids = (qG(RangeV(6,8))-qG(RangeV(0,2)))/l0;

    Vec tangent = ag->computet(phi);
    Vec normal = ag->computen(phi);
    Vec binormal = ag->computeb(phi);

    SqrMat dtangentdphi = ag->computetq(phi);
    SqrMat dnormaldphi = ag->computenq(phi);
    SqrMat dbinormaldphi = ag->computebq(phi);

    return 0.5*l0*(G*I0*pow(binormal.T()*dnormaldphi*dphids,2.)+E*I1*pow(tangent.T()*dbinormaldphi*dphids-k10,2.)+E*I2*pow(normal.T()*dtangentdphi*dphids-k20,2.));
  }

  const SymMat& FiniteElement1s33CosseratRotation::getM() const {
    throw runtime_error("(FiniteElement1s33CosseratRotation::getM): Not implemented");
  }

  void  FiniteElement1s33CosseratRotation::computeM(const Vec& qG) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::computeM): Not implemented");
  }

  void  FiniteElement1s33CosseratRotation::computedhdz(const Vec& qG, const Vec& qGt) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::computedhdz): Not implemented");
  }

  double FiniteElement1s33CosseratRotation::computeKineticEnergy(const Vec& qG, const Vec& qGt) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::computeKineticEnergy): Not implemented");
  }

  double FiniteElement1s33CosseratRotation::computeGravitationalEnergy(const Vec& qG) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::computeGravitationalEnergy): Not implemented");
  }

  Vec3 FiniteElement1s33CosseratRotation::getPosition(const Vec& q, double s) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::getPosition): Not implemented!");
  }

  SqrMat3 FiniteElement1s33CosseratRotation::getOrientation(const Vec& q, double s) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::getOrientation): Not implemented!");
  }

  Vec3 FiniteElement1s33CosseratRotation::getVelocity(const Vec& q, const Vec& u, double s) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::getVelocity): Not implemented!");
  }

  Vec3 FiniteElement1s33CosseratRotation::getAngularVelocity(const Vec& q, const Vec& u, double s) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::getAngularVelocity): Not implemented!");
  }

  Mat FiniteElement1s33CosseratRotation::computeJXqG(const Vec& qG, double s) {
    throw runtime_error("(FiniteElement1s33CosseratRotation::computeJXqG): Not implemented!");
  }

}

