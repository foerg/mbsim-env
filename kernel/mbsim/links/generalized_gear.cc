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
 * Contact: martin.o.foerg@gmail.com
 */

#include <config.h>
#include "mbsim/links/generalized_gear.h"
#include "mbsim/objects/rigid_body.h"
#include <mbsim/constitutive_laws/generalized_force_law.h>
#include <mbsim/constitutive_laws/bilateral_impact.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, GeneralizedGear)

  GeneralizedGear::~GeneralizedGear() {
    delete fl;
    if(il) delete il;
  }

  bool GeneralizedGear::isSetValued() const {
    return fl->isSetValued();
  }

  void GeneralizedGear::setGeneralizedForceLaw(GeneralizedForceLaw * fl_) {
    fl=fl_;
    fl->setParent(this);
  }

  void GeneralizedGear::updateGeneralizedForces() {
    if(isSetValued())
      lambda = evalla();
    else
      lambda(0) = (*fl)(evalGeneralizedRelativePosition()(0),evalGeneralizedRelativeVelocity()(0));
    updla = false;
  }

  void GeneralizedGear::init(InitStage stage, const InitConfigSet &config) {
    if(stage==resolveStringRef) {
      if(not saved_gearOutput.empty())
        setGearOutput(getByPath<RigidBody>(saved_gearOutput));
      for(const auto & i : saved_gearInput)
        body.push_back(getByPath<RigidBody>(i));
      if(not body[0])
        throwError("No gear output given!");
      if(body.size()==1)
        throwError("No gear inputs given!");
    }
    else if(stage==unknownStage) {
      for(auto & i : body) {
        if(i->getGeneralizedVelocitySize()!=1)
          throwError("rigid bodies must have 1 dof!");
      }
      if(fl->isSetValued()) {
        il = new BilateralImpact;
        il->setParent(this);
      }
    }
    RigidBodyLink::init(stage, config);
    if(fl) fl->init(stage, config);
    if(il) il->init(stage, config);
  }

  void GeneralizedGear::initializeUsingXML(DOMElement* element) {
    RigidBodyLink::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"gearOutput");
    saved_gearOutput=E(e)->getAttribute("ref");
    e=e->getNextElementSibling();
    while(e && E(e)->getTagName()==MBSIM%"gearInput") {
      saved_gearInput.push_back(E(e)->getAttribute("ref"));
      ratio.push_back(stod(E(e)->getAttribute("ratio")));
      e=e->getNextElementSibling();
    }
    e = E(element)->getFirstElementChildNamed(MBSIM%"generalizedForceLaw");
    setGeneralizedForceLaw(ObjectFactory::createAndInit<GeneralizedForceLaw>(e->getFirstElementChild()));
  }

}
