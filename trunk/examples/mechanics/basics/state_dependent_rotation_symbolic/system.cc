#include "system.h"
#include "mbsim/rigid_body.h"
#include "mbsim/frame.h"
#include "mbsim/environment.h"
#include "mbsim/functions/symbolic_functions.h"
#include "mbsim/functions/basic_functions.h"
#include "mbsim/functions/kinematic_functions.h"
#include "mbsim/observers/frame_observer.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "openmbvcppinterface/arrow.h"
#include "openmbvcppinterface/cuboid.h"
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
  double m = 1;
  double l = 2;
  double h = 1;
  double d = 2;
  SymMat Theta(3);
  Theta(1,1) = m*l*l/12.;
  Theta(2,2) = Theta(1,1);

  RigidBody *body = new RigidBody("Box1");
  addObject(body);

  body->setMass(m);
  body->setInertiaTensor(Theta);

  body->setFrameOfReference(getFrame("I"));
  body->setFrameForKinematics(body->getFrame("C"));

  double R = 10;
  vector<SXElement> sq(1);
  sq[0] = SXElement("q1");

  vector<SXElement> pos(3);
  pos[0] = -cos(sq[0]/R)*R;
  pos[1] = -sin(sq[0]/R)*R;
  pos[2] = 0; 

  SXFunction spos(sq,pos);
  
  body->setTranslation(new SymbolicFunction<Vec3(VecV)>(spos));

  vector<SXElement> al(1);
  al[0] = M_PI/2+sq[0]/R;

  SXFunction sangle(sq,al);
  SymbolicFunction<double(VecV)> *angle = new SymbolicFunction<double(VecV)>(sangle);
  body->setRotation(new NestedFunction<RotMat3(double(VecV))>(new RotationAboutFixedAxis<double>("[0;0;1]"), angle));
  body->setTranslationDependentRotation(true);
  
  body->getFrame("C")->setPlotFeature(globalPosition,enabled);
  body->getFrame("C")->setPlotFeature(globalVelocity,enabled);
  body->getFrame("C")->setPlotFeature(globalAcceleration,enabled);

  body->enableOpenMBVWeight();
  body->enableOpenMBVJointForce();
  body->enableOpenMBVJointMoment();
  FrameObserver *o = new FrameObserver("Observer");
  addObserver(o);
  o->setFrame(body->getFrame("C"));
 // OpenMBV::Arrow *arrow = new OpenMBV::Arrow;
 // arrow->setReferencePoint(OpenMBV::Arrow::fromPoint);
 // arrow->setDiffuseColor(1/3.0, 1, 1);
  o->enableOpenMBVVelocity();

//  arrow = new OpenMBV::Arrow;
//  arrow->setReferencePoint(OpenMBV::Arrow::fromPoint);
//  arrow->setType(OpenMBV::Arrow::toDoubleHead);
//  arrow->setDiffuseColor(0.4, 1, 1);
  o->enableOpenMBVAngularVelocity();

#ifdef HAVE_OPENMBVCPPINTERFACE
  // ----------------------- Visualisierung in OpenMBV --------------------  
  OpenMBV::Cuboid *cuboid=new OpenMBV::Cuboid;
  cuboid->setLength(l,h,d);
  cuboid->setDiffuseColor(160./360.,1,1);
  body->setOpenMBVRigidBody(cuboid);

  getFrame("I")->enableOpenMBV();

#endif

}
