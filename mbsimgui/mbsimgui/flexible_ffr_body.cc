/*
   MBSimGUI - A fronted for MBSim.
   Copyright (C) 2016 Martin Förg

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
#include "flexible_ffr_body.h"
#include "objectfactory.h"
#include "frame.h"
#include "contour.h"
#include "group.h"
#include "embed.h"
#include <xercesc/dom/DOMProcessingInstruction.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  GenericFlexibleFfrBody::GenericFlexibleFfrBody() {
    InternalFrame *K = new InternalFrame("K",MBSIMFLEX%"enableOpenMBVFrameK","plotFeatureFrameK");
    addFrame(K);
  }

  void GenericFlexibleFfrBody::removeXMLElements() {
    DOMNode *e = element->getFirstChild();
    while(e) {
      DOMNode *en=e->getNextSibling();
      if((e != frames) and (e != contours) and (E(e)->getTagName() != MBSIMFLEX%"enableOpenMBVFrameK") and (E(e)->getTagName() != MBSIMFLEX%"plotFeatureFrameK"))
        element->removeChild(e);
      e = en;
    }
  }

  DOMElement* GenericFlexibleFfrBody::createXMLElement(DOMNode *parent) {
    DOMElement *ele0 = Element::createXMLElement(parent);
    xercesc::DOMDocument *doc=ele0->getOwnerDocument();
    frames = D(doc)->createElement( MBSIMFLEX%"frames" );
    ele0->insertBefore( frames, nullptr );
    contours = D(doc)->createElement( MBSIMFLEX%"contours" );
    ele0->insertBefore( contours, nullptr );

    DOMElement *ele1 = D(doc)->createElement( MBSIMFLEX%"enableOpenMBVFrameK" );
    ele0->insertBefore( ele1, nullptr );

    for(size_t i=1; i<frame.size(); i++)
      frame[i]->createXMLElement(frames);
    for(auto & i : contour)
      i->createXMLElement(contours);
    return ele0;
  }

  DOMElement* GenericFlexibleFfrBody::processIDAndHref(DOMElement *element) {
    element = Body::processIDAndHref(element);

    // frames
    DOMElement *ELE=E(element)->getFirstElementChildNamed(MBSIMFLEX%"frames")->getFirstElementChild();
    for(size_t i=1; i<frame.size(); i++) {
      frame[i]->processIDAndHref(ELE);
      ELE=ELE->getNextElementSibling();
    }

    // contours
    ELE=E(element)->getFirstElementChildNamed(MBSIMFLEX%"contours")->getFirstElementChild();
    for(auto & i : contour) {
      i->processIDAndHref(ELE);
      ELE=ELE->getNextElementSibling();
    }

    ELE=E(element)->getFirstElementChildNamed(MBSIMFLEX%"enableOpenMBVFrameK");
    if(ELE) {
      xercesc::DOMDocument *doc=element->getOwnerDocument();
      DOMProcessingInstruction *id=doc->createProcessingInstruction(X()%"OPENMBV_ID", X()%getFrame(0)->getID().toStdString());
      ELE->insertBefore(id, nullptr);
    }

    return element;
  }

  void GenericFlexibleFfrBody::create() {
    Body::create();

    frames = E(element)->getFirstElementChildNamed(MBSIMFLEX%"frames");
    DOMElement *e=frames->getFirstElementChild();
    Frame *f;
    while(e) {
      f = Embed<Frame>::create(e,this);
      if(f) addFrame(f);
      e=e->getNextElementSibling();
    }

    contours = E(element)->getFirstElementChildNamed(MBSIMFLEX%"contours");
    e=contours->getFirstElementChild();
    Contour *c;
    while(e) {
      c = Embed<Contour>::create(e,this);
      if(c) addContour(c);
      e=e->getNextElementSibling();
    }
  }

  DOMElement* FlexibleFfrBody::processIDAndHref(DOMElement *element) {
    element = GenericFlexibleFfrBody::processIDAndHref(element);

    DOMElement *ELE=E(element)->getFirstElementChildNamed(MBSIMFLEX%"openMBVFlexibleBody");
    if(ELE) {
      ELE = ELE->getFirstElementChild();
      if(ELE) {
        xercesc::DOMDocument *doc=element->getOwnerDocument();
        DOMProcessingInstruction *id=doc->createProcessingInstruction(X()%"OPENMBV_ID", X()%getID().toStdString());
        ELE->insertBefore(id, nullptr);
      }
    }

    return element;
  }

  DOMElement* CalculixBody::processIDAndHref(DOMElement *element) {
    element = GenericFlexibleFfrBody::processIDAndHref(element);

    DOMElement *ELE=E(element)->getFirstElementChildNamed(MBSIMFLEX%"enableOpenMBV");
    if(ELE) {
      xercesc::DOMDocument *doc=element->getOwnerDocument();
      DOMProcessingInstruction *id=doc->createProcessingInstruction(X()%"OPENMBV_ID", X()%getID().toStdString());
      ELE->insertBefore(id, nullptr);
    }

    return element;
  }

}
