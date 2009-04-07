/* Copyright (C) 2004-2009 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: mfoerg@users.berlios.de
 */

#include <config.h>
#include "mbsim/link_mechanics.h"
#include "mbsim/dynamic_system.h"
#include "mbsim/frame.h"
#include "mbsim/contour.h"

#ifdef HAVE_AMVIS
#include "arrow.h"
using namespace AMVis;
#endif

using namespace fmatvec;
using namespace std;

namespace MBSim {

  LinkMechanics::~LinkMechanics() { 

#ifdef HAVE_AMVIS   
    for (unsigned int i=0; i<arrowAMVis.size(); i++) {
      delete arrowAMVis[i];
      delete arrowAMVisUserFunctionColor[i];
    }
#endif

  }

  void LinkMechanics::updater(double t) {

    for(unsigned i=0; i<frame.size(); i++) 
      r[i] += W[i]*la;

    for(unsigned i=0; i<contour.size(); i++) 
      r[i] += W[i]*la;
  }

  //void LinkMechanics::load(const string &path, ifstream& inputfile) {
  //  Link::load(path, inputfile);
  //  string dummy;

  //  getline(inputfile,dummy); // # Connected cosy
  //  int n = getNumberOfElements(inputfile);
  //  for(int i=0; i<n; i++) {
  //    getline(inputfile,dummy); // Connected cosy
  //    connect(getDynamicSystemSolver()->findFrame(dummy));
  //  }
  //  getline(inputfile,dummy); // newline

  //  getline(inputfile,dummy); // # Connected contours
  //  n = getNumberOfElements(inputfile);
  //  for(int i=0; i<n; i++) {
  //    getline(inputfile,dummy); // Connected contour
  //    connect(getDynamicSystemSolver()->findContour(dummy));
  //  }
  //  getline(inputfile,dummy); // newline
  //}

  //void LinkMechanics::save(const string &path, ofstream& outputfile) {
  //  Link::save(path,outputfile);

  //  outputfile << "# Connected coordinate sytems:" << endl;
  //  for(unsigned int i=0; i<frame.size(); i++) {
  //    outputfile << frame[i]->getFullName() << endl;
  //  }
  //  outputfile << endl;

  //  outputfile << "# Connected contours:" << endl;
  //  for(unsigned int i=0; i<contour.size(); i++) {
  //    outputfile << contour[i]->getFullName() << endl;
  //  }
  //  outputfile << endl;
  //}

  void LinkMechanics::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      //#ifdef HAVE_AMVIS TODO force visualisation
      //      Vec WrOToPoint;
      //      Vec LoadArrow(6,NONINIT);
      //      for (unsigned int i=0; i<arrowAMVis.size(); i++) {
      //        WrOToPoint = frame[arrowAMVisID[i]]->getPosition();
      //        if(setValued){ 
      //          if (isActive()) {
      //            LoadArrow(0,2) = fF[arrowAMVisID[i]]*la/dt;
      //            LoadArrow(3,5) = fM[arrowAMVisID[i]]*la/dt;
      //          }
      //          else {
      //            LoadArrow = Vec(6,INIT,0.0);
      //            WrOToPoint= Vec(3,INIT,0.0);
      //          }
      //        }
      //        else {
      //          LoadArrow(0,2) = WF[arrowAMVisID[i]];
      //          LoadArrow(3,5) = WM[arrowAMVisID[i]];
      //        }
      //        LoadArrow= LoadArrow/1000*arrowAMVisScale[i]; // scaling: 1KN or 1KNm scaled to arrowlenght one
      //
      //        arrowAMVis[i]->setTime(t);
      //        arrowAMVis[i]->setToPoint(WrOToPoint(0),WrOToPoint(1),WrOToPoint(2));
      //        double color;
      //        if (arrowAMVisMoment[i]) {
      //          arrowAMVis[i]->setDirection(LoadArrow(3),LoadArrow(4),LoadArrow(5));
      //          color=0.5;
      //        }
      //        else {
      //          arrowAMVis[i]->setDirection(LoadArrow(0),LoadArrow(1),LoadArrow(2));
      //          color =1.0;
      //        }
      //        if (arrowAMVisUserFunctionColor[i]) {
      //          color = (*arrowAMVisUserFunctionColor[i])(t)(0);
      //          if (color>1) color=1;
      //          if (color<0) color=0;
      //        }  
      //        arrowAMVis[i]->setColor(color);
      //        arrowAMVis[i]->appendDataset(0);
      //      }
      //
      //      for (unsigned int i=0; i<arrowAMVis.size(); i++) {
      //        if(setValued) { 
      //          if (isActive()) {
      //            LoadArrow(0,2) = fF[arrowAMVisID[i]]*la/dt;
      //            LoadArrow(3,5) = fM[arrowAMVisID[i]]*la/dt;
      //          }
      //          else {
      //            LoadArrow = Vec(6,INIT,0.0);
      //            WrOToPoint= Vec(3,INIT,0.0);
      //          }
      //        }
      //        else {
      //          LoadArrow(0,2) = WF[arrowAMVisID[i]];
      //          LoadArrow(3,5) = WM[arrowAMVisID[i]];
      //        }
      //        LoadArrow(0,2)= LoadArrow(0,2)/1000*arrowAMVisScale[i]; // Scaling: 1KN or 1KNm scaled to arrowlenght one
      //
      //        arrowAMVis[i]->setTime(t);
      //        arrowAMVis[i]->setToPoint(WrOToPoint(0),WrOToPoint(1),WrOToPoint(2));
      //        double color;
      //        if (arrowAMVisMoment[i]) {
      //          arrowAMVis[i]->setDirection(LoadArrow(3),LoadArrow(4),LoadArrow(5));
      //          color=0.5;
      //        }
      //        else {
      //          arrowAMVis[i]->setDirection(LoadArrow(0),LoadArrow(1),LoadArrow(2));
      //          color =1.0;
      //        }
      //        if (arrowAMVisUserFunctionColor[i]) {
      //          color = (*arrowAMVisUserFunctionColor[i])(t)(0);
      //          if (color>1) color=1;
      //          if (color<0) color=0;
      //        }  
      //        arrowAMVis[i]->setColor(color);
      //        arrowAMVis[i]->appendDataset(0);
      //      }
      //#endif
      Link::plot(t,dt);
    }
  }

  void LinkMechanics::updateWRef(const Mat& WParent, int j) {
    for(unsigned i=0; i<frame.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(frame[i]->getParent()->gethInd(parent,j),frame[i]->getParent()->gethInd(parent,j)+frame[i]->getJacobianOfTranslation().cols()-1);
      W[i].resize()>>WParent(I,J);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(contour[i]->getParent()->gethInd(parent,j),contour[i]->getParent()->gethInd(parent,j)+contour[i]->gethSize(j)-1);
      W[i]>>WParent(I,J);
    }
  } 

  void LinkMechanics::updateVRef(const Mat& VParent, int j) {
    for(unsigned i=0; i<frame.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(frame[i]->getParent()->gethInd(parent,j),frame[i]->getParent()->gethInd(parent,j)+frame[i]->getJacobianOfTranslation().cols()-1);
      V[i].resize()>>VParent(I,J);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      Index J = Index(laInd,laInd+laSize-1);
      Index I = Index(contour[i]->getParent()->gethInd(parent,j),contour[i]->getParent()->gethInd(parent,j)+contour[i]->getWJP().cols()-1);
      V[i]>>VParent(I,J);
    }
  } 

  void LinkMechanics::updatehRef(const Vec &hParent, int j) {
    for(unsigned i=0; i<frame.size(); i++) {
      Index I = Index(frame[i]->getParent()->gethInd(parent,j),frame[i]->getParent()->gethInd(parent,j)+frame[i]->getJacobianOfTranslation().cols()-1);
      h[i]>>hParent(I);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      Index I = Index(contour[i]->getParent()->gethInd(parent,j),contour[i]->getParent()->gethInd(parent,j)+contour[i]->getWJP().cols()-1);
      h[i]>>hParent(I);
    }
  } 

  void LinkMechanics::updaterRef(const Vec &rParent) {
    for(unsigned i=0; i<frame.size(); i++) {
      int hInd =  frame[i]->getParent()->gethInd(parent);
      Index I = Index(hInd,hInd+frame[i]->getJacobianOfTranslation().cols()-1);
      r[i]>>rParent(I);
    }
    for(unsigned i=0; i<contour.size(); i++) {
      int hInd =  contour[i]->getParent()->gethInd(parent);
      Index I = Index(hInd,hInd+contour[i]->getWJP().cols()-1);
      r[i]>>rParent(I);
    }
  } 

  void LinkMechanics::init() {
    Link::init();

    for(unsigned i=0; i<frame.size(); i++) {
      W.push_back(Mat(frame[i]->getJacobianOfTranslation().cols(),laSize));
      V.push_back(Mat(frame[i]->getJacobianOfTranslation().cols(),laSize));
      h.push_back(Vec(frame[i]->getJacobianOfTranslation().cols()));
      r.push_back(Vec(frame[i]->getJacobianOfTranslation().cols()));
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

  void LinkMechanics::initPlot() {
    updatePlotFeatures(parent);

    if(getPlotFeature(plotRecursive)==enabled) {
//#ifdef HAVE_AMVIS
//      for (unsigned int i=0; i<arrowAMVis.size(); i++)
//        arrowAMVis[i]->writeBodyFile();
//#endif
      Link::initPlot();
    }
  }

  void LinkMechanics::connect(Frame *frame_) {
    frame.push_back(frame_);
  }

  void LinkMechanics::connect(Contour *contour_) {
    contour.push_back(contour_);
  }

#ifdef HAVE_AMVIS
  void LinkMechanics::addAMVisForceArrow(AMVis::Arrow *arrow, double scale, int ID, UserFunction *funcColor) {
    assert(ID >= 0);
    assert(ID < 2);
    arrowAMVis.push_back(arrow);
    arrowAMVisScale.push_back(scale);
    arrowAMVisID.push_back(ID);
    arrowAMVisUserFunctionColor.push_back(funcColor);
    arrowAMVisMoment.push_back(false);
  }

  void LinkMechanics::addAMVisMomentArrow(AMVis::Arrow *arrow,double scale ,int ID, UserFunction *funcColor) {
    assert(ID >= 0);
    assert(ID < 2);
    arrowAMVis.push_back(arrow);
    arrowAMVisScale.push_back(scale);
    arrowAMVisID.push_back(ID);
    arrowAMVisUserFunctionColor.push_back(funcColor);
    arrowAMVisMoment.push_back(true);
  }
#endif

}

