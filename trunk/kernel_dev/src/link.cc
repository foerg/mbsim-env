/* Copyright (C) 2004-2006  Martin Förg
 
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
 *
 */
#include <config.h>
#include <string.h>
#include "link.h"
#include "coordinate_system.h"
#include "contour.h"
#include "object.h"
#include "multi_body_system.h"
#include "subsystem.h"

#ifdef HAVE_AMVIS
#include "arrow.h"
using namespace AMVis;
#endif

namespace MBSim {

  Link::Link(const string &name, bool setValued_) : Element(name), xSize(0), xInd(0), svSize(0), svInd(0), setValued(setValued_), gSize(0), gInd(0), gdSize(0), gdInd(0), laSize(0), laInd(0), rFactorSize(0), scaleTolQ(1e-9), scaleTolp(1e-5), gdTol(1e-8), laTol(1e-2), rMax(1.0), HSLink(0), checkHSLink(false) { // , active(true), parent(0), 
  }

  Link::~Link() { 
#ifdef HAVE_AMVIS   
    for (unsigned int i=0; i<arrowAMVis.size(); i++) {
      delete arrowAMVis[i];
      delete arrowAMVisUserFunctionColor[i];
    }
#endif

  }

  bool Link::isSetValued() const {
    return setValued;
  }

  void Link::init() {
    gdn.resize(gdSize);
    rFactorUnsure.resize(rFactorSize);

    for(unsigned i=0; i<port.size(); i++) {
      W.push_back(Mat(port[i]->getJacobianOfTranslation().cols(),laSize));
      V.push_back(Mat(port[i]->getJacobianOfTranslation().cols(),laSize));
      h.push_back(Vec(port[i]->getJacobianOfTranslation().cols()));
      r.push_back(Vec(port[i]->getJacobianOfTranslation().cols()));
      WF.push_back(Vec(3));
      WM.push_back(Vec(3));
      fF.push_back(Mat(3,laSize));
      fM.push_back(Mat(3,laSize));
    }
    for(unsigned i=0; i<contour.size(); i++) {
      W.push_back(Mat(contour[i]->getWJP().cols(),laSize));
      V.push_back(Mat(contour[i]->getWJP().cols(),laSize));
      h.push_back(Vec(contour[i]->getWJP().cols()));
      r.push_back(Vec(contour[i]->getWJP().cols()));
      WF.push_back(Vec(3));
      WM.push_back(Vec(3));
      fF.push_back(Mat(3,laSize));
      fM.push_back(Mat(3,laSize));
    }
  }

  void Link::initz() {
    x=x0;
  }

  //string Link::getFullName() const {
  //  return parent->getFullName() + "." + name;
  //}

 // void Link::updatesvRef() {
 //   sv >> parent->getsv()(svInd,svInd+svSize-1);
 // }

 // void Link::updatejsvRef() {
 //   jsv >> parent->getjsv()(svInd,svInd+svSize-1);
 // }

  void Link::updatelaRef(const Vec& laParent) {
    la.resize() >> laParent(laInd,laInd+laSize-1);
  }

  void Link::updategRef(const Vec& gParent) {
    g.resize() >> gParent(gInd,gInd+gSize-1);
  }

  void Link::updategdRef(const Vec& gdParent) {
    gd.resize() >> gdParent(gdInd,gdInd+gdSize-1);
  }

  void Link::updatebRef(const Vec& bParent) {
    b.resize() >> bParent(laInd,laInd+laSize-1);
  }

  void Link::updatesRef(const Vec& sParent) {
    s.resize() >> sParent(gdInd,gdInd+gdSize-1);
  }

  void Link::updateresRef(const Vec& resParent) {
    res.resize() >> resParent(laInd,laInd+laSize-1);
  }

  void Link::updaterFactorRef(const Vec& rFactorParent) {
    rFactor.resize() >> rFactorParent(rFactorInd,rFactorInd+rFactorSize-1);
  }

  void Link::plot(double t, double dt) {

    Element::plot(t,dt);

    if(plotfile>0) {
      if(plotLevel > 1) {

	for(int i=0; i<xSize; ++i)
	  plotfile<<" "<<x(i);

	for(int i=0; i<xSize; ++i)
	  plotfile<<" "<<xd(i)/dt;
      }
      for(int i=0; i<gSize; ++i)
	plotfile<<" "<<g(i);

    }
    if(isActive() || plotLevel > 2) {

      if(plotLevel>0) {
	if(plotLevel>1) {
	  for(int i=0; i<laSize; ++i)
	    plotfile<<" "<<gd(i);
	}
	if(setValued)
	  for(int i=0; i<laSize; ++i)
	    plotfile<<" "<<la(i)/dt;
	else
	  for(int i=0; i<laSize; ++i)
	    plotfile<<" "<<la(i);
      }
    } else {

      if(plotLevel>0) {
	if(plotLevel>1) {
	  for(int i=0; i<laSize; ++i)
	    plotfile<<" "<<0;
	}
	for(int i=0; i<laSize; ++i)
	  plotfile<<" "<<0;

      }
    }
    if(plotLevel>2) {
      plotfile<<" "<<computePotentialEnergy(); 
    }

#ifdef HAVE_AMVIS
    Vec WrOToPoint;
    Vec LoadArrow(6,NONINIT);
    for (unsigned int i=0; i<arrowAMVis.size(); i++) {
      WrOToPoint = port[arrowAMVisID[i]]->getPosition();
      if(setValued){ 
	if (isActive()) {
	  LoadArrow(0,2) = fF[arrowAMVisID[i]]*la/dt;
	  LoadArrow(3,5) = fM[arrowAMVisID[i]]*la/dt;
	}
	else {
	  LoadArrow = Vec(6,INIT,0.0);
	  WrOToPoint= Vec(3,INIT,0.0);
	}
      }
      else {
	LoadArrow(0,2) = WF[arrowAMVisID[i]];
	LoadArrow(3,5) = WM[arrowAMVisID[i]];
      }
      // Scaling: 1KN or 1KNm scaled to arrowlenght one
      LoadArrow= LoadArrow/1000*arrowAMVisScale[i];

      arrowAMVis[i]->setTime(t);
      arrowAMVis[i]->setToPoint(WrOToPoint(0),WrOToPoint(1),WrOToPoint(2));
      double color;
      if (arrowAMVisMoment[i]) {
	arrowAMVis[i]->setDirection(LoadArrow(3),LoadArrow(4),LoadArrow(5));
	color=0.5;
      }
      else {
	arrowAMVis[i]->setDirection(LoadArrow(0),LoadArrow(1),LoadArrow(2));
	color =1.0;
      }
      if (arrowAMVisUserFunctionColor[i]) {
	color = (*arrowAMVisUserFunctionColor[i])(t)(0);
	if (color>1) color=1;
	if (color<0) color=0;
      }  
      arrowAMVis[i]->setColor(color);
      arrowAMVis[i]->appendDataset(0);
    }

    for (unsigned int i=0; i<arrowAMVis.size(); i++) {
      WrOToPoint = cpData[arrowAMVisID[i]].WrOC;
      if(setValued) { 
	if (isActive()) {
	  LoadArrow(0,2) = fF[arrowAMVisID[i]]*la/dt;
	  LoadArrow(3,5) = fM[arrowAMVisID[i]]*la/dt;
	}
	else {
	  LoadArrow = Vec(6,INIT,0.0);
	  WrOToPoint= Vec(3,INIT,0.0);
	}
      }
      else {
	LoadArrow(0,2) = WF[arrowAMVisID[i]];
	LoadArrow(3,5) = WM[arrowAMVisID[i]];
      }
      // Scaling: 1KN or 1KNm scaled to arrowlenght one
      LoadArrow(0,2)= LoadArrow(0,2)/1000*arrowAMVisScale[i];

      arrowAMVis[i]->setTime(t);
      arrowAMVis[i]->setToPoint(WrOToPoint(0),WrOToPoint(1),WrOToPoint(2));
      double color;
      if (arrowAMVisMoment[i]) {
	arrowAMVis[i]->setDirection(LoadArrow(3),LoadArrow(4),LoadArrow(5));
	color=0.5;
      }
      else {
	arrowAMVis[i]->setDirection(LoadArrow(0),LoadArrow(1),LoadArrow(2));
	color =1.0;
      }
      if (arrowAMVisUserFunctionColor[i]) {
	color = (*arrowAMVisUserFunctionColor[i])(t)(0);
	if (color>1) color=1;
	if (color<0) color=0;
      }  
      arrowAMVis[i]->setColor(color);
      arrowAMVis[i]->appendDataset(0);
    }
#endif

  }

  void Link::initPlotFiles() {

    Element::initPlotFiles();

#ifdef HAVE_AMVIS
    for (unsigned int i=0; i<arrowAMVis.size(); i++)
      arrowAMVis[i]->writeBodyFile();
#endif

    if(plotLevel>0) {
      if(plotLevel>1) {
	for(int i=0; i<xSize; ++i)
	  plotfile <<"# "<< plotNr++ << ": x(" << i << ")" << endl;
	for(int i=0; i<xSize; ++i)
	  plotfile <<"# "<< plotNr++ <<": xd("<<i<<")" << endl;

      }
      for(int i=0; i<gSize; ++i)
	plotfile <<"# "<< plotNr++ << ": g(" << i << ")" << endl;

      if(plotLevel>1) {
	for(int i=0; i<laSize; ++i)
	  plotfile <<"# "<< plotNr++ << ": gd(" << i << ")" << endl;
      }

      for(int i=0; i<laSize; ++i)
	plotfile <<"# "<< plotNr++ << ": la(" << i << ")" << endl;

      if(plotLevel>2) {
	plotfile <<"# "<< plotNr++ << ": V" << endl;
      }
    }
  }

  void Link::savela() {
    la0 = la;
  }

  void Link::initla() {
    // TODO Prufen ob initilisierung auf 0 besser, wenn vorher inaktiv
    la = la0;
  }

  void Link::decreaserFactors() {
    for(int i=0; i<rFactor.size(); i++)
      if(rFactorUnsure(i))
	rFactor(i) *= 0.9;
  }

  void Link::updater(double t) {

    for(unsigned i=0; i<port.size(); i++) 
      r[i] += W[i]*la;
    
    for(unsigned i=0; i<contour.size(); i++) 
      r[i] += W[i]*la;
  }

  void Link::updateb(double t) {
    for(unsigned i=0; i<port.size(); i++) 
      b += trans(fF[i])*port[i]->getGyroscopicAccelerationOfTranslation() + trans(fM[i])*port[i]->getGyroscopicAccelerationOfRotation();
    for(unsigned i=0; i<contour.size(); i++) 
      b += trans(fF[i])*contour[i]->getMovingFrame()->getGyroscopicAccelerationOfTranslation();
      //b += trans(fF[i])*contour[i]->getMovingFrame()->getGyroscopicAccelerationOfTranslation() + trans(fM[i])*contour[i]->getMovingFrame()->getGyroscopicAccelerationOfRotation();
  }

  void Link::updateh(double t) {
    if(isActive()) {
      for(unsigned int i=0; i<port.size(); i++)
	h[i] += trans(port[i]->getJacobianOfTranslation())*WF[i] + trans(port[i]->getJacobianOfRotation())*WM[i];
      for(unsigned int i=0; i<contour.size(); i++) {
	contour[i]->updateMovingFrame(t, cpData[i]);
	h[i] += trans(contour[i]->getMovingFrame()->getJacobianOfTranslation())*WF[i];
      }
    }
  }

  void Link::updateW(double t) {
    for(unsigned int i=0; i<port.size(); i++)
      W[i] += trans(port[i]->getJacobianOfTranslation())*fF[i] + trans(port[i]->getJacobianOfRotation())*fM[i];
    for(unsigned int i=0; i<contour.size(); i++) {
      contour[i]->updateMovingFrame(t, cpData[i]);
      W[i] += trans(contour[i]->getMovingFrame()->getJacobianOfTranslation())*fF[i];
    }
  }

  //int Link::getlaIndMBS() const {
  //  return parent->getlaIndMBS() + laInd;
  //}

  void Link::save(const string &path, ofstream& outputfile) {
    Element::save(path,outputfile);

    outputfile << "# Connected coordinate sytems:" << endl;
    for(unsigned int i=0; i<port.size(); i++) {
      outputfile << port[i]->getFullName() << endl;
    }
    outputfile << endl;

    outputfile << "# Connected contours:" << endl;
    for(unsigned int i=0; i<contour.size(); i++) {
      outputfile << contour[i]->getFullName() << endl;
    }
    outputfile << endl;
  }

  void Link::load(const string &path, ifstream& inputfile) {
    Element::load(path, inputfile);
    string dummy;

    getline(inputfile,dummy); // # Connected cosy
    int n = getNumberOfElements(inputfile);
    for(int i=0; i<n; i++) {
      getline(inputfile,dummy); // Connected cosy
      connect(getMultiBodySystem()->findCoordinateSystem(dummy),i);
    }
    getline(inputfile,dummy); // newline

    getline(inputfile,dummy); // # Connected contours
    n = getNumberOfElements(inputfile);
    for(int i=0; i<n; i++) {
      getline(inputfile,dummy); // Connected contour
      connect(getMultiBodySystem()->findContour(dummy),i);
    }
    getline(inputfile,dummy); // newline
  }

#ifdef HAVE_AMVIS
  void Link::addAMVisForceArrow(AMVis::Arrow *arrow, double scale, int ID, UserFunction *funcColor) {
    assert(ID >= 0);
    assert(ID < 2);
    arrowAMVis.push_back(arrow);
    arrowAMVisScale.push_back(scale);
    arrowAMVisID.push_back(ID);
    arrowAMVisUserFunctionColor.push_back(funcColor);
    arrowAMVisMoment.push_back(false);
  }

  void Link::addAMVisMomentArrow(AMVis::Arrow *arrow,double scale ,int ID, UserFunction *funcColor) {
    assert(ID >= 0);
    assert(ID < 2);
    arrowAMVis.push_back(arrow);
    arrowAMVisScale.push_back(scale);
    arrowAMVisID.push_back(ID);
    arrowAMVisUserFunctionColor.push_back(funcColor);
    arrowAMVisMoment.push_back(true);
  }
#endif

  void Link::connect(CoordinateSystem *port_, int id) {
    port.push_back(port_);
  }

  void Link::connect(Contour *contour_, int id) {
    contour.push_back(contour_);
  //  W.push_back(Mat());
  //  V.push_back(Mat());
  //  h.push_back(Vec());
  //  r.push_back(Vec());
  //  WF.push_back(Vec(3));
  //  WM.push_back(Vec(3));
  //  fF.push_back(Mat());
  //  fM.push_back(Mat());
  }

  void Link::updatexRef(const Vec &xParent) {
    x >> xParent(xInd,xInd+xSize-1);
  } 

  void Link::updatexdRef(const Vec &xdParent) {
    xd >> xdParent(xInd,xInd+xSize-1);
  } 

  void Link::updateVRef(const Mat& VParent) {
    for(unsigned i=0; i<port.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(port[i]->gethInd(),port[i]->gethInd()+port[i]->getJacobianOfTranslation().cols()-1);
      V[i]>>VParent(I,J);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(contour[i]->gethInd(),contour[i]->gethInd()+contour[i]->getWJP().cols()-1);
      V[i]>>VParent(I,J);
    }
  } 

  void Link::updateWRef(const Mat& WParent) {
    for(unsigned i=0; i<port.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(port[i]->gethInd(),port[i]->gethInd()+port[i]->gethSize()-1);
      W[i]>>WParent(I,J);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(contour[i]->gethInd(),contour[i]->gethInd()+contour[i]->gethSize()-1);
      W[i]>>WParent(I,J);
    }
  } 

  void Link::updatehRef(const Vec &hParent) {
    for(unsigned i=0; i<port.size(); i++) {
      Index I = Index(port[i]->gethInd(),port[i]->gethInd()+port[i]->getJacobianOfTranslation().cols()-1);
      h[i]>>hParent(I);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      Index I = Index(contour[i]->gethInd(),contour[i]->gethInd()+contour[i]->getWJP().cols()-1);
      h[i]>>hParent(I);
    }
  } 

  void Link::updaterRef(const Vec &rParent) {
    for(unsigned i=0; i<port.size(); i++) {
      Index I = Index(port[i]->gethInd(),port[i]->gethInd()+port[i]->getJacobianOfTranslation().cols()-1);
      r[i]>>rParent(I);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      Index I = Index(contour[i]->gethInd(),contour[i]->gethInd()+contour[i]->getWJP().cols()-1);
      r[i]>>rParent(I);
    }
  } 

}
