/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

  This library is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version. 
   
  This library is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details. 
   
  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include <config.h>
#include "embed.h"
#include "dynamic_system_solver.h"
#include "parameter.h"
#include "project.h"
#include "objectfactory.h"

namespace MBSimGUI {

  template <>
    DynamicSystemSolver* Embed<DynamicSystemSolver>::create(xercesc::DOMElement *element) {
      return static_cast<DynamicSystemSolver*>(ObjectFactory::getInstance()->createGroup(element));
    }

  template <>
    Group* Embed<Group>::create(xercesc::DOMElement *element) {
      return static_cast<Group*>(ObjectFactory::getInstance()->createGroup(element));
    }

  template <>
    Contour* Embed<Contour>::create(xercesc::DOMElement *element) {
      return static_cast<Contour*>(ObjectFactory::getInstance()->createContour(element));
    }

  template <>
    Frame* Embed<Frame>::create(xercesc::DOMElement *element) {
      return static_cast<Frame*>(ObjectFactory::getInstance()->createFrame(element));
    }

  template <>
    Object* Embed<Object>::create(xercesc::DOMElement *element) {
      return static_cast<Object*>(ObjectFactory::getInstance()->createObject(element));
    }

  template <>
    Link* Embed<Link>::create(xercesc::DOMElement *element) {
      return static_cast<Link*>(ObjectFactory::getInstance()->createLink(element));
    }

  template <>
    Constraint* Embed<Constraint>::create(xercesc::DOMElement *element) {
      return static_cast<Constraint*>(ObjectFactory::getInstance()->createConstraint(element));
    }

  template <>
    Observer* Embed<Observer>::create(xercesc::DOMElement *element) {
      return static_cast<Observer*>(ObjectFactory::getInstance()->createObserver(element));
    }

  template <>
    Solver* Embed<Solver>::create(xercesc::DOMElement *element) {
      return static_cast<Solver*>(ObjectFactory::getInstance()->createSolver(element));
    }

  template <>
    Project* Embed<Project>::create(xercesc::DOMElement *element) {
      return new Project;
    }

}
