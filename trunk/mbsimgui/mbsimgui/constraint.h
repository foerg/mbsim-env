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

#ifndef _CONSTRAINT__H_
#define _CONSTRAINT__H_

#include "object.h"
#include "extended_properties.h"

class RigidBody;

class Constraint : public Object {
  public:
    Constraint(const std::string &str, Element *parent);
};

class GearConstraint : public Constraint {
  friend class GearConstraintPropertyDialog;
  public:
    GearConstraint(const std::string &str, Element *parent);
    std::string getType() const { return "GearConstraint"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void initialize();
    ElementPropertyDialog* createPropertyDialog() {return new GearConstraintPropertyDialog(this);}
  protected:
    ExtProperty dependentBody, independentBodies, gearForceArrow, gearMomentArrow;
};

class KinematicConstraint : public Constraint {
  friend class KinematicConstraintPropertyDialog;
  public:
    KinematicConstraint(const std::string &str, Element *parent);
    std::string getType() const { return "KinematicConstraint"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void initialize();
    void deinitialize();
    ElementPropertyDialog* createPropertyDialog() {return new KinematicConstraintPropertyDialog(this);}
  protected:
    ExtProperty dependentBody, constraintForceArrow, constraintMomentArrow;
};

class TimeDependentKinematicConstraint : public KinematicConstraint {
  friend class TimeDependentKinematicConstraintPropertyDialog;
  public:
    TimeDependentKinematicConstraint(const std::string &str, Element *parent);
    std::string getType() const { return "TimeDependentKinematicConstraint"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    ElementPropertyDialog* createPropertyDialog() {return new TimeDependentKinematicConstraintPropertyDialog(this);}
  protected:
    ExtProperty generalizedPositionFunction;
};

class StateDependentKinematicConstraint : public KinematicConstraint {
  friend class StateDependentKinematicConstraintPropertyDialog;
  public:
    StateDependentKinematicConstraint(const std::string &str, Element *parent);
    std::string getType() const { return "StateDependentKinematicConstraint"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    ElementPropertyDialog* createPropertyDialog() {return new StateDependentKinematicConstraintPropertyDialog(this);}
  protected:
    ExtProperty generalizedVelocityFunction;
};

class JointConstraint : public Constraint {
  friend class JointConstraintPropertyDialog;
  public:
    JointConstraint(const std::string &str, Element *parent);
    std::string getType() const { return "JointConstraint"; }
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void initialize();
    ElementPropertyDialog* createPropertyDialog() {return new JointConstraintPropertyDialog(this);}
  protected:
    ExtProperty force, moment, connections, independentBody, dependentBodiesFirstSide, dependentBodiesSecondSide, jointForceArrow, jointMomentArrow;

};

#endif
