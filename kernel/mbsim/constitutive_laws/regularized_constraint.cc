/* Copyright (C) 2004-2018 MBSim Development Team
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
#include "mbsim/constitutive_laws/regularized_constraint.h"
#include "mbsim/objectfactory.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  void RegularizedConstraint::init(InitStage stage, const InitConfigSet &config) {
    Element::init(stage, config);
    assert(forceFunc);
    forceFunc->init(stage, config);
  }

  void RegularizedConstraint::initializeUsingXML(DOMElement *element) {
    GeneralizedForceLaw::initializeUsingXML(element);
    DOMElement *e = E(element)->getFirstElementChildNamed(MBSIM%"forceFunction");
    auto *f = ObjectFactory::createAndInit<Function<double(double,double)>>(e->getFirstElementChild());
    setForceFunction(f);
  }

}
