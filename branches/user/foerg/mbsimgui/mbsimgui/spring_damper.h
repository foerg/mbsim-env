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

#ifndef _SPRING_DAMPER__H_
#define _SPRING_DAMPER__H_

#include "link.h"
#include "extended_properties.h"

class SpringDamper : public Link {
  friend class SpringDamperPropertyDialog;
  public:
    SpringDamper(const std::string &str, Element *element);
    ~SpringDamper();
    std::string getType() const { return "SpringDamper"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void initialize();
    ElementPropertyDialog* createPropertyDialog() {return new SpringDamperPropertyDialog(this);}
  protected:
    ExtProperty forceFunction, connections, coilSpring, forceArrow;
};

class DirectionalSpringDamper : public Link {
  friend class DirectionalSpringDamperPropertyDialog;
  public:
    DirectionalSpringDamper(const std::string &str, Element *element);
    ~DirectionalSpringDamper();
    std::string getType() const { return "DirectionalSpringDamper"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void initialize();
    ElementPropertyDialog* createPropertyDialog() {return new DirectionalSpringDamperPropertyDialog(this);}
  protected:
    ExtProperty forceDirection, forceFunction, connections, coilSpring, forceArrow;
};

class RelativeSpringDamper : public Link {
  friend class RelativeSpringDamperPropertyDialog;
  public:
    RelativeSpringDamper(const std::string &str, Element *element);
    ~RelativeSpringDamper();
    std::string getType() const { return "RelativeSpringDamper"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void initialize();
    ElementPropertyDialog* createPropertyDialog() {return new RelativeSpringDamperPropertyDialog(this);}
  protected:
    ExtProperty function, body, connections, coilSpring, forceArrow, momentArrow;
};

#endif
