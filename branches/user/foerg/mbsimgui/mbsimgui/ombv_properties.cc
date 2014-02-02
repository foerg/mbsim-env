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
#include "ombv_properties.h"
#include "variable_properties.h"
#include "extended_properties.h"
#include "ombv_widgets.h"
#include "rigidbody.h"
#include "frame.h"
#include "mainwindow.h"
#include <xercesc/dom/DOMProcessingInstruction.hpp>

extern MainWindow *mw;

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

OMBVBodyPropertyFactory::OMBVBodyPropertyFactory(const string &ID_) : ID(ID_), count(0) {
  name.push_back(OPENMBV%"Cube");
  name.push_back(OPENMBV%"Cuboid");
  name.push_back(OPENMBV%"Frustum");
  name.push_back(OPENMBV%"Sphere");
  name.push_back(OPENMBV%"IvBody");
  name.push_back(OPENMBV%"CompoundRigidBody");
  name.push_back(OPENMBV%"InvisibleBody");
}

Property* OMBVBodyPropertyFactory::createProperty(int i) {
  if(i==0)
    return new CubeProperty("Body"+toStr(count++),ID);
  if(i==1)
    return new CuboidProperty("Body"+toStr(count++),ID);
  if(i==2)
    return new FrustumProperty("Body"+toStr(count++),ID);
  if(i==3)
    return new SphereProperty("Body"+toStr(count++),ID);
  if(i==4)
    return new IvBodyProperty("Body"+toStr(count++),ID);
  if(i==5)
    return new CompoundRigidBodyProperty("Body"+toStr(count++),ID);
  if(i==6)
    return new InvisibleBodyProperty("Body"+toStr(count++),ID);
}

void OMBVObjectProperty::writeXMLFileID(DOMNode *parent) {
  if(!ID.empty()) {
    DOMDocument *doc=parent->getOwnerDocument();
    DOMProcessingInstruction *id=doc->createProcessingInstruction(X()%"OPENMBV_ID", X()%ID);
    parent->insertBefore(id, parent->getFirstChild());
  }
}

OMBVFrameProperty::OMBVFrameProperty(const string &name, const FQN &xmlName_, const std::string &ID) : OMBVObjectProperty(name,ID), xmlName(xmlName_) {

  property.push_back(new Scalar_Property("size",LengthUnits()));
  property.push_back(new Scalar_Property("offset",NoUnitUnits()));
}

DOMElement* OMBVFrameProperty::initializeUsingXML(DOMElement *element) {
  DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
  if(e) {
    DOMElement *ee = E(e)->getFirstElementChildNamed( MBSIM%"size" );
    property[0]->initializeUsingXML(ee);
    ee = E(e)->getFirstElementChildNamed( MBSIM%"offset" );
    property[1]->initializeUsingXML(ee);
  }
  return e;
}

DOMElement* OMBVFrameProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  if(xmlName!=FQN()) {
    DOMElement *e=D(doc)->createElement(xmlName);
    writeXMLFileID(e);
    parent->insertBefore(e, NULL);
    for(int i=0; i<property.size(); i++)
      property[i]->writeXMLFile(e);
    return e;
  }
  else {
    writeXMLFileID(parent);
    DOMElement *e=D(doc)->createElement(MBSIM%"size");
    property[0]->writeXMLFile(e);
    parent->insertBefore(e, NULL);
    e=D(doc)->createElement(MBSIM%"offset");
    property[1]->writeXMLFile(e);
    parent->insertBefore(e, NULL);
    return 0;
  }
}

OMBVDynamicColoredObjectProperty::OMBVDynamicColoredObjectProperty(const string &name, const std::string &ID, bool readXMLType_) : OMBVObjectProperty(name,ID), readXMLType(readXMLType_) {

  property.push_back(new ChoiceProperty2("minimal color value",new ScalarPropertyFactory("0",NoUnitUnits()),"",4));
  property[0]->setDisabling(true);
  property[0]->setDisabled(true);
  property.push_back(new ChoiceProperty2("maximal color value",new ScalarPropertyFactory("1",NoUnitUnits()),"",4));
  property[1]->setDisabling(true);
  property[1]->setDisabled(true);
//
//  diffuseColor.setProperty(new ColorProperty(OPENMBV%"diffuseColor"));
//
//  input.clear();
//  input.push_back(PhysicalVariableProperty(new ScalarProperty("0.3"), "-", OPENMBV%"transparency"));
//  transparency.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* OMBVDynamicColoredObjectProperty::initializeUsingXML(DOMElement *element) {
  DOMElement *e=readXMLType?E(element)->getFirstElementChildNamed(OPENMBV%getType()):element;
//  if(e) {
//    minimalColorValue.initializeUsingXML(e);
//    maximalColorValue.initializeUsingXML(e);
//    diffuseColor.initializeUsingXML(e);
//    transparency.initializeUsingXML(e);
//  }
  return e;
}

DOMElement* OMBVDynamicColoredObjectProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *e=D(doc)->createElement(OPENMBV%getType());
  parent->insertBefore(e, NULL);
  E(e)->setAttribute("name", name);
  writeXMLFileID(e);
  if(not(property[0]->isDisabled())) {
    DOMElement *ee=D(doc)->createElement(OPENMBV%"minimalColorValue");
    property[0]->writeXMLFile(ee);
    e->insertBefore(ee, NULL);
  }
  if(not(property[1]->isDisabled())) {
    DOMElement *ee=D(doc)->createElement(OPENMBV%"maximalColorValue");
    property[1]->writeXMLFile(ee);
    e->insertBefore(ee, NULL);
  }
//  minimalColorValue.writeXMLFile(e);
//  maximalColorValue.writeXMLFile(e);
//  diffuseColor.writeXMLFile(e);
//  transparency.writeXMLFile(e);
  return e;
}

void OMBVDynamicColoredObjectProperty::fromWidget(QWidget *widget) {
//  minimalColorValue.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->minimalColorValue);
//  maximalColorValue.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->maximalColorValue);
//  diffuseColor.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->diffuseColor);
//  transparency.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->transparency);
}

void OMBVDynamicColoredObjectProperty::toWidget(QWidget *widget) {
//  minimalColorValue.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->minimalColorValue);
//  maximalColorValue.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->maximalColorValue);
//  diffuseColor.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->diffuseColor);
//  transparency.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->transparency);
}

OMBVArrowProperty::OMBVArrowProperty(const string &name, const std::string &ID, bool fromPoint) : OMBVDynamicColoredObjectProperty(name,ID), referencePoint(0,false) {
  readXMLType = true;

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0.1"), "m", OPENMBV%"diameter"));
  diameter.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0.2"), "m", OPENMBV%"headDiameter"));
  headDiameter.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0.2"), "m", OPENMBV%"headLength"));
  headLength.setProperty(new ExtPhysicalVarProperty(input));

  type.setProperty(new TextProperty("","toHead", OPENMBV%"type", true));

  referencePoint.setProperty(new TextProperty("",fromPoint?"fromPoint":"toPoint", OPENMBV%"referencePoint", true));
//  if(fromPoint)
//    referencePoint.setActive(true);

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", OPENMBV%"scaleLength"));
  scaleLength.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* OMBVArrowProperty::initializeUsingXML(DOMElement *element) {
  DOMElement *e = OMBVDynamicColoredObjectProperty::initializeUsingXML(element);
  if(e) {
    diameter.initializeUsingXML(e);
    headDiameter.initializeUsingXML(e);
    headLength.initializeUsingXML(e);
    type.initializeUsingXML(e);
    referencePoint.initializeUsingXML(e);
    scaleLength.initializeUsingXML(e);
  }
  return e;
}

DOMElement* OMBVArrowProperty::writeXMLFile(DOMNode *parent) {
  DOMElement *e=OMBVDynamicColoredObjectProperty::writeXMLFile(parent);
  diameter.writeXMLFile(e);
  headDiameter.writeXMLFile(e);
  headLength.writeXMLFile(e);
  type.writeXMLFile(e);
  referencePoint.writeXMLFile(e);
  scaleLength.writeXMLFile(e);
  return e;
}

void OMBVArrowProperty::fromWidget(QWidget *widget) {
  OMBVDynamicColoredObjectProperty::fromWidget(widget);
  diameter.fromWidget(static_cast<OMBVArrowWidget*>(widget)->diameter);
  headDiameter.fromWidget(static_cast<OMBVArrowWidget*>(widget)->headDiameter);
  headLength.fromWidget(static_cast<OMBVArrowWidget*>(widget)->headLength);
  type.fromWidget(static_cast<OMBVArrowWidget*>(widget)->type);
  referencePoint.fromWidget(static_cast<OMBVArrowWidget*>(widget)->referencePoint);
  scaleLength.fromWidget(static_cast<OMBVArrowWidget*>(widget)->scaleLength);
}

void OMBVArrowProperty::toWidget(QWidget *widget) {
  OMBVDynamicColoredObjectProperty::toWidget(widget);
  diameter.toWidget(static_cast<OMBVArrowWidget*>(widget)->diameter);
  headDiameter.toWidget(static_cast<OMBVArrowWidget*>(widget)->headDiameter);
  headLength.toWidget(static_cast<OMBVArrowWidget*>(widget)->headLength);
  type.toWidget(static_cast<OMBVArrowWidget*>(widget)->type);
  referencePoint.toWidget(static_cast<OMBVArrowWidget*>(widget)->referencePoint);
  scaleLength.toWidget(static_cast<OMBVArrowWidget*>(widget)->scaleLength);
}

OMBVCoilSpringProperty::OMBVCoilSpringProperty(const string &name, const std::string &ID) : OMBVObjectProperty(name,ID), crossSectionRadius(0,false), nominalLength(0,false) {

  type.setProperty(new TextProperty("","tube", OPENMBV%"type", true));

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("3"), "-", OPENMBV%"numberOfCoils"));
  numberOfCoils.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0.1"), "m", OPENMBV%"springRadius"));
  springRadius.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("-1"), "m", OPENMBV%"crossSectionRadius"));
  crossSectionRadius.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("-1"), "m", OPENMBV%"nominalLength"));
  nominalLength.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", OPENMBV%"scaleFactor"));
  scaleFactor.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* OMBVCoilSpringProperty::initializeUsingXML(DOMElement *element) {
  DOMElement *e=E(element)->getFirstElementChildNamed(OPENMBV%getType());
  if(e) {
    type.initializeUsingXML(e);
    numberOfCoils.initializeUsingXML(e);
    springRadius.initializeUsingXML(e);
    crossSectionRadius.initializeUsingXML(e);
    nominalLength.initializeUsingXML(e);
    scaleFactor.initializeUsingXML(e);
  }
  return e;
}

DOMElement* OMBVCoilSpringProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *e=D(doc)->createElement(OPENMBV%getType());
  parent->insertBefore(e, NULL);
  E(e)->setAttribute("name", "dummy");
  writeXMLFileID(e);
  type.writeXMLFile(e);
  numberOfCoils.writeXMLFile(e);
  springRadius.writeXMLFile(e);
  crossSectionRadius.writeXMLFile(e);
  nominalLength.writeXMLFile(e);
  scaleFactor.writeXMLFile(e);
  return e;
}

void OMBVCoilSpringProperty::fromWidget(QWidget *widget) {
  type.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->type);
  numberOfCoils.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->numberOfCoils);
  springRadius.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->springRadius);
  crossSectionRadius.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->crossSectionRadius);
  nominalLength.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->nominalLength);
  scaleFactor.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->scaleFactor);
}

void OMBVCoilSpringProperty::toWidget(QWidget *widget) {
  type.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->type);
  numberOfCoils.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->numberOfCoils);
  springRadius.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->springRadius);
  crossSectionRadius.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->crossSectionRadius);
  nominalLength.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->nominalLength);
  scaleFactor.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->scaleFactor);
}

OMBVBodyProperty::OMBVBodyProperty(const string &name, const std::string &ID) : OMBVDynamicColoredObjectProperty(name,ID) {

//  transparency.setActive(true);

  property.push_back(new ChoiceProperty2("initial translation",new VecPropertyFactory(3,LengthUnits()),"",4));
  property.push_back(new ChoiceProperty2("initial rotation",new VecPropertyFactory(3,AngleUnits()),"",4));
  property.push_back(new ChoiceProperty2("scale factor",new ScalarPropertyFactory("1",NoUnitUnits()),"",4));
//  vector<PhysicalVariableProperty> input;
//  input.push_back(PhysicalVariableProperty(new VecProperty(3), "m", OPENMBV%"initialTranslation"));
//  trans.setProperty(new ExtPhysicalVarProperty(input));
//
//  input.clear();
//  input.push_back(PhysicalVariableProperty(new VecProperty(3), "rad", OPENMBV%"initialRotation"));
//  rot.setProperty(new ExtPhysicalVarProperty(input));
//
//  input.clear();
//  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", OPENMBV%"scaleFactor"));
//  scale.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* OMBVBodyProperty::initializeUsingXML(DOMElement *element) {
  DOMElement *e = OMBVDynamicColoredObjectProperty::initializeUsingXML(element);
//  if(e) {
//    trans.initializeUsingXML(e);
//    rot.initializeUsingXML(e);
//    scale.initializeUsingXML(e);
//  }
  return e;
}

DOMElement* OMBVBodyProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *e=OMBVDynamicColoredObjectProperty::writeXMLFile(parent);
  DOMElement *ee=D(doc)->createElement(OPENMBV%"initialTranslation");
  property[2]->writeXMLFile(ee);
  e->insertBefore(ee, NULL);
  ee=D(doc)->createElement(OPENMBV%"initialRotation");
  property[3]->writeXMLFile(ee);
  e->insertBefore(ee, NULL);
  ee=D(doc)->createElement(OPENMBV%"scaleFactor");
  property[4]->writeXMLFile(ee);
  e->insertBefore(ee, NULL);
//  trans.writeXMLFile(e);
//  rot.writeXMLFile(e);
//  scale.writeXMLFile(e);
  return e;
}

void OMBVBodyProperty::fromWidget(QWidget *widget) {
//  OMBVDynamicColoredObjectProperty::fromWidget(widget);
//  trans.fromWidget(static_cast<OMBVBodyWidget*>(widget)->trans);
//  rot.fromWidget(static_cast<OMBVBodyWidget*>(widget)->rot);
//  scale.fromWidget(static_cast<OMBVBodyWidget*>(widget)->scale);
}

void OMBVBodyProperty::toWidget(QWidget *widget) {
//  OMBVDynamicColoredObjectProperty::toWidget(widget);
//  trans.toWidget(static_cast<OMBVBodyWidget*>(widget)->trans);
//  rot.toWidget(static_cast<OMBVBodyWidget*>(widget)->rot);
//  scale.toWidget(static_cast<OMBVBodyWidget*>(widget)->scale);
}

//OpenMBVRigidBodyChoiceProperty::OpenMBVRigidBodyChoiceProperty(const string &name, const string &ID_) : Property(name), property(0), index(-1), ID(ID_) {
//  setIndex(0);
//}

//void OMBVBodyProperty::setIndex(int i) {
//  if(index != i) {
//    index = i;
//    Property *parent = getParent();
//    delete parent->getProperty(0);
//    if(i==0)
//      parent->setProperty(new CubeProperty("Cube",ID));
//    else
//      parent->setProperty(new CuboidProperty("Cuboid",ID));
//    mw->removeProperty();
////    Property::property.clear();
////    for(int i=0; i<property->getNumberOfProperties(); i++)
////      addProperty(property->getProperty(i));
//  }
//}

CubeProperty::CubeProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {
  property.push_back(new Scalar_Property("length",LengthUnits()));
}

DOMElement* CubeProperty::initializeUsingXML(DOMElement *element) {
  OMBVBodyProperty::initializeUsingXML(element);
  DOMElement *ele1 = E(element)->getFirstElementChildNamed(OPENMBV%"length");
  property[5]->initializeUsingXML(ele1);
  return element;
}

DOMElement* CubeProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
  DOMElement *ee=D(doc)->createElement(OPENMBV%"length");
  property[5]->writeXMLFile(ee);
  e->insertBefore(ee, NULL);
  return e;
}

void CubeProperty::fromWidget(QWidget *widget) {
//  OMBVBodyProperty::fromWidget(widget);
//  length.fromWidget(static_cast<CubeWidget*>(widget)->length);
}

void CubeProperty::toWidget(QWidget *widget) {
//  OMBVBodyProperty::toWidget(widget);
//  length.toWidget(static_cast<CubeWidget*>(widget)->length);
}

CuboidProperty::CuboidProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

  //property.push_back(new ChoiceProperty2("length",new VecPropertyFactory(getScalars<string>(3,"1"),LengthUnits()),"",4));
  property.push_back(new Vec_Property("length",getScalars<string>(3,"1"),LengthUnits()));

//  vector<PhysicalVariableProperty> input;
//  input.push_back(PhysicalVariableProperty(new VecProperty(getScalars<string>(3,"1")), "m", OPENMBV%"length"));
//  length.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* CuboidProperty::initializeUsingXML(DOMElement *element) {
  OMBVBodyProperty::initializeUsingXML(element);
  DOMElement *ele1 = E(element)->getFirstElementChildNamed(OPENMBV%"length");
  property[5]->initializeUsingXML(ele1);
  return element;
}

DOMElement* CuboidProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
  DOMElement *ee=D(doc)->createElement(OPENMBV%"length");
  property[5]->writeXMLFile(ee);
  e->insertBefore(ee, NULL);
//  length.writeXMLFile(e);
  return e;
}

void CuboidProperty::fromWidget(QWidget *widget) {
//  OMBVBodyProperty::fromWidget(widget);
//  length.fromWidget(static_cast<CuboidWidget*>(widget)->length);
}

void CuboidProperty::toWidget(QWidget *widget) {
//  OMBVBodyProperty::toWidget(widget);
//  length.toWidget(static_cast<CuboidWidget*>(widget)->length);
}

SphereProperty::SphereProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

  property.push_back(new Scalar_Property("radius",LengthUnits()));

 // vector<PhysicalVariableProperty> input;
 // input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", OPENMBV%"radius"));
 // radius.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* SphereProperty::initializeUsingXML(DOMElement *element) {
  DOMElement *ele0 = OMBVBodyProperty::initializeUsingXML(element);
  DOMElement *ele1 = E(element)->getFirstElementChildNamed(OPENMBV%"radius");
  property[5]->initializeUsingXML(ele1);
  return element;
//  OMBVBodyProperty::initializeUsingXML(element);
//  radius.initializeUsingXML(element);
//  return element;
}

DOMElement* SphereProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
  DOMElement *ee=D(doc)->createElement(OPENMBV%"radius");
  property[5]->writeXMLFile(ee);
  e->insertBefore(ee, NULL);
  return e;
}

void SphereProperty::fromWidget(QWidget *widget) {
//  OMBVBodyProperty::fromWidget(widget);
//  radius.fromWidget(static_cast<SphereWidget*>(widget)->radius);
}

void SphereProperty::toWidget(QWidget *widget) {
//  OMBVBodyProperty::toWidget(widget);
//  radius.toWidget(static_cast<SphereWidget*>(widget)->radius);
}

FrustumProperty::FrustumProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", OPENMBV%"topRadius"));
  top.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", OPENMBV%"baseRadius"));
  base.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", OPENMBV%"height"));
  height.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0"), "m", OPENMBV%"innerTopRadius"));
  innerTop.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0"), "m", OPENMBV%"innerBaseRadius"));
  innerBase.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* FrustumProperty::initializeUsingXML(DOMElement *element) {
  OMBVBodyProperty::initializeUsingXML(element);
  DOMElement *e;
  base.initializeUsingXML(element);
  top.initializeUsingXML(element);
  height.initializeUsingXML(element);
  innerBase.initializeUsingXML(element);
  innerTop.initializeUsingXML(element);
  return element;
}

DOMElement* FrustumProperty::writeXMLFile(DOMNode *parent) {
  DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
  base.writeXMLFile(e);
  top.writeXMLFile(e);
  height.writeXMLFile(e);
  innerBase.writeXMLFile(e);
  innerTop.writeXMLFile(e);
  return e;
}

void FrustumProperty::fromWidget(QWidget *widget) {
  OMBVBodyProperty::fromWidget(widget);
  base.fromWidget(static_cast<FrustumWidget*>(widget)->base);
  top.fromWidget(static_cast<FrustumWidget*>(widget)->top);
  height.fromWidget(static_cast<FrustumWidget*>(widget)->height);
  innerBase.fromWidget(static_cast<FrustumWidget*>(widget)->innerBase);
  innerTop.fromWidget(static_cast<FrustumWidget*>(widget)->innerTop);
}

void FrustumProperty::toWidget(QWidget *widget) {
  OMBVBodyProperty::toWidget(widget);
  base.toWidget(static_cast<FrustumWidget*>(widget)->base);
  top.toWidget(static_cast<FrustumWidget*>(widget)->top);
  height.toWidget(static_cast<FrustumWidget*>(widget)->height);
  innerBase.toWidget(static_cast<FrustumWidget*>(widget)->innerBase);
  innerTop.toWidget(static_cast<FrustumWidget*>(widget)->innerTop);
}

IvBodyProperty::IvBodyProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

  ivFileName.setProperty(new FileProperty(OPENMBV%"ivFileName"));

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("-1"), "rad", OPENMBV%"creaseEdges"));
  creaseEdges.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0"), "", OPENMBV%"boundaryEdges"));
  boundaryEdges.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* IvBodyProperty::initializeUsingXML(DOMElement *element) {
  OMBVBodyProperty::initializeUsingXML(element);
  ivFileName.initializeUsingXML(element);
  creaseEdges.initializeUsingXML(element);
  boundaryEdges.initializeUsingXML(element);
  return element;
}

DOMElement* IvBodyProperty::writeXMLFile(DOMNode *parent) {
  DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
  ivFileName.writeXMLFile(e);
  creaseEdges.writeXMLFile(e);
  boundaryEdges.writeXMLFile(e);
  return e;
}

void IvBodyProperty::fromWidget(QWidget *widget) {
  OMBVBodyProperty::fromWidget(widget);
  ivFileName.fromWidget(static_cast<IvBodyWidget*>(widget)->ivFileName);
  creaseEdges.fromWidget(static_cast<IvBodyWidget*>(widget)->creaseEdges);
  boundaryEdges.fromWidget(static_cast<IvBodyWidget*>(widget)->boundaryEdges);
}

void IvBodyProperty::toWidget(QWidget *widget) {
  OMBVBodyProperty::toWidget(widget);
  ivFileName.toWidget(static_cast<IvBodyWidget*>(widget)->ivFileName);
  creaseEdges.toWidget(static_cast<IvBodyWidget*>(widget)->creaseEdges);
  boundaryEdges.toWidget(static_cast<IvBodyWidget*>(widget)->boundaryEdges);
}

CompoundRigidBodyProperty::CompoundRigidBodyProperty(const std::string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {
  bodies.setProperty(new ListProperty(new ChoicePropertyFactory(new OMBVBodyPropertyFactory(ID),"",1),"",0,1));
  //bodies.setXMLName(MBSIM%"bodies");
}

DOMElement* CompoundRigidBodyProperty::initializeUsingXML(DOMElement *element) {
  OMBVBodyProperty::initializeUsingXML(element);
  DOMElement *e=E(element)->getFirstElementChildNamed(OPENMBV%"scaleFactor");
  DOMElement *ee = e->getNextElementSibling();
  bodies.initializeUsingXML(ee);
  return element;
}

DOMElement* CompoundRigidBodyProperty::writeXMLFile(DOMNode *parent) {
  DOMElement *ele0 = OMBVBodyProperty::writeXMLFile(parent);
  bodies.writeXMLFile(ele0);
  return ele0;
}

void CompoundRigidBodyProperty::fromWidget(QWidget *widget) {
  bodies.fromWidget(static_cast<CompoundRigidBodyWidget*>(widget)->bodies);
}

void CompoundRigidBodyProperty::toWidget(QWidget *widget) {
  bodies.toWidget(static_cast<CompoundRigidBodyWidget*>(widget)->bodies);
}

OMBVBodySelectionProperty::OMBVBodySelectionProperty(RigidBody *body) : ombv(0,true), ref(0,false) {
  ombv.setProperty(new ChoiceProperty2(new OMBVBodyPropertyFactory(body->getID()),MBSIM%"openMBVRigidBody"));
  ref.setProperty(new LocalFrameOfReferenceProperty("","Frame[C]",body)); 
}

DOMElement* OMBVBodySelectionProperty::initializeUsingXML(DOMElement *element) {
  ombv.initializeUsingXML(element);
  ref.initializeUsingXML(element);
  return element;
}

DOMElement* OMBVBodySelectionProperty::writeXMLFile(DOMNode *parent) {
  ombv.writeXMLFile(parent);
  ref.writeXMLFile(parent);
  return 0;
}

void OMBVBodySelectionProperty::fromWidget(QWidget *widget) {
  ref.fromWidget(static_cast<OMBVBodySelectionWidget*>(widget)->ref);
  ombv.fromWidget(static_cast<OMBVBodySelectionWidget*>(widget)->ombv);
}

void OMBVBodySelectionProperty::toWidget(QWidget *widget) {
  ref.toWidget(static_cast<OMBVBodySelectionWidget*>(widget)->ref);
  ombv.toWidget(static_cast<OMBVBodySelectionWidget*>(widget)->ombv);
}

DOMElement* OMBVEmptyProperty::initializeUsingXML(DOMElement *parent) {
  return E(parent)->getFirstElementChildNamed(xmlName);
}

DOMElement* OMBVEmptyProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *ele = D(doc)->createElement(xmlName);
  writeXMLFileID(ele);
  parent->insertBefore(ele, NULL);
  return 0;
}

OMBVPlaneProperty::OMBVPlaneProperty(const FQN &xmlName_, const std::string &ID) : OMBVObjectProperty("Plane",ID), xmlName(xmlName_) {

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", MBSIM%"size"));
  size.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("10"), "", MBSIM%"numberOfLines"));
  numberOfLines.setProperty(new ExtPhysicalVarProperty(input));
}

DOMElement* OMBVPlaneProperty::initializeUsingXML(DOMElement *element) {
  DOMElement *e=E(element)->getFirstElementChildNamed(xmlName);
  if(e) {
    size.initializeUsingXML(e);
    numberOfLines.initializeUsingXML(e);
  }
  return e;
}

DOMElement* OMBVPlaneProperty::writeXMLFile(DOMNode *parent) {
  DOMDocument *doc=parent->getOwnerDocument();
  DOMElement *e=D(doc)->createElement(xmlName);
  writeXMLFileID(e);
  parent->insertBefore(e, NULL);
  size.writeXMLFile(e);
  numberOfLines.writeXMLFile(e);
  return e;
}

void OMBVPlaneProperty::fromWidget(QWidget *widget) {
  size.fromWidget(static_cast<OMBVPlaneWidget*>(widget)->size);
  numberOfLines.fromWidget(static_cast<OMBVPlaneWidget*>(widget)->numberOfLines);
}

void OMBVPlaneProperty::toWidget(QWidget *widget) {
  size.toWidget(static_cast<OMBVPlaneWidget*>(widget)->size);
  numberOfLines.toWidget(static_cast<OMBVPlaneWidget*>(widget)->numberOfLines);
}
