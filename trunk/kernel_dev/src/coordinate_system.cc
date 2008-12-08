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
#include "coordinate_system.h"
#include "object.h"
#include "subsystem.h"
#ifdef HAVE_AMVIS
#include "crigidbody.h"
#include "data_interface_base.h"
#include "rotarymatrices.h"
using namespace AMVis;
#endif

namespace MBSim {

  void transformCoordinateSystem(CoordinateSystem &cosy1, const Vec &r, CoordinateSystem &cosy2) {
    cosy2.setAngularVelocity(cosy1.getAngularVelocity());
    cosy2.setVelocity(cosy1.getVelocity() + crossProduct(cosy1.getAngularVelocity(),r));

    Mat tr = tilde(r);
    cosy2.setJacobianOfTranslation(cosy1.getJacobianOfTranslation() - tr*cosy1.getJacobianOfRotation());
    cosy2.setJacobianOfRotation(cosy1.getJacobianOfRotation());
    cosy2.setGyroscopicAccelerationOfTranslation(cosy1.getGyroscopicAccelerationOfTranslation() - tr*cosy1.getGyroscopicAccelerationOfRotation() + crossProduct(cosy1.getAngularVelocity(),crossProduct(cosy1.getAngularVelocity(),r)));
    cosy2.setGyroscopicAccelerationOfRotation(cosy1.getGyroscopicAccelerationOfRotation());
  }


  CoordinateSystem::CoordinateSystem(const string &name) : Element(name), parent(0), hSize(0), hInd(0), adress(0), WrOP(3), WvP(3), WomegaP(3), AWP(3), WjP(3), WjR(3) {
#ifdef HAVE_AMVIS
    bodyAMVisUserFunctionColor= NULL;
    bodyAMVis = NULL;
#endif
    AWP(0,0) = 1;
    AWP(1,1) = 1;
    AWP(2,2) = 1;
    WJP.resize(3,0);
    WJR.resize(3,0);
    plotLevel= 0;
  }

  // string CoordinateSystem::getFullName() const {
  //   return parent->getFullName() + "." + name;
  // }

  void CoordinateSystem::init() {
    getJacobianOfTranslation().resize(3,hSize);
    getJacobianOfRotation().resize(3,hSize);
  }

  //int CoordinateSystem::gethInd(Subsystem* sys) {
  //  return parent->gethInd(sys);
  // }


#ifdef HAVE_AMVIS
  void CoordinateSystem::setAMVisBody(CRigidBody *AMVisBody, DataInterfaceBase *funcColor){
    bodyAMVis = AMVisBody;
    bodyAMVisUserFunctionColor = funcColor;
    if (!plotLevel) plotLevel=1;
  }
#endif

  void CoordinateSystem::plot(double t, double dt) {				// HR 03.01.07
    Element::plot(t,dt);
    if (plotLevel > 0) {
      for(int i=0; i<3; i++)
	plotfile<<" "<< WrOP(i);
      Vec AlpBetGam = AIK2Cardan(AWP);
      for(int i=0; i<3; i++)
	plotfile<<" "<< AlpBetGam(i);
    }
#ifdef HAVE_AMVIS
    if (bodyAMVis) {
      Vec AlpBetGam;
      AlpBetGam = AIK2Cardan(AWP);
      if (bodyAMVisUserFunctionColor) {
	double color = (*bodyAMVisUserFunctionColor)(t)(0);
	if (color>1)   color =1;
	if (color<0) color =0;
	bodyAMVis->setColor(color);
      }
      bodyAMVis->setTime(t);
      bodyAMVis->setTranslation(WrOP(0),WrOP(1),WrOP(2));
      bodyAMVis->setRotation(AlpBetGam(0),AlpBetGam(1),AlpBetGam(2));
      bodyAMVis->appendDataset(0);
    }
#endif
  }

  void CoordinateSystem::initPlotFiles() {					// HR 03.01.07
#ifdef HAVE_AMVIS
    if(bodyAMVis)
      bodyAMVis->writeBodyFile();  
#endif

    Element::initPlotFiles();
    if(plotLevel > 0) {
      for(int i=0; i<3; i++)
	plotfile <<"# "<< plotNr++ <<": WrOP("<<i<<")" << endl;
      plotfile <<"# "<< plotNr++ <<": alpha" << endl;
      plotfile <<"# "<< plotNr++ <<": beta" << endl;
      plotfile <<"# "<< plotNr++ <<": gamma" << endl;
    }
  }

}
