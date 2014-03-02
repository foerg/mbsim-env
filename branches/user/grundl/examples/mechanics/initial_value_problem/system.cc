#include "system.h"

#include "mbsim/rigid_body.h"
#include "mbsim/contact.h"
#include "mbsim/constitutive_laws.h"
#include "mbsim/contours/circle_solid.h"
#include "mbsim/contours/contour1s_analytical.h"
#include "mbsim/utils/rotarymatrices.h"
#include "tools/rigid_contour_functions1s.h"
#include "mbsim/functions/kinematic_functions.h"

#include "mbsim/contact_kinematics/circlesolid_contour1s.h"
#include "mbsim/utils/nonlinear_algebra.h"

#include <iostream>

using namespace std;
using namespace MBSim;
using namespace fmatvec;

class initLink : public Link {
  public:
    initLink(const std::string &name, RigidBody * rocker_, Contact * contactCamRocker_) : Link(name), rocker(rocker_), contactCamRocker(contactCamRocker_) {}
    void updateWRef(const fmatvec::Mat&, int){}
    void updateVRef(const fmatvec::Mat&, int){}
    void updatehRef(const fmatvec::Vec&, int){}
    void updatedhdqRef(const fmatvec::Mat&, int){}
    void updatedhduRef(const fmatvec::SqrMat&, int){}
    void updatedhdtRef(const fmatvec::Vec&, int){}
    void updaterRef(const fmatvec::Vec&, int) {}
    bool isActive() const {return false; }
    virtual bool isSingleValued() const { return true; }
    bool gActiveChanged() {return false; }
    void init(InitStage stage) {
      if (stage==MBSim::calculateLocalInitialValues) {
        class CamRockerDistance : public fmatvec::Function<double(double)> {
          public:
            CamRockerDistance(RigidBody * rocker_, Contact * contactCamRocker_) : rocker(rocker_), contactCamRocker(contactCamRocker_) {}
            double operator()(const double &phi) {
              rocker->setInitialGeneralizedPosition(Vec(1, INIT, phi));
              rocker->initz();
              rocker->getDynamicSystemSolver()->updateStateDependentVariables(0);
              ((ContactKinematicsCircleSolidContour1s*)(contactCamRocker->getContactKinematics()))->setSearchAllCP(true);
              contactCamRocker->updateg(0);
              return contactCamRocker->getg()(0);
            }
          private:
            RigidBody * rocker;
            Contact * contactCamRocker;
        };
        CamRockerDistance gCamRocker(rocker, contactCamRocker);
        RegulaFalsi gCamRockerSolver(&gCamRocker);
        gCamRockerSolver.setTolerance(2.*epsroot()*epsroot());
        const double phi0Rocker=gCamRockerSolver.solve(-M_PI/2., M_PI/20.);
        rocker->setInitialGeneralizedPosition(phi0Rocker);
        rocker->initz();
        rocker->getDynamicSystemSolver()->updateStateDependentVariables(0);
        contactCamRocker->updateg(0);

        int nltol=0;
        while (!((contactCamRocker->getg()(0))<0)) {
          const double phi0RockerNew = phi0Rocker+nltol*epsroot();
          rocker->setInitialGeneralizedPosition(phi0RockerNew);
          rocker->initz();
          rocker->getDynamicSystemSolver()->updateStateDependentVariables(0);
          contactCamRocker->updateg(0);
          nltol++;
        }
        class CamRockerVelocity : public fmatvec::Function<double(double)> {
          public:
            CamRockerVelocity(RigidBody * rocker_, Contact * contactCamRocker_) : rocker(rocker_), contactCamRocker(contactCamRocker_) {}
            double operator()(const double &omega) {
              rocker->setInitialGeneralizedVelocity(omega);
              rocker->initz();
              rocker->getDynamicSystemSolver()->updateStateDependentVariables(0);
              contactCamRocker->updateg(0);
              if (contactCamRocker->isSetValued())
                contactCamRocker->isActive();
              contactCamRocker->updategd(0);
              return contactCamRocker->getgd()(0);
            }
          private:
            RigidBody * rocker;
            Contact * contactCamRocker;
        };
        CamRockerVelocity gdCamRocker(rocker, contactCamRocker);
        RegulaFalsi gdCamRockerSolver(&gdCamRocker);
        gdCamRockerSolver.setTolerance(2.*epsroot()*epsroot());
        const double omega0Rocker=gdCamRockerSolver.solve(-1e3, 1e3);

        rocker->setInitialGeneralizedPosition(phi0Rocker);
        rocker->setInitialGeneralizedVelocity(omega0Rocker);
        rocker->initz();
        rocker->getDynamicSystemSolver()->updateStateDependentVariables(0);
        contactCamRocker->updateg(0);
        if (contactCamRocker->isSetValued())
          contactCamRocker->isActive();
        contactCamRocker->updategd(0);
        cout << "    Found inital phi0 at " << phi0Rocker/M_PI*180. << " [deg]. g(0)=" << contactCamRocker->getg()(0) << " [m]."<< endl;
        cout << "    Found initial omega0 at " << omega0Rocker/M_PI*30 << " [rpm]. gd(0)=" << contactCamRocker->getgd()(0) << " [m/s]."<< endl;
      }
    }
    void updateg(double t) {}
    void updategd(double t) {}
    void plot(double t, double dt) {}
  private:
    RigidBody * rocker;
    Contact * contactCamRocker;
};

System::System(const string &name) : DynamicSystemSolver(name) {

  double m1=1.0;
  double l1=0.1;

  SymMat Theta(3);
  Theta(2,2) = 1./12.*m1*l1*l1;

  Mat YZ;
  ifstream file("contour.asc");
  file >> YZ;
  file.close();

  Vec searchpoints(72,INIT,0);
  for (int j=0; j<searchpoints.size(); j++)
    searchpoints(j)=j/double((searchpoints.size()-1))*2.*M_PI;

  FuncCrPC *funcCamContour=new FuncCrPC();
  funcCamContour->setYZ(YZ);

  RigidBody * cam = new RigidBody("Cam");
  cam->setFrameOfReference(this->getFrame("I"));
  cam->setFrameForKinematics(cam->getFrame("C"));
  cam->setMass(m1);
  cam->setInertiaTensor(10000.*Theta);
  cam->setRotation(new RotationAboutFixedAxis<VecV>("[0;0;1]"));
  cam->setInitialGeneralizedPosition(.5*M_PI);
  cam->setInitialGeneralizedVelocity(100.*M_PI/30.);
  this->addObject(cam);

  Contour1sAnalytical * camContour = new Contour1sAnalytical("Contour");
  camContour->setContourFunction1s(funcCamContour);
  camContour->setAlphaStart(0.);
  camContour->setAlphaEnd(2.*M_PI);
  camContour->setNodes(searchpoints);
  cam->addFrame(new FixedRelativeFrame("Contour", "[.003; .01; 0]", Cardan2AIK(-M_PI/2., 0, -M_PI/2. )));
  camContour->setFrameOfReference(cam->getFrame("Contour"));
  cam->addContour(camContour);

  addFrame(new FixedRelativeFrame("I2", "[0.25; 0.09; 0.0]", SqrMat(3, EYE)));

  RigidBody * rocker = new RigidBody("Rocker");
  rocker->setFrameOfReference(this->getFrame("I2"));
  rocker->addFrame(new FixedRelativeFrame("Joint", Vec("[0.2; 0; 0]"), SqrMat(3, EYE)));
  rocker->setFrameForKinematics(rocker->getFrame("Joint"));
  rocker->setMass(m1*10);
  rocker->setInertiaTensor(Theta);
  rocker->setRotation(new RotationAboutFixedAxis<VecV>("[0;0;1]"));
  this->addObject(rocker);

  CircleSolid * rockerContour = new CircleSolid("Contour");
  rockerContour->setRadius(.022);
  rocker->addContour(rockerContour);

  Contact * contactCamRocker = new Contact("Contact");
  contactCamRocker->setNormalForceLaw(new UnilateralConstraint);
  contactCamRocker->setNormalImpactLaw(new UnilateralNewtonImpact);
  contactCamRocker->setTangentialForceLaw(new PlanarCoulombFriction(.1));
  contactCamRocker->setTangentialImpactLaw(new PlanarCoulombImpact(.1));
  contactCamRocker->connect(cam->getContour("Contour"), rocker->getContour("Contour"));
  addLink(contactCamRocker);

  addLink(new initLink("InitialValueSearch", rocker, contactCamRocker));

#if HAVE_OPENMBVCPPINTERFACE
  camContour->enableOpenMBV();
  getFrame("I2")->enableOpenMBV(.1*l1);
  cam->getFrame("C")->enableOpenMBV(.1*l1);
  rocker->getFrame("C")->enableOpenMBV(.1*l1);
  rocker->getFrame("Joint")->enableOpenMBV(.1*l1);
  contactCamRocker->enableOpenMBVContactPoints(.005);
  rockerContour->enableOpenMBV();
#endif

}
