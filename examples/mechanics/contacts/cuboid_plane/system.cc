#include "system.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/constitutive_laws/constitutive_laws.h"
#include "mbsim/links/contact.h"
#include "mbsim/contours/plane.h"
#include "mbsim/contours/cuboid.h"
#include "openmbvcppinterface/cube.h"
#include "mbsim/environment.h"


using namespace fmatvec;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {
  Vec grav(3);
  grav(1)=-9.81;
  getMBSimEnvironment()->setAccelerationOfGravity(grav);

  addFrame(new FixedRelativeFrame("Os",Vec(3),SqrMat(3,EYE)));

  Plane *wall = new Plane("WandUnten");
  double phi = M_PI/2;
  SqrMat AWK(3);
  AWK(0,0) = cos(phi);
  AWK(0,1) = -sin(phi);
  AWK(1,1) = cos(phi);
  AWK(1,0) = sin(phi);
  AWK(2,2) = 1;
  addFrame(new FixedRelativeFrame("P",Vec(3),AWK));
  wall->setFrameOfReference(getFrame("P"));
  addContour(wall);

  addContour(new Plane("WandTest"));

  RigidBody* body = new RigidBody("Wuerfel");
  addObject(body);

  body->setFrameOfReference(getFrame("I"));
  body->setFrameForKinematics(body->getFrame("C"));

  Mat J("[1,0,0;0,1,0;0,0,1]");
  body->setTranslation(new TranslationAlongAxesXYZ<VecV>);
  body->setRotation(new RotationAboutAxesXYZ<VecV>);
  body->setGeneralizedVelocityOfRotation(RigidBody::coordinatesOfAngularVelocityWrtFrameOfReference);
  double m = 0.1;
  double l,b,h;
  l = 0.1;
  b = 0.1;
  h = 0.1;
  Vec q0(6);
  q0(1) = 0.6;
  body->setGeneralizedInitialPosition(q0);
  body->setGeneralizedInitialVelocity("[2;0;1;3;2;-1]");
  body->setMass(m);
  SymMat Theta(3);
  double A=m/12.*(b*b+h*h);
  double  B=m/12.*(h*h+l*l);
  double  C=m/12.*(l*l+b*b);
  Theta(0,0)=A;
  Theta(1,1)=B;
  Theta(2,2)=C;
  body->setInertiaTensor(Theta);

  Cuboid *cuboid = new Cuboid("Wuerfel");
  cuboid->setLength(l,h,b);
  body->addContour(cuboid);

  Contact *cnf = new Contact("Kontakt_Wuerfel");
  cnf->setNormalForceLaw(new UnilateralConstraint);
  cnf->setNormalImpactLaw(new UnilateralNewtonImpact(0.4));
  cnf->setTangentialForceLaw(new SpatialCoulombFriction(0.3));
  cnf->setTangentialImpactLaw(new SpatialCoulombImpact(0.3));
  //cnf->setPlotFeature(linkKinematics,false);
  //cnf->setNormalForceLaw(new LinearRegularizedUnilateralConstraint(1e4,100));
  //cnf->setTangentialForceLaw(new LinearRegularizedSpatialCoulombFriction(0.3));
  cnf->connect(getContour("WandUnten"), body->getContour("Wuerfel"));
  addLink(cnf);

  std::shared_ptr<OpenMBV::Cube> obj = OpenMBV::ObjectFactory::create<OpenMBV::Cube>();
  body->setOpenMBVRigidBody(obj);
  obj->setLength(l);

  setPlotFeatureRecursive(generalizedPosition, true);
  setPlotFeatureRecursive(generalizedVelocity, true);
  setPlotFeatureRecursive(generalizedRelativePosition, true);
  setPlotFeatureRecursive(generalizedRelativeVelocity, true);
  setPlotFeatureRecursive(generalizedForce, true);
}

