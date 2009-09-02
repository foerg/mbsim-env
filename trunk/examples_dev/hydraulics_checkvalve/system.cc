#include "system.h"

#include "mbsimHydraulics/hydline.h"
#include "mbsimHydraulics/hydnode.h"
#include "mbsimHydraulics/hydnode_mec.h"
#include "mbsim/userfunction.h"
#include "mbsim/contact.h"
#include "mbsim/constitutive_laws.h"
#include "mbsimHydraulics/checkvalve.h"
#include "mbsim/rigid_body.h"
#include "mbsim/spring_damper.h"
#include "mbsimHydraulics/pressure_loss.h"
#include "mbsim/utils/function_library.h"
#include "mbsimControl/object_sensors.h"

#include "mbsim/utils/rotarymatrices.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/arrow.h>
#endif

using namespace std;
using namespace MBSim;
using namespace fmatvec;

System::System(const string &name, bool bilateral, bool unilateral) : Group(name) {

  addFrame("ref", Vec("[.01; .02; .00]"), BasicRotAIKy(1)*BasicRotAIKz(-1.));

  RigidBody * b = new RigidBody("Body");
  addObject(b);
  b->setMass(1);
  b->setInertiaTensor(SymMat(3, EYE));
  b->setFrameOfReference(getFrame("ref"));
  b->addFrame("ref", Vec("[.01; .02; .00]"), BasicRotAIKy(-2.)*BasicRotAIKz(-1.));
  b->setFrameForKinematics(b->getFrame("C"));
  b->setTranslation(new LinearTranslation("[1; 0; 0]"));
  b->setInitialGeneralizedVelocity(.1);

  Checkvalve * lCV = new Checkvalve("lCV");
  addGroup(lCV);
  lCV->setFrameOfReference(b->getFrame("ref"));
  lCV->setLineLength(.05);
  lCV->setLineDiameter(4e-3);
  if (unilateral)
    lCV->setLinePressureLoss(new VariablePressureLossCheckvalveGamma("CheckvalveGamma", lCV->getXOpen(), 1e-4, 6e-3, .1, M_PI/4.));
  else
    lCV->setLinePressureLoss(new RegularizedVariablePressureLossCheckvalveGamma("CheckvalveGamma", lCV->getXOpen(), 1e-4, 6e-3, .1, M_PI/4.));
  lCV->setBallMass(0.1);
  lCV->setSpringForceFunction(new LinearSpringDamperForce(200, 5, 5e-3));
  double c=1e5;
  double d=1e3;
  if (unilateral) {
    lCV->setSeatContactForceLaw(new UnilateralConstraint);
    lCV->setSeatContactImpactLaw(new UnilateralNewtonImpact);
  }
  else
    lCV->setSeatContactForceLaw(new RegularizedUnilateralConstraint(new LinearRegularizedUnilateralConstraint(c, d)));
  lCV->setMaximalOpening(.003);
  if (unilateral) {
    lCV->setMaximalContactForceLaw(new UnilateralConstraint);
    lCV->setMaximalContactImpactLaw(new UnilateralNewtonImpact);
  }
  else
    lCV->setMaximalContactForceLaw(new RegularizedUnilateralConstraint(new LinearRegularizedUnilateralConstraint(c, d)));

  ConstrainedNodeMec * n1 = new ConstrainedNodeMec("n1");
  n1->setpFunction(new Function1_SS_from_VS(new  TabularFunction1_VS(Vec("[0; .9; 1.1; 2.9; 3.1; 5]")*.1, "[4e5; 4e5; 2e5; 2e5; 4e5; 4e5]")));
  addLink(n1);
  n1->addOutFlow(lCV->getLine());

  ConstrainedNodeMec * n2 = new ConstrainedNodeMec("n2");
  addLink(n2);
  n2->setpFunction(new ConstantFunction1<double,double>(3e5));
  n2->addInFlow(lCV->getLine());

#ifdef HAVE_OPENMBVCPPINTERFACE
  getFrame("I")->enableOpenMBV(.025, 1);
  getFrame("ref")->enableOpenMBV(.025, 1);
  b->getFrame("C")->enableOpenMBV(.025, 1);
  b->getFrame("ref")->enableOpenMBV(.025, 1);
  lCV->enableOpenMBVFrames();
  lCV->enableOpenMBVArrows();
  lCV->enableOpenMBVBodies();
  n1->enableOpenMBVArrows(.01);
  n2->enableOpenMBVArrows(.01);
#endif
}
