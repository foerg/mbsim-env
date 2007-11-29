/* Copyright (C) 2004-2006  Roland Zander
 
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
 * Contact:
 *   rzander@users.berlios.de
 *
 */
#include <config.h>

#include "body_rigid_rel_onflex.h"

#include "port.h"
#include "contour.h"
#include "link.h"
#include "multi_body_system.h"
#include "tree_rigid.h"
#include "tree_flexroot.h"
#include "body_flexible.h"

namespace MBSim {

  BodyRigidRelOnFlex::BodyRigidRelOnFlex(const string &name) : BodyRigidRel(name), precessor(0), AWP(3), constcPosition(false) {
    cPosition.type = CONTINUUM;
    cout << "WARINING: BodyRigidRelOnFlex:: > " << name <<  " < WARINING WARINING WARINING WARINING WARINING WARINING WARINING WARINING WARINING WARINING WARINING" << endl;
    cout << "WARINING: this still is TESTING -- blame the author in case of malfunction;-)\n" << endl;
  }

//  void BodyRigidRelOnFlex::setPrecessor(BodyFlexible *precessor_) {precessor=precessor_;}
  void BodyRigidRelOnFlex::sets0(const Vec& s0_) {
    constcPosition = true;
    cPosition.alpha = s0_;
    cPosition.alphap = Vec(0);
  }

//  void BodyRigidRelOnFlex::initStage2() {
//    BodyRigidRel::initStage2();
//    if(!constcPosition) {
////      cPosition.alpha .resize(iT.end()-iT.start()+1);
////      cPosition.alphap.resize(iT.end()-iT.start()+1);
//
//cout << "iT: start " << iT.start() << " end: " << iT.end() << endl;
//cout << q(iT) << endl;
//cout << u(iT) << endl;
//cout << cPosition.alpha << endl;
//cout << cPosition.alphap << endl;
//      cPosition.alpha  >> q(iT);
//      cPosition.alphap >> u(iT);
//    }
//  }

  void BodyRigidRelOnFlex::updateqRef() {
    BodyRigidRel::updateqRef();
    if(!constcPosition) cPosition.alpha  >> q(iT);
  }
  void BodyRigidRelOnFlex::updateuRef() {
    BodyRigidRel::updateuRef();
    if(!constcPosition) cPosition.alphap >> u(iT);
  }
 
//  void BodyRigidRelOnFlex::updateM(double t) {
//    tree->getM()(Index(0,uInd+uSize-1)) += JTMJ(Mh,J);
//    for(int i=0; i<successor.size(); i++) {
//      successor[i]->updateM(t);
//    }
//  }
  
  void BodyRigidRelOnFlex::updateh(double t) {
//static int i=0;
//cout << "void BodyRigidRelOnFlex::updateh(double t) i = " << i++ << endl;
//cout << "t = " << t << endl << endl;
//if(i == 8) throw i;

      sumUpForceElements(t);

//cout << "s  = " << trans(cPosition.alpha) << endl;
//cout << "sp = " << trans(cPosition.alphap) << endl;

  // Contour-Koordinate
/*    ContourPointData cp;
    cp.type = CONTINUUM;
    if(s0.size()>0) {
      cp.alpha  = s0;
      cp.alphap = Vec(0);
    } else {
      cp.alpha  = q(iT);
      cp.alphap = u(iT);
    }*/
    Vec KF = trans(AWK)*WF;
    Vec KM = trans(AWK)*WM;
    l(0,2) = KF - m*crossProduct(KomegaK,crossProduct(KomegaK,KrKS));
    l(3,5) = KM + crossProduct(I*KomegaK,KomegaK);

    Vec f(6,NONINIT);

// ---  KomegaK  = trans(AWK)*precessor->computeWomega(cp) + JR*u(iR);
// ---           = trans(AWK)*precessor->computeJacobianMatrix(iR)*precessor->u + JR*u(iR);
// ---  WomegaK  = precessor->computeWomega(cp) + AWK*JR*u(iR);

//       SqrMat AWKp = precessor->computeAWKp(cp)*APK  +  AWP*tilde(APK*JR*u(iR))*APK;
    SqrMat AWKp = tilde(WomegaK)*AWK;

    f(3,5) = trans(AWK)*AWKp*JR*u(iR);
    if(JT.cols()) {
      f(3,5) += trans(AWK)*precessor->computeKp(cPosition) *u(iT);

// ---   WrOK = precessor->computeWrOC(cp);
// ---   WvK  = precessor->computeWvC(cp) + precessor->computeDrDs(cp)*u(iT);
// ---   WaK  = precessor->computeWaC(cp) + precessor->computeDrDs(cp)*up(iT) + precessor->computeDrDsp(cp)*u(iT);
      f(0,2)  = trans(AWK)*precessor->computeDrDsp(cPosition)*u(iT);
    }
    else f(0,2) = Vec(3,INIT,0.0);

    C(Index(0,2),Index(0,2)) = trans(APK);
    C(Index(3,5),Index(3,5)) = trans(APK);
//    C(Index(0,2),Index(3,5)) = -trans(APK)*tilde(PrPK);
    C(Index(0,2),Index(3,5)).init(0.0);
    C(Index(3,5),Index(0,2)).init(0.0);

    Mat Jges_pre(6,precessor->JT.cols()+precessor->JR.cols(),INIT,0.0);
    Jges_pre(Index(0,2),Index(                   0,precessor->JT.cols()-1)) = trans(AWP)*precessor->JT;
    Jges_pre(Index(3,5),Index(precessor->JT.cols(),Jges_pre.cols()-1)     ) = trans(AWP)*precessor->JR;

      //      e = C*precessor->gete()+f;
    e = C*Jges_pre*trans(precessor->computeJp(cPosition))*precessor->getu() + f;

//      J(Index(0,2),IuT) = trans(APK)*JT;
    if(JT.cols()) {
      J(Index(0,2),IuT) = trans(AWK)*precessor->computeDrDs(cPosition);
      J(Index(3,5),IuT) = trans(AWK)*precessor->computeK(cPosition);
    }
    J(Index(3,5),IuR) = JR;

    J(Index(0,5),static_cast<TreeFlexRoot*>(tree)->Iflexible) = C*Jges_pre*trans(precessor->computeJacobianMatrix(cPosition));

    l -= Mh*e;

//cout << getFullName() << " J   = " << J << endl;
//cout << getFullName() << " l   = " << trans(l) << endl;
//cout << getFullName() << " J*l = " << trans(trans(J)*l) << endl;

    tree->geth()(Index(0,uInd+uSize-1)) += trans(J)*l;

    for(unsigned int i=0; i<successor.size(); i++)
      successor[i]->updateh(t);
  }

  void BodyRigidRelOnFlex::updateCenterOfGravity(double t) {
/*    // Contour-Koordinate
    ContourPointData cp;
    cp.type = CONTINUUM;
    if(s0.size()>0) {
      cp.alpha  >> s0;
      cp.alphap = Vec(0);
    } else {
      cp.alpha  = q(iT);
      cp.alphap = u(iT);
    }*/

    (this->*updateAK0K)();

    //    PrPK = JT*q(iT) + PrPK0;
    APK = APK0*AK0K;
    AWP = precessor->computeAWK(cPosition);
    AWK = AWP*APK;

    //    KomegaK = trans(APK)*precessor->getKomegaK() + JR*u(iR);
    KomegaK = trans(AWK)*precessor->computeWomega(cPosition) + JR*u(iR);
    //cout << "-----------------" << endl;
    //cout << "KomegaK ohne K " << trans(KomegaK) << endl;
    // precessor many have curved path, leading to angular processes when translating 
    if(JT.cols()) {
      KomegaK += trans(AWK)*precessor->computeK(cPosition) * u(iT);   
      //cout << "KomegaK mit  K " << trans(KomegaK) << endl;
      //cout << "precessor->computeK(cp)" << trans(precessor->computeK(cp)) << endl;
      //cout << "u(iT) " << trans(u(iT)) << endl;
      //cout << "+= trans(AWK)*precessor->computeK(cp) * u(iT) " << trans(trans(AWK)*precessor->computeK(cp) * u(iT)) << endl;
    }
    WomegaK = AWK * KomegaK;

    //      KrOK = trans(APK)*(PrPK + precessor->getKrOK());
    WrOK = precessor->computeWrOC(cPosition);
    //      WvK  = precessor->computeWvC(cp) + AWP*JT*u(iT);
    WvK  = precessor->computeWvC(cPosition);
    if(JT.cols()) WvK += precessor->computeDrDs(cPosition)*u(iT);

    KrOK = trans(AWK) * WrOK;
    KvK  = trans(AWK) * WvK;
  }

}
