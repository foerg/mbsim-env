/* Copyright (C) 2004-2010 MBSim Development Team
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
#include "mbsim/constraints/generalized_acceleration_constraint.h"
#include <mbsim/constitutive_laws/bilateral_constraint.h>
#include "mbsim/objects/rigid_body.h"
#include "mbsim/links/generalized_acceleration_excitation.h"
#include "mbsim/dynamic_system.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, GeneralizedAccelerationConstraint)

  void GeneralizedAccelerationConstraint::init(InitStage stage, const InitConfigSet &config) {
    GeneralizedDualConstraint::init(stage, config);
    f->init(stage, config);
  }

  void GeneralizedAccelerationConstraint::calcxSize() {
    xSize = bd->getGeneralizedPositionSize()+bd->getGeneralizedVelocitySize();
  }

  void GeneralizedAccelerationConstraint::updatexd() {
    xd(0,bd->getGeneralizedPositionSize()-1) = x(bd->getGeneralizedPositionSize(),bd->getGeneralizedPositionSize()+bd->getGeneralizedVelocitySize()-1);
    xd(bd->getGeneralizedPositionSize(),bd->getGeneralizedPositionSize()+bd->getGeneralizedVelocitySize()-1) = (*f)(x,getTime());
  }

  void GeneralizedAccelerationConstraint::updateGeneralizedCoordinates() {
    if(bi) {
      bd->setqRel(bi->evalGeneralizedPosition()+x(0,bd->getGeneralizedPositionSize()-1));
      bd->setuRel(bi->evalGeneralizedVelocity()+x(bd->getGeneralizedPositionSize(),bd->getGeneralizedPositionSize()+bd->getGeneralizedVelocitySize()-1));
    }
    else {
      bd->setqRel(x(0,bd->getGeneralizedPositionSize()-1));
      bd->setuRel(x(bd->getGeneralizedPositionSize(),bd->getGeneralizedPositionSize()+bd->getGeneralizedVelocitySize()-1));
    }
    updGC = false;
  }

  void GeneralizedAccelerationConstraint::updateGeneralizedJacobians(int jj) {
    if(bi) {
      bd->getJRel(0,false).set(Range<Var,Var>(0,bi->getGeneralizedVelocitySize()-1),Range<Var,Var>(0,bi->gethSize()-1),bi->evalJRel());
      bd->setjRel((*f)(x,getTime()));
    }
    else
      bd->setjRel((*f)(x,getTime()));
    updGJ = false;
  }

  void GeneralizedAccelerationConstraint::initializeUsingXML(DOMElement* element) {
    GeneralizedDualConstraint::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"initialState");
    if(e)
      x0 = E(e)->getText<Vec>();
    e=E(element)->getFirstElementChildNamed(MBSIM%"generalConstraintFunction");
    if(e) {
      auto *f=ObjectFactory::createAndInit<Function<VecV(VecV,double)> >(e->getFirstElementChild());
      setGeneralConstraintFunction(f);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"timeDependentConstraintFunction");
    if(e) {
      auto *f=ObjectFactory::createAndInit<Function<VecV(double)> >(e->getFirstElementChild());
      setTimeDependentConstraintFunction(f);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"stateDependentConstraintFunction");
    if(e) {
      auto *f=ObjectFactory::createAndInit<Function<VecV(VecV)> >(e->getFirstElementChild());
      setStateDependentConstraintFunction(f);
    }
  }

  void GeneralizedAccelerationConstraint::setUpInverseKinetics() {
    GeneralizedAccelerationExcitation *ke = new GeneralizedAccelerationExcitation(string("GeneralizedAccelerationExcitation_")+name);
    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(ke);
    if(bi) ke->connect(bi,bd);
    else ke->connect(bd);
    ke->setExcitationFunction(f);
    ke->setGeneralizedForceLaw(new BilateralConstraint);
    ke->setSupportFrame(support);
    ke->plotFeature[generalizedRelativePosition] = false;
    ke->plotFeature[generalizedRelativeVelocity] = false;
    link = ke;
  }

}
