#include "group2.h"
#include "group1.h"
#include "rigid_body.h"
#include "springs.h"
#include "cube.h"
#include "coilspring.h"

using namespace AMVis;

Group2::Group2(const string &name) : Group(name) {
 // Parameter der Körper
  double m1 = 5;
  double m2 = 2;
  SymMat Theta1(3,EYE);
  SymMat Theta2(3,EYE);
  double h1 = 0.5;
  double h2 = 0.5;

  // Parameter der Federn
  double c1 = 1e3;
  double c2 = 1e2;
  double d1 = 0;
  double d2 = 0;
  double l01 = 0.5;
  double l02 = 0.5;

  // ----------------------- Definition des 1. Körpers --------------------  
  RigidBody *box1 = new RigidBody(name+"_Box1");
  addObject(box1);
 
  // Masse und Trägheit definieren
  box1->setMass(m1);
  box1->setInertiaTensor(Theta1);

  // Kinematik: Bewegung des Schwerpunktes (Center of Gravity C) 
  // entlang der y-Richtung ausgehend vom I-System (Ursprung O)
  box1->setTranslation(new LinearTranslation("[0; 1; 0]"));
  box1->setFrameOfReference(getCoordinateSystem("I"));
  box1->setCoordinateSystemForKinematics(box1->getCoordinateSystem("C"));
  box1->setu0("[0.1]");

  Group1 *group = new Group1(name+"_Untergruppe");
  group->setFrameOfReference(getCoordinateSystem("I"));
  Vec r(3);
  r(0) = 1;
  SqrMat A(3);
  double a = M_PI/4;
  A(0,0) = cos(a);
  A(1,1) = cos(a);
  A(2,2) = 1;
  A(0,1) = sin(a);
  A(1,0) = -sin(a);
  addSubsystem(group);
  group->setTranslation(r);
  group->setRotation(A);


  {
  ostringstream os;
  os <<name<< "." << box1->getName();
  Cube * cuboid = new Cube(os.str(),1,false);
  cuboid->setLength(h1);
  cuboid->setColor(0.5);
  box1->setAMVisBody(cuboid);
  }


}
