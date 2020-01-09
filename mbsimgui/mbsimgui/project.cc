/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2017 Martin Förg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <config.h>
#include "project.h"
#include "dynamic_system_solver.h"
#include "solver.h"
#include "integrator.h"
#include "embed.h"
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  extern DOMImplementation *impl;
  extern DOMLSSerializer *serializer;

  Project::~Project() {
    delete dss;
    delete solver;
  }

  void Project::removeXMLElements() {
    DOMElement *ele = element->getFirstElementChild();
    if(E(ele)->getTagName()==PV%"evaluator")
      element->removeChild(ele);
  }

  DOMElement* Project::createXMLElement(DOMNode *parent) {
    auto *doc=static_cast<xercesc::DOMDocument*>(parent);
    element=D(doc)->createElement(getNameSpace()%getType().toStdString());
    E(element)->setAttribute("name","Project");
    parent->insertBefore(element, nullptr);
    setDynamicSystemSolver(new DynamicSystemSolver);
    dss->createXMLElement(element);
    setSolver(new DOPRI5Integrator);
    solver->createXMLElement(element);
    return element;
  }

  void Project::initializeUsingXML(DOMElement *element) {
    this->element = element;
    DOMElement *ele = element->getFirstElementChild();
    if(E(ele)->getTagName()==PV%"evaluator")
      ele = ele->getNextElementSibling();
    setDynamicSystemSolver(Embed<DynamicSystemSolver>::createAndInit(ele,this));
    ele = ele->getNextElementSibling();
    setSolver(Embed<Solver>::createAndInit(ele,this));
  }

  DOMElement* Project::processIDAndHref(DOMElement *element) {
    element = EmbedItemData::processIDAndHref(element);
    DOMElement* ele0 = element->getFirstElementChild();
    if(E(ele0)->getTagName()==PV%"evaluator")
      ele0 = ele0->getNextElementSibling();
    dss->processIDAndHref(ele0);
    ele0 = ele0->getNextElementSibling();
    solver->processIDAndHref(ele0);
    return element;
  }

  DOMElement* Project::processHref(DOMElement *element) {
    element = EmbedItemData::processHref(element);
    DOMElement* ele0 = element->getFirstElementChild();
    if(E(ele0)->getTagName()==PV%"evaluator")
      ele0 = ele0->getNextElementSibling();
    dss->processHref(ele0);
    ele0 = ele0->getNextElementSibling();
    solver->processHref(ele0);
    return element;
  }

  void Project::setDynamicSystemSolver(DynamicSystemSolver *dss_) {
    delete dss;
    dss = dss_;
    dss->setProject(this);
  }

  void Project::setSolver(Solver *solver_) {
    delete solver;
    solver = solver_;
    solver->setProject(this);
  }

  DOMElement* Project::createEmbedXMLElement() {
    if(not getEmbedXMLElement()) {
      xercesc::DOMDocument *doc=element->getOwnerDocument();
      setEmbedXMLElement(D(doc)->createElement(PV%"Embed"));
      doc->removeChild(getXMLElement());
      doc->insertBefore(getEmbedXMLElement(),nullptr);
      getEmbedXMLElement()->insertBefore(getXMLElement(),nullptr);
    }
    return getEmbedXMLElement();
  }

  void Project::setEmbeded(bool embeded) {
    EmbedItemData::setEmbeded(embeded);
    dss->setEmbeded(embeded);
    solver->setEmbeded(embeded);
  }

  void Project::maybeRemoveEmbedXMLElement() {
    if(embed and not getNumberOfParameters()) {
      DOMElement *param = E(embed)->getFirstElementChildNamed(PV%"Parameter");
      if(param) {
        DOMNode *ps = param->getPreviousSibling();
        if(ps and X()%ps->getNodeName()=="#text")
          embed->removeChild(ps);
        embed->removeChild(param);
      }
      if(not E(embed)->hasAttribute("count") and not E(embed)->hasAttribute("counterName") and not E(embed)->hasAttribute("href") and not E(embed)->hasAttribute("parameterHref")) {
        xercesc::DOMDocument *doc=element->getOwnerDocument();
        doc->removeChild(embed);
        doc->insertBefore(element,nullptr);
        embed = nullptr;
      }
    }
  }
}
