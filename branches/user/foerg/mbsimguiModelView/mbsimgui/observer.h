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

#ifndef _OBSERVER__H_
#define _OBSERVER__H_

#include "element.h"
#include "extended_properties.h"

class Observer : public Element {
  public:
    Observer(const QString &str, QTreeWidgetItem *parentItem, int ind);
    ~Observer();
    virtual int getxSize() {return 0;}
    virtual Element* getByPathSearch(QString path);
};

class AbsoluteKinematicsObserver : public Observer {
  public:
    AbsoluteKinematicsObserver(const QString &str, QTreeWidgetItem *parentItem, int ind);
    ~AbsoluteKinematicsObserver();

    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    void initializeDialog();
    virtual void fromWidget();
    virtual void toWidget();
    void initialize();

    virtual QString getType() const { return "AbsoluteKinematicsObserver"; }
  protected:
    ExtWidget *frameWidget, *positionWidget, *velocityWidget, *angularVelocityWidget, *accelerationWidget, *angularAccelerationWidget;
    ExtProperty frame, position, velocity, angularVelocity, acceleration, angularAcceleration;
};

#endif
