#include "system.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/environment.h"
#include "mbsim/functions/symbolic_function.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/functions/composite_function.h"
#include "mbsim/links/link.h"
#include "mbsim/observers/frame_observer.h"
#include "mbsim/observers/rigid_body_observer.h"

#include "openmbvcppinterface/arrow.h"
#include "openmbvcppinterface/cuboid.h"

using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {

  // Erdbeschleungigung definieren
  Vec g(3);
  g(1)=-9.81;
  getMBSimEnvironment()->setAccelerationOfGravity(g);

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
  Vector<Var, IndependentVariable> sq(1, NONINIT);

  Vector<Fixed<3>, SymbolicExpression> pos;
  pos(0) = -cos(sq(0)/R)*R;
  pos(1) = -sin(sq(0)/R)*R;
  pos(2) = 0;

  auto trans=new MBSim::SymbolicFunction<Vec3(VecV)>();
  trans->setIndependentVariable(sq);
  trans->setDependentFunction(pos);
  body->setTranslation(trans);

  SymbolicExpression al=M_PI/2+sq(0)/R;

  MBSim::SymbolicFunction<double(VecV)> *angle = new MBSim::SymbolicFunction<double(VecV)>();
  angle->setIndependentVariable(sq);
  angle->setDependentFunction(al);
  body->setRotation(new CompositeFunction<RotMat3(double(VecV))>(new RotationAboutFixedAxis<double>("[0;0;1]"), angle));
  body->setTranslationDependentRotation(true);
  
  body->getFrame("C")->setPlotFeature(position, true);
  body->getFrame("C")->setPlotFeature(MBSim::angle, true);
  body->getFrame("C")->setPlotFeature(velocity, true);
  body->getFrame("C")->setPlotFeature(angularVelocity, true);
  body->getFrame("C")->setPlotFeature(acceleration, true);
  body->getFrame("C")->setPlotFeature(angularAcceleration, true);

  FrameObserver *o = new FrameObserver("AKObserver");
  addObserver(o);
  o->setFrame(body->getFrame("C"));
  o->enableOpenMBVVelocity();
  o->enableOpenMBVAngularVelocity();

  // ----------------------- Visualisierung in OpenMBV --------------------  
  std::shared_ptr<OpenMBV::Cuboid> cuboid=OpenMBV::ObjectFactory::create<OpenMBV::Cuboid>();
  cuboid->setLength(l,h,d);
  cuboid->setDiffuseColor(160./360.,1,1);
  body->setOpenMBVRigidBody(cuboid);

  getFrame("I")->enableOpenMBV();

  RigidBodyObserver *observer = new RigidBodyObserver("RBObserver");
  addObserver(observer);
  observer->setRigidBody(body);
  observer->enableOpenMBVWeight();
  observer->enableOpenMBVJointForce(_colorRepresentation=OpenMBVArrow::absoluteValue);
  observer->enableOpenMBVJointMoment(_colorRepresentation=OpenMBVArrow::absoluteValue);

  setPlotFeatureRecursive(generalizedPosition, true);
  setPlotFeatureRecursive(generalizedVelocity, true);
  setPlotFeatureRecursive(generalizedRelativePosition, true);
  setPlotFeatureRecursive(generalizedRelativeVelocity, true);
  setPlotFeatureRecursive(generalizedForce, true);
}
