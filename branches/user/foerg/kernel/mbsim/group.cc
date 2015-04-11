/* Copyright (C) 2004-2014 MBSim Development Team
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
#include "mbsim/group.h"
#include "mbsim/object.h"
#include "mbsim/link.h"
#include "mbsim/constraint.h"
#include "mbsim/frame.h"
#include "mbsim/contour.h"
#include "mbsim/dynamic_system_solver.h"
#include "hdf5serie/simpleattribute.h"
#include "mbsim/objectfactory.h"
#include "mbsim/observer.h"
#include "mbsim/utils/utils.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/frame.h>
#endif

//#ifdef _OPENMP
//#include <omp.h>
//#endif

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Group, MBSIM%"Group")

  Group::Group(const string &name) : DynamicSystem(name) {}

  Group::~Group() {}

  void Group::facLLM(int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i)
      (*i)->facLLM(j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->facLLM(j);
  }

  void Group::updateStateDependentVariables(double t) {

    for(unsigned int i=0; i<elementOrdered.size(); i++) 
      for(unsigned int j=0; j<elementOrdered[i].size(); j++) 
	elementOrdered[i][j]->updateStateDependentVariables(t);
  }

  void Group::updateJacobians(double t, int k) {

    for(unsigned int i=0; i<elementOrdered.size(); i++) 
      for(unsigned int j=0; j<elementOrdered[i].size(); j++) 
        elementOrdered[i][j]->updateJacobians(t,k);

    for(unsigned int i=0; i<link.size(); i++)
      link[i]->updateJacobians(t,k);
  }

  void Group::updatedu(double t, double dt) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatedu(t,dt);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i)
      (*i)->updatedu(t,dt);
  }

  void Group::updateud(double t, int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateud(t,j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i)
      (*i)->updateud(t,j);
  }

  void Group::updatezd(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatezd(t);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->updatezd(t);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i)
      (*i)->updatexd(t);

    for(vector<Constraint*>::iterator i = constraint.begin(); i != constraint.end(); ++i)
      (*i)->updatexd(t);
  }

  void Group::initializeUsingXML(DOMElement *element) {
    DOMElement *e;
    Element::initializeUsingXML(element);
    e=element->getFirstElementChild();

    // search first element known by Group
    while(e && E(e)->getTagName()!=MBSIM%"frameOfReference" &&
        E(e)->getTagName()!=MBSIM%"position" &&
        E(e)->getTagName()!=MBSIM%"orientation" &&
        E(e)->getTagName()!=MBSIM%"frames")
      e=e->getNextElementSibling();

    if(e && E(e)->getTagName()==MBSIM%"frameOfReference") {
      saved_frameOfReference=E(e)->getAttribute("ref");
      e=e->getNextElementSibling();
    }

    if(e && E(e)->getTagName()==MBSIM%"position") {
      setPosition(getVec3(e));
      e=e->getNextElementSibling();
    }

    if(e && E(e)->getTagName()==MBSIM%"orientation") {
      setOrientation(getSqrMat3(e));
      e=e->getNextElementSibling();
    }

    // frames
    DOMElement *E=e->getFirstElementChild();
    while(E) {
      FixedRelativeFrame *f=new FixedRelativeFrame(MBXMLUtils::E(E)->getAttribute("name"));
      addFrame(f);
      f->initializeUsingXML(E);
      E=E->getNextElementSibling();
    }
    e=e->getNextElementSibling();

    // contours
    E=e->getFirstElementChild();
    while(E) {
      Contour *c=ObjectFactory::createAndInit<Contour>(E);
      addContour(c);
      E=E->getNextElementSibling();
    }
    e=e->getNextElementSibling();

    // groups
    E=e->getFirstElementChild();
    Group *g;
    while(E) {
      g=ObjectFactory::createAndInit<Group>(E);
      addGroup(g);
      E=E->getNextElementSibling();
    }
    e=e->getNextElementSibling();

    // objects
    E=e->getFirstElementChild();
    Object *o;
    while(E) {
      o=ObjectFactory::createAndInit<Object>(E);
      addObject(o);
      E=E->getNextElementSibling();
    }
    e=e->getNextElementSibling();

    // links
    E=e->getFirstElementChild();
    Link *l;
    while(E) {
      l=ObjectFactory::createAndInit<Link>(E);
      addLink(l);
      E=E->getNextElementSibling();
    }
    e=e->getNextElementSibling();

    // constraints
    if (e && MBXMLUtils::E(e)->getTagName()==MBSIM%"constraints") {
      E=e->getFirstElementChild();
      Constraint *crt;
      while(E) {
        crt=ObjectFactory::createAndInit<Constraint>(E);
        addConstraint(crt);
        E=E->getNextElementSibling();
      }
      e=e->getNextElementSibling();
    }

    // observers
    if (e && MBXMLUtils::E(e)->getTagName()==MBSIM%"observers") {
      E=e->getFirstElementChild();
      Observer *obsrv;
      while(E) {
        obsrv=ObjectFactory::createAndInit<Observer>(E);
        addObserver(obsrv);
        E=E->getNextElementSibling();
      }
    }
#ifdef HAVE_OPENMBVCPPINTERFACE

    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVFrameI");
    if(e) {
      OpenMBVFrame ombv;
      I->setOpenMBVFrame(ombv.createOpenMBV(e));
    }
#endif
  }

  DOMElement* Group::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = DynamicSystem::writeXMLFile(parent);

//    DOMElement *ele1;
//
//    if(getFrameOfReference()) {
//      ele1 = new DOMElement( MBSIM%"frameOfReference" );
//      ele1->SetAttribute("ref", R->getXMLPath(this,true));
//      ele0->LinkEndChild(ele1);
//    }
//
//    addElementText(ele0,MBSIM%"position",getPosition());
//    addElementText(ele0,MBSIM%"orientation", getOrientation());
//
//    ele1 = new DOMElement( MBSIM%"frames" );
//    for(vector<Frame*>::iterator i = frame.begin()+1; i != frame.end(); ++i) 
//      (*i)->writeXMLFile(ele1);
//    ele0->LinkEndChild( ele1 );
//
//    ele1 = new DOMElement( MBSIM%"contours" );
//    for(vector<Contour*>::iterator i = contour.begin(); i != contour.end(); ++i) 
//      (*i)->writeXMLFile(ele1);
//    ele0->LinkEndChild( ele1 );
//
//    ele1 = new DOMElement( MBSIM%"groups" );
//    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
//      (*i)->writeXMLFile(ele1);
//    ele0->LinkEndChild( ele1 );
//
//    ele1 = new DOMElement( MBSIM%"objects" );
//    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
//      (*i)->writeXMLFile(ele1);
//    ele0->LinkEndChild( ele1 );
//
//    ele1 = new DOMElement( MBSIM%"links" );
//    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i) 
//      (*i)->writeXMLFile(ele1);
//    ele0->LinkEndChild( ele1 );
//
//    if(I->getOpenMBVFrame()) {
//      ele1 = new DOMElement( MBSIM%"enableOpenMBVFrameI" );
//      addElementText(ele1,MBSIM%"size",I->getOpenMBVFrame()->getSize());
//      addElementText(ele1,MBSIM%"offset",I->getOpenMBVFrame()->getOffset());
//      ele0->LinkEndChild(ele1);
//    }
//
    return ele0;
  }

}

