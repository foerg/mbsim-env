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

#ifndef _GROUP__H_
#define _GROUP__H_

#include "element.h"
#include "extended_properties.h"

class Group : public Element {
  Q_OBJECT
  protected:
    QString getType() const { return "Group"; }
    QAction *actionPaste;
    ExtWidget *positionWidget, *orientationWidget, *frameOfReferenceWidget; 
    ExtProperty position, orientation, frameOfReference; 

  public:
    Group(const QString &str, QTreeWidgetItem *parentItem, int ind);
    int getqSize();
    int getuSize();
    int getxSize();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual Element *getByPathSearch(QString path);
    void setActionPasteDisabled(bool flag);
    void initializeDialog();
    virtual void fromWidget();
    virtual void toWidget();
    void initialize();

  protected slots:
    void addGroup();
    void addFrame();
    void addPoint();
    void addLine();
    void addPlane();
    void addSphere();
    void addRigidBody();
    void addRigidBodies();
    void addJointConstraint();
    void addKinematicConstraint();
    void addJoint();
    void addKineticExcitation();
    void addSpringDamper();
    void addContact();
    void addAbsolutePositionSensor();
    void addAbsoluteKinematicsObserver();
    //void remove();

  public slots:
    void addFromFile();
    void paste();
};

#endif
