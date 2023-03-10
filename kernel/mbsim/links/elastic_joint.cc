/* Copyright (C) 2004-2016 MBSim Development Team
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
#include "mbsim/links/elastic_joint.h"
#include "mbsim/objectfactory.h"
#include "mbsim/utils/eps.h"
#include "mbsim/utils/rotarymatrices.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, ElasticJoint)

  ElasticJoint::ElasticJoint(const string &name) : FloatingFrameLink(name) {
  }

  ElasticJoint::~ElasticJoint() {
    delete func;
  }

  void ElasticJoint::updateGeneralizedForces() {
    if(func) lambda = -(*func)(evalGeneralizedRelativePosition(),evalGeneralizedRelativeVelocity());
    updla = false;
  }

  void ElasticJoint::updatexd() {
    if(integrateGeneralizedRelativeVelocityOfRotation)
      xd = evalGeneralizedRelativeVelocity()(iM);
  }

  void ElasticJoint::init(InitStage stage, const InitConfigSet &config) {
    if(stage==preInit) {
      if(momentDir.cols()==1 and not integrateGeneralizedRelativeVelocityOfRotation) {
	msg(Warn) << "Evaluation of generalized relative position may be wrong for spatial rotation. In this case turn on integration of generalized relative velocity of rotation." << endl;
      }
    }
    else if(stage==unknownStage) {
      if(func and (func->getRetSize().first!=forceDir.cols()+momentDir.cols())) throwError("Size of generalized forces does not match!");
    }
    FloatingFrameLink::init(stage, config);
    if(func) func->init(stage, config);
  }

  VecV ElasticJoint::evalGeneralizedRelativePositionOfRotation() {
    if(integrateGeneralizedRelativeVelocityOfRotation)
      return x;
    else
      return evalGlobalMomentDirection().T()*frame[0]->evalOrientation()*AIK2Phi(evalGlobalRelativeOrientation());
  }

  void ElasticJoint::initializeUsingXML(DOMElement *element) {
    FloatingFrameLink::initializeUsingXML(element);
    DOMElement *e = E(element)->getFirstElementChildNamed(MBSIM%"forceDirection");
    if(e) setForceDirection(E(e)->getText<Mat>(3,0));
    e = E(element)->getFirstElementChildNamed(MBSIM%"momentDirection");
    if(e) setMomentDirection(E(e)->getText<Mat>(3,0));
    e=E(element)->getFirstElementChildNamed(MBSIM%"generalizedForceFunction");
    if(e) {
      auto *f=ObjectFactory::createAndInit<Function<VecV(VecV,VecV)>>(e->getFirstElementChild());
      setGeneralizedForceFunction(f);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"integrateGeneralizedRelativeVelocityOfRotation");
    if(e) setIntegrateGeneralizedRelativeVelocityOfRotation(E(e)->getText<bool>());
  }

}
