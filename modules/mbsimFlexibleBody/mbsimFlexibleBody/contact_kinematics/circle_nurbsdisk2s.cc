/* Copyright (C) 2004-2015 MBSim Development Team
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

#include <config.h> 
#include "mbsimFlexibleBody/contact_kinematics/circle_nurbsdisk2s.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsimFlexibleBody/functions_contact.h"
#include "mbsim/utils/nonlinear_algebra.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  ContactKinematicsCircleNurbsDisk2s::ContactKinematicsCircleNurbsDisk2s()  {
  }

  ContactKinematicsCircleNurbsDisk2s::~ContactKinematicsCircleNurbsDisk2s() = default;

  void ContactKinematicsCircleNurbsDisk2s::assignContours(const vector<Contour*> &contour) {
    if(dynamic_cast<Circle*>(contour[0])) {
      icircle = 0;
      inurbsdisk = 1;
      circle = static_cast<Circle*>(contour[0]);
      nurbsdisk = static_cast<NurbsDisk2s*>(contour[1]);
    }
    else {
      icircle = 1;
      inurbsdisk = 0;
      circle = static_cast<Circle*>(contour[1]);
      nurbsdisk = static_cast<NurbsDisk2s*>(contour[0]);
    }
  }

  void ContactKinematicsCircleNurbsDisk2s::updateg(SingleContact &contact, int i) {
    auto *func= new FuncPairCircleNurbsDisk2s(circle, nurbsdisk); // root function for searching contact parameters
    NewtonMethod search(func,nullptr);
    nextis(0) = search.solve(curis(0));
    contact.getContourFrame(icircle)->setEta(nextis(0)); // get contact parameter

    // point on the circle
    fmatvec::Vec P_circle(3,fmatvec::INIT,0);
    P_circle(0) = cos(contact.getContourFrame(icircle)->getEta(false));
    P_circle(1) = sin(contact.getContourFrame(icircle)->getEta(false));
    P_circle = circle->getFrame()->evalPosition() + circle->getRadius() * circle->getFrame()->evalOrientation() * P_circle;
    contact.getContourFrame(icircle)->setPosition(P_circle); // position of the point in world coordinates

    contact.getContourFrame(inurbsdisk)->setZeta(nurbsdisk->transformCW(nurbsdisk->evalOrientation().T()*(contact.getContourFrame(icircle)->getPosition(false) - nurbsdisk->evalPosition()))(RangeV(0,1)));

    double g;
    if(nurbsdisk->isZetaOutside(contact.getContourFrame(inurbsdisk)->getZeta(false)))
      g = 1.;
    else {

      contact.getContourFrame(inurbsdisk)->setPosition(nurbsdisk->evalPosition(contact.getContourFrame(inurbsdisk)->getZeta(false)));
      contact.getContourFrame(inurbsdisk)->getOrientation(false).set(0, nurbsdisk->evalWn(contact.getContourFrame(inurbsdisk)->getZeta(false)));
      contact.getContourFrame(inurbsdisk)->getOrientation(false).set(1, nurbsdisk->evalWu(contact.getContourFrame(inurbsdisk)->getZeta(false)));
      contact.getContourFrame(inurbsdisk)->getOrientation(false).set(2, nurbsdisk->evalWv(contact.getContourFrame(inurbsdisk)->getZeta(false)));

      contact.getContourFrame(icircle)->getOrientation(false).set(0, -contact.getContourFrame(inurbsdisk)->getOrientation(false).col(0));
      contact.getContourFrame(icircle)->getOrientation(false).set(1, -contact.getContourFrame(inurbsdisk)->getOrientation(false).col(1));
      contact.getContourFrame(icircle)->getOrientation(false).set(2,  contact.getContourFrame(inurbsdisk)->getOrientation(false).col(2));   // to have a legal framework the second tangent is not the negative of the tanget of the disk

      g = contact.getContourFrame(inurbsdisk)->getOrientation(false).col(0).T() * (contact.getContourFrame(icircle)->getPosition(false) - contact.getContourFrame(inurbsdisk)->getPosition(false));
    }
    contact.getGeneralizedRelativePosition(false)(0) = g;

    delete func;
  }

}
