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

#ifndef _ELEMENT_PROPERTY_DIALOG_H_
#define _ELEMENT_PROPERTY_DIALOG_H_

#include "property_dialog.h"

class Element;
class Frame;
class FixedRelativeFrame;
class Contour;
class Plane;
class Sphere;
class CircleSolid;
class Solver;
class Group;
class Object;
class Body;
class RigidBody;
class Constraint;
class GearConstraint;
class KinematicConstraint;
class TimeDependentKinematicConstraint;
class StateDependentKinematicConstraint;
class JointConstraint;
class ExtraDynamic;
class SignalProcessingSystem;
class LinearTransferSystem;
class Link;
class KineticExcitation;
class SpringDamper;
class Joint;
class Contact;
class Actuator;
class Observer;
class CoordinatesObserver;
class CartesianCoordinatesObserver;
class CylinderCoordinatesObserver;
class NaturalCoordinatesObserver;
class KinematicsObserver;
class AbsoluteKinematicsObserver;
class RelativeKinematicsObserver;
class NaturalCoordinatesObserver;
class Signal;
class Sensor;
class GeneralizedCoordinateSensor;
class GeneralizedPositionSensor;
class GeneralizedVelocitySensor;
class AbsoluteCoordinateSensor;
class AbsolutePositionSensor;
class AbsoluteVelocitySensor;
class FunctionSensor;
class SignalProcessingSystemSensor;
class SignalAddition;
class PIDController;
class UnarySignalOperation;
class BinarySignalOperation;
class TextWidget;
class VecWidget;
class ExtWidget;

class ElementPropertyDialog : public PropertyDialog {

  public:
    ElementPropertyDialog(Element *element, QWidget * parent = 0, Qt::WindowFlags f = 0, bool embedding=true);
    virtual void toWidget(Element *element);
    virtual void fromWidget(Element *element);
    void toWidget() {toWidget(element);}
    void fromWidget() {fromWidget(element);}
    Element* getElement() {return element;}
  protected:
    Element *element;
    ExtWidget *name, *embed;
};

class FramePropertyDialog : public ElementPropertyDialog {

  public:
    FramePropertyDialog(Frame *frame, QWidget * parent = 0, Qt::WindowFlags f = 0, bool embedding=false);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *visu;
};

class FixedRelativeFramePropertyDialog : public FramePropertyDialog {

  public:
    FixedRelativeFramePropertyDialog(FixedRelativeFrame *frame, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *refFrame, *position, *orientation;
};

class ContourPropertyDialog : public ElementPropertyDialog {

  public:
    ContourPropertyDialog(Contour *contour, QWidget * parent = 0, Qt::WindowFlags f = 0); 
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
   ExtWidget *refFrame;
};

class PlanePropertyDialog : public ContourPropertyDialog {

  public:
    PlanePropertyDialog(Plane *plane, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *radius, *visu;
};

class SpherePropertyDialog : public ContourPropertyDialog {

  public:
    SpherePropertyDialog(Sphere *sphere, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *radius, *visu;
};

class CircleSolidPropertyDialog : public ContourPropertyDialog {

  public:
    CircleSolidPropertyDialog(CircleSolid *circle, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *radius, *visu;
};

class GroupPropertyDialog : public ElementPropertyDialog {

  public:
    GroupPropertyDialog(Group *group, QWidget * parent = 0, Qt::WindowFlags f = 0, bool kinematics=true);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *position, *orientation, *frameOfReference; 
};

class SolverPropertyDialog : public GroupPropertyDialog {
  protected:
    ExtWidget *environment, *solverParameters, *inverseKinetics;

  public:
    SolverPropertyDialog(Solver *solver, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
};

class ObjectPropertyDialog : public ElementPropertyDialog {
  Q_OBJECT;

  public:
    ObjectPropertyDialog(Object *object, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
    virtual void resizeGeneralizedPosition() {}
    virtual void resizeGeneralizedVelocity() {}
  protected:
    ExtWidget *q0, *u0;
    VecWidget *q0_, *u0_;
  public slots:
    void resizeVariables() {resizeGeneralizedPosition();resizeGeneralizedVelocity();}
};

class BodyPropertyDialog : public ObjectPropertyDialog {

  public:
    BodyPropertyDialog(Body *body, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *R;
};

class RigidBodyPropertyDialog : public BodyPropertyDialog {

  public:
    RigidBodyPropertyDialog(RigidBody *body, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
    void resizeGeneralizedPosition();
    void resizeGeneralizedVelocity();
    int getqRelSize() const; 
    int getuRelSize() const; 
  protected:
    ExtWidget *K, *mass, *inertia, *translation, *rotation, *ombvEditor, *weightArrow, *jointForceArrow, *jointMomentArrow, *isFrameOfBodyForRotation;
    RigidBody *body;
};

class ConstraintPropertyDialog : public ObjectPropertyDialog {

  public:
    ConstraintPropertyDialog(Constraint *constraint, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class GearConstraintPropertyDialog : public ConstraintPropertyDialog {
  Q_OBJECT

  public:
    GearConstraintPropertyDialog(GearConstraint *constraint, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *dependentBody, *independentBodies, *gearForceArrow, *gearMomentArrow;
    RigidBody *refBody;
  protected slots:
    void updateReferenceBody();
};

class KinematicConstraintPropertyDialog : public ConstraintPropertyDialog {

  public:
    KinematicConstraintPropertyDialog(KinematicConstraint *constraint, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *dependentBody, *constraintForceArrow, *constraintMomentArrow;
    RigidBody *refBody;
};

class TimeDependentKinematicConstraintPropertyDialog : public KinematicConstraintPropertyDialog {
  Q_OBJECT

  public:
    TimeDependentKinematicConstraintPropertyDialog(TimeDependentKinematicConstraint *constraint, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *generalizedPositionFunction;
  protected slots:
    void resizeVariables();
};

class StateDependentKinematicConstraintPropertyDialog : public KinematicConstraintPropertyDialog {
  Q_OBJECT

  public:
    StateDependentKinematicConstraintPropertyDialog(StateDependentKinematicConstraint *constraint, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
    void resizeGeneralizedPosition();
  protected:
    ExtWidget *generalizedVelocityFunction;
  protected slots:
    void resizeVariables();
};

class JointConstraintPropertyDialog : public ConstraintPropertyDialog {

  public:
    JointConstraintPropertyDialog(JointConstraint *constraint, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
    void resizeGeneralizedPosition();
  protected:
    ExtWidget *force, *moment, *connections, *independentBody, *dependentBodiesFirstSide, *dependentBodiesSecondSide, *jointForceArrow, *jointMomentArrow;
};

class ExtraDynamicPropertyDialog : public ElementPropertyDialog {

  public:
    ExtraDynamicPropertyDialog(ExtraDynamic *ed, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
};

class SignalProcessingSystemPropertyDialog : public ExtraDynamicPropertyDialog {

  public:
    SignalProcessingSystemPropertyDialog(SignalProcessingSystem *sps, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *signalRef;
};

class LinearTransferSystemPropertyDialog : public SignalProcessingSystemPropertyDialog {

  public:
    LinearTransferSystemPropertyDialog(LinearTransferSystem *lts, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *choice;
};

class LinkPropertyDialog : public ElementPropertyDialog {

  public:
    LinkPropertyDialog(Link *link, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
};

class KineticExcitationPropertyDialog : public LinkPropertyDialog {

  public:
    KineticExcitationPropertyDialog(KineticExcitation *kineticExcitation, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *force, *moment, *connections, *frameOfReference, *forceArrow, *momentArrow;
};

class SpringDamperPropertyDialog : public LinkPropertyDialog {

  public:
    SpringDamperPropertyDialog(SpringDamper *springDamper, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *forceFunction, *connections, *forceDirection, *coilSpring;
};

class JointPropertyDialog : public LinkPropertyDialog {

  public:
    JointPropertyDialog(Joint *joint, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *force, *moment, *connections, *forceArrow, *momentArrow;
};

class ContactPropertyDialog : public LinkPropertyDialog {

  public:
    ContactPropertyDialog(Contact *contact, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *contactForceLaw, *contactImpactLaw, *frictionForceLaw, *frictionImpactLaw, *connections, *enableOpenMBVContactPoints, *normalForceArrow, *frictionArrow;
};

class ActuatorPropertyDialog : public LinkPropertyDialog {

  public:
    ActuatorPropertyDialog(Actuator *kineticExcitation, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *forceDir, *momentDir, *frameOfReference, *inputSignal, *connections, *actuatorForceArrow, *actuatorMomentArrow;
};

class ObserverPropertyDialog : public ElementPropertyDialog {

  public:
    ObserverPropertyDialog(Observer *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class CoordinatesObserverPropertyDialog : public ObserverPropertyDialog {

  public:
    CoordinatesObserverPropertyDialog(CoordinatesObserver *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *frame, *position, *velocity, *acceleration;
};

class CartesianCoordinatesObserverPropertyDialog : public CoordinatesObserverPropertyDialog {

  public:
    CartesianCoordinatesObserverPropertyDialog(CartesianCoordinatesObserver *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class CylinderCoordinatesObserverPropertyDialog : public CoordinatesObserverPropertyDialog {

  public:
    CylinderCoordinatesObserverPropertyDialog(CylinderCoordinatesObserver *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class NaturalCoordinatesObserverPropertyDialog : public CoordinatesObserverPropertyDialog {

  public:
    NaturalCoordinatesObserverPropertyDialog(NaturalCoordinatesObserver *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class KinematicsObserverPropertyDialog : public ObserverPropertyDialog {

  public:
    KinematicsObserverPropertyDialog(KinematicsObserver *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *frame, *position, *velocity, *angularVelocity, *acceleration, *angularAcceleration;
};

class AbsoluteKinematicsObserverPropertyDialog : public KinematicsObserverPropertyDialog {

  public:
    AbsoluteKinematicsObserverPropertyDialog(AbsoluteKinematicsObserver *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class RelativeKinematicsObserverPropertyDialog : public KinematicsObserverPropertyDialog {

  public:
    RelativeKinematicsObserverPropertyDialog(RelativeKinematicsObserver *observer, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *refFrame;
};

class SignalPropertyDialog: public LinkPropertyDialog {

  public:
    SignalPropertyDialog(Signal *signal, QWidget * parent = 0, Qt::WindowFlags f = 0); 
};

class SensorPropertyDialog : public SignalPropertyDialog {

  public:
    SensorPropertyDialog(Sensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0); 
};

class GeneralizedCoordinateSensorPropertyDialog : public SensorPropertyDialog {

  public:
    GeneralizedCoordinateSensorPropertyDialog(GeneralizedCoordinateSensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *object, *index;
};

class GeneralizedPositionSensorPropertyDialog : public GeneralizedCoordinateSensorPropertyDialog {

  public:
    GeneralizedPositionSensorPropertyDialog(GeneralizedPositionSensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class GeneralizedVelocitySensorPropertyDialog : public GeneralizedCoordinateSensorPropertyDialog {

  public:
    GeneralizedVelocitySensorPropertyDialog(GeneralizedVelocitySensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class AbsoluteCoordinateSensorPropertyDialog : public SensorPropertyDialog {

  public:
    AbsoluteCoordinateSensorPropertyDialog(AbsoluteCoordinateSensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *frame, *direction;
};

class AbsolutePositionSensorPropertyDialog : public AbsoluteCoordinateSensorPropertyDialog {

  public:
    AbsolutePositionSensorPropertyDialog(AbsolutePositionSensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class AbsoluteVelocitySensorPropertyDialog : public AbsoluteCoordinateSensorPropertyDialog {

  public:
    AbsoluteVelocitySensorPropertyDialog(AbsoluteVelocitySensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
};

class FunctionSensorPropertyDialog : public SensorPropertyDialog {

  public:
    FunctionSensorPropertyDialog(FunctionSensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *function;
};

class SignalProcessingSystemSensorPropertyDialog : public SensorPropertyDialog {

  public:
    SignalProcessingSystemSensorPropertyDialog(SignalProcessingSystemSensor *sensor, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *spsRef;
};

class SignalAdditionPropertyDialog : public SignalPropertyDialog {

  public:
    SignalAdditionPropertyDialog(SignalAddition *signal, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *signalReferences;
};

class PIDControllerPropertyDialog : public SignalPropertyDialog {

  public:
    PIDControllerPropertyDialog(PIDController *signal, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *sRef, *sdRef, *P, *I, *D;
};

class UnarySignalOperationPropertyDialog : public SignalPropertyDialog {

  public:
    UnarySignalOperationPropertyDialog(UnarySignalOperation *signal, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *sRef, *f;
};

class BinarySignalOperationPropertyDialog : public SignalPropertyDialog {

  public:
    BinarySignalOperationPropertyDialog(BinarySignalOperation *signal, QWidget * parent = 0, Qt::WindowFlags f = 0);
    void toWidget(Element *element);
    void fromWidget(Element *element);
  protected:
    ExtWidget *s1Ref, *s2Ref, *f;
};

#endif
