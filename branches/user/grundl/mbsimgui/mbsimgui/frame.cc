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
#include "frame.h"
#include "ombv_widgets.h"
#include <QMenu>

using namespace std;

Frame::Frame(const QString &str, QTreeWidgetItem *parentItem, int ind, bool grey) : Element(str, parentItem, ind, grey) {

  setText(1,getType());

  if(!grey) {
    QAction *action=new QAction(Utils::QIconCached("newobject.svg"),"Remove", this);
    connect(action,SIGNAL(triggered()),this,SLOT(remove()));
    contextMenu->addAction(action);
  }

  properties->addTab("Visualisation");

  //visu = new ExtXMLWidget("OpenMBV frame",new OMBVObjectChoiceWidget(new OMBVFrameWidget, grey?"":MBSIMNS"enableOpenMBV"));
  visu = new ExtXMLWidget("OpenMBV frame",new OMBVFrameWidget("NOTSET",grey?"":MBSIMNS"enableOpenMBV"),true);
  ((OMBVFrameWidget*)visu->getWidget())->setID(getID());
  properties->addToTab("Visualisation", visu);

  properties->addTab("Plotting");
  plotFeature.push_back(new ExtXMLWidget("Plot global position", new PlotFeature("globalPosition"),true));
  properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);
  plotFeature.push_back(new ExtXMLWidget("Plot global velocity", new PlotFeature("globalVelocity"),true));
  properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);

  properties->addStretch();
}

Frame::~Frame() {
}

void Frame::initializeUsingXML(TiXmlElement *element) {
  Element::initializeUsingXML(element);
  visu->initializeUsingXML(element);
}

TiXmlElement* Frame::writeXMLFile(TiXmlNode *parent) {

  TiXmlElement *ele0 = Element::writeXMLFile(parent);
  visu->writeXMLFile(ele0);
  return ele0;
}

void Frame::initializeUsingXML2(TiXmlElement *element) {
  visu->initializeUsingXML(element);
}

TiXmlElement* Frame::writeXMLFile2(TiXmlNode *parent) {

  visu->writeXMLFile(parent);
  return 0;
}
