#include "system.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/contours/sphere.h"
#include "mbsim/contours/plane.h"
#include "mbsim/contours/sphere.h"
#include "mbsim/constitutive_laws/constitutive_laws.h"
#include "mbsim/links/contact.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematics/kinematics.h"

#include <openmbvcppinterface/invisiblebody.h>

using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {

  // Gravitation
  Vec grav(3);
  grav(1)=-9.81;
  getMBSimEnvironment()->setAccelerationOfGravity(grav);

  addFrame(new FixedRelativeFrame("Os",Vec(3),SqrMat(3,EYE)));

  double phi = M_PI/2;
  SqrMat AWK(3);
  AWK(0,0) = cos(phi);
  AWK(0,1) = -sin(phi);
  AWK(1,1) = cos(phi);
  AWK(1,0) = sin(phi);
  AWK(2,2) = 1;
  addFrame(new FixedRelativeFrame("P",Vec(3),AWK));
  addContour(new Plane("Plane",getFrame("P")));

  RigidBody* body = new RigidBody("Body");
  addObject(body);

  body->setFrameOfReference(getFrame("I"));
  body->setFrameForKinematics(body->getFrame("C"));

  body->setTranslation(new TranslationAlongAxesXYZ<VecV>);
  body->setRotation(new RotationAboutAxesXYZ<VecV>);
  body->setGeneralizedVelocityOfRotation(RigidBody::coordinatesOfAngularVelocityWrtFrameOfReference);
  double m = 0.1;
  double r = 0.1;
  Vec q0(6);
  q0(0) = 0;
  q0(1) = 2.2*r;
  q0(2) = 0;
  body->setGeneralizedInitialPosition(q0);
  //body->setu0("[0;0;0;0;0;-30]");
  body->setGeneralizedInitialVelocity("[3;0;1;0;0;0]");
  //body->setu0("[0;0;0;20;0;-30]");
  //body->setu0("[0;0;0;20;10;-30]");
  //body->setu0("[3;0;0;4;-3;2]");
  //body->setu0("[0;0;0;20;10;-30]");
  body->setMass(m);
  SymMat Theta(3);
  Theta(0,0) =  2./5.*m*r*r;
  Theta(1,1) =  2./5.*m*r*r;
  Theta(2,2) =  2./5.*m*r*r;
  body->setInertiaTensor(Theta);
  Sphere *sphere = new Sphere("Sphere");
  sphere->setRadius(r);
  sphere->enableOpenMBV();
  body->addContour(sphere);

  Contact *cnf = new Contact("Contact");
  cnf->setNormalForceLaw(new UnilateralConstraint);
  cnf->setNormalImpactLaw(new UnilateralNewtonImpact(0.0));
  //cnf->setNormalForceLaw(new LinearRegularizedUnilateralConstraint(1e-6,100));
  //cnf->setTangentialForceLaw(new LinearRegularizedSpatialCoulombFriction(0.3));
  cnf->setTangentialForceLaw(new SpatialCoulombFriction(0.1));
  cnf->setTangentialImpactLaw(new SpatialCoulombImpact(0.1));
  cnf->connect(getContour("Plane"), body->getContour("Sphere"));
  // cnf->setFrictionCoefficient(0.3);
  //cnf->setPlotLevel(2);
  addLink(cnf);

  std::shared_ptr<OpenMBV::InvisibleBody> obj=OpenMBV::ObjectFactory::create<OpenMBV::InvisibleBody>();
  body->setOpenMBVRigidBody(obj);
  body->getFrame("C")->enableOpenMBV(2*r*1.2,0);

  setPlotFeatureRecursive(generalizedPosition, true);
  setPlotFeatureRecursive(generalizedVelocity, true);
  setPlotFeatureRecursive(generalizedRelativePosition, true);
  setPlotFeatureRecursive(generalizedRelativeVelocity, true);
  setPlotFeatureRecursive(generalizedForce, true);
}
