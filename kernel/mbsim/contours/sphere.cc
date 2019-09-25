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
 */

#include <config.h>
#include "mbsim/contours/sphere.h"
#include "mbsim/frames/frame.h"

#include <openmbvcppinterface/sphere.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, Sphere)

  void Sphere::init(InitStage stage, const InitConfigSet &config) {
    if(stage==plotting) {
      if(plotFeature[openMBV] && openMBVRigidBody)
        static_pointer_cast<OpenMBV::Sphere>(openMBVRigidBody)->setRadius(r);
    }
    RigidContour::init(stage, config);
  }

  Vec3 Sphere::evalKs(const fmatvec::Vec2 &zeta) {
    Vec3 Ks(NONINIT);
    double a = zeta(0);
    double b = zeta(1);
    Ks(0) = -r*sin(a)*cos(b);
    Ks(1) = r*cos(a)*cos(b);
    Ks(2) = 0;
    return Ks;
  }

  Vec3 Sphere::evalKt(const fmatvec::Vec2 &zeta) {
    Vec3 Kt(NONINIT);
    double a = zeta(0);
    double b = zeta(1);
    Kt(0) = -r*cos(a)*sin(b);
    Kt(1) = -r*sin(a)*sin(b);
    Kt(2) = r*cos(b);
    return Kt;
  }

  Vec3 Sphere::evalParDer1Ku(const fmatvec::Vec2 &zeta) {
    Vec3 parDer1Ku(NONINIT);
    double a = zeta(0);
    parDer1Ku(0) = -cos(a);
    parDer1Ku(1) = -sin(a);
    parDer1Ku(2) = 0;
    return parDer1Ku;
  }

  Vec3 Sphere::evalParDer2Ku(const fmatvec::Vec2 &zeta) {
    static Vec3 parDer2Ku;
    return parDer2Ku;
  }

  Vec3 Sphere::evalParDer1Kv(const fmatvec::Vec2 &zeta) {
    Vec3 parDer1Kv(NONINIT);
    double a = zeta(0);
    double b = zeta(1);
    parDer1Kv(0) = sin(a)*sin(b);
    parDer1Kv(1) = -cos(a)*sin(b);
    parDer1Kv(2) = 0;
    return parDer1Kv;
  }

  Vec3 Sphere::evalParDer2Kv(const fmatvec::Vec2 &zeta) {
    Vec3 parDer2Kv(NONINIT);
    double a = zeta(0);
    double b = zeta(1);
    parDer2Kv(0) = -cos(a)*cos(b);
    parDer2Kv(1) = -sin(a)*cos(b);
    parDer2Kv(2) = -sin(b);
    return parDer2Kv;
  }

  Vec3 Sphere::evalParDer1Wn(const Vec2 &zeta) {
    Vec3 parDer1Wn(NONINIT);
    double a = zeta(0);
    double b = zeta(1);
    parDer1Wn(0) = -sin(a)*cos(b);
    parDer1Wn(1) = cos(a)*cos(b);
    parDer1Wn(2) = 0;
    return parDer1Wn;
  }

  Vec3 Sphere::evalParDer2Wn(const Vec2 &zeta) {
    Vec3 parDer2Wn(NONINIT);
    double a = zeta(0);
    double b = zeta(1);
    parDer2Wn(0) = -cos(a)*sin(b);
    parDer2Wn(1) = -sin(a)*sin(b);
    parDer2Wn(2) = cos(b);
    return parDer2Wn;
  }

  Vec2 Sphere::evalZeta(const fmatvec::Vec3 &WrPoint) {
    Vec3 SrPoint = R->evalOrientation().T() * (WrPoint - R->evalPosition());
    Vec2 zeta;
    double r = nrm2(SrPoint);
    zeta(0) = acos(SrPoint(2) / r); // inclination
    zeta(1) = atan2(SrPoint(1), SrPoint(0)); // azimuth
    return zeta;
  }

  void Sphere::initializeUsingXML(DOMElement *element) {
    RigidContour::initializeUsingXML(element);
    DOMElement* e;
    e=E(element)->getFirstElementChildNamed(MBSIM%"radius");
    setRadius(E(e)->getText<double>());
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBV");
    if(e) {
      OpenMBVColoredBody ombv;
      ombv.initializeUsingXML(e);
      openMBVRigidBody=ombv.createOpenMBV<OpenMBV::Sphere>();
    }
  }

}
