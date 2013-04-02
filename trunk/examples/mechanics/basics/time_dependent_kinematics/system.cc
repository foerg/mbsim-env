#include "system.h"
#include "mbsim/rigid_body.h"
#include "mbsim/frame.h"
#include "mbsim/environment.h"
#include "mbsim/utils/casadi_function.h"
#include "mbsim/observers/frame_observer.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "openmbvcppinterface/arrow.h"
#include "openmbvcppinterface/cube.h"
#include "openmbvcppinterface/coilspring.h"
#endif

using namespace MBSim;
using namespace CasADi;
using namespace fmatvec;
using namespace std;


System::System(const string &projectName) : DynamicSystemSolver(projectName) {

  // Erdbeschleungigung definieren
  Vec g(3);
  g(1)=-9.81;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(g);

  // Parameter der Körper
  double m1 = 3;
  SymMat Theta1(3,EYE);
  double h1 = 2;
  double freq1 = M_PI;
  double freq2 = M_PI/3;
  double v0y = 1;

  RigidBody *body1 = new RigidBody("Box1");
  addObject(body1);

  body1->setMass(m1);
  body1->setInertiaTensor(Theta1);

  body1->setFrameOfReference(getFrame("I"));
  body1->setFrameForKinematics(body1->getFrame("C"));

  SX x("x");
  vector<SX> fexp(3);
  fexp[0] = sin(freq1*x + M_PI/2);
  fexp[1] = v0y*x; 
  fexp[2] = 0; 
  SXFunction foo(x,fexp);

  CasadiFunction1<Vec3> *f = new CasadiFunction1<Vec3>(foo);
  body1->setTranslation(new TimeDependentTranslation(f));

  SX fexp2 = 5*sin(freq2*x);
  SXFunction foo2(x,fexp2);

  CasadiFunction1<double> *f2 = new CasadiFunction1<double>(foo2);
  body1->setRotation(new TimeDependentRotationAboutFixedAxis(f2,"[0;0;1]"));

  body1->getFrame("C")->setPlotFeature(globalPosition,enabled);
  body1->getFrame("C")->setPlotFeature(globalVelocity,enabled);
  body1->getFrame("C")->setPlotFeature(globalAcceleration,enabled);

  FrameObserver *o = new FrameObserver("Observer");
  addObserver(o);
  o->setFrame(body1->getFrame("C"));
  o->enableOpenMBVVelocity();
  o->enableOpenMBVAngularVelocity();

#ifdef HAVE_OPENMBVCPPINTERFACE
  // ----------------------- Visualisierung in OpenMBV --------------------  
  OpenMBV::Cube *cuboid=new OpenMBV::Cube;
  cuboid->setLength(h1);
  cuboid->setStaticColor(0.0);
  body1->setOpenMBVRigidBody(cuboid);

#endif
  // Just to have somtething to integrate ;-)
  RigidBody *body2 = new RigidBody("Rod2");
  addObject(body2);
  body2->setFrameOfReference(getFrame("I"));
  body2->setFrameForKinematics(body2->getFrame("C"));
  body2->setMass(1);
  body2->setInertiaTensor(SymMat3(EYE));
  body2->setTranslation(new LinearTranslation("[0; 1; 0]"));

}
