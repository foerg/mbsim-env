#include "system.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/links/spring_damper.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/functions/kinetics/kinetics.h"

#include "openmbvcppinterface/sphere.h"

using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {

  double m = 0.1;
  SymMat Theta(3,EYE,0.000001);
  double g = 9.81;
  double a = 5;
  double theta0 = 20;
  double psid = sqrt(g/(a*cos(theta0/180*M_PI)));

  Vec grav(3);
  grav(1)=-g;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(grav);

  RigidBody *body = new RigidBody("Body");
  addObject(body);
  body->setMass(m);
  body->setInertiaTensor(Theta);
  body->setFrameOfReference(getFrameI());
  Vec r(3); r(1) = a;
  body->addFrame(new FixedRelativeFrame("P",r,SqrMat(3,EYE)));
  body->setFrameOfReference(getFrame("I"));

  body->setFrameForKinematics(body->getFrame("P"));
  body->setRotation(new RotationAboutAxesYZ<VecV>);
  Vec q0(2), u0(2);
  q0(1) = theta0/180*M_PI;
  u0(0) = psid;
  body->setGeneralizedInitialPosition(q0);
  body->setGeneralizedInitialVelocity(u0);

  std::shared_ptr<OpenMBV::Sphere> sphere=OpenMBV::ObjectFactory::create<OpenMBV::Sphere>();
  sphere->setRadius(0.1);
  sphere->setTransparency(0.3);
  body->setOpenMBVRigidBody(sphere);

  body->getFrame("C")->enableOpenMBV();
//  body->getFrame("P")->enableOpenMBV();
  getFrame("I")->enableOpenMBV();
}

