#include "system.h"
#include "mbsim/frame.h"
#include "mbsim/rigid_body.h"
#include "mbsim/spring_damper.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematic_functions.h"
#include "mbsim/functions/kinetic_functions.h"
//#include "mbsim/functions/symbolic_functions.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "openmbvcppinterface/cube.h"
#include "openmbvcppinterface/coilspring.h"
#endif

using namespace MBSim;
using namespace fmatvec;
using namespace std;
//using namespace CasADi;

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

  //vector<SXElement> sq(2);
  //sq[0] = SXElement("al");
  //sq[1] = SXElement("be");

  //vector<SXElement> pos(3);
  //pos[0] = a*cos(sq[0])*sin(sq[1]);
  //pos[1] = -a*cos(sq[1]);
  //pos[2] = -a*sin(sq[0])*sin(sq[1]); 

  //SXFunction spos(sq,pos);
  //
  //body->setTranslation(new SymbolicFunction<Vec3(VecV)>(spos));

  body->setFrameForKinematics(body->getFrame("P"));
  body->setRotation(new RotationAboutAxesYZ<VecV>);
  Vec q0(2), u0(2);
  q0(1) = theta0/180*M_PI;
  u0(0) = psid;
  body->setInitialGeneralizedPosition(q0);
  body->setInitialGeneralizedVelocity(u0);

  OpenMBV::Sphere *sphere=new OpenMBV::Sphere;
  sphere->setRadius(0.1);
  sphere->setTransparency(0.3);
  body->setOpenMBVRigidBody(sphere);

  body->getFrame("C")->enableOpenMBV();
//  body->getFrame("P")->enableOpenMBV();
  getFrame("I")->enableOpenMBV();
}

