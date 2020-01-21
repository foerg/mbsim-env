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
#include "body.h"
#include "frame.h"
#include "contour.h"

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  Body::~Body() {
    for(auto & i : frame) 
      delete i;
    for(auto & i : contour) 
      delete i;
    for(auto & i : removedElement) 
      delete i;
  }

  void Body::addFrame(Frame* frame_) {
    frame.push_back(frame_);
    frame_->setParent(this);
    frame_->updateStatus();
  }

  void Body::addContour(Contour* contour_) {
    contour.push_back(contour_);
    contour_->setParent(this);
    contour_->updateStatus();
  }

  void Body::removeElement(Element* element) {
    if(dynamic_cast<Frame*>(element)) {
      for (auto it = frame.begin() ; it != frame.end(); ++it)
        if(*it==element) {
          frame.erase(it);
          break;
        }
    }
    else if(dynamic_cast<Contour*>(element)) {
      for (auto it = contour.begin() ; it != contour.end(); ++it)
        if(*it==element) {
          contour.erase(it);
          break;
        }
    }
    removedElement.push_back(element);
  }

  Frame* Body::getFrame(const QString &name) const {
    size_t i;
    for(i=0; i<frame.size(); i++) {
      if(frame[i]->getName() == name)
        return frame[i];
    }
    return nullptr;
  }

  Contour* Body::getContour(const QString &name) const {
    size_t i;
    for(i=0; i<contour.size(); i++) {
      if(contour[i]->getName() == name)
        return contour[i];
    }
    return nullptr;
  }

  Element* Body::getChildByContainerAndName(const QString &container, const QString &name) const {
    if (container=="Frame")
      return getFrame(name);
    else if (container=="Contour")
      return getContour(name);
    else
      return nullptr;
  }

  int Body::getIndexOfFrame(Frame *frame_) {
    for(size_t i=0; i<frame.size(); i++)
      if(frame[i] == frame_)
        return i;
    return -1;
  }

  int Body::getIndexOfContour(Contour *contour_) {
    for(size_t i=0; i<contour.size(); i++)
      if(contour[i] == contour_)
        return i;
    return -1;
  }

  void Body::setEmbeded(bool embeded) {
    Object::setEmbeded(embeded);
    for(auto & i : frame)
      i->setEmbeded(embeded);
    for(auto & i : contour)
      i->setEmbeded(embeded);
  }

  void Body::updateStatus() {
    Element::updateStatus();
    for(auto & i : frame)
      i->updateStatus();
    for(auto & i : contour)
      i->updateStatus();
  }

}
