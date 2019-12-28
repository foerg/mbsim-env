#include "system.h"

#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/links/contact.h"
#include "mbsim/constitutive_laws/constitutive_laws.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/functions/kinetics/kinetics.h"
#include "mbsim/contours/circle.h"
#include "mbsim/contours/planar_contour.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/observers/contact_observer.h"
#include "tools/rigid_contour_functions1s.h"

using namespace std;
using namespace MBSim;
using namespace fmatvec;

System::System(const string &name) : DynamicSystemSolver(name) {

  double m1=1.0;
  double l1=0.1;

  bool rigid=true;

  SymMat Theta(3);
  Theta(2,2) = 1./12.*m1*l1*l1;

  Mat YZ;
  ifstream file("contour.asc");
  file >> YZ;
  file.close();

  // Kontur der Nocke 
  vector<double> searchpoints(72,0);
  for (size_t j=0; j<searchpoints.size(); j++)
    searchpoints[j]=j/double((searchpoints.size()-1))*2.*M_PI;

  //FuncCrPC *funcCamContour=new FuncCrPC();
  FuncCrPC_PlanePolar *funcCamContour=new FuncCrPC_PlanePolar();
  funcCamContour->setYZ(YZ);

  RigidBody * cam = new RigidBody("Cam");
  cam->setFrameOfReference(this->getFrame("I"));
  cam->setFrameForKinematics(cam->getFrame("C"));
  cam->setMass(m1);
  cam->setInertiaTensor(1000.*Theta);
  cam->setRotation(new RotationAboutFixedAxis<VecV>("[0;0;1]"));
  cam->setGeneralizedInitialVelocity(4.*M_PI);
  this->addObject(cam);

  //cam->addFrame(new FixedRelativeFrame("Contour", "[.003; .01; 0]", Cardan2AIK(-M_PI/2., 0, -M_PI/2. )));
  cam->addFrame(new FixedRelativeFrame("P", "[.003; .01; 0]", SqrMat3(EYE)));
  PlanarContour * camContour = new PlanarContour("Contour");
  camContour->setContourFunction(funcCamContour);
  camContour->setFrameOfReference(cam->getFrame("P"));
  camContour->setNodes(searchpoints);
  camContour->setThickness(1);
  cam->addContour(camContour);
  camContour->enableOpenMBV();

  addFrame(new FixedRelativeFrame("I2", "[0.05; 0.09; 0.0]", SqrMat(3, EYE)));

  RigidBody * roll = new RigidBody("Roll");
  roll->setFrameOfReference(this->getFrame("I2"));
  roll->setFrameForKinematics(roll->getFrame("C"));
  roll->setMass(m1);
  roll->setInertiaTensor(Theta);
  roll->setRotation(new RotationAboutZAxis<VecV>);
  roll->setTranslation(new TranslationAlongYAxis<VecV>);
  roll->setGeneralizedInitialVelocity("[0;1.0]");
  this->addObject(roll);

  Circle * rollContour = new Circle("Contour");
  rollContour->setRadius(.01);
  roll->addContour(rollContour);
  rollContour->enableOpenMBV();

  Contact * contactCamRoll = new Contact("Contact");
  if (rigid) {
    contactCamRoll->setNormalForceLaw(new UnilateralConstraint);
    contactCamRoll->setNormalImpactLaw(new UnilateralNewtonImpact);
    contactCamRoll->setTangentialForceLaw(new PlanarCoulombFriction(.1));
    contactCamRoll->setTangentialImpactLaw(new PlanarCoulombImpact(.1));
  }
  else {
    contactCamRoll->setNormalForceLaw(new RegularizedUnilateralConstraint(new LinearRegularizedUnilateralConstraint(1e6, 1e4)));
    contactCamRoll->setTangentialForceLaw(new RegularizedPlanarFriction(new LinearRegularizedCoulombFriction(1.)));
  }
  contactCamRoll->connect(cam->getContour("Contour"), roll->getContour("Contour"));
  addLink(contactCamRoll);

  ContactObserver *observer = new ContactObserver("Observer");
  addObserver(observer);
  observer->setContact(contactCamRoll);
  observer->enableOpenMBVContactPoints(.005);

  setPlotFeatureRecursive(generalizedPosition, true);
  setPlotFeatureRecursive(generalizedVelocity, true);
  setPlotFeatureRecursive(generalizedRelativePosition, true);
  setPlotFeatureRecursive(generalizedRelativeVelocity, true);
  setPlotFeatureRecursive(generalizedForce, true);
}
