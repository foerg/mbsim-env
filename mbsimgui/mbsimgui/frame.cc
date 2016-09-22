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
#include "special_properties.h"
#include "ombv_properties.h"
#include "objectfactory.h"
#include "mainwindow.h"
#include "embed.h"

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  extern MainWindow *mw;

  Frame::Frame(const string &str, Element *parent, bool grey) : Element(str,parent), visu(0,true) {

    // properties->addTab("Plotting");
    // plotFeature.push_back(new ExtWidget("Plot global position", new PlotFeature("globalPosition"),true));
    // properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);
    // plotFeature.push_back(new ExtWidget("Plot global velocity", new PlotFeature("globalVelocity"),true));
    // properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);
    // plotFeature.push_back(new ExtWidget("Plot global acceleration", new PlotFeature("globalAcceleration"),true));
    // properties->addToTab("Plotting",plotFeature[plotFeature.size()-1]);

    visu.setProperty(new OMBVFrameProperty("NOTSET",grey?"":MBSIM%"enableOpenMBV",getID()));
  }

  Frame* Frame::readXMLFile(const string &filename, Element *parent) {
    shared_ptr<DOMDocument> doc=mw->parser->parse(filename);
    DOMElement *e=doc->getDocumentElement();
    //Frame *frame=ObjectFactory::getInstance()->createFrame(e, parent);
    Frame *frame=Embed<Frame>::createAndInit(e,parent);
    if(frame) {
//      frame->initializeUsingXML(e);
      frame->initialize();
    }
    return frame;
  }

  DOMElement* Frame::initializeUsingXML(DOMElement *element) {
    Element::initializeUsingXML(element);
    visu.initializeUsingXML(element);
    return element;
  }

  DOMElement* Frame::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = Element::writeXMLFile(parent);
    visu.writeXMLFile(ele0);
    return ele0;
  }

  void Frame::initializeUsingXML2(DOMElement *element) {
    visu.initializeUsingXML(element);
  }

  DOMElement* Frame::writeXMLFile2(DOMNode *parent) {
    visu.writeXMLFile(parent);
    return 0;
  }

  FixedRelativeFrame::FixedRelativeFrame(const string &str, Element *parent) : Frame(str,parent,false), refFrame(0,false), position(0,false), orientation(0,false) {

    position.setProperty(new ChoiceProperty2(new VecPropertyFactory(3,MBSIM%"relativePosition"),"",4));

    orientation.setProperty(new ChoiceProperty2(new RotMatPropertyFactory(MBSIM%"relativeOrientation"),"",4));

    refFrame.setProperty(new ParentFrameOfReferenceProperty(getParent()->getFrame(0)->getXMLPath(this,true),this,MBSIM%"frameOfReference"));
  }

  void FixedRelativeFrame::initialize() {
    Frame::initialize();
    refFrame.initialize();
  }

  DOMElement* FixedRelativeFrame::initializeUsingXML(DOMElement *element) {
    Frame::initializeUsingXML(element);
    refFrame.initializeUsingXML(element);
    position.initializeUsingXML(element);
    orientation.initializeUsingXML(element);
    return element;
  }

  DOMElement* FixedRelativeFrame::writeXMLFile(DOMNode *parent) {

    DOMElement *ele0 = Frame::writeXMLFile(parent);
    refFrame.writeXMLFile(ele0);
    position.writeXMLFile(ele0);
    orientation.writeXMLFile(ele0);
    return ele0;
  }

  FixedNodalFrame::FixedNodalFrame(const string &str, Element *parent) : Frame(str,parent,false), position(0,false), orientation(0,false), Psi(0,false), sigmahel(0,false), sigmahen(0,false), sigma0(0,false), K0F(0,false), K0M(0,false) {

    position.setProperty(new ChoiceProperty2(new VecPropertyFactory(3,MBSIMFLEX%"relativePosition"),"",4));

    orientation.setProperty(new ChoiceProperty2(new RotMatPropertyFactory(MBSIMFLEX%"relativeOrientation"),"",4));

    Phi.setProperty(new ChoiceProperty2(new MatPropertyFactory(getMat<string>(3,1,"0"),MBSIMFLEX%"shapeMatrixOfTranslation",vector<string>(3,"")),"",4));

    Psi.setProperty(new ChoiceProperty2(new MatPropertyFactory(getMat<string>(3,1,"0"),MBSIMFLEX%"shapeMatrixOfRotation",vector<string>(3,"")),"",4));

    sigmahel.setProperty(new ChoiceProperty2(new MatPropertyFactory(getMat<string>(6,1,"0"),MBSIMFLEX%"stressMatrix",vector<string>(3,"")),"",4));

    sigmahen.setProperty(new OneDimMatArrayProperty(1,6,1,MBSIMFLEX%"nonlinearStressMatrix",true));

    sigma0.setProperty(new ChoiceProperty2(new VecPropertyFactory(6,MBSIMFLEX%"initialStress",vector<string>(3,"")),"",4));

    K0F.setProperty(new OneDimMatArrayProperty(3,1,1,MBSIMFLEX%"geometricStiffnessMatrixDueToForce"));

    K0M.setProperty(new OneDimMatArrayProperty(3,1,1,MBSIMFLEX%"geometricStiffnessMatrixDueToMoment"));
  }

  DOMElement* FixedNodalFrame::initializeUsingXML(DOMElement *element) {
    Frame::initializeUsingXML(element);
    position.initializeUsingXML(element);
    orientation.initializeUsingXML(element);
    Phi.initializeUsingXML(element);
    Psi.initializeUsingXML(element);
    sigmahel.initializeUsingXML(element);
    sigmahen.initializeUsingXML(element);
    sigma0.initializeUsingXML(element);
    K0F.initializeUsingXML(element);
    K0M.initializeUsingXML(element);
    return element;
  }

  DOMElement* FixedNodalFrame::writeXMLFile(DOMNode *parent) {

    DOMElement *ele0 = Frame::writeXMLFile(parent);
    position.writeXMLFile(ele0);
    orientation.writeXMLFile(ele0);
    Phi.writeXMLFile(ele0);
    Psi.writeXMLFile(ele0);
    sigmahel.writeXMLFile(ele0);
    sigmahen.writeXMLFile(ele0);
    sigma0.writeXMLFile(ele0);
    K0F.writeXMLFile(ele0);
    K0M.writeXMLFile(ele0);
    return ele0;
  }

}
