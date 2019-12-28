#include "group2.h"
#include "group1.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include <openmbvcppinterface/cuboid.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;

Group2::Group2(const string &name) : Group(name) {
  // Parameter der Körper
  double m1 = 5;
  SymMat Theta1(3,EYE);
  SymMat Theta2(3,EYE);
  double h1 = 0.5;

  // ----------------------- Definition des 1. Körpers --------------------  
  RigidBody *box1 = new RigidBody("Box1");
  box1->setPlotFeatureRecursive(position, true);
  box1->setPlotFeatureRecursive(angle, true);
  addObject(box1);

  // Masse und Trägheit definieren
  box1->setMass(m1);
  box1->setInertiaTensor(Theta1);

  // Kinematik: Bewegung des Schwerpunktes (Center of Gravity C) 
  // entlang der y-Richtung ausgehend vom I-System (Ursprung O)
  box1->setTranslation(new LinearTranslation<VecV>("[0; 1; 0]"));
  box1->setFrameOfReference(getFrame("I"));
  box1->setFrameForKinematics(box1->getFrame("C"));
  box1->setGeneralizedInitialVelocity("[0.1]");

  Group1 *group = new Group1("Untergruppe");
  Vec r(3);
  r(0) = 1;
  SqrMat A(3);
  double a = M_PI/4;
  A(0,0) = cos(a);
  A(1,1) = cos(a);
  A(2,2) = 1;
  A(0,1) = sin(a);
  A(1,0) = -sin(a);
  addFrame(new FixedRelativeFrame("Q",r,A));
  group->setFrameOfReference(getFrame("Q"));
  addGroup(group);


  std::shared_ptr<OpenMBV::Cuboid> body1=OpenMBV::ObjectFactory::create<OpenMBV::Cuboid>();
  body1->setLength(vector<double>(3,h1));
  box1->setOpenMBVRigidBody(body1);
  box1->getFrame("C")->enableOpenMBV();


}
