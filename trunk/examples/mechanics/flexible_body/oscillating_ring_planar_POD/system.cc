#include "system.h"
#include "mbsim/joint.h"
#include "mbsim/contact.h"
#include "mbsim/contours/point.h"
#include "mbsim/contours/plane.h"
#include "mbsim/constitutive_laws.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/environment.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/spineextrusion.h>
#include <openmbvcppinterface/cuboid.h>
#include <openmbvcppinterface/polygonpoint.h>
#endif

using namespace MBSimFlexibleBody;
using namespace MBSim;
using namespace fmatvec;
using namespace std;
using namespace H5;

#include <hdf5serie/vectorserie.h>
#include <fmatvec.h>
#include <iostream>

Mat read3D(const string SysName) {
  H5File *file = new H5File(SysName, H5F_ACC_RDONLY);
  H5::Group group = file->openGroup("Rod");
  H5::VectorSerie<double> * data = new H5::VectorSerie<double>;
  data->open(group, "data");

  int qsize = (data->getColumns() - 1) / 4;
  int tsize = data->getColumn(0).size();

  int i;
  int j;
  int elements = (qsize) / 3;
  Mat X_(tsize, qsize, INIT, 0.);

  for (i = 0; i < elements; i++) {
    j = 3 * i;
    Vec tmp_x(data->getColumn(2 * j + 1));
    Vec tmp_y(data->getColumn(2 * j + 1 + 1));
    Vec tmp_gamma(data->getColumn(2 * j + 5 + 1));

    X_(Index(0, tsize - 1), Index(j, j)) = tmp_x(Index(0, tsize - 1));
    X_(Index(0, tsize - 1), Index(j + 1, j + 1)) = tmp_y(Index(0, tsize - 1));
    X_(Index(0, tsize - 1), Index(j + 2, j + 2)) = tmp_gamma(Index(0, tsize - 1));
  }

  return X_.T();
}

System::System(const string &projectName) :
    DynamicSystemSolver(projectName) {

  // acceleration of gravity
  Vec grav(3, INIT, 0.); //grav(1) = -9.81;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(grav);

  // Geometry
  double l = 5.; 				// length ring
  double b0 = 0.1; 				// width
  double A = b0 * b0; 				// cross-section area
  double I1 = 1. / 12. * b0 * b0 * b0 * b0; // moment inertia
  int elements = 20; 				// number of finite elements
  int DOF = 3;					// DOFs per nod (x,y,gamma)
  // Material
  double E = 5e7; 			// E-Modul alu
  double mu = 0.3; 			// Poisson ratio
  double G = E / (2 * (1 + mu)); 	// shear modulus
  double rho = 9.2e2; 		// density alu

  // Set Parameters for 2D Cosserat Rod
  rod = new FlexibleBody1s21Cosserat("Rod", false);
  rod->setLength(l);
  rod->setEGModuls(E, G);
  rod->setCrossSectionalArea(A);
  rod->setMomentsInertia(I1);
  rod->setDensity(rho);
  rod->setFrameOfReference(this->getFrame("I"));
  rod->setNumberElements(elements);
  rod->setCuboid(b0, b0);
  rod->setCurlRadius(l / (2 * M_PI));
  //  rod->setMassProportionalDamping(20.);
  //  rod->setMaterialDamping(0.1,0.1);

  double stretchFactor = 1.1;
  Vec q0 = Vec(DOF * elements, INIT, 0.);
  double R = (l / elements) / (2. * sin(M_PI / elements)) * stretchFactor;
  double dphi = (2 * M_PI) / elements;
  for (int i = 0; i < elements; i++) {
    double phi = M_PI / 2. - i * dphi;
    q0(DOF * i) = R * cos(phi);
    q0(DOF * i + 1) = R * sin(phi);
    q0(DOF * i + 2) = phi - dphi / 2. - M_PI / 2.;
  }

  rod->setq0(q0);
  rod->setu0(Vec(q0.size(), INIT, 0.));
  addObject(rod);

#ifdef HAVE_OPENMBVCPPINTERFACE
  OpenMBV::SpineExtrusion *cuboid=new OpenMBV::SpineExtrusion;
  cuboid->setNumberOfSpinePoints(elements*4); // resolution of visualisation
  cuboid->setStaticColor(0.5);// color in (minimalColorValue, maximalColorValue)
  cuboid->setScaleFactor(1.);// orthotropic scaling of cross section
  vector<OpenMBV::PolygonPoint*> *rectangle = new vector<OpenMBV::PolygonPoint*>;// clockwise ordering, no doubling for closure
  OpenMBV::PolygonPoint *corner1 = new OpenMBV::PolygonPoint(b0*0.5,b0*0.5,1);
  rectangle->push_back(corner1);
  OpenMBV::PolygonPoint *corner2 = new OpenMBV::PolygonPoint(b0*0.5,-b0*0.5,1);
  rectangle->push_back(corner2);
  OpenMBV::PolygonPoint *corner3 = new OpenMBV::PolygonPoint(-b0*0.5,-b0*0.5,1);
  rectangle->push_back(corner3);
  OpenMBV::PolygonPoint *corner4 = new OpenMBV::PolygonPoint(-b0*0.5,b0*0.5,1);
  rectangle->push_back(corner4);

  cuboid->setContour(rectangle);
  rod->setOpenMBVSpineExtrusion(cuboid);
#endif
}

void System::reduce(const string & h5file) {

  rod->enablePOD(h5file, 1, 5);

}
