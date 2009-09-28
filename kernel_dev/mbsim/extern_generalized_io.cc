/* Copyright (C) 2004-2009 MBSim Development Team
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
 * Contact: mfoerg@users.berlios.de
 */

#include "config.h"
#include "mbsim/extern_generalized_io.h"
#include "mbsim/dynamic_system.h"

using namespace std;
using namespace fmatvec;

namespace MBSim {

  ExternGeneralizedIO::ExternGeneralizedIO(const string &name) : Link(name),
    connectedObject(NULL), qInd(0), uInd(0), saved_connectedObject("") {
  }

  void ExternGeneralizedIO::updateh(double) {
    connectedObject->geth()(uInd)+=la(0);
  }

  void ExternGeneralizedIO::updateg(double) {
    g(0)=connectedObject->getq()(qInd);
  } 

  void ExternGeneralizedIO::updategd(double) {
    gd(0)=connectedObject->getu()(uInd);
  }

  void ExternGeneralizedIO::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_connectedObject!="") {
        if(saved_connectedObject.substr(0,3)=="../")
          connectedObject=parent->getObjectByPath(saved_connectedObject.substr(3));
        else
          connectedObject=parent->getObjectByPath(saved_connectedObject);
      }
      Link::init(stage);
    }
    else if(stage==resize) {
      Link::init(stage);
      g.resize(1);
      gd.resize(1);
      la.resize(1);
    }
    else if(stage==MBSim::plot) {
      updatePlotFeatures(parent);
      if(getPlotFeature(plotRecursive)==enabled) {
        plotColumns.push_back("la(0)");
        Link::init(stage);
      }
    }
    else
      Link::init(stage);
  }

  void ExternGeneralizedIO::plot(double t,double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      plotVector.push_back(la(0));
      Link::plot(t,dt);
    }
  }

  void ExternGeneralizedIO::initializeUsingXML(TiXmlElement *element) {
    Link::initializeUsingXML(element);
    saved_connectedObject=element->FirstChildElement(MBSIMNS"connectedObject")->Attribute("ref");
    qInd=(int)(getDouble(element->FirstChildElement(MBSIMNS"qIndex"))+0.5);
    uInd=(int)(getDouble(element->FirstChildElement(MBSIMNS"uIndex"))+0.5);
  }

}
