#include "system.h"
#include "mbsim/rigid_body.h"
#include "mbsim/mbsim_event.h"
#include "mbsim/contact.h"
#include "mbsim/constitutive_laws.h"
#include "mbsim/contour.h"
#include "mbsim/contours/sphere.h"
#include "mbsim/contours/frustum.h"
#include "mbsim/contact_kinematics/circle_frustum.h"
#include "mbsim/environment.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/frustum.h>
#include <openmbvcppinterface/arrow.h>
#endif

using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName, const int contactlaw, const int nB) : DynamicSystemSolver(projectName) {

  /* preliminaries */

  //balls
  vector<RigidBody*> balls;
  vector<Sphere*> spheres;
  double mass = 1;

  //contact
  double mu = 0.01;

  Vec WrOK(3);
  Vec KrKS(3);
  Vec KrKP(3);
  SymMat Theta(3);
  SqrMat AWK(3);

  Vec grav(3);
  grav(1)=-9.81;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(grav);

  RigidBody* groundBase = new RigidBody("GroundBase");
  Frustum* ground = new Frustum("Ground");

  double h = 0.5; // height and offset of frustum
  double rI = 0.01; // inner radius
  double rO = 0.4; // outer radius
  ground->setOutCont(false);


  this->addFrame("K",WrOK,SqrMat(3,EYE),this->getFrame("I"));
  groundBase->setFrameOfReference(this->getFrame("K"));
  groundBase->setFrameForKinematics(groundBase->getFrame("C"));
  groundBase->setMass(1.);
  groundBase->setInertiaTensor(SymMat(3,EYE));
  this->addObject(groundBase);

  /*Add contour to ground base*/
  Vec radii(2,INIT,0);
  radii(0) = rI;
  radii(1) = rO;
  ground->setRadii(radii);
  ground->setHeight(h);

#ifdef HAVE_OPENMBVCPPINTERFACE
  ground->enableOpenMBV();
  ground->getOpenMBVRigidBody()->setDrawMethod(OpenMBV::Body::lines);
#endif


  groundBase->addContour(ground,Vec(3,INIT,0.),SqrMat(3,EYE));

  /* contact */
  Contact *contact = new Contact("Contact");

#ifdef HAVE_OPENMBVCPPINTERFACE
  /*Print arrows for contacts*/
  OpenMBV::Arrow *normalArrow = new OpenMBV::Arrow();
  normalArrow->setScaleLength(0.001);
  OpenMBV::Arrow *frArrow = new OpenMBV::Arrow();
  frArrow->setScaleLength(0.001);
  frArrow->setStaticColor(0.75);

  //fancy stuff
  contact->enableOpenMBVContactPoints(0.01);
  contact->setOpenMBVNormalForceArrow(normalArrow);
  contact->setOpenMBVFrictionArrow(frArrow);
#endif

  if(contactlaw == 0) { //Maxwell Contact
    //Normal force
    InfluenceFunction* infl = new FlexibilityInfluenceFunction(ground->getShortName(), 1e-5);
    MaxwellContactLaw* mfl = new MaxwellContactLaw(10000);
    mfl->addContourCoupling(ground, ground, infl);
    contact->setContactForceLaw(mfl);

    //Frictional force
//    contact->setFrictionForceLaw(new RegularizedSpatialFriction(new LinearRegularizedCoulombFriction(mu)));
    contact->setFrictionForceLaw(new SpatialCoulombFriction(mu));
    contact->setFrictionImpactLaw(new SpatialCoulombImpact(mu));
  }
  else if(contactlaw == 1) { //Regularized Unilateral Contact
    //Normal force
    contact->setContactForceLaw(new RegularizedUnilateralConstraint(new LinearRegularizedUnilateralConstraint(1e5,10000)));

    //Frictional force
    contact->setFrictionForceLaw(new RegularizedSpatialFriction(new LinearRegularizedCoulombFriction(mu)));
  }
  else if (contactlaw == 2) { //Unilateral Constraint Contact
    //Normal force
    contact->setContactForceLaw(new UnilateralConstraint);
    contact->setContactImpactLaw(new UnilateralNewtonImpact);

    //Frictional force
    contact->setFrictionForceLaw(new SpatialCoulombFriction(mu));
    contact->setFrictionImpactLaw(new SpatialCoulombImpact(mu));
  }
  this->addLink(contact);

  // bodies
  for(int k=0; k<nB; k++) {
    stringstream name;
    name << "Ball_" << k;
    balls.push_back(new RigidBody(name.str()));
    this->addObject(balls[k]);

    stringstream frame; // ball location
    frame << "B_"  << k;
    Vec WrOS0B(3,INIT,0.);
    WrOS0B(0) = (rI+rO)*0.25*cos(k*2.*M_PI/nB);
    WrOS0B(1) = h;
    WrOS0B(2) = (rI+rO)*0.35*sin(k*2.*M_PI/nB);
    this->addFrame(frame.str(),WrOS0B,SqrMat(3,EYE),this->getFrame("I"));

    balls[k]->setFrameOfReference(this->getFrame(frame.str()));
    balls[k]->setFrameForKinematics(balls[k]->getFrame("C"));
    balls[k]->setMass(mass);
    balls[k]->setInertiaTensor(Theta);
    balls[k]->setTranslation(new LinearTranslation(SqrMat(3,EYE))); // only translational dof because of point masses

    Vec u0(3,INIT,0);
    u0(1) = -1;
    balls[k]->setInitialGeneralizedVelocity(u0);

    stringstream spherename;
    spherename << "sphere_" << k;
    spheres.push_back(new Sphere(spherename.str()));
    spheres[k]->setRadius(0.01);
    spheres[k]->enableOpenMBV();

    balls[k]->addContour(spheres[k],Vec(3,INIT,0.), SqrMat(3,EYE));

    contact->connect(groundBase->getContour("Ground"),spheres[k]);
  }
}

