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
#include <xercesc/dom/DOMProcessingInstruction.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  OMBVBodyPropertyFactory::OMBVBodyPropertyFactory(const string &ID_) : ID(ID_), count(0) {
    name.push_back(OPENMBV%"Cube");
    name.push_back(OPENMBV%"Cuboid");
    name.push_back(OPENMBV%"Frustum");
    name.push_back(OPENMBV%"Extrusion");
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
      return new ExtrusionProperty("Body"+toStr(count++),ID);
    if(i==4)
      return new SphereProperty("Body"+toStr(count++),ID);
    if(i==5)
      return new IvBodyProperty("Body"+toStr(count++),ID);
    if(i==6)
      return new CompoundRigidBodyProperty("Body"+toStr(count++),ID);
    if(i==7)
      return new InvisibleBodyProperty("Body"+toStr(count++),ID);
  }

  void OMBVObjectProperty::writeXMLFileID(DOMNode *parent) {
    if(!ID.empty()) {
      DOMDocument *doc=parent->getOwnerDocument();
      DOMProcessingInstruction *id=doc->createProcessingInstruction(X()%"OPENMBV_ID", X()%ID);
      parent->insertBefore(id, parent->getFirstChild());
    }
  }

  MBSOMBVProperty::MBSOMBVProperty(const string &name, const FQN &xmlName_, const std::string &ID) : OMBVObjectProperty(name,ID), diffuseColor(0,false), transparency(0,false), xmlName(xmlName_) {

    diffuseColor.setProperty(new ColorProperty(MBSIM%"diffuseColor"));

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0.3"), "-", MBSIM%"transparency"));
    transparency.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* MBSOMBVProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
    if(e) {
      diffuseColor.initializeUsingXML(e);
      transparency.initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* MBSOMBVProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=initXMLFile(parent);
    writeProperties(e);
  }

  DOMElement* MBSOMBVProperty::initXMLFile(DOMNode *parent) {
    DOMDocument *doc=parent->getOwnerDocument();
    DOMElement *e=D(doc)->createElement(xmlName);
    writeXMLFileID(e);
    parent->insertBefore(e, NULL);
    return e;
  }

  DOMElement* MBSOMBVProperty::writeProperties(DOMElement *e) {
    diffuseColor.writeXMLFile(e);
    transparency.writeXMLFile(e);
  }

  void MBSOMBVProperty::fromWidget(QWidget *widget) {
    diffuseColor.fromWidget(static_cast<MBSOMBVWidget*>(widget)->diffuseColor);
    transparency.fromWidget(static_cast<MBSOMBVWidget*>(widget)->transparency);
  }

  void MBSOMBVProperty::toWidget(QWidget *widget) {
    diffuseColor.toWidget(static_cast<MBSOMBVWidget*>(widget)->diffuseColor);
    transparency.toWidget(static_cast<MBSOMBVWidget*>(widget)->transparency);
  }

  PointMBSOMBVProperty::PointMBSOMBVProperty(const string &name, const FQN &xmlName, const std::string &ID) : MBSOMBVProperty(name,xmlName,ID), size(0,false) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0.001"), "m", MBSIM%"size"));
    size.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* PointMBSOMBVProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=MBSOMBVProperty::initializeUsingXML(element);
    if(e) size.initializeUsingXML(e);
    return e;
  }

  DOMElement* PointMBSOMBVProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=MBSOMBVProperty::initXMLFile(parent);
    size.writeXMLFile(e);
    writeProperties(e);
    return e;
  }

  void PointMBSOMBVProperty::fromWidget(QWidget *widget) {
    MBSOMBVProperty::fromWidget(widget);
    size.fromWidget(static_cast<PointMBSOMBVWidget*>(widget)->size);
  }

  void PointMBSOMBVProperty::toWidget(QWidget *widget) {
    MBSOMBVProperty::toWidget(widget);
    size.toWidget(static_cast<PointMBSOMBVWidget*>(widget)->size);
  }

  LineMBSOMBVProperty::LineMBSOMBVProperty(const string &name, const FQN &xmlName, const std::string &ID) : MBSOMBVProperty(name,xmlName,ID), length(0,false) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", MBSIM%"length"));
    length.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* LineMBSOMBVProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=MBSOMBVProperty::initializeUsingXML(element);
    if(e) length.initializeUsingXML(e);
    return e;
  }

  DOMElement* LineMBSOMBVProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=MBSOMBVProperty::initXMLFile(parent);
    length.writeXMLFile(e);
    writeProperties(e);
    return e;
  }

  void LineMBSOMBVProperty::fromWidget(QWidget *widget) {
    MBSOMBVProperty::fromWidget(widget);
    length.fromWidget(static_cast<LineMBSOMBVWidget*>(widget)->length);
  }

  void LineMBSOMBVProperty::toWidget(QWidget *widget) {
    MBSOMBVProperty::toWidget(widget);
    length.toWidget(static_cast<LineMBSOMBVWidget*>(widget)->length);
  }

  PlaneMBSOMBVProperty::PlaneMBSOMBVProperty(const string &name, const FQN &xmlName, const std::string &ID) : MBSOMBVProperty(name,xmlName,ID), length(0,false) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new VecProperty(getScalars<string>(2,"1")), "m", MBSIM%"length"));
    length.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* PlaneMBSOMBVProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=MBSOMBVProperty::initializeUsingXML(element);
    if(e) length.initializeUsingXML(e);
    return e;
  }

  DOMElement* PlaneMBSOMBVProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=MBSOMBVProperty::initXMLFile(parent);
    length.writeXMLFile(e);
    writeProperties(e);
    return e;
  }

  void PlaneMBSOMBVProperty::fromWidget(QWidget *widget) {
    MBSOMBVProperty::fromWidget(widget);
    length.fromWidget(static_cast<PlaneMBSOMBVWidget*>(widget)->length);
  }

  void PlaneMBSOMBVProperty::toWidget(QWidget *widget) {
    MBSOMBVProperty::toWidget(widget);
    length.toWidget(static_cast<PlaneMBSOMBVWidget*>(widget)->length);
  }

  OMBVFrameProperty::OMBVFrameProperty(const string &name, const FQN &xmlName_, const std::string &ID) : OMBVObjectProperty(name,ID), size(0,false), offset(0,false), transparency(0,false), xmlName(xmlName_) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", MBSIM%"size"));
    size.setProperty(new ExtPhysicalVarProperty(input));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", MBSIM%"offset"));
    offset.setProperty(new ExtPhysicalVarProperty(input));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0.3"), "-", MBSIM%"transparency"));
    transparency.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* OMBVFrameProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
    if(e) {
      size.initializeUsingXML(e);
      offset.initializeUsingXML(e);
      transparency.initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* OMBVFrameProperty::writeXMLFile(DOMNode *parent) {
    DOMDocument *doc=parent->getOwnerDocument();
    if(xmlName!=FQN()) {
      DOMElement *e=D(doc)->createElement(xmlName);
      writeXMLFileID(e);
      parent->insertBefore(e, NULL);
      size.writeXMLFile(e);
      offset.writeXMLFile(e);
      transparency.writeXMLFile(e);
      return e;
    }
    else {
      writeXMLFileID(parent);
      size.writeXMLFile(parent);
      offset.writeXMLFile(parent);
      transparency.writeXMLFile(parent);
      return 0;
    }
  }

  void OMBVFrameProperty::fromWidget(QWidget *widget) {
    size.fromWidget(static_cast<OMBVFrameWidget*>(widget)->size);
    offset.fromWidget(static_cast<OMBVFrameWidget*>(widget)->offset);
    transparency.fromWidget(static_cast<OMBVFrameWidget*>(widget)->transparency);
  }

  void OMBVFrameProperty::toWidget(QWidget *widget) {
    size.toWidget(static_cast<OMBVFrameWidget*>(widget)->size);
    offset.toWidget(static_cast<OMBVFrameWidget*>(widget)->offset);
    transparency.toWidget(static_cast<OMBVFrameWidget*>(widget)->transparency);
  }

  OMBVDynamicColoredObjectProperty::OMBVDynamicColoredObjectProperty(const string &name, const std::string &ID, bool readXMLType_) : OMBVObjectProperty(name,ID), minimalColorValue(0,false), maximalColorValue(0,false), diffuseColor(0,false), transparency(0,false), readXMLType(readXMLType_) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0"), "-", OPENMBV%"minimalColorValue"));
    minimalColorValue.setProperty(new ExtPhysicalVarProperty(input));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", OPENMBV%"maximalColorValue"));
    maximalColorValue.setProperty(new ExtPhysicalVarProperty(input));

    diffuseColor.setProperty(new ColorProperty(OPENMBV%"diffuseColor"));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0.3"), "-", OPENMBV%"transparency"));
    transparency.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* OMBVDynamicColoredObjectProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=readXMLType?E(element)->getFirstElementChildNamed(OPENMBV%getType()):element;
    if(e) {
      minimalColorValue.initializeUsingXML(e);
      maximalColorValue.initializeUsingXML(e);
      diffuseColor.initializeUsingXML(e);
      transparency.initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* OMBVDynamicColoredObjectProperty::writeXMLFile(DOMNode *parent) {
    DOMDocument *doc=parent->getOwnerDocument();
    DOMElement *e=D(doc)->createElement(OPENMBV%getType());
    parent->insertBefore(e, NULL);
    E(e)->setAttribute("name", name);
    writeXMLFileID(e);
    minimalColorValue.writeXMLFile(e);
    maximalColorValue.writeXMLFile(e);
    diffuseColor.writeXMLFile(e);
    transparency.writeXMLFile(e);
    return e;
  }

  void OMBVDynamicColoredObjectProperty::fromWidget(QWidget *widget) {
    minimalColorValue.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->minimalColorValue);
    maximalColorValue.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->maximalColorValue);
    diffuseColor.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->diffuseColor);
    transparency.fromWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->transparency);
  }

  void OMBVDynamicColoredObjectProperty::toWidget(QWidget *widget) {
    minimalColorValue.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->minimalColorValue);
    maximalColorValue.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->maximalColorValue);
    diffuseColor.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->diffuseColor);
    transparency.toWidget(static_cast<OMBVDynamicColoredObjectWidget*>(widget)->transparency);
  }

  OMBVArrowProperty::OMBVArrowProperty(const string &name, const MBXMLUtils::FQN &xmlName_, const std::string &ID, bool fromPoint) : OMBVObjectProperty(name,ID), scaleLength(0,false), scaleSize(0,false), referencePoint(0,false), diffuseColor(0,false), transparency(0,false), xmlName(xmlName_) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", MBSIM%"scaleLength"));
    scaleLength.setProperty(new ExtPhysicalVarProperty(input));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", MBSIM%"scaleSize"));
    scaleSize.setProperty(new ExtPhysicalVarProperty(input));

    referencePoint.setProperty(new TextProperty(fromPoint?"fromPoint":"toPoint", MBSIM%"referencePoint", true));
    if(fromPoint)
      referencePoint.setActive(true);

    diffuseColor.setProperty(new ColorProperty(MBSIM%"diffuseColor"));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0.3"), "-", MBSIM%"transparency"));
    transparency.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* OMBVArrowProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
    if(e) {
      scaleLength.initializeUsingXML(e);
      scaleSize.initializeUsingXML(e);
      referencePoint.initializeUsingXML(e);
      diffuseColor.initializeUsingXML(e);
      transparency.initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* OMBVArrowProperty::writeXMLFile(DOMNode *parent) {
    DOMDocument *doc=parent->getOwnerDocument();
    if(xmlName!=FQN()) {
      DOMElement *e=D(doc)->createElement(xmlName);
      writeXMLFileID(e);
      parent->insertBefore(e, NULL);
      scaleLength.writeXMLFile(e);
      scaleSize.writeXMLFile(e);
      referencePoint.writeXMLFile(e);
      diffuseColor.writeXMLFile(e);
      transparency.writeXMLFile(e);
      return e;
    }
    else {
      writeXMLFileID(parent);
      scaleLength.writeXMLFile(parent);
      scaleSize.writeXMLFile(parent);
      referencePoint.writeXMLFile(parent);
      diffuseColor.writeXMLFile(parent);
      transparency.writeXMLFile(parent);
      return 0;
    }
  }

  void OMBVArrowProperty::fromWidget(QWidget *widget) {
    scaleLength.fromWidget(static_cast<OMBVArrowWidget*>(widget)->scaleLength);
    scaleSize.fromWidget(static_cast<OMBVArrowWidget*>(widget)->scaleSize);
    referencePoint.fromWidget(static_cast<OMBVArrowWidget*>(widget)->referencePoint);
    diffuseColor.fromWidget(static_cast<OMBVArrowWidget*>(widget)->diffuseColor);
    transparency.fromWidget(static_cast<OMBVArrowWidget*>(widget)->transparency);
  }

  void OMBVArrowProperty::toWidget(QWidget *widget) {
    scaleLength.toWidget(static_cast<OMBVArrowWidget*>(widget)->scaleLength);
    scaleSize.toWidget(static_cast<OMBVArrowWidget*>(widget)->scaleSize);
    referencePoint.toWidget(static_cast<OMBVArrowWidget*>(widget)->referencePoint);
    diffuseColor.toWidget(static_cast<OMBVArrowWidget*>(widget)->diffuseColor);
    transparency.toWidget(static_cast<OMBVArrowWidget*>(widget)->transparency);
  }

  OMBVCoilSpringProperty::OMBVCoilSpringProperty(const string &name, const MBXMLUtils::FQN &xmlName_, const std::string &ID) : OMBVObjectProperty(name,ID), type(0,false), numberOfCoils(0,false), springRadius(0,false), crossSectionRadius(0,false), nominalLength(0,false), diffuseColor(0,false), transparency(0,false), xmlName(xmlName_) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("3"), "", MBSIM%"numberOfCoils"));
    numberOfCoils.setProperty(new ExtPhysicalVarProperty(input));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", MBSIM%"springRadius"));
    springRadius.setProperty(new ExtPhysicalVarProperty(input));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("-1"), "m", MBSIM%"crossSectionRadius"));
    crossSectionRadius.setProperty(new ExtPhysicalVarProperty(input));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("-1"), "m", MBSIM%"nominalLength"));
    nominalLength.setProperty(new ExtPhysicalVarProperty(input));

    type.setProperty(new TextProperty("tube", MBSIM%"type", true));

    diffuseColor.setProperty(new ColorProperty(MBSIM%"diffuseColor"));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0.3"), "-", MBSIM%"transparency"));
    transparency.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* OMBVCoilSpringProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
    if(e) {
      numberOfCoils.initializeUsingXML(e);
      springRadius.initializeUsingXML(e);
      crossSectionRadius.initializeUsingXML(e);
      nominalLength.initializeUsingXML(e);
      type.initializeUsingXML(e);
      diffuseColor.initializeUsingXML(e);
      transparency.initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* OMBVCoilSpringProperty::writeXMLFile(DOMNode *parent) {
    DOMDocument *doc=parent->getOwnerDocument();
    if(xmlName!=FQN()) {
      DOMElement *e=D(doc)->createElement(xmlName);
      writeXMLFileID(e);
      parent->insertBefore(e, NULL);
      numberOfCoils.writeXMLFile(e);
      springRadius.writeXMLFile(e);
      crossSectionRadius.writeXMLFile(e);
      nominalLength.writeXMLFile(e);
      type.writeXMLFile(e);
      diffuseColor.writeXMLFile(e);
      transparency.writeXMLFile(e);
      return e;
    } else {
      writeXMLFileID(parent);
      numberOfCoils.writeXMLFile(parent);
      springRadius.writeXMLFile(parent);
      crossSectionRadius.writeXMLFile(parent);
      nominalLength.writeXMLFile(parent);
      type.writeXMLFile(parent);
      diffuseColor.writeXMLFile(parent);
      transparency.writeXMLFile(parent);
      return 0;
    }
  }

  void OMBVCoilSpringProperty::fromWidget(QWidget *widget) {
    type.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->type);
    numberOfCoils.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->numberOfCoils);
    springRadius.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->springRadius);
    crossSectionRadius.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->crossSectionRadius);
    nominalLength.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->nominalLength);
    diffuseColor.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->diffuseColor);
    transparency.fromWidget(static_cast<OMBVCoilSpringWidget*>(widget)->transparency);
  }

  void OMBVCoilSpringProperty::toWidget(QWidget *widget) {
    type.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->type);
    numberOfCoils.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->numberOfCoils);
    springRadius.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->springRadius);
    crossSectionRadius.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->crossSectionRadius);
    nominalLength.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->nominalLength);
    diffuseColor.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->diffuseColor);
    transparency.toWidget(static_cast<OMBVCoilSpringWidget*>(widget)->transparency);
  }

  OMBVBodyProperty::OMBVBodyProperty(const string &name, const std::string &ID) : OMBVDynamicColoredObjectProperty(name,ID) {

    transparency.setActive(true);

    trans.setProperty(new ChoiceProperty2(new VecPropertyFactory(3,OPENMBV%"initialTranslation"),"",4));

    rot.setProperty(new ChoiceProperty2(new VecPropertyFactory(3,OPENMBV%"initialRotation",vector<string>(3,"rad")),"",4));

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "-", OPENMBV%"scaleFactor"));
    scale.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* OMBVBodyProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e = OMBVDynamicColoredObjectProperty::initializeUsingXML(element);
    if(e) {
      trans.initializeUsingXML(e);
      rot.initializeUsingXML(e);
      scale.initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* OMBVBodyProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=OMBVDynamicColoredObjectProperty::writeXMLFile(parent);
    trans.writeXMLFile(e);
    rot.writeXMLFile(e);
    scale.writeXMLFile(e);
    return e;
  }

  void OMBVBodyProperty::fromWidget(QWidget *widget) {
    OMBVDynamicColoredObjectProperty::fromWidget(widget);
    trans.fromWidget(static_cast<OMBVBodyWidget*>(widget)->trans);
    rot.fromWidget(static_cast<OMBVBodyWidget*>(widget)->rot);
    scale.fromWidget(static_cast<OMBVBodyWidget*>(widget)->scale);
  }

  void OMBVBodyProperty::toWidget(QWidget *widget) {
    OMBVDynamicColoredObjectProperty::toWidget(widget);
    trans.toWidget(static_cast<OMBVBodyWidget*>(widget)->trans);
    rot.toWidget(static_cast<OMBVBodyWidget*>(widget)->rot);
    scale.toWidget(static_cast<OMBVBodyWidget*>(widget)->scale);
  }

  CubeProperty::CubeProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", OPENMBV%"length"));
    length.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* CubeProperty::initializeUsingXML(DOMElement *element) {
    OMBVBodyProperty::initializeUsingXML(element);
    length.initializeUsingXML(element);
    return element;
  }

  DOMElement* CubeProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
    length.writeXMLFile(e);
    return e;
  }

  void CubeProperty::fromWidget(QWidget *widget) {
    OMBVBodyProperty::fromWidget(widget);
    length.fromWidget(static_cast<CubeWidget*>(widget)->length);
  }

  void CubeProperty::toWidget(QWidget *widget) {
    OMBVBodyProperty::toWidget(widget);
    length.toWidget(static_cast<CubeWidget*>(widget)->length);
  }

  CuboidProperty::CuboidProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new VecProperty(getScalars<string>(3,"1")), "m", OPENMBV%"length"));
    length.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* CuboidProperty::initializeUsingXML(DOMElement *element) {
    OMBVBodyProperty::initializeUsingXML(element);
    length.initializeUsingXML(element);
    return element;
  }

  DOMElement* CuboidProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
    length.writeXMLFile(e);
    return e;
  }

  void CuboidProperty::fromWidget(QWidget *widget) {
    OMBVBodyProperty::fromWidget(widget);
    length.fromWidget(static_cast<CuboidWidget*>(widget)->length);
  }

  void CuboidProperty::toWidget(QWidget *widget) {
    OMBVBodyProperty::toWidget(widget);
    length.toWidget(static_cast<CuboidWidget*>(widget)->length);
  }

  SphereProperty::SphereProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"), "m", OPENMBV%"radius"));
    radius.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* SphereProperty::initializeUsingXML(DOMElement *element) {
    OMBVBodyProperty::initializeUsingXML(element);
    radius.initializeUsingXML(element);
    return element;
  }

  DOMElement* SphereProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
    radius.writeXMLFile(e);
    return e;
  }

  void SphereProperty::fromWidget(QWidget *widget) {
    OMBVBodyProperty::fromWidget(widget);
    radius.fromWidget(static_cast<SphereWidget*>(widget)->radius);
  }

  void SphereProperty::toWidget(QWidget *widget) {
    OMBVBodyProperty::toWidget(widget);
    radius.toWidget(static_cast<SphereWidget*>(widget)->radius);
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

  ExtrusionProperty::ExtrusionProperty(const string &name, const std::string &ID) : OMBVBodyProperty(name,ID) {

    windingRule.setProperty(new TextProperty("odd", OPENMBV%"windingRule", true));

    height.setProperty(new ChoiceProperty2(new ScalarPropertyFactory("1",OPENMBV%"height",vector<string>(2,"m")),"",4));

    contour.setProperty(new ChoiceProperty2(new MatPropertyFactory(getEye<string>(3,3,"1","0"),OPENMBV%"contour",vector<string>(3,"m")),"",4));

  }

  DOMElement* ExtrusionProperty::initializeUsingXML(DOMElement *element) {
    OMBVBodyProperty::initializeUsingXML(element);
    DOMElement *e;
    windingRule.initializeUsingXML(element);
    height.initializeUsingXML(element);
    contour.initializeUsingXML(element);
    return element;
  }

  DOMElement* ExtrusionProperty::writeXMLFile(DOMNode *parent) {
    DOMElement *e=OMBVBodyProperty::writeXMLFile(parent);
    windingRule.writeXMLFile(e);
    height.writeXMLFile(e);
    contour.writeXMLFile(e);
    return e;
  }

  void ExtrusionProperty::fromWidget(QWidget *widget) {
    OMBVBodyProperty::fromWidget(widget);
    windingRule.fromWidget(static_cast<ExtrusionWidget*>(widget)->windingRule);
    height.fromWidget(static_cast<ExtrusionWidget*>(widget)->height);
    contour.fromWidget(static_cast<ExtrusionWidget*>(widget)->contour);
  }

  void ExtrusionProperty::toWidget(QWidget *widget) {
    OMBVBodyProperty::toWidget(widget);
    windingRule.toWidget(static_cast<ExtrusionWidget*>(widget)->windingRule);
    height.toWidget(static_cast<ExtrusionWidget*>(widget)->height);
    contour.toWidget(static_cast<ExtrusionWidget*>(widget)->contour);
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
    ref.setProperty(new LocalFrameOfReferenceProperty("Frame[C]",body,MBSIM%"openMBVFrameOfReference")); 
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
    input.push_back(PhysicalVariableProperty(new VecProperty(getScalars<string>(2,"1")), "m", MBSIM%"length"));
    size.setProperty(new ExtPhysicalVarProperty(input));
  }

  DOMElement* OMBVPlaneProperty::initializeUsingXML(DOMElement *element) {
    DOMElement *e=E(element)->getFirstElementChildNamed(xmlName);
    if(e) {
      size.initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* OMBVPlaneProperty::writeXMLFile(DOMNode *parent) {
    DOMDocument *doc=parent->getOwnerDocument();
    DOMElement *e=D(doc)->createElement(xmlName);
    writeXMLFileID(e);
    parent->insertBefore(e, NULL);
    size.writeXMLFile(e);
    return e;
  }

  void OMBVPlaneProperty::fromWidget(QWidget *widget) {
    size.fromWidget(static_cast<OMBVPlaneWidget*>(widget)->size);
  }

  void OMBVPlaneProperty::toWidget(QWidget *widget) {
    size.toWidget(static_cast<OMBVPlaneWidget*>(widget)->size);
  }

}
