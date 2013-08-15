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
#include <cstdlib>
#include "integrator.h"
#include "mbsim/element.h"
#include "mbsim/utils/utils.h"
#include "mbsim/objectfactory.h"
#include <mbsim/xmlnamespacemapping.h>
#include "mbxmlutilstinyxml/tinynamespace.h"

using namespace std;
using namespace MBXMLUtils;

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
    TiXmlElement *ele0=new TiXmlElement(MBSIMINTNS+getType());
    parent->LinkEndChild(ele0);

    addElementText(ele0,MBSIMINTNS"startTime",getStartTime());
    addElementText(ele0,MBSIMINTNS"endTime",getEndTime());
    addElementText(ele0,MBSIMINTNS"plotStepSize",getPlotStepSize());
    if(getInitialState().size())
      addElementText(ele0,MBSIMINTNS"initialState",getInitialState());

    return ele0;
  }

  Integrator* Integrator::readXMLFile(const string &filename) {
    TiXmlDocument doc;
    bool ret=doc.LoadFile(filename);
    assert(ret==true);
    (void) ret;
    TiXml_PostLoadFile(&doc);
    TiXmlElement *e=doc.FirstChildElement();
    TiXml_setLineNrFromProcessingInstruction(e);
    map<string,string> dummy;
    incorporateNamespace(e, dummy);
    Integrator *integrator=ObjectFactory<Integrator>::create<Integrator>(e);
    integrator->initializeUsingXML(doc.FirstChildElement());
    return integrator;
  }

  void Integrator::writeXMLFile(const string &name) {
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0","UTF-8","");
    doc.LinkEndChild( decl );
    writeXMLFile(&doc);
    map<string, string> nsprefix=XMLNamespaceMapping::getNamespacePrefixMapping();
    unIncorporateNamespace(doc.FirstChildElement(), nsprefix);  
    doc.SaveFile((name.length()>13 && name.substr(name.length()-13,13)==".mbsimint.xml")?name:name+".mbsimint.xml");
  }

  // This function is called first by each implementation of Integrator::integrate.
  // We modify here some integrator date for debugging (valgrind) purposes.
  void Integrator::debugInit() {
    // set a minimal end time: integrate only up to the first plot time (+10%) after the plot at tStart
    if(getenv("MBSIM_SET_MINIMAL_TEND")!=NULL)
      setEndTime(getStartTime()+1.1*getPlotStepSize());
  }

}
