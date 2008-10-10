/* Copyright (C) 2004-2006  Martin Förg, Roland Zander
 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA

 *
 * Contact:
 *   mfoerg@users.berlios.de
 *   rzander@users.berlios.de
 *
 */
#include <config.h> 
#include "contact.h"
#include "object.h"
#include "contour.h"
#include "functions_contact.h"
#include "multi_body_system.h"
#include "nonlinear_algebra.h"

// --- List of contact kinematic implementations - BEGIN ---
#include "point_line.h"
#include "circlesolid_contour1s.h"
#include "circlesolid_line.h"
#include "circlesolid_plane.h"
#include "point_plane.h"
#include "point_area.h"
#include "edge_edge.h"
#include "circlehollow_cylinderflexible.h"
#include "point_cylinderflexible.h"
#include "sphere_plane.h"
#include "point_contourinterpolation.h"
#include "point_contour1s.h"
#include "line_contour1s.h"
#include "circlesolid_circlehollow.h"
#include "circlesolid_circlesolid.h"
#include "sphere_sphere.h"
#include "sphere_frustum.h"
#include "point_frustum.h"
#include "circlesolid_frustum2d.h"
// --- List of contact kinematic implementations -  END  ---

namespace MBSim {

  int min(int i, int j) {
    return i<j?i:j;
  }

  Contact::Contact(const string &name, bool setValued) : Link(name,setValued), nFric(0), contactKinematics(0) {

    active = false;
  }

  Contact::Contact(const Contact *master, const string &name) : Link(name,master->setValued), iT(1,master->iT.end()), nFric(master->iT.end()), contactKinematics(0) {

   // mu = master->mu; TODO

    active = false;
  }

  Contact::~Contact() {
    if (contactKinematics) delete contactKinematics;
  }
  
  void Contact::calcSize() {
    Link::calcSize();
    gSize = 1;
    laSize = 1+nFric;
    rFactorSize = setValued?1+min(nFric,1):0;
  }

  void Contact::init() {
    Link::init();

    if(contactKinematics);

    else if((dynamic_cast<Point*>(contour[0]) && dynamic_cast<Line*>(contour[1])) || (dynamic_cast<Point*>(contour[1]) && dynamic_cast<Line*>(contour[0]))) 
      contactKinematics = new ContactKinematicsPointLine; 

    else if((dynamic_cast<CircleSolid*>(contour[0]) && dynamic_cast<Line*>(contour[1])) || (dynamic_cast<CircleSolid*>(contour[1]) && dynamic_cast<Line*>(contour[0]))) 
      contactKinematics = new ContactKinematicsCircleSolidLine;

    else if((dynamic_cast<CircleSolid*>(contour[0]) && dynamic_cast<Plane*>(contour[1])) || (dynamic_cast<CircleSolid*>(contour[1]) && dynamic_cast<Plane*>(contour[0]))) 
      contactKinematics = new ContactKinematicsCircleSolidPlane;

    else if((dynamic_cast<Point*>(contour[0]) && dynamic_cast<Plane*>(contour[1])) || (dynamic_cast<Point*>(contour[1]) && dynamic_cast<Plane*>(contour[0]))) 
      contactKinematics = new ContactKinematicsPointPlane;

    else if((dynamic_cast<Point*>(contour[0]) && dynamic_cast<Area*>(contour[1])) || (dynamic_cast<Point*>(contour[1]) && dynamic_cast<Area*>(contour[0]))) 
      contactKinematics = new ContactKinematicsPointArea;

    else if(dynamic_cast<Edge*>(contour[0]) && dynamic_cast<Edge*>(contour[1])) 
      contactKinematics = new ContactKinematicsEdgeEdge;

    else if((dynamic_cast<Sphere*>(contour[0]) && dynamic_cast<Plane*>(contour[1])) || (dynamic_cast<Sphere*>(contour[1]) && dynamic_cast<Plane*>(contour[0]))) 
      contactKinematics = new ContactKinematicsSpherePlane;

    // INTERPOLATIONSGESCHICHTEN - Interpol-Point
    else if((dynamic_cast<Point*>(contour[0]) &&  dynamic_cast<ContourInterpolation*>(contour[1])) || (dynamic_cast<Point*>(contour[1]) &&  dynamic_cast<ContourInterpolation*>(contour[0]))) 
      contactKinematics = new ContactKinematicsPointContourInterpolation;
    // INTERPOLATIONSGESCHICHTEN

    else if((dynamic_cast<CircleHollow*>(contour[0]) && dynamic_cast<CylinderFlexible*>(contour[1])) || (dynamic_cast<CircleHollow*>(contour[1]) && dynamic_cast<CylinderFlexible*>(contour[0]))) 
      contactKinematics = new ContactKinematicsCircleHollowCylinderFlexible;

    else if((dynamic_cast<Point*>(contour[0]) && dynamic_cast<CylinderFlexible*>(contour[1])) || (dynamic_cast<Point*>(contour[1]) && dynamic_cast<CylinderFlexible*>(contour[0])))
      contactKinematics = new ContactKinematicsPointCylinderFlexible;

    else if((dynamic_cast<Point*>(contour[0]) && dynamic_cast<Contour1s*>(contour[1])) || (dynamic_cast<Point*>(contour[1]) && dynamic_cast<Contour1s*>(contour[0]))) 
      contactKinematics = new ContactKinematicsPointContour1s;

    else if((dynamic_cast<Line*>(contour[0]) && dynamic_cast<Contour1s*>(contour[1])) || (dynamic_cast<Line*>(contour[1]) && dynamic_cast<Contour1s*>(contour[0]))) 
      contactKinematics = new ContactKinematicsLineContour1s;

    else if((dynamic_cast<CircleSolid*>(contour[0]) && dynamic_cast<Contour1s*>(contour[1])) || (dynamic_cast<CircleSolid*>(contour[1]) && dynamic_cast<Contour1s*>(contour[0])))
      contactKinematics = new ContactKinematicsCircleSolidContour1s;


    else if((dynamic_cast<CircleSolid*>(contour[0]) && dynamic_cast<CircleHollow*>(contour[1])) || (dynamic_cast<CircleSolid*>(contour[1]) && dynamic_cast<CircleHollow*>(contour[0])))
      contactKinematics = new ContactKinematicsCircleSolidCircleHollow;

    else if(dynamic_cast<CircleSolid*>(contour[0]) && dynamic_cast<CircleSolid*>(contour[1]))
      contactKinematics = new ContactKinematicsCircleSolidCircleSolid;

    else if(dynamic_cast<Sphere*>(contour[0]) && dynamic_cast<Sphere*>(contour[1]))
      contactKinematics = new ContactKinematicsSphereSphere;

    else if((dynamic_cast<Sphere*>(contour[0]) && dynamic_cast<Frustum*>(contour[1])) || (dynamic_cast<Sphere*>(contour[1]) && dynamic_cast<Frustum*>(contour[0])))
      contactKinematics = new ContactKinematicsSphereFrustum;

    else if((dynamic_cast<CircleSolid*>(contour[0]) && dynamic_cast<Frustum2D*>(contour[1])) || (dynamic_cast<CircleSolid*>(contour[1]) && dynamic_cast<Frustum2D*>(contour[0]))) 
      contactKinematics = new ContactKinematicsCircleSolidFrustum2D;

    else if((dynamic_cast<Point*>(contour[0]) && dynamic_cast<Frustum*>(contour[1])) || (dynamic_cast<Point*>(contour[1]) && dynamic_cast<Frustum*>(contour[0])))
      contactKinematics = new ContactKinematicsPointFrustum;  

    else {
      cout << "Unkown contact pairing" <<endl;
      throw 5;
    }

    ContourPointData cpd[2];
    cpData.push_back(cpd[0]);
    cpData.push_back(cpd[1]);
    cpData[0].type = CONTINUUM; // default-Wert
    cpData[0].Wn.resize(3,1);
    cpData[0].Wt.resize(3,nFric);
    cpData[1].type = CONTINUUM; // default-Wert
    cpData[1].Wn.resize(3,1);
    cpData[1].Wt.resize(3,nFric);
    iT = Index(1,nFric);
    connectHitSpheres(contour[0],contour[1]);
    contactKinematics->assignContours(contour);
  }

  void Contact::connect(Contour *contour0, Contour* contour1) {
    Link::connect(contour0,0);
    Link::connect(contour1,1);
  }

  void Contact::connectHitSpheres(Contour *contour0, Contour* contour1) {
    // wird eh erst im init() ausgefuehrt
    if(checkHSLink) {
      Object* obj0 = contour0->getObject();
      Object* obj1 = contour1->getObject();

      HSLink = parent->getHitSphereLink(obj0,obj1);
      HSLink->setParents(obj0,obj1,this);
    }
  }

  void Contact::updateStage1(double t) {
    contactKinematics->stage1(g,cpData);
    checkActive();
  }

  void Contact::updateStage2(double t) {
    contactKinematics->stage2(g,gd,cpData);
    if(active) 
      updateKinetics(t);
  }

  void Contact::updateW(double t) {
    contour[0]->updateMovingFrame(t, cpData[0]);
    contour[1]->updateMovingFrame(t, cpData[1]);
    Link::updateW(t);
  }

  void Contact::checkActive() {
    if(isActive())
      active = true;
    else {
      active = false;
    }
  }

  void Contact::save(const string& path, ofstream &outputfile) {
    Link::save(path, outputfile);

   // outputfile << "# Number of friction directions:" << endl;
   // outputfile << nFric << endl;
   // outputfile << endl;

   // outputfile << "# Friction coefficient:" << endl;
   // //outputfile << mu << endl;
   // outputfile << endl;
  }

  void Contact::load(const string& path, ifstream &inputfile) {
    Link::load(path,inputfile);
   // string dummy;

   // getline(inputfile,dummy); // # Number of friction directions
   // inputfile >> nFric;
   // getline(inputfile,dummy); // Rest of line
   // getline(inputfile,dummy); // Newline

   // getline(inputfile,dummy); // # Friction coefficient
   // //inputfile >> mu;
   // getline(inputfile,dummy); // Rest of line
   // getline(inputfile,dummy); // Newline
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// ZUR NUMERISCHEN BESTIMMUNG DER GENERALISIERTEN KRAFTRICHTUNGEN WN
/////////////////////////////////////////////////////////////////////////////////////////////////////////

//       if (false) {
// // Numerische Ableitung gp(0) -> WN    
//     static bool rootCall = true;
//     static double eps = 5.e-9;

//     if(rootCall) {
// 	rootCall = false;

// 	int WNSize = cylinder->getObject()->getqSize();
// 	Mat WN(WNSize,1);
// 	const double gAlt = g(0);

// 	for(int i=0;i<WNSize;i++) {
// 	    Vec qTemp = cylinder->getObject()->getq();
// 	    qTemp(i) += eps;
// 	    cylinder->getObject()->setq(qTemp);
// 	    pairingCircleHollowCylinderFlexibleStage1(icircle,icylinder);
// 	    WN(i,0) = (g(0) - gAlt) / eps ;
// 	    qTemp(i) -= eps;
// 	    cylinder->getObject()->setq(qTemp);
// 	}

// 	// reset
// 	pairingCircleHollowCylinderFlexibleStage2(icircle,icylinder);

// // 	cout << "numerisches WN = " << trans(WN) << endl;

// 	rootCall = true;
//     }

//       }
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

