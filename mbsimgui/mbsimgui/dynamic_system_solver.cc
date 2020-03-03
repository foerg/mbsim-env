/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

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
#include "dynamic_system_solver.h"
#include "project.h"
#include "objectfactory.h"
#include <xercesc/dom/DOMDocument.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  extern bool currentTask;

  Environment *Environment::instance=nullptr;

  void DynamicSystemSolver::removeXMLElements() {
    DOMNode *e = element->getFirstChild();
    while(e) {
      DOMNode *en=e->getNextSibling();
      if(e == environments) {
        DOMElement *env = E(static_cast<DOMElement*>(e))->getFirstElementChildNamed(MBSIM%"MBSimEnvironment");
        DOMElement *grav = E(env)->getFirstElementChildNamed(MBSIM%"accelerationOfGravity");
        DOMNode *ee = env->getFirstChild();
        while(ee) {
          DOMNode *een=ee->getNextSibling();
          if(ee == grav)
            env->removeChild(ee);
          ee = een;
        }
      }
      else if((e != frames) and (e != contours) and (e != groups) and (e != objects) and (e != links) and (e != constraints) and (e != observers) and (E(e)->getTagName() != MBSIM%"enableOpenMBVFrameI") and (E(e)->getTagName() != MBSIM%"plotFeatureFrameI"))
        element->removeChild(e);
      e = en;
    }
  }

  DOMElement* DynamicSystemSolver::createXMLElement(DOMNode *parent) {
    DOMElement *ele0 = Group::createXMLElement(parent);
    DOMDocument *doc=ele0->getOwnerDocument();

    E(ele0)->setAttribute("name", "MBS");
    environments = D(doc)->createElement( MBSIM%"environments" );
    DOMElement *ele2 = D(doc)->createElement( MBSIM%"MBSimEnvironment" );
    DOMElement *ele3 = D(doc)->createElement( MBSIM%"accelerationOfGravity" );
    E(ele3)->setAttribute("unit", "m/s^2");
    DOMElement *ele4 = D(doc)->createElement( PV%"xmlVector" );
    vector<string> g(3);
    g[0] = "0";
    g[1] = "-9.81";
    g[2] = "0";
    for(int i=0; i<3; i++) {
      DOMElement *ele5 = D(doc)->createElement( PV%"ele" );
      DOMText *text = doc->createTextNode(X()%g[i]);
      ele5->insertBefore(text, nullptr);
      ele4->insertBefore(ele5, nullptr);
    }
    ele3->insertBefore( ele4, nullptr );
    ele2->insertBefore( ele3, nullptr );
    environments->insertBefore( ele2, nullptr );
    ele0->insertBefore( environments, nullptr );
    return ele0;
  }

  void DynamicSystemSolver::createAndInit() {
    Group::createAndInit();
    environments = E(element)->getFirstElementChildNamed(MBSIM%"environments");
  }

  DOMElement* DynamicSystemSolver::processIDAndHref(DOMElement *element) {
    element = Group::processIDAndHref(element);
    if(currentTask==1) {
      E(element)->setAttribute("name","MBS_tmp");
      DOMElement *ele1 = D(element->getOwnerDocument())->createElement( MBSIM%"plotFeatureRecursive" );
      E(ele1)->setAttribute("value","plotRecursive");
      ele1->insertBefore(element->getOwnerDocument()->createTextNode(X()%project->getVarFalse().toStdString()), nullptr);
      element->insertBefore( ele1, element->getFirstElementChild() );
    }
    return element;
  }

  EmbedItemData* DynamicSystemSolver::getEmbedItemParent() {
    return project;
  }

}
