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

#ifndef _SIGNAL__H_
#define _SIGNAL__H_

#include "link.h"
#include "extended_properties.h"

class Signal : public Link {
  public:
    Signal(const std::string &str, QTreeWidgetItem *parentItem, int ind);
    ~Signal(); 
};

class Sensor : public Signal {
  public:
    Sensor(const std::string &str, QTreeWidgetItem *parentItem, int ind);
    ~Sensor(); 
};

class AbsolutCoordinateSensor : public Sensor {
  public:
    AbsolutCoordinateSensor(const std::string &str, QTreeWidgetItem *parentItem, int ind); 
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    void initializeDialog();
    virtual void fromWidget();
    virtual void toWidget();
    void initialize();
  protected:
    ExtProperty frame, direction;
};

class AbsolutePositionSensor : public AbsolutCoordinateSensor {
  public:
    AbsolutePositionSensor(const std::string &str, QTreeWidgetItem *parentItem, int ind); 
};


#endif
