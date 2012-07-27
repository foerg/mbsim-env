/* Copyright (C) 2004-2009 MBSim Development Team
 *
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
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include "integrator.h"
#include "mbsim/element.h"

namespace MBSim {

  DynamicSystemSolver * Integrator::system = 0;

  Integrator::Integrator() : tStart(0.), tEnd(1.), dtPlot(1e-4), warnLevel(0), output(true), name("Integrator") {}

  void Integrator::initializeUsingXML(TiXmlElement *element) {
    TiXmlElement *e;
    e=element->FirstChildElement(MBSIMINTNS"startTime");
    setStartTime(Element::getDouble(e));
    e=element->FirstChildElement(MBSIMINTNS"endTime");
    setEndTime(Element::getDouble(e));
    e=element->FirstChildElement(MBSIMINTNS"plotStepSize");
    setPlotStepSize(Element::getDouble(e));
    e=element->FirstChildElement(MBSIMINTNS"initialState");
    if(e) setInitialState(Element::getVec(e));
  }

  TiXmlElement* Integrator::writeXMLFile(TiXmlNode *parent) {
    TiXmlElement *ele0=new TiXmlElement(getType());
    parent->LinkEndChild(ele0);
    ele0->SetAttribute("xmlns", "http://mbsim.berlios.de/MBSimIntegrator");
//    TiXmlElement *ele1 = new TiXmlElement( "initialGeneralizedPosition" );
//    TiXmlText *text = new TiXmlText(vec2str(q0));
//    ele1->LinkEndChild(text);
//    ele0->LinkEndChild(ele1);
    return ele0;
  }
}

