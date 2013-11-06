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
#include "linear_transfer_system.h"
#include "extended_widgets.h"
#include "variable_widgets.h"

using namespace std;
using namespace MBXMLUtils;

LinearTransferSystemPropertyDialog::LinearTransferSystemPropertyDialog(LinearTransferSystem *lts, QWidget * parent, Qt::WindowFlags f) : SignalProcessingSystemPropertyDialog(lts,parent,f) {

  choice = new ExtWidget("Type",new ChoiceWidget2(new LinearTransferSystemWidgetFactory));
  addToTab("General", choice);
}

void LinearTransferSystemPropertyDialog::toWidget(Element *element) {
  SignalProcessingSystemPropertyDialog::toWidget(element);
  static_cast<LinearTransferSystem*>(element)->choice.toWidget(choice);
}

void LinearTransferSystemPropertyDialog::fromWidget(Element *element) {
  SignalProcessingSystemPropertyDialog::fromWidget(element);
  static_cast<LinearTransferSystem*>(element)->choice.fromWidget(choice);
}

LinearTransferSystem::LinearTransferSystem(const string &str, Element *parent) : SignalProcessingSystem(str, parent) {

  choice.setProperty(new ChoiceProperty2(new LinearTransferSystemPropertyFactory,"",3));
}

void LinearTransferSystem::initializeUsingXML(TiXmlElement *element) {
  SignalProcessingSystem::initializeUsingXML(element);
  choice.initializeUsingXML(element);
}

TiXmlElement* LinearTransferSystem::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = SignalProcessingSystem::writeXMLFile(parent);
  choice.writeXMLFile(ele0);
  return ele0;
}

LinearTransferSystemWidgetFactory::LinearTransferSystemWidgetFactory() {
  name.push_back("PID type");
  name.push_back("ABCD type");
  name.push_back("Integrator type");
  name.push_back("PT1 type");
}

QWidget* LinearTransferSystemWidgetFactory::createWidget(int i) {
  if(i==0) {
    ContainerWidget *widgetContainer = new ContainerWidget;

    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("P",new ExtPhysicalVarWidget(input)));

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("I",new ExtPhysicalVarWidget(input)));

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("D",new ExtPhysicalVarWidget(input)));

    return widgetContainer;
  }
  if(i==1) {
    ContainerWidget *widgetContainer = new ContainerWidget;

    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("A",new ExtPhysicalVarWidget(input)));

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("B",new ExtPhysicalVarWidget(input)));

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("C",new ExtPhysicalVarWidget(input)));

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("D",new ExtPhysicalVarWidget(input)));

    return widgetContainer;
  }
  if(i==2) {
    ContainerWidget *widgetContainer = new ContainerWidget;

    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("gain",new ExtPhysicalVarWidget(input)));

    return widgetContainer;
  }
  if(i==3) {
    ContainerWidget *widgetContainer = new ContainerWidget;

    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("P",new ExtPhysicalVarWidget(input)));

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0.1"),noUnitUnits(),1));
    widgetContainer->addWidget(new ExtWidget("T",new ExtPhysicalVarWidget(input)));

    return widgetContainer;
  }
}

LinearTransferSystemPropertyFactory::LinearTransferSystemPropertyFactory() {
  name.push_back(MBSIMCONTROLNS"pidType");
  name.push_back(MBSIMCONTROLNS"abcdType");
  name.push_back(MBSIMCONTROLNS"integratorType");
  name.push_back(MBSIMCONTROLNS"pt1Type");
}

Property* LinearTransferSystemPropertyFactory::createProperty(int i) {
  if(i==0) {
    ContainerProperty *propertyContainer = new ContainerProperty(MBSIMCONTROLNS"pidType",0);

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"),"-",MBSIMCONTROLNS"P"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0"),"-",MBSIMCONTROLNS"I"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0"),"-",MBSIMCONTROLNS"D"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    return propertyContainer;
  }
  if(i==1) {
    ContainerProperty *propertyContainer = new ContainerProperty(MBSIMCONTROLNS"abcdType",0);

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0"),"-",MBSIMCONTROLNS"A"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0"),"-",MBSIMCONTROLNS"B"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0"),"-",MBSIMCONTROLNS"C"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0"),"-",MBSIMCONTROLNS"D"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    return propertyContainer;
  }
  if(i==2) {
    ContainerProperty *propertyContainer = new ContainerProperty(MBSIMCONTROLNS"integratorType",0);

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"),"-",MBSIMCONTROLNS"gain"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    return propertyContainer;
  }
  if(i==3) {
    ContainerProperty *propertyContainer = new ContainerProperty(MBSIMCONTROLNS"pt1Type",0);

    vector<PhysicalVariableProperty> input;
    input.push_back(PhysicalVariableProperty(new ScalarProperty("1"),"-",MBSIMCONTROLNS"P"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    input.clear();
    input.push_back(PhysicalVariableProperty(new ScalarProperty("0.1"),"-",MBSIMCONTROLNS"T"));
    propertyContainer->addProperty(new ExtProperty(new ExtPhysicalVarProperty(input)));

    return propertyContainer;
  }

}
