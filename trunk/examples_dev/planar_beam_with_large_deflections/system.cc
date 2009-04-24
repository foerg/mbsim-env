#include "system.h"
#include "mbsim/flexible_body/flexible_body_1s_21_rcm.h"
#include "mbsim/rigid_body.h"
#include "mbsim/joint.h"
#include "mbsim/contact.h"
#include "mbsim/contours/flexible_band.h"
#include "mbsim/contact_kinematics/point_flexibleband.h"
#include "mbsim/constitutive_laws.h"

using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {
  
  Vec g(3); g(1) = -9.81;
  this->setAccelerationOfGravity(g);

  double l0 = 1.5; // length
  double b0 = 0.1; // width
  double E = 1.e8; // E-Modul  
  double A = b0*b0; // cross-section area
  double I1 = 1./12.*b0*b0*b0*b0; // moment inertia
  double rho = 2.7e3; // density  
  int elements = 10; // number of finite elements

  double mass = 100.; // mass of ball
  double r = 1.e-2; // radius of ball

  FlexibleBody1s21RCM *rod = new FlexibleBody1s21RCM("Rod", true);
  rod->setLength(l0);
  rod->setEModul(E);
  rod->setCrossSectionalArea(A);
  rod->setMomentInertia(I1);
  rod->setDensity(rho);
  rod->setFrameOfReference(this->getFrame("I"));
  rod->setNumberElements(elements);
  Vec q0 = Vec(5*elements+3,INIT,0.);
  for(int i=1;i<=elements;i++) q0(5*i) = l0*i/elements;
  rod->setq0(q0);
  this->addObject(rod);

  FlexibleBand *top = new FlexibleBand("Top");
  Vec nodes(elements+1);
  for(int i=0;i<=elements;i++) nodes(i) = i*l0/elements;
  top->setNodes(nodes);
  top->setWidth(b0);
  top->setCn(Vec("[1.;0.]"));
  top->setAlphaStart(0.);
  top->setAlphaEnd(l0);  
  top->setNormalDistance(0.5*b0);
  rod->addContour(top);

  RigidBody *ball = new RigidBody("Ball");
  Vec WrOS0B(3,INIT,0.);
  WrOS0B(0) = l0*0.9; WrOS0B(1) = b0*0.5+0.05;
  this->addFrame("B",WrOS0B,SqrMat(3,EYE),this->getFrame("I"));
  ball->setFrameOfReference(this->getFrame("B"));
  ball->setFrameForKinematics(ball->getFrame("C"));
  ball->setMass(mass);
  SymMat Theta(3);
  Theta(0,0) =  2./5.*mass*r*r;
  Theta(1,1) =  2./5.*mass*r*r;
  Theta(2,2) =  2./5.*mass*r*r;
  ball->setInertiaTensor(Theta);
  ball->setTranslation(new LinearTranslation(Mat(3,3,EYE)));
  Point *point = new Point("Point");
  Vec BR(3,INIT,0.); BR(1)=-r;
  ball->addContour(point,BR,SqrMat(3,EYE),ball->getFrame("C"));
  this->addObject(ball);

  PointFlexibleBand *ck = new PointFlexibleBand();
  Contact *contact = new Contact("Contact");
  contact->setContactKinematics(ck);
  contact->setContactForceLaw(new UnilateralConstraint);
  contact->setContactImpactLaw(new UnilateralNewtonImpact(1.0));
  contact->connect(ball->getContour("Point"),rod->getContour("Top"));
  this->addLink(contact);

  ContourPointData cpdata;
  cpdata.getLagrangeParameterPosition() = Vec(1,INIT,0.);
  cpdata.getContourParameterType() = CONTINUUM;
  rod->addFrame("RJ",cpdata);
  Joint *joint = new Joint("Clamping");
  joint->connect(this->getFrame("I"),rod->getFrame("RJ")); 
  joint->setForceDirection(Mat("[1,0; 0,1; 0,0]"));
  joint->setForceLaw(new BilateralConstraint);
  joint->setImpactForceLaw(new BilateralImpact);
  joint->setMomentDirection("[0; 0; 1]");
  joint->setMomentLaw(new BilateralConstraint);
  joint->setImpactMomentLaw(new BilateralImpact);
  this->addLink(joint);
}

