#include "system.h"
#include "mbsim/rigid_body.h"
#include "pendulum.h"

System::System(const string &projectName) : MultiBodySystem(projectName) {

  setProjectDirectory("plot");

  Vec grav(3);
  grav(1)=-9.81;
  setAccelerationOfGravity(grav);

  Tree* tree = new Tree("Master");
  addSubsystem(tree,Vec(3),SqrMat(3,EYE));

  Pendulum *pendel1 = new Pendulum("Pendel1"); 
  Node* node = tree->addSubsystem(0,pendel1,Vec(3),SqrMat(3,EYE));

  Vec x(3);
  x(0) = 0.15;

  SqrMat A(3);
  for(int i=0; i<3; i++)
    A(i,i) = 1;

  pendel1->getRod2()->addFrame("P",x,A,pendel1->getRod2()->getFrame("R"));

  Pendulum *pendel2 = new Pendulum("Pendel2"); 
  tree->addSubsystem(node,pendel2,Vec(3),SqrMat(3,EYE));
  pendel2->getRod1()->setFrameOfReference(pendel1->getRod2()->getFrame("P"));
}

