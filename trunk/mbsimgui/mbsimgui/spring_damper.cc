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

#include "spring_damper.h"
#include "utils.h"
#include <QtGui/QMenu>
#include "frame.h"

using namespace std;


SpringDamper::SpringDamper(const QString &str, QTreeWidgetItem *parentItem, int ind) : Link(str, parentItem, ind) {

  setText(1,getType());

  properties->addTab("Kinetics");
  //properties->addTab("Constitutive laws");

  connections=new XMLEditor(properties, Utils::QIconCached("lines.svg"), "Connections", "Kinetics", new ConnectWidget(2,this));
  force=new XMLEditor(properties, Utils::QIconCached("lines.svg"), "Force", "Kinetics", new ForceLawChoiceWidget2(this));

  properties->addStretch();
}

SpringDamper::~SpringDamper() {
}

void SpringDamper::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e;
  Link::initializeUsingXML(element);
  force->initializeUsingXML(element);
  connections->initializeUsingXML(element);
}

TiXmlElement* SpringDamper::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = Link::writeXMLFile(parent);
  force->writeXMLFile(ele0);
  connections->writeXMLFile(ele0);
  return ele0;
}
