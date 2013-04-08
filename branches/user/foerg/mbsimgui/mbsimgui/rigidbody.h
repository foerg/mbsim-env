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

#ifndef _RIGIDBODY__H_
#define _RIGIDBODY__H_

#include "body.h"
#include "extended_properties.h"

class RigidBody : public Body {
  public:
    RigidBody(const std::string &str, Element *parent);
    ~RigidBody();
    const std::string getType() const { return "RigidBody"; }
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    void setConstrained(bool b) {constrained = b;}
    void initialize();
  protected:
    bool constrained;
    ExtProperty R, K, mass, inertia, translation, rotation, ombvEditor, weightArrow, jointForceArrow, jointMomentArrow, isFrameOfBodyForRotation;
};

#endif
