/* Copyright (C) 2004-2018 MBSim Development Team
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
#include "gearwheel_gearwheel.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsim/contours/gear_wheel.h"
#include <mbsim/utils/rotarymatrices.h>
#include <mbsim/utils/eps.h>

using namespace fmatvec;
using namespace std;

namespace MBSim {

  void ContactKinematicsGearWheelGearWheel::assignContours(const vector<Contour*> &contour) {
    if(not(static_cast<GearWheel*>(contour[0])->getSolid())) {
      gearwheel[0] = static_cast<GearWheel*>(contour[1]);
      gearwheel[1] = static_cast<GearWheel*>(contour[0]);
      igearwheel[0] = 1;
      igearwheel[1] = 0;
    }
    else {
      gearwheel[0] = static_cast<GearWheel*>(contour[0]);
      gearwheel[1] = static_cast<GearWheel*>(contour[1]);
      igearwheel[0] = 0;
      igearwheel[1] = 1;
    }
    m = static_cast<GearWheel*>(contour[0])->getModule()/cos(static_cast<GearWheel*>(contour[0])->getHelixAngle());
    al0 = atan(tan(static_cast<GearWheel*>(contour[0])->getPressureAngle())/cos(static_cast<GearWheel*>(contour[0])->getHelixAngle()));
    double phi0 = tan(al0)-al0;
    double s0 = m*M_PI/2;
    for(int i=0; i<2; i++) {
      z[i] = gearwheel[i]->getNumberOfTeeth();
      d0[i] = m*z[i];
      db[i] = d0[i]*cos(al0);
      rb[i] = db[i]/2;
      sb[i] = db[i]*(s0/d0[i]+phi0)-((i==1 and not gearwheel[1]->getSolid())?-gearwheel[i]->getBacklash():gearwheel[i]->getBacklash());
      ga[i] = sb[i]/rb[i]/2;
      beta[i] = gearwheel[i]->getHelixAngle();
    }
  }

  void ContactKinematicsGearWheelGearWheel::updateg(SingleContact &contact, int ii) {
    contact.getGeneralizedRelativePosition(false)(0) = 1e10;
    double eta[2], del[2];
    Vec3 rS1S2 = gearwheel[1]->getSolid()?(gearwheel[1]->getFrame()->evalPosition() - gearwheel[0]->getFrame()->evalPosition()):(gearwheel[0]->getFrame()->evalPosition() - gearwheel[1]->getFrame()->evalPosition());
    double a = nrm2(rS1S2);
    double a0 = ((gearwheel[1]->getSolid()?d0[0]:-d0[0])+d0[1])/2;
    al = acos(a0/a*cos(al0));

    for(int i=0; i<2; i++) {
      int signi = i?-1:1;
      Vec3 rOP[2], rSP[2];
      double k[2]{0,0};
      for(int j=0; j<2; j++) {
        Vec3 rP1S2 = rS1S2 - (gearwheel[1]->getSolid()?rSP[0]:-rSP[0]);
        int signj = j?(gearwheel[1]->getSolid()?-1:1):1;
        double cdel = 0;
        for(int k_=0; k_<z[j]; k_++) {
          double ep = k_*2*M_PI/z[j]+signi*ga[j];
          rSP[j](0) = -sin(ep);
          rSP[j](1) = cos(ep);
          rSP[j] = gearwheel[j]->getFrame()->getOrientation()*rSP[j];
          double cdel_ = signj*(rSP[j].T()*rP1S2/a);
          if(cdel_>cdel) {
            cdel = cdel_;
            k[j] = k_;
          }
        }
        rSP[j](0) = -sin(k[j]*2*M_PI/z[j]);
        rSP[j](1) = cos(k[j]*2*M_PI/z[j]);
        rSP[j] = gearwheel[j]->getFrame()->getOrientation()*rSP[j];
        cdel = signj*(rSP[j].T()*rS1S2/a);
        del[j] = signi*signj*(gearwheel[j]->getFrame()->getOrientation().col(2).T()*crossProduct(rS1S2,rSP[j])>=0?acos(cdel):-acos(cdel));
        eta[j] = ga[j] + del[j] + al;
        rSP[j](0) = signi*rb[j]*(sin(eta[j])-cos(eta[j])*eta[j]);
        rSP[j](1) = rb[j]*(cos(eta[j])+sin(eta[j])*eta[j]);
        rSP[j] = gearwheel[j]->getFrame()->getOrientation()*BasicRotAIKz(signi*ga[j]+k[j]*2*M_PI/z[j])*rSP[j];
        rOP[j] = gearwheel[j]->getFrame()->getPosition() + rSP[j];
      }

      Vec3 n1(NONINIT);
      n1(0) = -signi*cos(eta[0]);
      n1(1) = sin(eta[0]);
      n1(2) = -signi*cos(al)*tan(beta[0]);
      n1 /= sqrt(1+pow(cos(al)*tan(beta[0]),2));
      n1 = gearwheel[0]->getFrame()->getOrientation()*BasicRotAIKz(signi*ga[0]+k[0]*2*M_PI/z[0])*n1;

      Vec3 u1(NONINIT);
      u1(0) = signi*sin(eta[0]);
      u1(1) = cos(eta[0]);
      u1(2) = 0;
      u1 = gearwheel[0]->getFrame()->getOrientation()*BasicRotAIKz(signi*ga[0]+k[0]*2*M_PI/z[0])*u1;

      double g = n1.T()*(rOP[1]-rOP[0]);
      if(g<contact.getGeneralizedRelativePosition(false)(0)) {
        ksave[0] = k[0];
        ksave[1] = k[1];
        etasave[0] = eta[0];
        etasave[1] = eta[1];
        signisave = signi;
        delsave = del[0];

        contact.getContourFrame(igearwheel[0])->setPosition(rOP[0]);
        contact.getContourFrame(igearwheel[0])->getOrientation(false).set(0,n1);
        contact.getContourFrame(igearwheel[0])->getOrientation(false).set(1,u1);
        contact.getContourFrame(igearwheel[0])->getOrientation(false).set(2,crossProduct(n1,contact.getContourFrame(igearwheel[0])->getOrientation(false).col(1)));

        contact.getContourFrame(igearwheel[1])->setPosition(rOP[1]);
        contact.getContourFrame(igearwheel[1])->getOrientation(false).set(0, -contact.getContourFrame(igearwheel[0])->getOrientation(false).col(0));
        contact.getContourFrame(igearwheel[1])->getOrientation(false).set(1, -contact.getContourFrame(igearwheel[0])->getOrientation(false).col(1));
        contact.getContourFrame(igearwheel[1])->getOrientation(false).set(2, contact.getContourFrame(igearwheel[0])->getOrientation(false).col(2));

        contact.getGeneralizedRelativePosition(false)(0) = g;
      }
    }
  }

  void ContactKinematicsGearWheelGearWheel::updatewb(SingleContact &contact, int ii) {
    const Vec3 n1 = contact.getContourFrame(igearwheel[0])->evalOrientation().col(0);
    const Vec3 vC1 = contact.getContourFrame(igearwheel[0])->evalVelocity();
    const Vec3 vC2 = contact.getContourFrame(igearwheel[1])->evalVelocity();
    const Vec3 u1 = contact.getContourFrame(igearwheel[0])->evalOrientation().col(1);
    const Vec3 u2 = contact.getContourFrame(igearwheel[1])->evalOrientation().col(1);
    Vec3 R[2], N1, U2;
    for(int j=0; j<2; j++) {
      R[j](0) = signisave*rb[j]*sin(etasave[j])*etasave[j];
      R[j](1) = rb[j]*cos(etasave[j])*etasave[j];
      R[j] = gearwheel[j]->getFrame()->getOrientation()*BasicRotAIKz(signisave*ga[j]+ksave[j]*2*M_PI/z[j])*R[j];
    }
    N1(0) = signisave*sin(etasave[0])/sqrt(1+pow(cos(al)*tan(beta[0]),2));
    N1(1) = cos(etasave[0])/sqrt(1+pow(cos(al)*tan(beta[0]),2));
    N1 = gearwheel[0]->getFrame()->getOrientation()*BasicRotAIKz(signisave*ga[0]+ksave[0]*2*M_PI/z[0])*N1;
    int sign2 = gearwheel[1]->getSolid()?1:-1;
    U2(0) = signisave*sign2*cos(etasave[1]);
    U2(1) = -sign2*sin(etasave[1]);
    U2 = gearwheel[1]->getFrame()->getOrientation()*BasicRotAIKz(signisave*ga[1]+ksave[1]*2*M_PI/z[1])*U2;
    const Vec3 parnPart1 = crossProduct(gearwheel[0]->getFrame()->evalAngularVelocity(),n1);
    const Vec3 paruPart2 = crossProduct(gearwheel[1]->getFrame()->evalAngularVelocity(),u2);
    const Vec3 parWvCParZeta1 = crossProduct(gearwheel[0]->getFrame()->evalAngularVelocity(),R[0]);
    const Vec3 parWvCParZeta2 = crossProduct(gearwheel[1]->getFrame()->evalAngularVelocity(),R[1]);

    SqrMat A(2,NONINIT);
    A(0,0)=-u1.T()*R[0].col(0);
    A(0,1)=u1.T()*R[1].col(0);
    A(1,0)=u2.T()*N1.col(0);
    A(1,1)=n1.T()*U2.col(0);

    Vec b(2,NONINIT);
    b(0)=-u1.T()*(vC2-vC1);
    b(1)=-u2.T()*parnPart1-n1.T()*paruPart2;

    Vec zetad = slvLU(A,b);

    if(contact.isNormalForceLawSetValued())
      contact.getwb(false)(0) += (N1*zetad(0)+parnPart1).T()*(vC2-vC1)+n1.T()*(parWvCParZeta2*zetad(1)-parWvCParZeta1*zetad(0));
    if(contact.isTangentialForceLawSetValuedAndActive())
      throw runtime_error("Tangential force law must be single valued for gear wheel to gear wheel contacts");
  }

}
