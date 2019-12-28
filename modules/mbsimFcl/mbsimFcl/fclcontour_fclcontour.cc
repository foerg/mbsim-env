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
#include "fclcontour_fclcontour.h"
#include "mbsimFcl/fcl_contour.h"
#include "mbsimFcl/fcl_utils.h"
#include "mbsim/utils/contact_utils.h"
#include "mbsim/frames/contour_frame.h"
#include "fcl/narrowphase/distance.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace fcl;

namespace MBSimFcl {

  void ContactKinematicsContourContour::assignContours(const vector<MBSim::Contour*> &contour) {
    icontour0 = 0; icontour1 = 1;
    contour0 = static_cast<FclContour*>(contour[0]);
    contour1 = static_cast<FclContour*>(contour[1]);
    obj0 = shared_ptr<CollisionObject<double>>(new CollisionObject<double>(contour0->getCollisionGeometry()));
    obj1 = shared_ptr<CollisionObject<double>>(new CollisionObject<double>(contour1->getCollisionGeometry()));
  }

  void ContactKinematicsContourContour::updateg(vector<SingleContact> &contact) {
    obj0->setTranslation(Vec3ToVector3d(contour0->getFrame()->evalPosition()));
    obj0->setRotation(SqrMat3ToMatrix3d(contour0->getFrame()->getOrientation()));
    obj1->setTranslation(Vec3ToVector3d(contour1->getFrame()->evalPosition()));
    obj1->setRotation(SqrMat3ToMatrix3d(contour1->getFrame()->getOrientation()));

    CollisionRequest<double> request(maxNumContacts,true);
    CollisionResult<double> result;
    collide<double>(obj0.get(), obj1.get(), request, result);
//    cout << result.isCollision() << endl;
//    cout << result.numContacts() << endl;
    if(result.isCollision()) {
//      cout << "numContacts = " << result.numContacts() << endl;
      for(size_t i=0; i<result.numContacts(); i++) {
        Vec3 n = Vector3dToVec3(result.getContact(i).normal);
        Vec3 r = Vector3dToVec3(result.getContact(i).pos);
        double g = -result.getContact(i).penetration_depth;
        Vec3 t1 = orthonormal(n);
        Vec3 t2 = crossProduct(n,t1);
        contact[i].getGeneralizedRelativePosition(false)(0) = g;
        contact[i].getContourFrame(icontour0)->getOrientation(false).set(0, n);
        contact[i].getContourFrame(icontour0)->getOrientation(false).set(1, t1);
        contact[i].getContourFrame(icontour0)->getOrientation(false).set(2, t2);
        contact[i].getContourFrame(icontour1)->getOrientation(false).set(0, -n);
        contact[i].getContourFrame(icontour1)->getOrientation(false).set(1, -t1);
        contact[i].getContourFrame(icontour1)->getOrientation(false).set(2, t2);
        contact[i].getContourFrame(icontour0)->setPosition(r + n*g/2.);
        contact[i].getContourFrame(icontour1)->setPosition(r - n*g/2.);
      }
      for(int i=result.numContacts(); i<maxNumContacts; i++) {
        contact[i].getGeneralizedRelativePosition(false)(0) = 1;
        contact[i].getContourFrame(icontour0)->setPosition(contour0->getFrame()->getPosition());
        contact[i].getContourFrame(icontour0)->setOrientation(contour0->getFrame()->getOrientation());
        contact[i].getContourFrame(icontour1)->setPosition(contour1->getFrame()->getPosition());
        contact[i].getContourFrame(icontour1)->setOrientation(contour1->getFrame()->getOrientation());
      }
    }
    else {
//      DistanceRequest<double> request;
//      DistanceResult<double> result;
//      distance<double>(obj0.get(), obj1.get(), request, result);
//      g = result.min_distance;
      for(int i=0; i<maxNumContacts; i++) {
      contact[i].getGeneralizedRelativePosition(false)(0) = 1;
      contact[i].getContourFrame(icontour0)->setPosition(contour0->getFrame()->getPosition());
      contact[i].getContourFrame(icontour0)->setOrientation(contour0->getFrame()->getOrientation());
      contact[i].getContourFrame(icontour1)->setPosition(contour1->getFrame()->getPosition());
      contact[i].getContourFrame(icontour1)->setOrientation(contour1->getFrame()->getOrientation());
      }
    }
  }

}
