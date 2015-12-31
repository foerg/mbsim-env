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
#include "circlesolid_linesegment.h"
#include "mbsim/contours/line_segment.h"
#include "mbsim/contours/circle_solid.h"


using namespace fmatvec;
using namespace std;

namespace MBSim {

  void ContactKinematicsCircleSolidLineSegment::assignContours(const vector<Contour*> &contour) {
    if(dynamic_cast<CircleSolid*>(contour[0])) {
      icircle = 0; iline = 1;
      circlesolid = static_cast<CircleSolid*>(contour[0]);
      linesegment = static_cast<LineSegment*>(contour[1]);
    } 
    else {
      icircle = 1; iline = 0;
      circlesolid = static_cast<CircleSolid*>(contour[1]);
      linesegment = static_cast<LineSegment*>(contour[0]);
    }
  }

  void ContactKinematicsCircleSolidLineSegment::updateg(double t, double &g, ContourPointData *cpData, int index) {

    const Vec3 WC=circlesolid->getFrame()->getPosition(t);
    const Vec3 WL=linesegment->getFrame()->getPosition(t);
    const Vec3 WLdir=linesegment->getFrame()->getOrientation().col(1);
    const Vec3 WL0=WL-linesegment->getLength()/2*WLdir;
    const double s=WLdir.T() * (-WL0+WC);

    if ((s>=0) && (s<=linesegment->getLength())) {
      cpData[iline].getFrameOfReference().setOrientation(linesegment->getFrame()->getOrientation());
      cpData[icircle].getFrameOfReference().getOrientation(false).set(0, -linesegment->getFrame()->getOrientation(false).col(0));
      cpData[icircle].getFrameOfReference().getOrientation(false).set(1, -linesegment->getFrame()->getOrientation(false).col(1));
      cpData[icircle].getFrameOfReference().getOrientation(false).set(2, linesegment->getFrame()->getOrientation(false).col(2));
      g = cpData[iline].getFrameOfReference().getOrientation(false).col(0).T()*(WC - WL) - circlesolid->getRadius();
      cpData[icircle].getFrameOfReference().setPosition(WC - cpData[iline].getFrameOfReference().getOrientation(false).col(0)*circlesolid->getRadius());
      cpData[iline].getFrameOfReference().setPosition(cpData[icircle].getFrameOfReference().getPosition(false) - cpData[iline].getFrameOfReference().getOrientation(false).col(0)*g);
    }
    else {
      cpData[iline].getFrameOfReference().setPosition((s<0)?WL0:WL+linesegment->getLength()/2*WLdir);
      const Vec3 WrD = -WC + cpData[iline].getFrameOfReference().getPosition(false);
      cpData[icircle].getFrameOfReference().getOrientation(false).set(0, WrD/nrm2(WrD));
      cpData[iline].getFrameOfReference().getOrientation(false).set(0, -cpData[icircle].getFrameOfReference().getOrientation(false).col(0));
      cpData[icircle].getFrameOfReference().getOrientation(false).set(2, circlesolid->getFrame()->getOrientation(false).col(2));
      cpData[iline].getFrameOfReference().getOrientation(false).set(2, linesegment->getFrame()->getOrientation(false).col(2));
      cpData[icircle].getFrameOfReference().getOrientation(false).set(1, crossProduct(cpData[icircle].getFrameOfReference().getOrientation(false).col(2), cpData[icircle].getFrameOfReference().getOrientation(false).col(0)));
      cpData[iline].getFrameOfReference().getOrientation(false).set(1, -cpData[icircle].getFrameOfReference().getOrientation(false).col(1));
      cpData[icircle].getFrameOfReference().setPosition(WC + cpData[icircle].getFrameOfReference().getOrientation(false).col(0)*circlesolid->getRadius());
      g = cpData[icircle].getFrameOfReference().getOrientation(false).col(0).T()*WrD - circlesolid->getRadius();
    }
  }

  void ContactKinematicsCircleSolidLineSegment::updatewb(double t, Vec &wb, double g, ContourPointData *cpData) {
    const Vec3 WC=circlesolid->getFrame()->getPosition(t);
    const Vec3 WL=linesegment->getFrame()->getPosition(t);
    const Vec3 WLdir=linesegment->getFrame()->getOrientation().col(1);
    const Vec3 WL0=WL-linesegment->getLength()/2*WLdir;
    const double s=WLdir.T() * (-WL0+WC);

    if ((s>=0) && (s<=linesegment->getLength())) {
      Vec3 v2 = cpData[icircle].getFrameOfReference().getOrientation(t).col(2);
      Vec3 n1 = cpData[iline].getFrameOfReference().getOrientation(t).col(0);
      Vec3 u1 = cpData[iline].getFrameOfReference().getOrientation().col(1);
      Vec3 u2 = cpData[icircle].getFrameOfReference().getOrientation().col(1);
      Vec3 vC1 = cpData[iline].getFrameOfReference().getVelocity(t);
      Vec3 vC2 = cpData[icircle].getFrameOfReference().getVelocity(t);
      Vec3 Om1 = cpData[iline].getFrameOfReference().getAngularVelocity();
      Vec3 Om2 = cpData[icircle].getFrameOfReference().getAngularVelocity();
      double r = circlesolid->getRadius();

      double ad2 = -v2.T()*(Om2-Om1);
      double ad1 = u1.T()*(vC2-vC1) - r*ad2;
      Vec3 s2 = u2*r;

      wb(0) += n1.T()*(-crossProduct(Om1,vC2-vC1) - crossProduct(Om1,u1)*ad1 + crossProduct(Om2,s2)*ad2);

      if(wb.size() > 1) 
        wb(1) += u1.T()*(-crossProduct(Om1,vC2-vC1) - crossProduct(Om1,u1)*ad1 + crossProduct(Om2,s2)*ad2);
    }
    else
      throw runtime_error("ContactKinematicsCircleSolidLineSegment::updatewb not implemented for contact on edge.");
  }
      
  void ContactKinematicsCircleSolidLineSegment::getCurvatures(Vec &r, ContourPointData* cpData) {
    r(icircle)=circlesolid->getCurvature(cpData[icircle]);
    r(iline)=linesegment->getCurvature(cpData[iline]);
  }

}

