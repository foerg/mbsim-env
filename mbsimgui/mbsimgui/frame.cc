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
#include "utils.h"
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMProcessingInstruction.hpp>
#include "mainwindow.h"

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  extern MainWindow *mw;

  Frame::Frame() {
    icon = Utils::QIconCached(QString::fromStdString((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"frame.svg").string()));
  }

  DOMElement* Frame::processIDAndHref(DOMElement *element) {
    element = Element::processIDAndHref(element);
    DOMElement *ELE=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBV");
    if(ELE) {
      xercesc::DOMDocument *doc=element->getOwnerDocument();
      DOMProcessingInstruction *id=doc->createProcessingInstruction(X()%"OPENMBV_ID", X()%getID());
      ELE->insertBefore(id, nullptr);
    }
    return element;
  }

  InternalFrame::InternalFrame(const QString &name_, MBXMLUtils::FQN xmlFrameName_, const QString &plotFeatureType_) : name(name_), xmlFrameName(std::move(xmlFrameName_)), plotFeatureType(plotFeatureType_) {
  }

  void InternalFrame::removeXMLElements() {
    DOMElement *e = E(parent->getXMLElement())->getFirstElementChildNamed(getXMLFrameName());
    if(e) {
      if(X()%e->getPreviousSibling()->getNodeName()=="#text")
        parent->getXMLElement()->removeChild(e->getPreviousSibling());
      parent->getXMLElement()->removeChild(e);
    }
    e = E(parent->getXMLElement())->getFirstElementChildNamed(MBSIM%getPlotFeatureType().toStdString());
    while (e and E(e)->getTagName()==MBSIM%getPlotFeatureType().toStdString()) {
      DOMElement *en = e->getNextElementSibling();
      if(X()%e->getPreviousSibling()->getNodeName()=="#text")
        parent->getXMLElement()->removeChild(e->getPreviousSibling());
      parent->getXMLElement()->removeChild(e);
      e = en;
    }
  }

}
