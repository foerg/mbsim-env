/* Copyright (C) 2004-2014 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include "mbsim/constitutive_laws/planar_stribeck_friction.h"
#include "mbsim/objectfactory.h"
#include "mbsim/utils/utils.h"
#include "mbsim/utils/nonsmooth_algebra.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, PlanarStribeckFriction)

  Vec PlanarStribeckFriction::project(const Vec& la, const Vec& gdn, double laN, double r) {
    return Vec(1, INIT, proxCT2D(la(0) - r * gdn(0), (*fmu)(0) * fabs(laN)));
  }

  Mat PlanarStribeckFriction::diff(const Vec& la, const Vec& gdn, double laN, double r) {
    double argT = la(0) - r * gdn(0);
    Mat d(1, 3, NONINIT);
    if (fabs(argT) < (*fmu)(0) * fabs(laN)) {
      d(0, 0) = 1;
      d(0, 1) = -r;
      d(0, 2) = 0;
    }
    else {
      d(0, 0) = 0;
      d(0, 1) = 0;
      d(0, 2) = sign(argT) * sign(laN) * (*fmu)(0);
    }
    return d;
  }

  Vec PlanarStribeckFriction::solve(const SqrMat& G, const Vec& gdn, double laN) {
    double laNmu = fabs(laN) * (*fmu)(0);
    double sdG = -gdn(0) / G(0, 0);
    if (fabs(sdG) <= laNmu)
      return Vec(1, INIT, sdG);
    else
      return Vec(1, INIT, (laNmu <= sdG) ? laNmu : -laNmu);
  }

  bool PlanarStribeckFriction::isFulfilled(const Vec& la, const Vec& gdn, double laN, double laTol, double gdTol) {
    if (fabs(la(0) + sign(gdn(0)) * (*fmu)(0) * fabs(laN)) <= laTol)
      return true;
    else if (fabs(la(0)) <= (*fmu)(0) * fabs(laN) + laTol && fabs(gdn(0)) <= gdTol)
      return true;
    else
      return false;
  }

  Vec PlanarStribeckFriction::dlaTdlaN(const Vec& gd) {
    return Vec(1, INIT, -(*fmu)(fabs(gd(0))) * sign(gd(0)));
  }

  void PlanarStribeckFriction::initializeUsingXML(DOMElement *element) {
    FrictionForceLaw::initializeUsingXML(element);
    DOMElement *e;
    e = E(element)->getFirstElementChildNamed(MBSIM%"frictionFunction");
    setFrictionFunction(ObjectFactory::createAndInit<Function<double(double)>>(e->getFirstElementChild()));
  }

}
