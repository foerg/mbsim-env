/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2013 Martin Förg

  This library is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version. 
   
  This library is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details. 
   
  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

#ifndef _BASICITEMDATA__H_
#define _BASICITEMDATA__H_

#include "treeitemdata.h"
#include "element.h"
#include "utils.h"
#include "mainwindow.h"

namespace MBSimGUI {

  extern MainWindow *mw;

  class ContainerItemData : public TreeItemData {
    protected:
      Element *element;
    public:
      ContainerItemData(Element *element_);
      QString getValue() const override { return ""; }
      QString getType() const override { return ""; }
      QString getReference() const override { return ""; }
      bool getEnabled() const override { return element->getEnabled(); }
      Element* getElement() { return element; }
  };

  class FrameItemData : public ContainerItemData {
    public:
      FrameItemData(Element *element) : ContainerItemData(element) {
        icon = QIcon(new OverlayIconEngine((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"container.svg").string(),
                                           (mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"frame.svg").string()));
      }
      QString getName() const override { return "frames"; }
      QMenu* createContextMenu() override { return element->createFrameContextMenu(); }
  };

  class ContourItemData : public ContainerItemData {
    public:
      ContourItemData(Element *element) : ContainerItemData(element) {
        icon = QIcon(new OverlayIconEngine((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"container.svg").string(),
                                           (mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"contour.svg").string()));
      }
      QString getName() const override { return "contours"; }
      QMenu* createContextMenu() override { return new ContoursContextMenu(element); }
  };

  class GroupItemData : public ContainerItemData {
    public:
      GroupItemData(Element *element) : ContainerItemData(element) {
        icon = QIcon(new OverlayIconEngine((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"container.svg").string(),
                                           (mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"group.svg").string()));
      }
      QString getName() const override { return "groups"; }
      QMenu* createContextMenu() override { return new GroupsContextMenu(element); }
  };


  class ObjectItemData : public ContainerItemData {
    public:
      ObjectItemData(Element *element) : ContainerItemData(element) {
        icon = QIcon(new OverlayIconEngine((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"container.svg").string(),
                                           (mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"body.svg").string()));
      }
      QString getName() const override { return "objects"; }
      QMenu* createContextMenu() override { return new ObjectsContextMenu(element); }
  };

  class LinkItemData : public ContainerItemData {
    public:
      LinkItemData(Element *element) : ContainerItemData(element) {
        icon = QIcon(new OverlayIconEngine((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"container.svg").string(),
                                           (mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"link.svg").string()));
      }
      QString getName() const override { return "links"; }
      QMenu* createContextMenu() override { return new LinksContextMenu(element); }
  };

  class ConstraintItemData : public ContainerItemData {
    public:
      ConstraintItemData(Element *element) : ContainerItemData(element) {
        icon = QIcon(new OverlayIconEngine((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"container.svg").string(),
                                           (mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"constraint.svg").string()));
      }
      QString getName() const override { return "constraints"; }
      QMenu* createContextMenu() override { return new ConstraintsContextMenu(element); }
  };

  class ObserverItemData : public ContainerItemData {
    public:
      ObserverItemData(Element *element) : ContainerItemData(element) {
        icon = QIcon(new OverlayIconEngine((mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"container.svg").string(),
                                           (mw->getInstallPath()/"share"/"mbsimgui"/"icons"/"observer.svg").string()));
      }
      QString getName() const override { return "observers"; }
      QMenu* createContextMenu() override { return new ObserversContextMenu(element); }
  };

}

#endif
