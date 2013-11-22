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
#include "objectfactory.h"
#include "frame.h"
#include "contour.h"
#include "solver.h"
#include "group.h"
#include "rigidbody.h"
#include "constraint.h"
#include "linear_transfer_system.h"
#include "kinetic_excitation.h"
#include "joint.h"
#include "spring_damper.h"
#include "contact.h"
#include "actuator.h"
#include "sensor.h"
#include "widget.h"
#include "parameter.h"
#include "observer.h"
#include "integrator.h"
#include "ombv_properties.h"
#include "kinematic_functions_properties.h"

using namespace std;
using namespace MBXMLUtils;

ObjectFactory* ObjectFactory::instance=NULL;

Environment *ObjectFactory::getEnvironment(TiXmlElement *element) {
  if(element==NULL) return NULL;
  Environment *obj;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    if((obj=(*i)->getEnvironment(element))) return obj;
  cout << string("No Environment of type ")+element->ValueStr()+" exists.";
  return 0;
}

Environment *MBSimObjectFactory::getEnvironment(TiXmlElement *element) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMNS"MBSimEnvironment")
    return Environment::getInstance();
  return 0;
}

MBSimObjectFactory *MBSimObjectFactory::instance=NULL;

void MBSimObjectFactory::initialize() {
  if(instance==0) {
    instance=new MBSimObjectFactory;
    ObjectFactory::getInstance()->registerObjectFactory(instance);
  }
}

Frame* ObjectFactory::createFrame(TiXmlElement *element, Element *parent) {
  if(element==NULL) return NULL;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (*i)->createFrame(element,parent);
  return 0;
}
Frame* MBSimObjectFactory::createFrame(TiXmlElement *element, Element *parent) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMNS"FixedRelativeFrame")
    return new FixedRelativeFrame(element->Attribute("name"),parent);
  return 0;
}

Contour* ObjectFactory::createContour(TiXmlElement *element, Element *parent) {
  if(element==NULL) return NULL;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (*i)->createContour(element,parent);
  return 0;
}
Contour* MBSimObjectFactory::createContour(TiXmlElement *element, Element *parent) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMNS"Point")
    return new Point(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"Line")
    return new Line(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"Plane")
    return new Plane(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"Sphere")
    return new Sphere(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"CircleSolid")
    return new CircleSolid(element->Attribute("name"),parent);
  return 0;
}

Group* ObjectFactory::createGroup(TiXmlElement *element, Element *parent) {
  if(element==NULL) return NULL;
  //Group *obj;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (((*i)->createGroup(element,parent)));
  return 0;
}
Group* MBSimObjectFactory::createGroup(TiXmlElement *element, Element *parent) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMNS"DynamicSystemSolver")
    return new Solver(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"Group")
    return new Group(element->Attribute("name"),parent);
  return 0;
}

Object* ObjectFactory::createObject(TiXmlElement *element, Element *parent) {
  if(element==NULL) return NULL;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (*i)->createObject(element,parent);
  return 0;
}
Object* MBSimObjectFactory::createObject(TiXmlElement *element, Element *parent) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMNS"RigidBody")
    return new RigidBody(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"GearConstraint")
    return new GearConstraint(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"GeneralizedPositionConstraint")
    return new GeneralizedPositionConstraint(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"GeneralizedVelocityConstraint")
    return new GeneralizedVelocityConstraint(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"GeneralizedAccelerationConstraint")
    return new GeneralizedAccelerationConstraint(element->Attribute("name"),parent);
  else if(element->ValueStr()==MBSIMNS"JointConstraint")
    return new JointConstraint(element->Attribute("name"),parent);
  return 0;
}

Link* ObjectFactory::createLink(TiXmlElement *element, Element *parent) {
  if(element==NULL) return NULL;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (*i)->createLink(element,parent);
  return 0;
}
Link* MBSimObjectFactory::createLink(TiXmlElement *element, Element *parent) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMNS"KineticExcitation")
    return new KineticExcitation(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"SpringDamper")
    return new SpringDamper(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"DirectionalSpringDamper")
    return new DirectionalSpringDamper(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"GeneralizedSpringDamper")
    return new GeneralizedSpringDamper(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"Joint")
    return new Joint(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"Contact")
    return new Contact(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"Actuator")
    return new Actuator(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"GeneralizedPositionSensor")
    return new GeneralizedPositionSensor(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"GeneralizedVelocitySensor")
    return new GeneralizedVelocitySensor(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"AbsolutePositionSensor")
    return new AbsolutePositionSensor(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"AbsoluteVelocitySensor")
    return new AbsoluteVelocitySensor(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"FunctionSensor")
    return new FunctionSensor(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"SignalProcessingSystemSensor")
    return new SignalProcessingSystemSensor(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"SignalAddition")
    return new SignalAddition(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"PIDController")
    return new PIDController(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"UnarySignalOperation")
    return new UnarySignalOperation(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"BinarySignalOperation")
    return new BinarySignalOperation(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMCONTROLNS"LinearTransferSystem")
    return new LinearTransferSystem(element->Attribute("name"),parent);
  //if(element->ValueStr()==MBSIMNS"ExternGeneralizedIO")
  //  return new ExternGeneralizedIO(element->Attribute("name"));
  return 0;
}  

Observer* ObjectFactory::createObserver(TiXmlElement *element, Element *parent) {
  if(element==NULL) return NULL;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (*i)->createObserver(element,parent);
  return 0;
}
Observer* MBSimObjectFactory::createObserver(TiXmlElement *element, Element *parent) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMNS"CartesianCoordinatesObserver")
    return new CartesianCoordinatesObserver(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"CylinderCoordinatesObserver")
    return new CylinderCoordinatesObserver(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"NaturalCoordinatesObserver")
    return new NaturalCoordinatesObserver(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"AbsoluteKinematicsObserver")
    return new AbsoluteKinematicsObserver(element->Attribute("name"),parent);
  if(element->ValueStr()==MBSIMNS"RelativeKinematicsObserver")
    return new RelativeKinematicsObserver(element->Attribute("name"),parent);
  return 0;
}  

Integrator* ObjectFactory::createIntegrator(TiXmlElement *element) {
  if(element==NULL) return NULL;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (*i)->createIntegrator(element);
  return 0;
}

Integrator* MBSimObjectFactory::createIntegrator(TiXmlElement *element) {
  if(element==0) return 0;
  if(element->ValueStr()==MBSIMINTNS"DOPRI5Integrator")
    return new DOPRI5Integrator;
  else if(element->ValueStr()==MBSIMINTNS"RADAU5Integrator")
    return new RADAU5Integrator;
  else if(element->ValueStr()==MBSIMINTNS"LSODEIntegrator")
    return new LSODEIntegrator;
  else if(element->ValueStr()==MBSIMINTNS"LSODARIntegrator")
    return new LSODARIntegrator;
  else if(element->ValueStr()==MBSIMINTNS"TimeSteppingIntegrator")
    return new TimeSteppingIntegrator;
  else if(element->ValueStr()==MBSIMINTNS"EulerExplicitIntegrator")
    return new EulerExplicitIntegrator;
  else if(element->ValueStr()==MBSIMINTNS"RKSuiteIntegrator")
    return new RKSuiteIntegrator;
  return 0;
}

Parameter* ObjectFactory::createParameter(TiXmlElement *element) {
  if(element==NULL) return NULL;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    return (*i)->createParameter(element);
  return 0;
}

Parameter* MBSimObjectFactory::createParameter(TiXmlElement *element) {
  if(element==0) return 0;
  if(element->ValueStr()==PARAMNS"stringParameter")
    return new StringParameter(element->Attribute("name"));
  else if(element->ValueStr()==PARAMNS"scalarParameter")
    return new ScalarParameter(element->Attribute("name"));
  else if(element->ValueStr()==PARAMNS"vectorParameter")
    return new VectorParameter(element->Attribute("name"));
  else if(element->ValueStr()==PARAMNS"matrixParameter")
    return new MatrixParameter(element->Attribute("name"));
  return 0;
}

ObjectFactoryBase::M_NSPRE ObjectFactory::getNamespacePrefixMapping() {
  // collect all priority-namespace-prefix mappings
  MM_PRINSPRE priorityNSPrefix;
  for(set<ObjectFactoryBase*>::iterator i=factories.begin(); i!=factories.end(); i++)
    priorityNSPrefix.insert((*i)->getPriorityNamespacePrefix().begin(), (*i)->getPriorityNamespacePrefix().end());
#ifdef HAVE_OPENMBVCPPINTERFACE
  // add the openmbv mapping
  priorityNSPrefix.insert(OpenMBV::ObjectFactory::getPriorityNamespacePrefix().begin(),
      OpenMBV::ObjectFactory::getPriorityNamespacePrefix().end());
#endif

  // generate the namespace-prefix mapping considering the priority
  M_NSPRE nsprefix;
  set<string> prefix;
  for(MM_PRINSPRE::reverse_iterator i=priorityNSPrefix.rbegin(); i!=priorityNSPrefix.rend(); i++) {
    // insert only if the prefix does not already exist
    if(prefix.find(i->second.second)!=prefix.end())
      continue;
    // insert only if the namespace does not already exist
    pair<M_NSPRE::iterator, bool> ret=nsprefix.insert(i->second);
    if(ret.second)
      prefix.insert(i->second.second);
  }

  return nsprefix;
}

OMBVBodyFactory::OMBVBodyFactory() {
  names.push_back("Cube");
  names.push_back("Cuboid");
  names.push_back("Sphere");
}

OMBVBodyProperty* OMBVBodyFactory::createBody(const std::string &name, const std::string &ID) {
  for(int i=0; i<names.size(); i++)
    if(name==names[i])
      return createBody(i,ID);
}

OMBVBodyProperty* OMBVBodyFactory::createBody(TiXmlElement *element, const std::string &ID) {
  for(int i=0; i<names.size(); i++)
    if(element->ValueStr()==(OPENMBVNS+names[i]))
      return createBody(i,ID);
}

OMBVBodyProperty* OMBVBodyFactory::createBody(int i, const std::string &ID) {
  if(i==0)
    return new CubeProperty(names[i],ID);
  else if(i==1)
    return new CuboidProperty(names[i],ID);
  else if(i==2)
    return new SphereProperty(names[i],ID);
  else
    return 0;
}

FunctionFactory::FunctionFactory() {
  names.push_back("TranslationAlongXAxis");
  names.push_back("TranslationAlongYAxis");
  names.push_back("TranslationAlongZAxis");
}

FunctionProperty* FunctionFactory::createFunction(const std::string &name) {
  for(int i=0; i<names.size(); i++)
    if(name==names[i])
      return createFunction(i);
}

FunctionProperty* FunctionFactory::createFunction(TiXmlElement *element) {
  for(int i=0; i<names.size(); i++)
    if(element->ValueStr()==(MBSIMNS+names[i]))
      return createFunction(i);
}

FunctionProperty* FunctionFactory::createFunction(int i) {
  if(i==0)
    return new TranslationAlongXAxisProperty(names[i]);
  else if(i==1)
    return new TranslationAlongYAxisProperty(names[i]);
  else if(i==2)
    return new TranslationAlongZAxisProperty(names[i]);
  else
    return 0;
}

VariableFactory::VariableFactory() {
  names.push_back("xmlMatrix");
  names.push_back("cardan");
  names.push_back("plain");
}

VariableProperty* VariableFactory::createVariable(const std::string &name) {
  for(int i=0; i<names.size(); i++)
    if(name==names[i])
      return createVariable(i);
}

VariableProperty* VariableFactory::createVariable(TiXmlElement *element) {
  for(int i=0; i<names.size(); i++)
    if(element->ValueStr()==(PVNS+names[i]))
      return createVariable(i);
}

VariableProperty* VariableFactory::createVariable(int i) {
  if(i==0)
    return new MatProperty(3,3);
  else if(i==1)
    return new CardanProperty();
  else
    return new OctaveExpressionProperty(names[i]);
}

