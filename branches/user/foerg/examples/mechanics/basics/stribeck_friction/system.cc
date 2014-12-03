#include "system.h"
#include "stribeck_friction.h"
#include "mbsim/rigid_body.h"
#include "mbsim/contact.h"
#include "mbsim/environment.h"
#include "mbsim/contours/sphere.h"
#include "mbsim/contours/plane.h"
#include "mbsim/constitutive_laws.h"
#include "mbsim/functions/kinematic_functions.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "openmbvcppinterface/arrow.h"
#endif

using namespace std;
using namespace MBSim;
using namespace fmatvec;

System::System(const string &projectName)  : DynamicSystemSolver(projectName) { 
  // gravitation
  Vec grav(3,INIT,0.);
  grav(1)=-9.81;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(grav);

  // parameters
  double D = 0.1;              		
  double m = 0.1;
  SymMat Theta(3);
  Theta(0,0) = m*D*D* 2./5.;
  Theta(1,1) = Theta(0,0);
  Theta(2,2) = Theta(0,0);
  double mu0 = 0.;
  double mu1 = 0.3;
  double mu2 = 0.1;
  double kP = 2.;

  // ball
  RigidBody *ball = new RigidBody("Ball");
  this->addObject(ball);
  ball->setMass(m);
  ball->setInertiaTensor(Theta);
  ball->setRotation(new RotationAboutAxesXYZ<VecV>);
  ball->setTranslation(new TranslationAlongAxesXYZ<VecV>);
  ball->setFrameForKinematics(ball->getFrame("C"));

  // initial settings
  Vec u0(6,INIT,0.);
  u0(0) = 5.0;
  u0(3) = 20.0;
  ball->setInitialGeneralizedVelocity(u0);
  this->addFrame(new FixedRelativeFrame("R",Vec("[0;0.15;0]"),SqrMat(3,EYE)));
  ball->setFrameOfReference(this->getFrame("R"));

  // contour
  Sphere *sp = new Sphere("Sphere");
  sp->setRadius(D/2.);
#ifdef HAVE_OPENMBVCPPINTERFACE
  sp->enableOpenMBV();
#endif
  ball->addContour(sp);

  // obstacles
  Plane* pl = new Plane("Table");
  addFrame(new FixedRelativeFrame("P",Vec(3,INIT,0.),SqrMat("[0,-1,0;1,0,0;0,0,1]")));
  pl->setFrameOfReference(getFrame("P"));
  this->addContour(pl);

  // contacts
  Contact *cr = new Contact("Contact");
  cr->setNormalForceLaw(new UnilateralConstraint);
  cr->setNormalImpactLaw(new UnilateralNewtonImpact(0.));
  cr->setTangentialForceLaw(new SpatialStribeckFriction(new Friction(mu0,mu1,mu2,kP)));
  cr->setTangentialImpactLaw(new SpatialStribeckImpact(new Friction(mu0,mu1,mu2,kP)));
  cr->connect(pl,ball->getContour("Sphere"));
#ifdef HAVE_OPENMBVCPPINTERFACE
  cr->enableOpenMBVNormalForce();
  cr->enableOpenMBVTangentialForce();
#endif
  this->addLink(cr);
}

