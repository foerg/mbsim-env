#include "system.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/links/link.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematics/kinematics.h"

#include "openmbvcppinterface/ivbody.h"

using namespace MBSim;
using namespace fmatvec;
using namespace std;

Pendulum::Pendulum(const string &projectName) : DynamicSystemSolver(projectName) {

  Vec grav(3);
  grav(1)=-9.81;
  getMBSimEnvironment()->setAccelerationOfGravity(grav);

  double mStab = 0.2;
  double lStab = 0.3;
  double JStab = 1.0/12.0 * mStab * lStab * lStab; 
  double a1 = 0.15*lStab;
  double a2 = 0.15*lStab;

  Vec KrRP(3), KrCR(3);
  SymMat Theta(3);

  RigidBody* stab1 = new RigidBody("Stab1");
  this->addObject(stab1);
  KrCR(0) = a1;

  stab1->addFrame(new FixedRelativeFrame("R",KrCR,SqrMat(3,EYE)));

  stab1->setFrameOfReference(getFrame("I"));
  stab1->setFrameForKinematics(stab1->getFrame("R"));

  stab1->setMass(mStab);
  Theta(2,2) = JStab;
  stab1->setInertiaTensor(Theta);
  stab1->setRotation(new RotationAboutFixedAxis<VecV>(Vec("[0;0;1]")));

  std::shared_ptr<OpenMBV::IvBody> obj=OpenMBV::ObjectFactory::create<OpenMBV::IvBody>();
  obj->setIvFileName("wrl/pendel1.wrl");
  obj->setScaleFactor(0.1*0.3);
  obj->setInitialRotation(0,0,M_PI/2);
  stab1->setOpenMBVRigidBody(obj);
  stab1->setOpenMBVFrameOfReference(stab1->getFrame("R"));

  RigidBody* stab2 = new RigidBody("Stab2");
  this->addObject(stab2);
  KrRP(0) = lStab/2;
  KrRP(2) = 0.006;
  stab1->addFrame(new FixedRelativeFrame("P",KrRP,SqrMat(3,EYE),stab1->getFrame("R")));
  KrCR(0) = a2;
  stab2->addFrame(new FixedRelativeFrame("R",-KrCR,SqrMat(3,EYE)));
  stab2->setFrameOfReference(stab1->getFrame("P"));
  stab2->setFrameForKinematics(stab2->getFrame("R"));
  stab2->setMass(mStab);
  Theta(2,2) = JStab;
  stab2->setInertiaTensor(Theta);
  stab2->setRotation(new RotationAboutFixedAxis<VecV>(Vec("[0;0;1]")));
  stab2->setGeneralizedInitialPosition(Vec("[-1.6]"));

  obj=OpenMBV::ObjectFactory::create<OpenMBV::IvBody>();
  obj->setIvFileName("wrl/pendel2.wrl");
  obj->setScaleFactor(0.1*0.3);
  obj->setInitialRotation(0,0,M_PI/2);
  stab2->setOpenMBVRigidBody(obj);
  stab2->setOpenMBVFrameOfReference(stab2->getFrame("R"));

  setPlotFeatureRecursive(generalizedPosition, true);
  setPlotFeatureRecursive(generalizedVelocity, true);
  setPlotFeatureRecursive(generalizedRelativePosition, true);
  setPlotFeatureRecursive(generalizedRelativeVelocity, true);
  setPlotFeatureRecursive(generalizedForce, true);

}
