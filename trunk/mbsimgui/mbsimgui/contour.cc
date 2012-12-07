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
#include "contour.h"
#include "basic_widgets.h"
#include "string_widgets.h"
#include <QMenu>

using namespace std;

Contour::Contour(const QString &str, QTreeWidgetItem *parentItem, int ind) : Element(str,parentItem,ind) {
}

Contour::~Contour() {
}

//virtual void Contour::initializeUsingXML(TiXmlElement *element) {
//  Element::initializeUsingXML(element);
//}
//
//TiXmlElement* Contour::writeXMLFile(TiXmlNode *parent) {
//  return Element::writeXMLFile(parent);
//}

Point::Point(const QString &str, QTreeWidgetItem *parentItem, int ind) : Contour(str,parentItem,ind) {
  setText(1,getType());
  properties->addStretch();
}

Point::~Point() {
}

Line::Line(const QString &str, QTreeWidgetItem *parentItem, int ind) : Contour(str,parentItem,ind) {
  setText(1,getType());
  properties->addStretch();
}

Line::~Line() {
}

Plane::Plane(const QString &str, QTreeWidgetItem *parentItem, int ind) : Contour(str,parentItem,ind) {
  setText(1,getType());
 // visu = new ExtXMLWidget("OpenMBV frame",new OMBVFrameWidget("NOTSET",grey?"":MBSIMNS"enableOpenMBV"), true);
 // properties->addToTab("Visualisation", visu);
  properties->addStretch();
}

Plane::~Plane() {
}

Sphere::Sphere(const QString &str, QTreeWidgetItem *parentItem, int ind) : Contour(str,parentItem,ind) {
  setText(1,getType());
  properties->addTab("Visualisation");
 // visu = new ExtXMLWidget("OpenMBV frame",new OMBVFrameWidget("NOTSET",grey?"":MBSIMNS"enableOpenMBV"), true);
 // properties->addToTab("Visualisation", visu);
  vector<PhysicalStringWidget*> input;
  input.push_back(new PhysicalStringWidget(new ScalarWidget("1"), MBSIMNS"radius", lengthUnits(), 4));
  radius= new ExtXMLWidget("Radius",new ExtPhysicalVarWidget(input));
  properties->addToTab("General", radius);

  visu= new ExtXMLWidget("OpenMBV",new EmptyWidget(MBSIMNS"enableOpenMBV"),true);
  properties->addToTab("Visualisation", visu);

  properties->addStretch();
}

Sphere::~Sphere() {
}

void Sphere::initializeUsingXML(TiXmlElement *element) {
  Contour::initializeUsingXML(element);
  radius->initializeUsingXML(element);
  visu->initializeUsingXML(element);
}

TiXmlElement* Sphere::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *e = Contour::writeXMLFile(parent);
  radius->writeXMLFile(e);
  visu->writeXMLFile(e);
  return e;
}

