#include "system.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/environment.h"
#include "mbsim/functions/symbolic_function.h"
#include "mbsim/observers/frame_observer.h"

#include "openmbvcppinterface/arrow.h"
#include "openmbvcppinterface/cube.h"

using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {

  // Erdbeschleungigung definieren
  Vec g(3);
  g(1)=-9.81;
  getMBSimEnvironment()->setAccelerationOfGravity(g);

  // Parameter der Körper
  double m1 = 3;
  SymMat Theta1(3,EYE);
  double h1 = 0.5;
  double freq1 = M_PI;
  double freq2 = M_PI/3;
  double v0y = 1;

  RigidBody *body1 = new RigidBody("Box1");
  addObject(body1);

  body1->setMass(m1);
  body1->setInertiaTensor(Theta1);

  body1->setFrameOfReference(getFrame("I"));
  body1->setFrameForKinematics(body1->getFrame("C"));

  Vector<Fixed<2>, IndependentVariable> sq(NONINIT);

  IndependentVariable st;

  Vector<Fixed<3>, SymbolicExpression> pos;
  pos(0) = cos(sq(0));
  pos(1) = sin(sq(0));
  pos(2) = sq(1);
  
  MBSim::SymbolicFunction<Vec3(VecV,double)> *position = new MBSim::SymbolicFunction<Vec3(VecV,double)>();
  position->setIndependentVariable1(sq);
  position->setIndependentVariable2(st);
  position->setDependentFunction(pos);
  body1->setTranslation(position);
  
  body1->setGeneralizedInitialVelocity("[0;1]");

  body1->getFrame("C")->setPlotFeature(MBSim::position, true);
  body1->getFrame("C")->setPlotFeature(angle, true);
  body1->getFrame("C")->setPlotFeature(velocity, true);
  body1->getFrame("C")->setPlotFeature(angularVelocity, true);
  body1->getFrame("C")->setPlotFeature(acceleration, true);
  body1->getFrame("C")->setPlotFeature(angularAcceleration, true);

  FrameObserver *o = new FrameObserver("Observer");
  addObserver(o);
  o->setFrame(body1->getFrame("C"));
  o->enableOpenMBVVelocity();

  // ----------------------- Visualisierung in OpenMBV --------------------  
  std::shared_ptr<OpenMBV::Cube> cuboid=OpenMBV::ObjectFactory::create<OpenMBV::Cube>();
  cuboid->setLength(h1);
  cuboid->setDiffuseColor(240./360.,1,1);
  body1->setOpenMBVRigidBody(cuboid);

  getFrame("I")->enableOpenMBV();

  setPlotFeatureRecursive(generalizedPosition, true);
  setPlotFeatureRecursive(generalizedVelocity, true);
}
