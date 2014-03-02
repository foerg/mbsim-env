#include "system.h"
#include "mbsim/joint.h"
#include "mbsim/contact.h"
#include "mbsim/contours/point.h"
#include "mbsim/contours/plane.h"
#include "mbsim/constitutive_laws.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/environment.h"
#include <mbsim/functions/kinematic_functions.h>
#include <mbsim/functions/kinetic_functions.h>

#include "mbsimFlexibleBody/contours/neutral_contour/contour_2s_neutral_linear_external_FFR.h"
#include "mbsimFlexibleBody/contours/flexible_band.h"
// Beginning Contact
//#include "mbsim/rigid_body.h"
//#include "mbsim/contour.h"
//#include "mbsim/constitutive_laws.h"
// End Contact
#include "mbsim/environment.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/spineextrusion.h>
#include "openmbvcppinterface/sphere.h" // ball
#include <openmbvcppinterface/cuboid.h>
#include <openmbvcppinterface/polygonpoint.h>
#include <openmbvcppinterface/arrow.h> // Contact
#endif

using namespace MBSimFlexibleBody;
using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName) :
    DynamicSystemSolver(projectName) {
  Vec grav(3, INIT, 0.); //grav(1) = -9.81;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(grav);

  FlexibleBodyLinearExternalFFR *beam = new FlexibleBodyLinearExternalFFR("beam", false);

  beam->readFEMData("spatial_beam_model", false);
  beam->enableFramePlot(1);

  int nf = beam->getNumberModes();

  beam->setFrameOfReference(this->getFrame("I"));

  // set a initial angle for the body reference
  Vec q0 = Vec(nf + 6, INIT, 0.0);
  Vec u0Beam = Vec(nf + 6, INIT, 0.0);
//  q0(0) = 0.2;

  beam->setq0(q0);
  beam->setu0(u0Beam);

  // Fix the beam at its FFR
  Joint * fix = new Joint("Fix");
  fix->connect(getFrameI(), beam->getFloatingFrameOfReference());
  fix->setForceDirection(Mat3x3(EYE));
  fix->setMomentDirection(Mat3x3(EYE));
  fix->setForceLaw(new BilateralConstraint);
  fix->setMomentLaw(new BilateralConstraint);
  addLink(fix);

  this->addObject(beam);

  // add neutral contour to the rod
  Contour2sNeutralLinearExternalFFR* ncc = new Contour2sNeutralLinearExternalFFR("neutralFibre");
  beam->addContour(ncc);
  ncc->readTransNodes("spatial_beam_model/Example_Contour.txt");
  ncc->setFrameOfReference(beam->getFrameOfReference());

  ncc->setAlphaStart(Vec(2, INIT, 0));
  ncc->setAlphaEnd(Vec(2, INIT, 1));
  // set the grid for contact2Ssearch, if these nodes vector is not given, the Contact2sSearch::setEqualSpacing() will be called
  // in pointContour2s to create a default grid for the initial searching.
//  Vec nodesU(ncc->getNumberOfTransNodesU(), NONINIT);
//  Vec nodesV(ncc->getNumberOfTransNodesV(), NONINIT);
//  for (int i = 0; i < ncc->getNumberOfTransNodesU(); i++)
//    nodesU(i) = 1. / (ncc->getNumberOfTransNodesU() - 1) * i;
//  for (int i = 0; i < ncc->getNumberOfTransNodesV(); i++)
//    nodesV(i) = 1. / (ncc->getNumberOfTransNodesV() - 1) * i;
//  ncc->setNodesU(nodesU);
//  ncc->setNodesV(nodesV);
//  cout << "nodesU:" << nodesU << endl;
//  cout << "nodesV:" << nodesV << endl;

//  FlexibleBand * top = new FlexibleBand("Top", true);
//  top->setWidth(b0);
//  top->setNormalDistance(0);
//  top->setCn(Vec("[1.;0.]"));
//  top->setNeutral(ncc);
//  top->setAlphaStart(uMin);
//  top->setAlphaEnd(uMax);
//
//  Vec contourNodes(numOfTransNodes); // open structure
//  for (int i = 0; i < numOfTransNodes; i++)
//    contourNodes(i) = (uMax - uMin) / (numOfTransNodes - 1) * i;
//  top->setNodes(contourNodes);  // depend on the range of the langrange parameter of the neutral fibre
//
//  beam->addContour(top);

//#ifdef HAVE_OPENMBVCPPINTERFACE
//  OpenMBV::SpineExtrusion *cuboid = new OpenMBV::SpineExtrusion;
//  cuboid->setNumberOfSpinePoints(elements * 4 + 1);
//  cuboid->setStaticColor(0.5);
//  cuboid->setScaleFactor(1.);
//  vector<OpenMBV::PolygonPoint*> *rectangle = new vector<OpenMBV::PolygonPoint*>;
//  OpenMBV::PolygonPoint *corner1 = new OpenMBV::PolygonPoint(0, b0 * 0.5, 1);
//  rectangle->push_back(corner1);
//  OpenMBV::PolygonPoint *corner2 = new OpenMBV::PolygonPoint(0, -b0 * 0.5, 1);
//  rectangle->push_back(corner2);
////  OpenMBV::PolygonPoint *corner3 = new OpenMBV::PolygonPoint(b0 * 0.5, -b0 * 0.5, 1);
////  rectangle->push_back(corner3);
////  OpenMBV::PolygonPoint *corner4 = new OpenMBV::PolygonPoint(b0 * 0.5, b0 * 0.5, 1);
////  rectangle->push_back(corner4);
//  cuboid->setContour(rectangle);
//  top->setOpenMBVSpineExtrusion(cuboid, ncc);
//#endif

// Beginning Contact ---------------------------------------------------
// for exciting the beam : ball mass 500; position(90, 200, 10), velocity(-100)

  double mass = 5000000.; // mass of ball
  double r = 1; // radius of ball

  RigidBody *ball = new RigidBody("Ball");
  Vec3 WrOS0B;
  WrOS0B(0) = 83;  // between node 231-232 (68.     10.,          10.)
  WrOS0B(1) = 25;
  WrOS0B(2) = 18;
  FixedRelativeFrame * ballRef = new FixedRelativeFrame("BallRef", WrOS0B);
  addFrame(ballRef);
  ball->setFrameOfReference(ballRef);
  ball->setMass(mass);
  SymMat Theta(3);
  Theta(0, 0) = 2. / 5. * mass * r * r;
  Theta(1, 1) = 2. / 5. * mass * r * r;
  Theta(2, 2) = 2. / 5. * mass * r * r;
  ball->setInertiaTensor(Theta);
  ball->setTranslation(new TranslationAlongAxesXYZ<VecV>());

  Vec3 u0Ball;
  u0Ball(1) = -50;
  ball->setInitialGeneralizedVelocity(u0Ball);

  Vec BR(3, INIT, 0.);
  BR(1) = -r;
  FixedRelativeFrame * pointRef = new FixedRelativeFrame("pointRef", BR, SqrMat3(EYE), ball->getFrameC());
  ball->addFrame(pointRef);

  MBSim::Point *point = new MBSim::Point("Point");
  point->setFrameOfReference(pointRef);
  ball->addContour(point);
  this->addObject(ball);

#ifdef HAVE_OPENMBVCPPINTERFACE
  OpenMBV::Sphere *sphere = new OpenMBV::Sphere;
  sphere->setRadius(r);
  sphere->setDiffuseColor(0.5, 1, 0);
  ball->setOpenMBVRigidBody(sphere);
#endif

  Contact *contact = new Contact("Contact");
  if (0) {
    contact->setNormalForceLaw(new RegularizedUnilateralConstraint(new LinearRegularizedUnilateralConstraint(1e7, 0.)));
  }
  else {
    contact->setNormalForceLaw(new UnilateralConstraint);
    contact->setNormalImpactLaw(new UnilateralNewtonImpact(0.0));
  }

  contact->connect(ball->getContour("Point"), ncc);
  contact->enableOpenMBVNormalForce();
  contact->enableOpenMBVTangentialForce();
  contact->enableOpenMBVContactPoints();

  this->addLink(contact);

}

