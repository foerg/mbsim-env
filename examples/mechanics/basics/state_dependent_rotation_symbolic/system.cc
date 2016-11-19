#include "system.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/environment.h"
#include "mbsim/functions/symbolic_function.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/functions/nested_function.h"
#include "mbsim/observers/absolute_kinematics_observer.h"

#include "openmbvcppinterface/arrow.h"
#include "openmbvcppinterface/cuboid.h"

using namespace MBSim;
using namespace casadi;
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
  SX sq=SX::sym("q");

  SX pos=SX::zeros(3);
  pos[0] = -cos(sq[0]/R)*R;
  pos[1] = -sin(sq[0]/R)*R;
  pos[2] = 0;

  body->setTranslation(new SymbolicFunction<Vec3(VecV)>(pos, sq));

  SX al=M_PI/2+sq[0]/R;

  SymbolicFunction<double(VecV)> *angle = new SymbolicFunction<double(VecV)>(al, sq);
  body->setRotation(new NestedFunction<RotMat3(double(VecV))>(new RotationAboutFixedAxis<double>("[0;0;1]"), angle));
  body->setTranslationDependentRotation(true);
  
  body->getFrame("C")->setPlotFeature(globalPosition,enabled);
  body->getFrame("C")->setPlotFeature(globalVelocity,enabled);
  body->getFrame("C")->setPlotFeature(globalAcceleration,enabled);

  body->enableOpenMBVWeight();
  body->enableOpenMBVJointForce();
  body->enableOpenMBVJointMoment();
  AbsoluteKinematicsObserver *o = new AbsoluteKinematicsObserver("Observer");
  addObserver(o);
  o->setFrame(body->getFrame("C"));
 // std::shared_ptr<OpenMBV::Arrow> arrow = OpenMBV::ObjectFactory::create<OpenMBV::Arrow>();
 // arrow->setReferencePoint(OpenMBV::Arrow::fromPoint);
 // arrow->setDiffuseColor(1/3.0, 1, 1);
  o->enableOpenMBVVelocity();

//  arrow = OpenMBV::ObjectFactory::create<OpenMBV::Arrow>();
//  arrow->setReferencePoint(OpenMBV::Arrow::fromPoint);
//  arrow->setType(OpenMBV::Arrow::toDoubleHead);
//  arrow->setDiffuseColor(0.4, 1, 1);
  o->enableOpenMBVAngularVelocity();

  // ----------------------- Visualisierung in OpenMBV --------------------  
  std::shared_ptr<OpenMBV::Cuboid> cuboid=OpenMBV::ObjectFactory::create<OpenMBV::Cuboid>();
  cuboid->setLength(l,h,d);
  cuboid->setDiffuseColor(160./360.,1,1);
  body->setOpenMBVRigidBody(cuboid);

  getFrame("I")->enableOpenMBV();


}
