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
 * Contact: markus.ms.schneider@gmail.com
 */

#include <config.h>
#include "mbsim/functions/kinetics/linear_regularized_coulomb_friction.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, LinearRegularizedCoulombFriction)

  Vec LinearRegularizedCoulombFriction::operator()(const Vec &gd, const double& laN) {
    int nFric = gd.size();
    Vec la(nFric, NONINIT);
    double normgd = nrm2(gd(RangeV(0, nFric - 1)));
    if (normgd < gdLim)
      la(RangeV(0, nFric - 1)) = gd(RangeV(0, nFric - 1)) * (-laN * mu / gdLim);
    else
      la(RangeV(0, nFric - 1)) = gd(RangeV(0, nFric - 1)) * (-laN * mu / normgd);
    return la;
  }

  Vec LinearRegularizedCoulombFriction::parDer2(const Vec &gd, const double& laN) {
    int nFric = gd.size();
    Vec la(nFric, NONINIT);
    double normgd = nrm2(gd(RangeV(0, nFric - 1)));
    if (normgd < gdLim)
      la(RangeV(0, nFric - 1)) = gd(RangeV(0, nFric - 1)) * (-mu / gdLim);
    else
      la(RangeV(0, nFric - 1)) = gd(RangeV(0, nFric - 1)) * (-mu / normgd);
    return la;
  }

  void LinearRegularizedCoulombFriction::initializeUsingXML(DOMElement *element) {
    Function<Vec(Vec,double)>::initializeUsingXML(element);
    DOMElement *e;
    e = E(element)->getFirstElementChildNamed(MBSIM%"marginalVelocity");
    if (e)
      gdLim = E(e)->getText<double>();
    e = E(element)->getFirstElementChildNamed(MBSIM%"frictionCoefficient");
    mu = E(e)->getText<double>();
  }

}
