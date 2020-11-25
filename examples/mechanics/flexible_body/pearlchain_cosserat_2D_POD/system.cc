#include "system.h"
#include "mbsim/links/joint.h"
#include "mbsim/links/contact.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/frames/fixed_contour_frame.h"
#include "mbsim/contours/point.h"
#include "mbsim/contours/plane.h"
#include "mbsim/constitutive_laws/constitutive_laws.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinetics/kinetics.h"
#include "mbsim/functions/kinematics/kinematics.h"

#include "mbsim/contact_kinematics/point_planarcontour.h"

#include <mbsimFlexibleBody/contours/neutral_contour/contour_1s_neutral_cosserat.h>
#include "mbsim/observers/contact_observer.h"

#include <openmbvcppinterface/spineextrusion.h>
#include <openmbvcppinterface/cuboid.h>
#include <openmbvcppinterface/polygonpoint.h>

using namespace MBSimFlexibleBody;
using namespace MBSim;
using namespace fmatvec;
using namespace std;
using namespace H5;

#include <hdf5serie/vectorserie.h>
#include <fmatvec/fmatvec.h>
#include <iostream>

System::System(const string &projectName) : DynamicSystemSolver(projectName) {
//---------------------------------------------------------------------------------------------------------------------------------------//
//--- Geometry/ Material Parameters -----------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------//

	Vec grav(3,INIT,0.); grav(1) = -9.81;
        getMBSimEnvironment()->setAccelerationOfGravity(grav);

	int DOF = 3;					// DOFs per nod (x,y,z,alpha,beta,gamma)
	int elements = 60; 				// number of finite elements
	double l0 = 2.; 				// length ring
	double b0 = 0.02; 				// width
	double A = b0*b0; 				// cross-section area
	double I1 = 1./12.*b0*b0*b0*b0; // moment inertia

	double E = 2.5e9; 			// E-Modul alu
	double mu = 0.3; 			// Poisson ratio
	double G = E/(2*(1+mu)); 	// shear modulus
	double rho = 2.5e3; 		// density alu

	int nBalls = 2; 		// number of balls
	double mass = 0.015; 	// mass of ball

	rod = new FlexibleBody1s21Cosserat("Rod",false);
	rod->setLength(l0);
	rod->setEGModuls(E,G);
	rod->setCrossSectionalArea(A);
	rod->setMomentsInertia(I1);
	rod->setDensity(rho);
	rod->setFrameOfReference(this->getFrame("I"));
	rod->setNumberElements(elements);
//	rod->setCuboid(b0,b0);
	rod->setCurlRadius(l0/(2*M_PI));
	//rod->setMassProportionalDamping(20.);
	//rod->setMaterialDamping(0.1,0.1;


//---------------------------------------------------------------------------------------------------------------------------------------//
//--- Initial positions cirlce shape ----------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------//
	double stretchfactor = 1.0;
	Vec q0 = Vec(DOF*elements,INIT,0.);
	double R = (l0/elements)/(2.*sin(M_PI/elements))*stretchfactor; // stretched circle
	double dphi = (2*M_PI)/elements;
	for(int i=0; i<elements; i++) {
		double phi = M_PI/2. - i*dphi;
		q0(DOF*i) = R*cos(phi);
		q0(DOF*i+1) = R*sin(phi);
		q0(DOF*i+2) = phi - dphi/2.-M_PI/2.;
	}
//---------------------------------------------------------------------------------------------------------------------------------------//
//--- Set initial positions/ Add Object -------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------//
	rod->setq0(q0);
	rod->setu0(Vec(q0.size(),INIT,0.));
	this->addObject(rod);


//---------------------------------------------------------------------------------------------------------------------------------------//
//--- OPENMBV ---------------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------//

	Contour1sNeutralFactory * rodCont = rod->createNeutralPhase();
	std::shared_ptr<OpenMBV::SpineExtrusion> cuboid = OpenMBV::ObjectFactory::create<OpenMBV::SpineExtrusion>();
	cuboid->setNumberOfSpinePoints(elements*4+1);
        cuboid->setDiffuseColor(0.6666,1,1);
	cuboid->setScaleFactor(1.);
	shared_ptr<vector<shared_ptr<OpenMBV::PolygonPoint> > > rectangle = make_shared<vector<shared_ptr<OpenMBV::PolygonPoint> > >();
        shared_ptr<OpenMBV::PolygonPoint>  corner1 = OpenMBV::PolygonPoint::create(b0*0.5,b0*0.5,1);
	rectangle->push_back(corner1);
        shared_ptr<OpenMBV::PolygonPoint>  corner2 = OpenMBV::PolygonPoint::create(b0*0.5,-b0*0.5,1);
	rectangle->push_back(corner2);
        shared_ptr<OpenMBV::PolygonPoint>  corner3 = OpenMBV::PolygonPoint::create(-b0*0.5,-b0*0.5,1);
	rectangle->push_back(corner3);
        shared_ptr<OpenMBV::PolygonPoint>  corner4 = OpenMBV::PolygonPoint::create(-b0*0.5,b0*0.5,1);
	rectangle->push_back(corner4);

	cuboid->setContour(rectangle);
	rodCont->setOpenMBVSpineExtrusion(cuboid);

//---------------------------------------------------------------------------------------------------------------------------------------//
//--- BALLS ...........------------------------------------------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------------------------------------------------------------//
	assert(nBalls>1);
	double d = 7.*l0/(8.*nBalls); // thickness
	double b = b0*1.5; // height / width

	for(int i=0;i<nBalls;i++) {
		stringstream name;
		name << "Element_" << i;
		RigidBody *ball = new RigidBody(name.str());
		balls.push_back(ball);
		balls[i]->setFrameOfReference(this->getFrame("I"));
		balls[i]->setFrameForKinematics(balls[i]->getFrame("C"));
		balls[i]->setTranslation(new TranslationAlongAxesXY<VecV>);
		balls[i]->setRotation(new RotationAboutZAxis<VecV>);
		balls[i]->setMass(mass);
		SymMat Theta(3,INIT,0.);
		Theta(0,0) = 1./6.*mass*b*b;
		Theta(1,1) = 1./12.*mass*(d*d + b*b);
		Theta(2,2) = 1./12.*mass*(d*d + b*b);
		balls[i]->setInertiaTensor(Theta);
		this->addObject(balls[i]);

		balls[i]->addContour(new MBSim::Point("COG"));

		balls[i]->addFrame(new FixedRelativeFrame("T",d*Vec("[0.5;0;0]") + b*Vec("[0;0.5;0]"),SqrMat(3,EYE),balls[i]->getFrame("C")));
		balls[i]->addContour(new MBSim::Point("topPoint",balls[i]->getFrame("T")));

		balls[i]->addFrame(new FixedRelativeFrame("B",d*Vec("[0.5;0;0]") - b*Vec("[0;0.5;0]"),SqrMat(3,EYE),balls[i]->getFrame("C")));
		balls[i]->addContour(new MBSim::Point("bottomPoint",balls[i]->getFrame("B")));

		SqrMat trafoPlane(3,INIT,0.); trafoPlane(0,0) = -1.; trafoPlane(1,1) = 1.; trafoPlane(2,2) = -1.;
		balls[i]->addFrame(new FixedRelativeFrame("P",-d*Vec("[0.5;0;0]"),trafoPlane,balls[i]->getFrame("C")));
		balls[i]->addContour(new Plane("Plane",balls[i]->getFrame("P")));

		std::shared_ptr<OpenMBV::Cuboid> cube=OpenMBV::ObjectFactory::create<OpenMBV::Cuboid>();
		cube->setLength(d,b,b);
		cube->setDiffuseColor(0.3333,0.6666,1);
		balls[i]->setOpenMBVRigidBody(cube);
	}
	//Set balls to correct position
	FlexibleBody1s21Cosserat * rodInfo = new FlexibleBody1s21Cosserat("InfoRod", false);

	rodInfo->setq0(rod->getq());
	rodInfo->setu0(rod->getu());
	rodInfo->setNumberElements(rod->getNumberElements());
	rodInfo->setLength(rod->getLength());
	rodInfo->setFrameOfReference(rod->getFrameOfReference());
	Contour1sNeutralFactory * cont = rodInfo->createNeutralPhase();

        rodInfo->setParent(this);
	rodInfo->initInfo();

        Vec r0(3);
        for(unsigned int i=0;i<balls.size();i++) {
          Vec q0(3,INIT,0.);
          double xL = i*1.0/balls.size(); // TODO
          Vec2 zeta;
          zeta(0) = xL;
          FixedContourFrame P("P",zeta,cont);
          P.setParent(rodInfo);
          Vec3 r = P.evalPosition();
          q0(0) = r(0);
          q0(1) = r(1);
          if(i==0) r0 = r;

          SqrMat3 A = P.evalOrientation();
          q0(2) = -AIK2Cardan(A)(2) + 0.5 * M_PI;
          balls[i]->setGeneralizedInitialPosition(q0);
	}

	delete rodInfo;

	// inertial ball constraint
	this->addFrame(new FixedRelativeFrame("BearingFrame",r0,SqrMat(3,EYE),this->getFrame("I")));
	Joint *joint = new Joint("BearingJoint");
	joint->setForceDirection(Mat("[1,0;0,1;0,0]"));
	joint->setForceLaw(new BilateralConstraint);
	joint->connect(this->getFrame("BearingFrame"),balls[0]->getFrame("C")); // topmost ball is suspended to hold rod
	this->addLink(joint);

	// constraints balls on flexible band
	for(int i=0;i<nBalls;i++) {
		Contact *contact = new Contact("Band_"+balls[i]->getName());
		contact->setNormalForceLaw(new BilateralConstraint);
		contact->setNormalImpactLaw(new BilateralImpact);
		contact->connect(balls[i]->getContour("COG"),rodCont);
		contact->setContactKinematics(new ContactKinematicsPointPlanarContour);
		contact->setDetermineInitialGuess(true);
		this->addLink(contact);

                ContactObserver *observer = new ContactObserver(contact->getName()+"_Observer");
                addObserver(observer);
                observer->setContact(contact);
                observer->enableOpenMBVContactPoints(0.01);
	}

	// inner-ball contacts
	for(int i=0;i<nBalls;i++) {
		stringstream namet,nameb;
		namet << "ContactTop_" << i;
		nameb << "ContactBot_" << i;
		Contact *ctrt = new Contact(namet.str());
		Contact *ctrb = new Contact(nameb.str());
		ctrt->setNormalForceLaw(new UnilateralConstraint);
		ctrt->setNormalImpactLaw(new UnilateralNewtonImpact(0.));
		ctrb->setNormalForceLaw(new UnilateralConstraint);
		ctrb->setNormalImpactLaw(new UnilateralNewtonImpact(0.));
		if(i==nBalls-1) {
			ctrt->connect(balls[0]->getContour("topPoint"),balls[i]->getContour("Plane"));
			ctrb->connect(balls[0]->getContour("bottomPoint"),balls[i]->getContour("Plane"));
		}
		else {
			ctrt->connect(balls[i+1]->getContour("topPoint"),balls[i]->getContour("Plane"));
			ctrb->connect(balls[i+1]->getContour("bottomPoint"),balls[i]->getContour("Plane"));
		}
		this->addLink(ctrt);
		this->addLink(ctrb);
	}

  setPlotFeatureRecursive(generalizedPosition, true);
  setPlotFeatureRecursive(generalizedVelocity, true);
  setPlotFeatureRecursive(generalizedRelativePosition, true);
  setPlotFeatureRecursive(generalizedRelativeVelocity, true);
  setPlotFeatureRecursive(generalizedForce, true);
}

void System::reduce(const string & h5file) {

  rod->enablePOD(h5file, 1, 40);

}

