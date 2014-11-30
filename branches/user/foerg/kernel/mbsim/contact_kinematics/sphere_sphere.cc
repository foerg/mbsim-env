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
 * Contact: martin.o.foerg@googlemail.com
 *          rzander@users.berlios.de
 */

#include <config.h> 
#include "sphere_sphere.h"
#include "mbsim/contours/sphere.h"
#include "mbsim/utils/eps.h"

using namespace fmatvec;
using namespace std;

namespace MBSim {

  void ContactKinematicsSphereSphere::assignContours(const vector<Contour*> &contour) {
    isphere0 = 0; isphere1 = 1;
    sphere0 = static_cast<Sphere*>(contour[0]);
    sphere1 = static_cast<Sphere*>(contour[1]);
  }

  void ContactKinematicsSphereSphere::updateg(double &g, ContourPointData *cpData, int index) {
    Vec3 Wd = sphere1->getFrame()->getPosition() - sphere0->getFrame()->getPosition();
    double l = nrm2(Wd);
    Wd = Wd/l;
    g = l-sphere0->getRadius()-sphere1->getRadius();
    Vec3 t;
    if(fabs(Wd(0))<epsroot() && fabs(Wd(1))<epsroot()) {
      t(0) = 1.;
      t(1) = 0.;
      t(2) = 0.;
    }
    else {
      t(0) = -Wd(1);
      t(1) = Wd(0);
      t(2) = 0.0;
    }
    t = t/nrm2(t);
    cpData[isphere0].getFrameOfReference().getOrientation().set(0, Wd);
    cpData[isphere1].getFrameOfReference().getOrientation().set(0, -cpData[isphere0].getFrameOfReference().getOrientation().col(0));
    cpData[isphere0].getFrameOfReference().getOrientation().set(1, t);
    cpData[isphere1].getFrameOfReference().getOrientation().set(1, -cpData[isphere0].getFrameOfReference().getOrientation().col(1));
    cpData[isphere0].getFrameOfReference().getOrientation().set(2, crossProduct(Wd,t));
    cpData[isphere1].getFrameOfReference().getOrientation().set(2, cpData[isphere0].getFrameOfReference().getOrientation().col(2));
    cpData[isphere0].getFrameOfReference().getPosition() = sphere0->getFrame()->getPosition() + sphere0->getRadius() * Wd;
    cpData[isphere1].getFrameOfReference().getPosition() = sphere1->getFrame()->getPosition() - sphere1->getRadius() * Wd;
  }

}


//  void ContactKinematicsSphereSphere::stage2(const Vec& g, Vec &gd, vector<ContourPointData> &cpData) {

//    Vec WrPC[2], WvC[2];
//
//    WrPC[isphere1] = cpData[isphere0].Wn*sphere1->getRadius();
//    cpData[isphere1].WrOC = sphere1->getWrOP()+WrPC[isphere1];
//    WrPC[isphere0] = cpData[isphere0].Wn*(-sphere0->getRadius());
//    cpData[isphere0].WrOC = sphere0->getWrOP()+WrPC[isphere0];
//    WvC[isphere0] = sphere0->getWvP()+crossProduct(sphere0->getWomegaC(),WrPC[isphere0]);
//    WvC[isphere1] = sphere1->getWvP()+crossProduct(sphere1->getWomegaC(),WrPC[isphere1]);
//    Vec WvD = WvC[isphere0] - WvC[isphere1];
//    gd(0) = trans(cpData[isphere0].Wn)*WvD;
//
//    if(cpData[isphere0].Wt.cols()) {
//      cpData[isphere0].Wt.col(0) = computeTangential(cpData[isphere0].Wn);
//      if(cpData[isphere0].Wt.cols()==2) {
//        cpData[isphere0].Wt.col(1) = crossProduct(cpData[isphere0].Wn ,cpData[isphere0].Wt.col(0));
//        cpData[isphere1].Wt  = -cpData[isphere0].Wt ; 
//        static Index iT(1,cpData[isphere0].Wt.cols());
//        gd(iT) = trans(cpData[isphere0].Wt)*WvD;
//      }
//    }
//  }
