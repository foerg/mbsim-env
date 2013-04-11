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
#include "ombv_properties.h"
#include "string_widgets.h"
#include "ombv_widgets.h"
#include <QMenu>

using namespace std;

Frame::Frame(const string &str, Element *parent, bool grey) : Element(str,parent), visuProperty(0,true) {

 // properties->addTab("Plotting");
 // plotFeature.push_back(new ExtWidget("Plot global position", new PlotFeature("globalPosition"),true));
 // properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);
 // plotFeature.push_back(new ExtWidget("Plot global velocity", new PlotFeature("globalVelocity"),true));
 // properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);
 // plotFeature.push_back(new ExtWidget("Plot global acceleration", new PlotFeature("globalAcceleration"),true));
 // properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);

  visuProperty.setProperty(new OMBVFrameProperty("NOTSET",grey?"":MBSIMNS"enableOpenMBV"));
  ((OMBVFrameProperty*)visuProperty.getProperty())->setID(getID());
}

Frame::~Frame() {
  //cout << "destroy frame" << endl;
}

void Frame::initializeUsingXML(TiXmlElement *element) {
  Element::initializeUsingXML(element);
  visuProperty.initializeUsingXML(element);
}

TiXmlElement* Frame::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = Element::writeXMLFile(parent);
  visuProperty.writeXMLFile(ele0);
  return ele0;
}

void Frame::initializeUsingXML2(TiXmlElement *element) {
  visuProperty.initializeUsingXML(element);
}

TiXmlElement* Frame::writeXMLFile2(TiXmlNode *parent) {
  visuProperty.writeXMLFile(parent);
  return 0;
}

Element *Frame::getByPathSearch(string path) {
  if (path.substr(0, 1)=="/") // absolut path
    if(getParent())
      return getParent()->getByPathSearch(path);
    else
      return getByPathSearch(path.substr(1));
  else if (path.substr(0, 3)=="../") // relative path
    return getParent()->getByPathSearch(path.substr(3));
  else { // local path
    throw;
  }
}

FixedRelativeFrame::FixedRelativeFrame(const string &str, Element *parent) : Frame(str,parent,false), refFrameProperty(0,false), positionProperty(0,false), orientationProperty(0,false) {

  vector<PhysicalStringProperty*> input;
  input.push_back(new PhysicalStringProperty(new VecProperty(3), "m", MBSIMNS"relativePosition"));
  positionProperty.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(new PhysicalStringProperty(new MatProperty(getEye<string>(3,3,"1","0")),"-",MBSIMNS"relativeOrientation"));
  orientationProperty.setProperty(new ExtPhysicalVarProperty(input));

  refFrameProperty.setProperty(new ParentFrameOfReferenceProperty(getParent()->getFrame(0),this,MBSIMNS"frameOfReference"));
}

FixedRelativeFrame::~FixedRelativeFrame() {
}

void FixedRelativeFrame::initialize() {
  Frame::initialize();
  refFrameProperty.initialize();
}

void FixedRelativeFrame::initializeUsingXML(TiXmlElement *element) {
  Frame::initializeUsingXML(element);
  refFrameProperty.initializeUsingXML(element);
  positionProperty.initializeUsingXML(element);
  orientationProperty.initializeUsingXML(element);
}

TiXmlElement* FixedRelativeFrame::writeXMLFile(TiXmlNode *parent) {

  TiXmlElement *ele0 = Frame::writeXMLFile(parent);
  refFrameProperty.writeXMLFile(ele0);
  positionProperty.writeXMLFile(ele0);
  orientationProperty.writeXMLFile(ele0);
  return ele0;
}

void FixedRelativeFrame::initializeUsingXML2(TiXmlElement *element) {
  refFrameProperty.initializeUsingXML(element);
  string ref = ((ParentFrameOfReferenceProperty*)refFrameProperty.getProperty())->getSavedFrameOfReference();
  if(ref[0]=='F')
    ((ParentFrameOfReferenceProperty*)refFrameProperty.getProperty())->setSavedFrameOfReference(string("../")+ref);
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)positionProperty.getProperty())->getPhysicalStringProperty(0))->setXmlName(MBSIMNS"position");
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)positionProperty.getProperty())->getPhysicalStringProperty(1))->setXmlName(MBSIMNS"position");
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)orientationProperty.getProperty())->getPhysicalStringProperty(0))->setXmlName(MBSIMNS"orientation");
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)orientationProperty.getProperty())->getPhysicalStringProperty(1))->setXmlName(MBSIMNS"orientation");
  positionProperty.initializeUsingXML(element);
  orientationProperty.initializeUsingXML(element);
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)positionProperty.getProperty())->getPhysicalStringProperty(0))->setXmlName(MBSIMNS"relativePosition");
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)positionProperty.getProperty())->getPhysicalStringProperty(1))->setXmlName(MBSIMNS"relativePosition");
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)orientationProperty.getProperty())->getPhysicalStringProperty(0))->setXmlName(MBSIMNS"relativeOrientation");
  ((PhysicalStringProperty*)((ExtPhysicalVarProperty*)orientationProperty.getProperty())->getPhysicalStringProperty(1))->setXmlName(MBSIMNS"relativeOrientation");
}

