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
#include "element_property_dialog.h"
#include "basic_widgets.h"
#include "variable_widgets.h"
#include "kinematics_widgets.h"
#include "kinetics_widgets.h"
#include "function_widgets.h"
#include "kinematic_functions_widgets.h"
#include "ombv_widgets.h"
#include "integrator_widgets.h"
#include "extended_widgets.h"
#include "solver.h"
#include "frame.h"
#include "contour.h"
#include "rigidbody.h"
#include "constraint.h"
#include "signal_processing_system.h"
#include "linear_transfer_system.h"
#include "kinetic_excitation.h"
#include "spring_damper.h"
#include "joint.h"
#include "contact.h"
#include "actuator.h"
#include "observer.h"
#include "parameter.h"
#include "integrator.h"
#include "sensor.h"
#include "function_widget_factory.h"
#include <QPushButton>

using namespace std;

class SignalAdditionWidgetFactory : public WidgetFactory {
  public:
    SignalAdditionWidgetFactory(Element *element_) : element(element_) { }
    Widget* createWidget(int i=0);
  protected:
    Element *element;
};

Widget* SignalAdditionWidgetFactory::createWidget(int i) {
  return new SignalReferenceWidget(element);
}

class GearConstraintWidgetFactory : public WidgetFactory {
  public:
    GearConstraintWidgetFactory(Element* element_, QWidget *parent_=0) : element(element_), parent(parent_) { }
    Widget* createWidget(int i=0);
  protected:
    Element *element;
    QWidget *parent;
};

Widget* GearConstraintWidgetFactory::createWidget(int i) {

  ContainerWidget *widget = new ContainerWidget;
  widget->addWidget(new RigidBodyOfReferenceWidget(element,0));

  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new ScalarWidget, QStringList(), 1));
  widget->addWidget(new ExtWidget("Transmission ratio",new ExtPhysicalVarWidget(input)));
  return widget;
}

class RigidBodyOfReferenceWidgetFactory : public WidgetFactory {
  public:
    RigidBodyOfReferenceWidgetFactory(Element* element_, QWidget *parent_=0) : element(element_), parent(parent_) { }
    Widget* createWidget(int i=0);
  protected:
    Element *element;
    QWidget *parent;
};

Widget* RigidBodyOfReferenceWidgetFactory::createWidget(int i) {
  RigidBodyOfReferenceWidget *widget = new RigidBodyOfReferenceWidget(element,0);
  if(parent)
    QObject::connect(widget,SIGNAL(bodyChanged()),parent,SLOT(resizeVariables()));
  return widget;
}

ElementPropertyDialog::ElementPropertyDialog(Element *element_, QWidget *parent, Qt::WindowFlags f, bool embedding) : PropertyDialog(parent,f), element(element_), embed(0) {
  addTab("General");
  name = new ExtWidget("Name",new TextWidget);
  name->setToolTip("Set the name of the element");
  addToTab("General", name);
  if(embedding) {
    addTab("Embedding");
    embed = new ExtWidget("Embed", new EmbedWidget, true);
    addToTab("Embedding",embed);
  }
}

void ElementPropertyDialog::toWidget(Element *element) {
  element->name.toWidget(name);
  if(embed)
    element->embed.toWidget(embed);
}

void ElementPropertyDialog::fromWidget(Element *element) {
  element->name.fromWidget(name);
  if(embed)
    element->embed.fromWidget(embed);
}

FramePropertyDialog::FramePropertyDialog(Frame *frame, QWidget *parent, Qt::WindowFlags f, bool embedding) : ElementPropertyDialog(frame,parent,f,embedding) {
  addTab("Visualisation",1);
  visu = new ExtWidget("OpenMBV frame",new OMBVFrameWidget("NOTSET"),true,true);
  visu->setToolTip("Set the visualisation parameters for the frame");
  addToTab("Visualisation", visu);
}

void FramePropertyDialog::toWidget(Element *element) {
  ElementPropertyDialog::toWidget(element);
  static_cast<Frame*>(element)->visu.toWidget(visu);
}

void FramePropertyDialog::fromWidget(Element *element) {
  ElementPropertyDialog::fromWidget(element);
  static_cast<Frame*>(element)->visu.fromWidget(visu);
}

FixedRelativeFramePropertyDialog::FixedRelativeFramePropertyDialog(FixedRelativeFrame *frame, QWidget *parent, Qt::WindowFlags f) : FramePropertyDialog(frame,parent,f,true) {
  addTab("Kinematics",1);

  position = new ExtWidget("Relative position",new ChoiceWidget2(new VecWidgetFactory(3)),true);
  addToTab("Kinematics", position);

  orientation = new ExtWidget("Relative orientation",new ChoiceWidget2(new RotMatWidgetFactory),true);
  addToTab("Kinematics", orientation);

  refFrame = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(frame,frame),true);
  addToTab("Kinematics", refFrame);
}

void FixedRelativeFramePropertyDialog::toWidget(Element *element) {
  FramePropertyDialog::toWidget(element);
  static_cast<FixedRelativeFrame*>(element)->position.toWidget(position);
  static_cast<FixedRelativeFrame*>(element)->orientation.toWidget(orientation);
  static_cast<FixedRelativeFrame*>(element)->refFrame.toWidget(refFrame);
}

void FixedRelativeFramePropertyDialog::fromWidget(Element *element) {
  FramePropertyDialog::fromWidget(element);
  static_cast<FixedRelativeFrame*>(element)->position.fromWidget(position);
  static_cast<FixedRelativeFrame*>(element)->orientation.fromWidget(orientation);
  static_cast<FixedRelativeFrame*>(element)->refFrame.fromWidget(refFrame);
}

ContourPropertyDialog::ContourPropertyDialog(Contour *contour, QWidget * parent, Qt::WindowFlags f) : ElementPropertyDialog(contour,parent,f) {
  refFrame = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(contour,0),true);
  addToTab("General", refFrame);
}

void ContourPropertyDialog::toWidget(Element *element) {
  ElementPropertyDialog::toWidget(element);
  static_cast<Contour*>(element)->refFrame.toWidget(refFrame);
}

void ContourPropertyDialog::fromWidget(Element *element) {
  ElementPropertyDialog::fromWidget(element);
  static_cast<Contour*>(element)->refFrame.fromWidget(refFrame);
}

PointPropertyDialog::PointPropertyDialog(Point *point, QWidget *parent, Qt::WindowFlags f) : ContourPropertyDialog(point,parent,f) {
  addTab("Visualisation",1);
 
  visu = new ExtWidget("OpenMBV Point",new PointMBSOMBVWidget("NOTSET"),true,true);
  addToTab("Visualisation", visu);
}

void PointPropertyDialog::toWidget(Element *element) {
  ContourPropertyDialog::toWidget(element);
  static_cast<Point*>(element)->visu.toWidget(visu);
}

void PointPropertyDialog::fromWidget(Element *element) {
  ContourPropertyDialog::fromWidget(element);
  static_cast<Point*>(element)->visu.fromWidget(visu);
}

LinePropertyDialog::LinePropertyDialog(Line *line, QWidget *parent, Qt::WindowFlags f) : ContourPropertyDialog(line,parent,f) {
  addTab("Visualisation",1);
 
  visu = new ExtWidget("OpenMBV Line",new LineMBSOMBVWidget("NOTSET"),true,true);
  addToTab("Visualisation", visu);
}

void LinePropertyDialog::toWidget(Element *element) {
  ContourPropertyDialog::toWidget(element);
  static_cast<Line*>(element)->visu.toWidget(visu);
}

void LinePropertyDialog::fromWidget(Element *element) {
  ContourPropertyDialog::fromWidget(element);
  static_cast<Line*>(element)->visu.fromWidget(visu);
}

PlanePropertyDialog::PlanePropertyDialog(Plane *plane, QWidget *parent, Qt::WindowFlags f) : ContourPropertyDialog(plane,parent,f) {
  addTab("Visualisation",1);
 
  visu = new ExtWidget("OpenMBV Plane",new PlaneMBSOMBVWidget("NOTSET"),true,true);
  addToTab("Visualisation", visu);
}

void PlanePropertyDialog::toWidget(Element *element) {
  ContourPropertyDialog::toWidget(element);
  static_cast<Plane*>(element)->visu.toWidget(visu);
}

void PlanePropertyDialog::fromWidget(Element *element) {
  ContourPropertyDialog::fromWidget(element);
  static_cast<Plane*>(element)->visu.fromWidget(visu);
}

SpherePropertyDialog::SpherePropertyDialog(Sphere *sphere, QWidget *parent, Qt::WindowFlags f) : ContourPropertyDialog(sphere,parent,f) {
  addTab("Visualisation",1);
 
  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"), lengthUnits(), 4));
  radius = new ExtWidget("Radius",new ExtPhysicalVarWidget(input));
  addToTab("General", radius);

  visu = new ExtWidget("OpenMBV Sphere",new MBSOMBVWidget("NOTSET"),true,true);
  addToTab("Visualisation", visu);
}

void SpherePropertyDialog::toWidget(Element *element) {
  ContourPropertyDialog::toWidget(element);
  static_cast<Sphere*>(element)->radius.toWidget(radius);
  static_cast<Sphere*>(element)->visu.toWidget(visu);
}

void SpherePropertyDialog::fromWidget(Element *element) {
  ContourPropertyDialog::fromWidget(element);
  static_cast<Sphere*>(element)->radius.fromWidget(radius);
  static_cast<Sphere*>(element)->visu.fromWidget(visu);
}

CircleSolidPropertyDialog::CircleSolidPropertyDialog(CircleSolid *circle, QWidget *parent, Qt::WindowFlags f) : ContourPropertyDialog(circle,parent,f) {
  addTab("Visualisation",1);
 
  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"), lengthUnits(), 4));
  radius = new ExtWidget("Radius",new ExtPhysicalVarWidget(input));
  addToTab("General", radius);

  visu = new ExtWidget("OpenMBV CircleSolid",new MBSOMBVWidget("NOTSET"),true);
  addToTab("Visualisation", visu);
}

void CircleSolidPropertyDialog::toWidget(Element *element) {
  ContourPropertyDialog::toWidget(element);
  static_cast<CircleSolid*>(element)->radius.toWidget(radius);
  static_cast<CircleSolid*>(element)->visu.toWidget(visu);
}

void CircleSolidPropertyDialog::fromWidget(Element *element) {
  ContourPropertyDialog::fromWidget(element);
  static_cast<CircleSolid*>(element)->radius.fromWidget(radius);
  static_cast<CircleSolid*>(element)->visu.fromWidget(visu);
}

GroupPropertyDialog::GroupPropertyDialog(Group *group, QWidget *parent, Qt::WindowFlags f, bool kinematics) : ElementPropertyDialog(group,parent,f), position(0), orientation(0), frameOfReference(0) {
  if(kinematics) {
    addTab("Kinematics",1);

    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new VecWidget(3),lengthUnits(),4));
    position = new ExtWidget("Position",new ExtPhysicalVarWidget(input),true); 
    addToTab("Kinematics", position);

    input.clear();
    input.push_back(new PhysicalVariableWidget(new MatWidget(getEye<QString>(3,3,"1","0")),noUnitUnits(),1));
    orientation = new ExtWidget("Orientation",new ExtPhysicalVarWidget(input),true); 
    addToTab("Kinematics", orientation);

    frameOfReference = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(group,0),true);
    addToTab("Kinematics", frameOfReference);
  }
}

void GroupPropertyDialog::toWidget(Element *element) {
  ElementPropertyDialog::toWidget(element);
  if(position) {
    static_cast<Group*>(element)->position.toWidget(position);
    static_cast<Group*>(element)->orientation.toWidget(orientation);
    static_cast<Group*>(element)->frameOfReference.toWidget(frameOfReference);
  }
}

void GroupPropertyDialog::fromWidget(Element *element) {
  ElementPropertyDialog::fromWidget(element);
  if(position) {
    static_cast<Group*>(element)->position.fromWidget(position);
    static_cast<Group*>(element)->orientation.fromWidget(orientation);
    static_cast<Group*>(element)->frameOfReference.fromWidget(frameOfReference);
  }
}

SolverPropertyDialog::SolverPropertyDialog(Solver *solver, QWidget *parent, Qt::WindowFlags f) : GroupPropertyDialog(solver,parent,f,false) {
  addTab("Environment",1);
  addTab("Solver parameters",2);
  addTab("Extra",3);

//  input.push_back(new PhysicalVariableWidget(new VecWidget(vector<QString>(3)),accelerationUnits(),0));
//  environment = new ExtWidget("Acceleration of gravity",new ExtPhysicalVarWidget(input));
  environment = new ExtWidget("Initial generalized position",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,accelerationUnits()))),true);
  addToTab("Environment", environment);

  solverParameters = new ExtWidget("Solver parameters",new SolverParametersWidget,true); 
  addToTab("Solver parameters",solverParameters);

  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new BoolWidget("1"),QStringList(),1));
  inverseKinetics = new ExtWidget("Inverse kinetics",new ExtPhysicalVarWidget(input),true); 
  addToTab("Extra", inverseKinetics);
}

void SolverPropertyDialog::toWidget(Element *element) {
  GroupPropertyDialog::toWidget(element);
  static_cast<Solver*>(element)->environment.toWidget(environment);
  static_cast<Solver*>(element)->solverParameters.toWidget(solverParameters);
  static_cast<Solver*>(element)->inverseKinetics.toWidget(inverseKinetics);
}

void SolverPropertyDialog::fromWidget(Element *element) {
  GroupPropertyDialog::fromWidget(element);
  static_cast<Solver*>(element)->environment.fromWidget(environment);
  static_cast<Solver*>(element)->solverParameters.fromWidget(solverParameters);
  static_cast<Solver*>(element)->inverseKinetics.fromWidget(inverseKinetics);
}

ObjectPropertyDialog::ObjectPropertyDialog(Object *object, QWidget *parent, Qt::WindowFlags f) : ElementPropertyDialog(object,parent,f) {
}

void ObjectPropertyDialog::toWidget(Element *element) {
  ElementPropertyDialog::toWidget(element);
}

void ObjectPropertyDialog::fromWidget(Element *element) {
  ElementPropertyDialog::fromWidget(element);
}

BodyPropertyDialog::BodyPropertyDialog(Body *body, QWidget *parent, Qt::WindowFlags f) : ObjectPropertyDialog(body,parent,f) {
  addTab("Kinematics");
  addTab("Initial conditions");

  q0 = new ExtWidget("Initial generalized position",new ChoiceWidget2(new VecWidgetFactory(0,vector<QStringList>(3,QStringList()))),true);
  addToTab("Initial conditions", q0);

  u0 = new ExtWidget("Initial generalized velocity",new ChoiceWidget2(new VecWidgetFactory(0,vector<QStringList>(3,QStringList()))),true);
  addToTab("Initial conditions", u0);

  connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));

  R = new ExtWidget("Frame of reference",new FrameOfReferenceWidget(body,0),true);
  addToTab("Kinematics",R);
}

void BodyPropertyDialog::toWidget(Element *element) {
  ObjectPropertyDialog::toWidget(element);
  static_cast<Body*>(element)->q0.toWidget(q0);
  static_cast<Body*>(element)->u0.toWidget(u0);
  static_cast<Body*>(element)->R.toWidget(R);
}

void BodyPropertyDialog::fromWidget(Element *element) {
  ObjectPropertyDialog::fromWidget(element);
  static_cast<Body*>(element)->q0.fromWidget(q0);
  static_cast<Body*>(element)->u0.fromWidget(u0);
  static_cast<Body*>(element)->R.fromWidget(R);
}

RigidBodyPropertyDialog::RigidBodyPropertyDialog(RigidBody *body_, QWidget *parent, Qt::WindowFlags f) : BodyPropertyDialog(body_,parent,f), body(body_) {
  addTab("Visualisation");
  addTab("Extra");

  K = new ExtWidget("Frame for kinematics",new LocalFrameOfReferenceWidget(body,0),true);
  addToTab("Kinematics",K);

  mass = new ExtWidget("Mass",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,massUnits()))));
  addToTab("General",mass);

  inertia = new ExtWidget("Inertia tensor",new ChoiceWidget2(new SymMatWidgetFactory(getEye<QString>(3,3,"0.01","0"),vector<QStringList>(3,inertiaUnits()),vector<int>(3,2))));
  addToTab("General",inertia);

  frameForInertiaTensor = new ExtWidget("Frame for inertia tensor",new LocalFrameOfReferenceWidget(body,0),true);
  addToTab("General",frameForInertiaTensor);

  translation = new ExtWidget("Translation",new ChoiceWidget2(new TranslationWidgetFactory4),true);
  addToTab("Kinematics", translation);

 // connect(static_cast<ExtWidget*>(static_cast<ChoiceWidget*>(translation->getWidget())->getWidget())->getWidget(),SIGNAL(widgetChanged()),this,SLOT(resizeVariables()));
 // connect(translation->getWidget(),SIGNAL(widgetChanged()),this,SLOT(resizeVariables()));
 // connect(static_cast<ChoiceWidget*>(translation->getWidget())->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  rotation = new ExtWidget("Rotation",new ChoiceWidget2(new RotationWidgetFactory4),true);
  addToTab("Kinematics", rotation);

//  connect(static_cast<const ChoiceWidget*>(static_cast<ExtWidget*>(static_cast<ChoiceWidget*>(rotation->getWidget())->getWidget())->getWidget()),SIGNAL(widgetChanged()),this,SLOT(resizeVariables()));
//  connect(rotation->getWidget(),SIGNAL(widgetChanged()),this,SLOT(resizeVariables()));
//  connect(static_cast<ChoiceWidget*>(rotation->getWidget())->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new BoolWidget("0"),QStringList(),1));
  translationDependentRotation = new ExtWidget("Translation dependent rotation",new ExtPhysicalVarWidget(input),true); 
  addToTab("Kinematics", translationDependentRotation);
  input.clear();
  input.push_back(new PhysicalVariableWidget(new BoolWidget("0"),QStringList(),1));
  coordinateTransformationForRotation = new ExtWidget("Coordinate transformation for rotation",new ExtPhysicalVarWidget(input),true); 
  addToTab("Kinematics", coordinateTransformationForRotation);

  ombvEditor = new ExtWidget("OpenMBV body",new OMBVBodySelectionWidget(body),true);
  addToTab("Visualisation", ombvEditor);

  weightArrow = new ExtWidget("OpenMBV weight arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",weightArrow);

  jointForceArrow = new ExtWidget("OpenMBV joint force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",jointForceArrow);

  jointMomentArrow = new ExtWidget("OpenMBV joint moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",jointMomentArrow);

}

void RigidBodyPropertyDialog::toWidget(Element *element) {
  BodyPropertyDialog::toWidget(element);
  static_cast<RigidBody*>(element)->K.toWidget(K);
  static_cast<RigidBody*>(element)->mass.toWidget(mass);
  static_cast<RigidBody*>(element)->inertia.toWidget(inertia);
  static_cast<RigidBody*>(element)->frameForInertiaTensor.toWidget(frameForInertiaTensor);
  static_cast<RigidBody*>(element)->translation.toWidget(translation);
  static_cast<RigidBody*>(element)->rotation.toWidget(rotation);
  static_cast<RigidBody*>(element)->translationDependentRotation.toWidget(translationDependentRotation);
  static_cast<RigidBody*>(element)->coordinateTransformationForRotation.toWidget(coordinateTransformationForRotation);
  static_cast<RigidBody*>(element)->ombvEditor.toWidget(ombvEditor);
  static_cast<RigidBody*>(element)->weightArrow.toWidget(weightArrow);
  static_cast<RigidBody*>(element)->jointForceArrow.toWidget(jointForceArrow);
  static_cast<RigidBody*>(element)->jointMomentArrow.toWidget(jointMomentArrow);
  //resizeVariables();
}

void RigidBodyPropertyDialog::fromWidget(Element *element) {
  BodyPropertyDialog::fromWidget(element);
  static_cast<RigidBody*>(element)->K.fromWidget(K);
  static_cast<RigidBody*>(element)->mass.fromWidget(mass);
  static_cast<RigidBody*>(element)->inertia.fromWidget(inertia);
  static_cast<RigidBody*>(element)->frameForInertiaTensor.fromWidget(frameForInertiaTensor);
  static_cast<RigidBody*>(element)->translation.fromWidget(translation);
  static_cast<RigidBody*>(element)->rotation.fromWidget(rotation);
  static_cast<RigidBody*>(element)->translationDependentRotation.fromWidget(translationDependentRotation);
  static_cast<RigidBody*>(element)->coordinateTransformationForRotation.fromWidget(coordinateTransformationForRotation);
  static_cast<RigidBody*>(element)->ombvEditor.fromWidget(ombvEditor);
  static_cast<RigidBody*>(element)->weightArrow.fromWidget(weightArrow);
  static_cast<RigidBody*>(element)->jointForceArrow.fromWidget(jointForceArrow);
  static_cast<RigidBody*>(element)->jointMomentArrow.fromWidget(jointMomentArrow);
}

int RigidBodyPropertyDialog::getqRelSize() const {
  int nqT=0, nqR=0;
  if(translation->isActive()) {
    ExtWidget *extWidget = static_cast<ExtWidget*>(static_cast<ChoiceWidget2*>(translation->getWidget())->getWidget());
    ChoiceWidget2 *trans = static_cast<ChoiceWidget2*>(extWidget->getWidget());
    nqT = static_cast<FunctionWidget*>(trans->getWidget())->getArg1Size();
  }
  if(rotation->isActive()) {
    ExtWidget *extWidget = static_cast<ExtWidget*>(static_cast<ChoiceWidget2*>(rotation->getWidget())->getWidget());
    ChoiceWidget2 *rot = static_cast<ChoiceWidget2*>(extWidget->getWidget());
    nqR = static_cast<FunctionWidget*>(rot->getWidget())->getArg1Size();
  }
  int nq = nqT + nqR;
  return nq;
}

int RigidBodyPropertyDialog::getuRelSize() const {
  return getqRelSize();
}

void RigidBodyPropertyDialog::resizeGeneralizedPosition() {
  int size =  body->isConstrained() ? 0 : getqRelSize();
  ChoiceWidget2 *choice = static_cast<ChoiceWidget2*>(q0->getWidget());
  choice->resize_(size,1);
}

void RigidBodyPropertyDialog::resizeGeneralizedVelocity() {
  int size =  body->isConstrained() ? 0 : getuRelSize();
  ChoiceWidget2 *choice = static_cast<ChoiceWidget2*>(u0->getWidget());
  choice->resize_(size,1);
}

ConstraintPropertyDialog::ConstraintPropertyDialog(Constraint *constraint, QWidget *parent, Qt::WindowFlags f) : ObjectPropertyDialog(constraint,parent,f) {
}

GearConstraintPropertyDialog::GearConstraintPropertyDialog(GearConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : ConstraintPropertyDialog(constraint,parent,f) {
  addTab("Visualisation");

  dependentBody = new ExtWidget("Dependent body",new RigidBodyOfReferenceWidget(constraint,0));
  addToTab("General", dependentBody);

//  independentBodies = new ExtWidget("Independent bodies",new GearDependenciesWidget(constraint));
//  //connect(dependentBodiesFirstSide_,SIGNAL(bodyChanged()),this,SLOT(resizeVariables()));
//  addToTab("General", independentBodies);

  independentBodies = new ExtWidget("Independent bodies",new ListWidget(new GearConstraintWidgetFactory(constraint,0),"Body"));
  addToTab("General",independentBodies);

  gearForceArrow = new ExtWidget("OpenMBV gear force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",gearForceArrow);

  gearMomentArrow = new ExtWidget("OpenMBV gear moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",gearMomentArrow);

  connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
}

void GearConstraintPropertyDialog::toWidget(Element *element) {
  ConstraintPropertyDialog::toWidget(element);
  static_cast<GearConstraint*>(element)->dependentBody.toWidget(dependentBody);
  static_cast<GearConstraint*>(element)->independentBodies.toWidget(independentBodies);
  static_cast<GearConstraint*>(element)->gearForceArrow.toWidget(gearForceArrow);
  static_cast<GearConstraint*>(element)->gearMomentArrow.toWidget(gearMomentArrow);
}

void GearConstraintPropertyDialog::fromWidget(Element *element) {
  ConstraintPropertyDialog::fromWidget(element);
  RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<GearConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
  if(body)
    body->setConstrained(false);
  static_cast<GearConstraint*>(element)->dependentBody.fromWidget(dependentBody);
  static_cast<GearConstraint*>(element)->independentBodies.fromWidget(independentBodies);
  static_cast<GearConstraint*>(element)->gearForceArrow.fromWidget(gearForceArrow);
  static_cast<GearConstraint*>(element)->gearMomentArrow.fromWidget(gearMomentArrow);
  body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<GearConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
  if(body)
    body->setConstrained(true);
}

KinematicConstraintPropertyDialog::KinematicConstraintPropertyDialog(KinematicConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : ConstraintPropertyDialog(constraint,parent,f) {
  addTab("Visualisation");

  dependentBody = new ExtWidget("Dependent body",new RigidBodyOfReferenceWidget(constraint,0));
  addToTab("General", dependentBody);

  constraintForceArrow = new ExtWidget("OpenMBV constraint force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",constraintForceArrow);

  constraintMomentArrow = new ExtWidget("OpenMBV constraint moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",constraintMomentArrow);
}

void KinematicConstraintPropertyDialog::toWidget(Element *element) {
  ConstraintPropertyDialog::toWidget(element);
  dependentBody->getWidget()->blockSignals(true);
  static_cast<KinematicConstraint*>(element)->dependentBody.toWidget(dependentBody);
  dependentBody->getWidget()->blockSignals(false);
  static_cast<KinematicConstraint*>(element)->constraintForceArrow.toWidget(constraintForceArrow);
  static_cast<KinematicConstraint*>(element)->constraintMomentArrow.toWidget(constraintMomentArrow);
}

void KinematicConstraintPropertyDialog::fromWidget(Element *element) {
  ConstraintPropertyDialog::fromWidget(element);
  RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<KinematicConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
  if(body)
    body->setConstrained(false);
  static_cast<KinematicConstraint*>(element)->dependentBody.fromWidget(dependentBody);
  static_cast<KinematicConstraint*>(element)->constraintForceArrow.fromWidget(constraintForceArrow);
  static_cast<KinematicConstraint*>(element)->constraintMomentArrow.fromWidget(constraintMomentArrow);
  body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<KinematicConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
  if(body)
    body->setConstrained(true);
}

GeneralizedPositionConstraintPropertyDialog::GeneralizedPositionConstraintPropertyDialog(GeneralizedPositionConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : KinematicConstraintPropertyDialog(constraint,parent,f) {

  constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new FunctionWidgetFactory2));
  addToTab("General", constraintFunction);
  connect(constraintFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
}

void GeneralizedPositionConstraintPropertyDialog::resizeVariables() {
  RigidBody *refBody = static_cast<RigidBodyOfReferenceWidget*>(dependentBody->getWidget())->getSelectedBody();
  int size = refBody?refBody->getqRelSize():0;
  static_cast<ChoiceWidget2*>(constraintFunction->getWidget())->resize_(size,1);
}

void GeneralizedPositionConstraintPropertyDialog::toWidget(Element *element) {
  KinematicConstraintPropertyDialog::toWidget(element);
  static_cast<GeneralizedPositionConstraint*>(element)->constraintFunction.toWidget(constraintFunction);
}

void GeneralizedPositionConstraintPropertyDialog::fromWidget(Element *element) {
  KinematicConstraintPropertyDialog::fromWidget(element);
  static_cast<GeneralizedPositionConstraint*>(element)->constraintFunction.fromWidget(constraintFunction);
}

GeneralizedVelocityConstraintPropertyDialog::GeneralizedVelocityConstraintPropertyDialog(GeneralizedVelocityConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : KinematicConstraintPropertyDialog(constraint,parent,f) {
  addTab("Initial conditions");

  constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new ConstraintWidgetFactory));
  addToTab("General", constraintFunction);
  connect(constraintFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  vector<PhysicalVariableWidget*> input;
  x0_ = new VecWidget(0);
  input.push_back(new PhysicalVariableWidget(x0_,QStringList(),1));
  ExtPhysicalVarWidget *var = new ExtPhysicalVarWidget(input);  
  x0 = new ExtWidget("Initial state",var,true);
  addToTab("Initial conditions", x0);

  connect(dependentBody->getWidget(),SIGNAL(bodyChanged()),this,SLOT(resizeVariables()));
  connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
}

void GeneralizedVelocityConstraintPropertyDialog::resizeVariables() {
  //RigidBody *refBody = static_cast<RigidBodyOfReferenceWidget*>(dependentBody->getWidget())->getSelectedBody();
  //int size = refBody?refBody->getqRelSize():0;
  //((ChoiceWidget*)constraintFunction->getWidget())->resize_(size,1);
  //if(x0_ && x0_->size() != size)
  //  x0_->resize_(size);
  //static_cast<FunctionWidget*>(static_cast<ChoiceWidget*>(constraintFunction->getWidget())->getWidget())->setArg1Size(size);
}

void GeneralizedVelocityConstraintPropertyDialog::toWidget(Element *element) {
  KinematicConstraintPropertyDialog::toWidget(element);
  static_cast<GeneralizedVelocityConstraint*>(element)->constraintFunction.toWidget(constraintFunction);
  static_cast<GeneralizedVelocityConstraint*>(element)->x0.toWidget(x0);
}

void GeneralizedVelocityConstraintPropertyDialog::fromWidget(Element *element) {
  KinematicConstraintPropertyDialog::fromWidget(element);
  static_cast<GeneralizedVelocityConstraint*>(element)->constraintFunction.fromWidget(constraintFunction);
  static_cast<GeneralizedVelocityConstraint*>(element)->x0.fromWidget(x0);
}

GeneralizedAccelerationConstraintPropertyDialog::GeneralizedAccelerationConstraintPropertyDialog(GeneralizedAccelerationConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : KinematicConstraintPropertyDialog(constraint,parent,f) {
  addTab("Initial conditions");

  constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new ConstraintWidgetFactory));
  addToTab("General", constraintFunction);
  connect(constraintFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

//  connect((ChoiceWidget*)constraintFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  vector<PhysicalVariableWidget*> input;
  x0_ = new VecWidget(0);
  input.push_back(new PhysicalVariableWidget(x0_,QStringList(),1));
  ExtPhysicalVarWidget *var = new ExtPhysicalVarWidget(input);  
  x0 = new ExtWidget("Initial state",var,true);
  addToTab("Initial conditions", x0);

  connect(dependentBody->getWidget(),SIGNAL(bodyChanged()),this,SLOT(resizeVariables()));
  connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
}

void GeneralizedAccelerationConstraintPropertyDialog::resizeVariables() {
  RigidBody *refBody = static_cast<RigidBodyOfReferenceWidget*>(dependentBody->getWidget())->getSelectedBody();
  int size = refBody?(refBody->getqRelSize()+refBody->getuRelSize()):0;
  static_cast<ChoiceWidget2*>(constraintFunction->getWidget())->resize_(size,1);
  if(x0_ && x0_->size() != size)
    x0_->resize_(size);
  static_cast<FunctionWidget*>(static_cast<ChoiceWidget2*>(constraintFunction->getWidget())->getWidget())->setArg1Size(size);
}

void GeneralizedAccelerationConstraintPropertyDialog::toWidget(Element *element) {
  KinematicConstraintPropertyDialog::toWidget(element);
  static_cast<GeneralizedAccelerationConstraint*>(element)->constraintFunction.toWidget(constraintFunction);
  static_cast<GeneralizedAccelerationConstraint*>(element)->x0.toWidget(x0);
  resizeVariables();
}

void GeneralizedAccelerationConstraintPropertyDialog::fromWidget(Element *element) {
  KinematicConstraintPropertyDialog::fromWidget(element);
  static_cast<GeneralizedAccelerationConstraint*>(element)->constraintFunction.fromWidget(constraintFunction);
  static_cast<GeneralizedAccelerationConstraint*>(element)->x0.fromWidget(x0);
}

JointConstraintPropertyDialog::JointConstraintPropertyDialog(JointConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : ConstraintPropertyDialog(constraint,parent,f) {

  addTab("Kinetics",1);
  addTab("Visualisation");
  addTab("Initial conditions");

  independentBody = new ExtWidget("Independent body",new RigidBodyOfReferenceWidget(constraint,0));
  addToTab("General", independentBody);

  dependentBodiesFirstSide = new ExtWidget("Dependendent bodies first side",new ListWidget(new RigidBodyOfReferenceWidgetFactory(constraint,this),"Body"));
  addToTab("General",dependentBodiesFirstSide);
  connect(dependentBodiesFirstSide->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  dependentBodiesSecondSide = new ExtWidget("Dependendent bodies second side",new ListWidget(new RigidBodyOfReferenceWidgetFactory(constraint,this),"Body"));
  addToTab("General",dependentBodiesSecondSide);
  connect(dependentBodiesSecondSide->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  refFrameID = new ExtWidget("Frame of reference ID",new SpinBoxWidget(0,0,1),true);
  addToTab("Kinetics", refFrameID);

  vector<PhysicalVariableWidget*> input;
  MatColsVarWidget *forceDirection_ = new MatColsVarWidget(3,1,1,3);
  input.push_back(new PhysicalVariableWidget(forceDirection_,noUnitUnits(),1));
  force = new ExtWidget("Force direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics", force);

  input.clear();
  MatColsVarWidget *momentDirection_ = new MatColsVarWidget(3,1,1,3);
  input.push_back(new PhysicalVariableWidget(momentDirection_,noUnitUnits(),1));
  moment = new ExtWidget("Moment direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics", moment);

  connections = new ExtWidget("Connections",new ConnectFramesWidget(2,constraint));
  addToTab("Kinetics", connections);

  jointForceArrow = new ExtWidget("OpenMBV joint force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",jointForceArrow);

  jointMomentArrow = new ExtWidget("OpenMBV joint moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",jointMomentArrow);

  input.clear();
  q0_ = new VecWidget(0);
  input.push_back(new PhysicalVariableWidget(q0_,QStringList(),1));
  ExtPhysicalVarWidget *var = new ExtPhysicalVarWidget(input);  
  q0 = new ExtWidget("Initial guess",var,true);
  addToTab("Initial conditions", q0);

  connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
}

void JointConstraintPropertyDialog::resizeVariables() {
  int size = 0;
  ListWidget *list = static_cast<ListWidget*>(dependentBodiesFirstSide->getWidget());
  for(int i=0; i<list->getSize(); i++) {
    RigidBody *body = static_cast<RigidBodyOfReferenceWidget*>(list->getWidget(i))->getSelectedBody();
    if(body)
      size += body->getqRelSize();
  }
  list = static_cast<ListWidget*>(dependentBodiesSecondSide->getWidget());
  for(int i=0; i<list->getSize(); i++) {
    RigidBody *body = static_cast<RigidBodyOfReferenceWidget*>(list->getWidget(i))->getSelectedBody();
    if(body)
      size += body->getqRelSize();
  }
  if(q0_->size() != size)
    q0_->resize_(size);
}

void JointConstraintPropertyDialog::toWidget(Element *element) {
  ConstraintPropertyDialog::toWidget(element);
  static_cast<JointConstraint*>(element)->independentBody.toWidget(independentBody);
  dependentBodiesFirstSide->getWidget()->blockSignals(true);
  static_cast<JointConstraint*>(element)->dependentBodiesFirstSide.toWidget(dependentBodiesFirstSide);
  dependentBodiesFirstSide->getWidget()->blockSignals(false);
  dependentBodiesSecondSide->getWidget()->blockSignals(true);
  static_cast<JointConstraint*>(element)->dependentBodiesSecondSide.toWidget(dependentBodiesSecondSide);
  dependentBodiesSecondSide->getWidget()->blockSignals(false);
  static_cast<JointConstraint*>(element)->refFrameID.toWidget(refFrameID);
  static_cast<JointConstraint*>(element)->force.toWidget(force);
  static_cast<JointConstraint*>(element)->moment.toWidget(moment);
  static_cast<JointConstraint*>(element)->connections.toWidget(connections);
  static_cast<JointConstraint*>(element)->jointForceArrow.toWidget(jointForceArrow);
  static_cast<JointConstraint*>(element)->jointMomentArrow.toWidget(jointMomentArrow);
  static_cast<JointConstraint*>(element)->q0.toWidget(q0);
  resizeVariables();
}

void JointConstraintPropertyDialog::fromWidget(Element *element) {
  ListProperty *list1 = static_cast<ListProperty*>(static_cast<JointConstraint*>(element)->dependentBodiesFirstSide.getProperty());
  for(int i=0; i<list1->getSize(); i++) {
    RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list1->getProperty(i))->getBodyPtr();
    if(body)
      body->setConstrained(false);
  }
  ListProperty *list2 = static_cast<ListProperty*>(static_cast<JointConstraint*>(element)->dependentBodiesSecondSide.getProperty());
  for(int i=0; i<list2->getSize(); i++) {
    RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list2->getProperty(i))->getBodyPtr();
    if(body)
      body->setConstrained(false);
  }
  static_cast<JointConstraint*>(element)->independentBody.fromWidget(independentBody);
  static_cast<JointConstraint*>(element)->dependentBodiesFirstSide.fromWidget(dependentBodiesFirstSide);
  static_cast<JointConstraint*>(element)->dependentBodiesSecondSide.fromWidget(dependentBodiesSecondSide);
  static_cast<JointConstraint*>(element)->refFrameID.fromWidget(refFrameID);
  static_cast<JointConstraint*>(element)->force.fromWidget(force);
  static_cast<JointConstraint*>(element)->moment.fromWidget(moment);
  static_cast<JointConstraint*>(element)->connections.fromWidget(connections);
  static_cast<JointConstraint*>(element)->jointForceArrow.fromWidget(jointForceArrow);
  static_cast<JointConstraint*>(element)->jointMomentArrow.fromWidget(jointMomentArrow);
  for(int i=0; i<list1->getSize(); i++) {
    RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list1->getProperty(i))->getBodyPtr();
    if(body)
      body->setConstrained(true);
  }
  for(int i=0; i<list2->getSize(); i++) {
    RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list2->getProperty(i))->getBodyPtr();
    if(body)
      body->setConstrained(true);
  }
  static_cast<JointConstraint*>(element)->q0.fromWidget(q0);
}

SignalProcessingSystemPropertyDialog::SignalProcessingSystemPropertyDialog(SignalProcessingSystem *sps, QWidget * parent, Qt::WindowFlags f) : LinkPropertyDialog(sps,parent,f) {
  signalRef = new ExtWidget("Input signal",new SignalOfReferenceWidget(sps,0));
  addToTab("General", signalRef);
}

void SignalProcessingSystemPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<SignalProcessingSystem*>(element)->signalRef.toWidget(signalRef);
}

void SignalProcessingSystemPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<SignalProcessingSystem*>(element)->signalRef.fromWidget(signalRef);
}

LinkPropertyDialog::LinkPropertyDialog(Link *link, QWidget *parent, Qt::WindowFlags f) : ElementPropertyDialog(link,parent,f) {
}

void LinkPropertyDialog::toWidget(Element *element) {
  ElementPropertyDialog::toWidget(element);
}

void LinkPropertyDialog::fromWidget(Element *element) {
  ElementPropertyDialog::fromWidget(element);
}

KineticExcitationPropertyDialog::KineticExcitationPropertyDialog(KineticExcitation *kineticExcitation, QWidget *parent, Qt::WindowFlags wf) : LinkPropertyDialog(kineticExcitation,parent,wf) {

  addTab("Kinetics",1);
  addTab("Visualisation",2);

  QStringList names;
  names << "Frame 1" << "Frame 2";
  refFrameID = new ExtWidget("Frame of reference ID",new ComboBoxWidget(names,1),true);
  addToTab("Kinetics", refFrameID);

  vector<PhysicalVariableWidget*> input;
  MatColsVarWidget *forceDirection_ = new MatColsVarWidget(3,1,1,3);
  input.push_back(new PhysicalVariableWidget(forceDirection_,noUnitUnits(),1));
  forceDirection = new ExtWidget("Force direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics",forceDirection);

  connect(forceDirection->getWidget(),SIGNAL(inputDialogChanged(int)),this,SLOT(resizeVariables()));
  connect(forceDirection_, SIGNAL(sizeChanged(int)), this, SLOT(resizeVariables()));

  forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new FunctionWidgetFactory2),true);
  addToTab("Kinetics",forceFunction);

  connect(forceFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  input.clear();
  MatColsVarWidget *momentDirection_ = new MatColsVarWidget(3,1,1,3);
  input.push_back(new PhysicalVariableWidget(momentDirection_,noUnitUnits(),1));
  momentDirection = new ExtWidget("Moment direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics",momentDirection);

  connect(momentDirection->getWidget(),SIGNAL(inputDialogChanged(int)),this,SLOT(resizeVariables()));
  connect(momentDirection_, SIGNAL(sizeChanged(int)), this, SLOT(resizeVariables()));

  momentFunction = new ExtWidget("Moment function",new ChoiceWidget2(new FunctionWidgetFactory2),true);
  addToTab("Kinetics",momentFunction);

  connect(momentFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

  connections = new ExtWidget("Connections",new ChoiceWidget2(new ConnectFramesWidgetFactory(kineticExcitation)));
  addToTab("Kinetics",connections);

  forceArrow = new ExtWidget("OpenMBV force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",forceArrow);

  momentArrow = new ExtWidget("OpenMBV moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",momentArrow);

  connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
}

void KineticExcitationPropertyDialog::resizeVariables() {
  int size = static_cast<MatColsVarWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ExtPhysicalVarWidget*>(forceDirection->getWidget())->getCurrentPhysicalVariableWidget())->getWidget())->cols();
  cout << size << endl;
  static_cast<ChoiceWidget2*>(forceFunction->getWidget())->resize_(size,1);
}

void KineticExcitationPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<KineticExcitation*>(element)->refFrameID.toWidget(refFrameID);
  static_cast<KineticExcitation*>(element)->forceDirection.toWidget(forceDirection);
  static_cast<KineticExcitation*>(element)->forceFunction.toWidget(forceFunction);
  static_cast<KineticExcitation*>(element)->momentDirection.toWidget(momentDirection);
  static_cast<KineticExcitation*>(element)->momentFunction.toWidget(momentFunction);
  static_cast<KineticExcitation*>(element)->connections.toWidget(connections);
  static_cast<KineticExcitation*>(element)->forceArrow.toWidget(forceArrow);
  static_cast<KineticExcitation*>(element)->momentArrow.toWidget(momentArrow);
}

void KineticExcitationPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<KineticExcitation*>(element)->refFrameID.fromWidget(refFrameID);
  static_cast<KineticExcitation*>(element)->forceDirection.fromWidget(forceDirection);
  static_cast<KineticExcitation*>(element)->forceFunction.fromWidget(forceFunction);
  static_cast<KineticExcitation*>(element)->momentDirection.fromWidget(momentDirection);
  static_cast<KineticExcitation*>(element)->momentFunction.fromWidget(momentFunction);
  static_cast<KineticExcitation*>(element)->connections.fromWidget(connections);
  static_cast<KineticExcitation*>(element)->forceArrow.fromWidget(forceArrow);
  static_cast<KineticExcitation*>(element)->momentArrow.fromWidget(momentArrow);
}

SpringDamperPropertyDialog::SpringDamperPropertyDialog(SpringDamper *springDamper, QWidget *parent, Qt::WindowFlags f) : LinkPropertyDialog(springDamper,parent,f) {
  addTab("Kinetics",1);
  addTab("Visualisation",2);

  forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new SpringDamperWidgetFactory));
  addToTab("Kinetics", forceFunction);

  connections = new ExtWidget("Connections",new ConnectFramesWidget(2,springDamper));
  addToTab("Kinetics", connections);

  coilSpring = new ExtWidget("OpenMBV coil spring",new OMBVCoilSpringWidget("NOTSET"),true);
  addToTab("Visualisation", coilSpring);

  forceArrow = new ExtWidget("OpenMBV force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation", forceArrow);
}

void SpringDamperPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<SpringDamper*>(element)->forceFunction.toWidget(forceFunction);
  static_cast<SpringDamper*>(element)->connections.toWidget(connections);
  static_cast<SpringDamper*>(element)->coilSpring.toWidget(coilSpring);
  static_cast<SpringDamper*>(element)->forceArrow.toWidget(forceArrow);
}

void SpringDamperPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<SpringDamper*>(element)->forceFunction.fromWidget(forceFunction);
  static_cast<SpringDamper*>(element)->connections.fromWidget(connections);
  static_cast<SpringDamper*>(element)->coilSpring.fromWidget(coilSpring);
  static_cast<SpringDamper*>(element)->forceArrow.fromWidget(forceArrow);
}

DirectionalSpringDamperPropertyDialog::DirectionalSpringDamperPropertyDialog(DirectionalSpringDamper *springDamper, QWidget *parent, Qt::WindowFlags f) : LinkPropertyDialog(springDamper,parent,f) {
  addTab("Kinetics",1);
  addTab("Visualisation",2);

  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new VecWidget(3),noUnitUnits(),1));
  forceDirection = new ExtWidget("Force direction",new ExtPhysicalVarWidget(input));
  addToTab("Kinetics", forceDirection);

  forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new SpringDamperWidgetFactory));
  addToTab("Kinetics", forceFunction);

  connections = new ExtWidget("Connections",new ConnectFramesWidget(2,springDamper));
  addToTab("Kinetics", connections);

  coilSpring = new ExtWidget("OpenMBV coil spring",new OMBVCoilSpringWidget("NOTSET"),true);
  addToTab("Visualisation", coilSpring);

  forceArrow = new ExtWidget("OpenMBV force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation", forceArrow);
}

void DirectionalSpringDamperPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<DirectionalSpringDamper*>(element)->forceFunction.toWidget(forceFunction);
  static_cast<DirectionalSpringDamper*>(element)->forceDirection.toWidget(forceDirection);
  static_cast<DirectionalSpringDamper*>(element)->connections.toWidget(connections);
  static_cast<DirectionalSpringDamper*>(element)->coilSpring.toWidget(coilSpring);
  static_cast<DirectionalSpringDamper*>(element)->forceArrow.toWidget(forceArrow);
}

void DirectionalSpringDamperPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<DirectionalSpringDamper*>(element)->forceFunction.fromWidget(forceFunction);
  static_cast<DirectionalSpringDamper*>(element)->forceDirection.fromWidget(forceDirection);
  static_cast<DirectionalSpringDamper*>(element)->connections.fromWidget(connections);
  static_cast<DirectionalSpringDamper*>(element)->coilSpring.fromWidget(coilSpring);
  static_cast<DirectionalSpringDamper*>(element)->forceArrow.fromWidget(forceArrow);
}

GeneralizedSpringDamperPropertyDialog::GeneralizedSpringDamperPropertyDialog(GeneralizedSpringDamper *springDamper, QWidget *parent, Qt::WindowFlags f) : LinkPropertyDialog(springDamper,parent,f) {
  addTab("Kinetics",1);
  addTab("Visualisation",2);

  function = new ExtWidget("GeneralizedForceFunction",new ChoiceWidget2(new SpringDamperWidgetFactory));
  addToTab("Kinetics", function);

  body = new ExtWidget("Rigid body",new RigidBodyOfReferenceWidget(springDamper,0));
  addToTab("General", body);

  coilSpring = new ExtWidget("OpenMBV coil spring",new OMBVCoilSpringWidget("NOTSET"),true);
  addToTab("Visualisation", coilSpring);

  forceArrow = new ExtWidget("OpenMBV force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation", forceArrow);

  momentArrow = new ExtWidget("OpenMBV moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation", momentArrow);
}

void GeneralizedSpringDamperPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<GeneralizedSpringDamper*>(element)->function.toWidget(function);
  static_cast<GeneralizedSpringDamper*>(element)->body.toWidget(body);
  static_cast<GeneralizedSpringDamper*>(element)->coilSpring.toWidget(coilSpring);
  static_cast<GeneralizedSpringDamper*>(element)->forceArrow.toWidget(forceArrow);
  static_cast<GeneralizedSpringDamper*>(element)->momentArrow.toWidget(momentArrow);
}

void GeneralizedSpringDamperPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<GeneralizedSpringDamper*>(element)->function.fromWidget(function);
  static_cast<GeneralizedSpringDamper*>(element)->body.fromWidget(body);
  static_cast<GeneralizedSpringDamper*>(element)->coilSpring.fromWidget(coilSpring);
  static_cast<GeneralizedSpringDamper*>(element)->forceArrow.fromWidget(forceArrow);
  static_cast<GeneralizedSpringDamper*>(element)->momentArrow.fromWidget(momentArrow);
}

JointPropertyDialog::JointPropertyDialog(Joint *joint, QWidget *parent, Qt::WindowFlags f) : LinkPropertyDialog(joint,parent,f) {
  addTab("Kinetics",1);
  addTab("Visualisation",2);

  refFrameID = new ExtWidget("Frame of reference ID",new SpinBoxWidget(0,0,1),true);
  addToTab("Kinetics", refFrameID);

  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new MatColsVarWidget(3,1,1,3),noUnitUnits(),1));
  forceDirection = new ExtWidget("Force direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics", forceDirection);

  forceLaw = new ExtWidget("Force law",new GeneralizedForceLawChoiceWidget,true);
  addToTab("Kinetics", forceLaw);

  input.clear();
  input.push_back(new PhysicalVariableWidget(new MatColsVarWidget(3,1,1,3),noUnitUnits(),1));
  momentDirection = new ExtWidget("Moment direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics", momentDirection);

  momentLaw = new ExtWidget("Moment law",new GeneralizedForceLawChoiceWidget,true);
  addToTab("Kinetics", momentLaw);

  connections = new ExtWidget("Connections",new ConnectFramesWidget(2,joint));
  addToTab("Kinetics", connections);

  forceArrow = new ExtWidget("OpenMBV force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",forceArrow);

  momentArrow = new ExtWidget("OpenMBV moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",momentArrow);
}

void JointPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<Joint*>(element)->refFrameID.toWidget(refFrameID);
  static_cast<Joint*>(element)->forceDirection.toWidget(forceDirection);
  static_cast<Joint*>(element)->forceLaw.toWidget(forceLaw);
  static_cast<Joint*>(element)->momentDirection.toWidget(momentDirection);
  static_cast<Joint*>(element)->momentLaw.toWidget(momentLaw);
  static_cast<Joint*>(element)->connections.toWidget(connections);
  static_cast<Joint*>(element)->forceArrow.toWidget(forceArrow);
  static_cast<Joint*>(element)->momentArrow.toWidget(momentArrow);
}

void JointPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<Joint*>(element)->refFrameID.fromWidget(refFrameID);
  static_cast<Joint*>(element)->forceDirection.fromWidget(forceDirection);
  static_cast<Joint*>(element)->forceLaw.fromWidget(forceLaw);
  static_cast<Joint*>(element)->momentDirection.fromWidget(momentDirection);
  static_cast<Joint*>(element)->momentLaw.fromWidget(momentLaw);
  static_cast<Joint*>(element)->connections.fromWidget(connections);
  static_cast<Joint*>(element)->forceArrow.fromWidget(forceArrow);
  static_cast<Joint*>(element)->momentArrow.fromWidget(momentArrow);
}

ContactPropertyDialog::ContactPropertyDialog(Contact *contact, QWidget *parent, Qt::WindowFlags f) : LinkPropertyDialog(contact,parent,f) {

  addTab("Kinetics",1);
  addTab("Visualisation",2);

  connections = new ExtWidget("Connections",new ConnectContoursWidget(2,contact));
  addToTab("Kinetics", connections);

  contactForceLaw = new ExtWidget("Normal force law",new GeneralizedForceLawChoiceWidget);
  addToTab("Kinetics", contactForceLaw);

  contactImpactLaw = new ExtWidget("Normal impact law",new GeneralizedImpactLawChoiceWidget,true);
  addToTab("Kinetics", contactImpactLaw);

  frictionForceLaw = new ExtWidget("Tangential force law",new FrictionForceLawChoiceWidget,true);
  addToTab("Kinetics", frictionForceLaw);

  frictionImpactLaw = new ExtWidget("Tangential impact law",new FrictionImpactLawChoiceWidget,true);
  addToTab("Kinetics", frictionImpactLaw);

//  vector<PhysicalVariableWidget*> input;
//  input.push_back(new PhysicalVariableWidget(new ScalarWidget("0.1"),lengthUnits(),4));
//  enableOpenMBVContactPoints = new ExtWidget("OpenMBV contact points",new ExtPhysicalVarWidget(input),true); 
  enableOpenMBVContactPoints = new ExtWidget("OpenMBV contact points",new OMBVFrameWidget("NOTSET"),true,true);
  addToTab("Visualisation",enableOpenMBVContactPoints);

  normalForceArrow = new ExtWidget("OpenMBV normal force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",normalForceArrow);

  frictionArrow = new ExtWidget("OpenMBV tangential force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",frictionArrow);
}

void ContactPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<Contact*>(element)->contactForceLaw.toWidget(contactForceLaw);
  static_cast<Contact*>(element)->contactImpactLaw.toWidget(contactImpactLaw);
  static_cast<Contact*>(element)->frictionForceLaw.toWidget(frictionForceLaw);
  static_cast<Contact*>(element)->frictionImpactLaw.toWidget(frictionImpactLaw);
  static_cast<Contact*>(element)->connections.toWidget(connections);
  static_cast<Contact*>(element)->enableOpenMBVContactPoints.toWidget(enableOpenMBVContactPoints);
  static_cast<Contact*>(element)->normalForceArrow.toWidget(normalForceArrow);
  static_cast<Contact*>(element)->frictionArrow.toWidget(frictionArrow);
}

void ContactPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<Contact*>(element)->contactForceLaw.fromWidget(contactForceLaw);
  static_cast<Contact*>(element)->contactImpactLaw.fromWidget(contactImpactLaw);
  static_cast<Contact*>(element)->frictionForceLaw.fromWidget(frictionForceLaw);
  static_cast<Contact*>(element)->frictionImpactLaw.fromWidget(frictionImpactLaw);
  static_cast<Contact*>(element)->connections.fromWidget(connections);
  static_cast<Contact*>(element)->enableOpenMBVContactPoints.fromWidget(enableOpenMBVContactPoints);
  static_cast<Contact*>(element)->normalForceArrow.fromWidget(normalForceArrow);
  static_cast<Contact*>(element)->frictionArrow.fromWidget(frictionArrow);
}

ActuatorPropertyDialog::ActuatorPropertyDialog(Actuator *actuator, QWidget *parent, Qt::WindowFlags wf) : LinkPropertyDialog(actuator,parent,wf) {

  addTab("Kinetics",1);
  addTab("Visualisation",2);

  vector<PhysicalVariableWidget*> input;
  MatColsVarWidget *forceDirection_ = new MatColsVarWidget(3,1,1,3);
  input.push_back(new PhysicalVariableWidget(forceDirection_,noUnitUnits(),1));
  forceDir = new ExtWidget("Force direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics", forceDir);

  input.clear();
  MatColsVarWidget *momentDirection_ = new MatColsVarWidget(3,1,1,3);
  input.push_back(new PhysicalVariableWidget(momentDirection_,noUnitUnits(),1));
  momentDir = new ExtWidget("Moment direction",new ExtPhysicalVarWidget(input),true);
  addToTab("Kinetics", momentDir);

  connections = new ExtWidget("Connections",new ConnectFramesWidget(2,actuator));
  addToTab("Kinetics",connections);

  frameOfReference = new ExtWidget("Frame of reference",new SpinBoxWidget(1,1,2),true); 
  addToTab("Kinetics", frameOfReference);

  inputSignal = new ExtWidget("Input signal",new SignalOfReferenceWidget(actuator,0));
  addToTab("Kinetics",inputSignal);

  actuatorForceArrow = new ExtWidget("OpenMBV actuator force arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",actuatorForceArrow);

  actuatorMomentArrow = new ExtWidget("OpenMBV actuator moment arrow",new OMBVArrowWidget("NOTSET"),true);
  addToTab("Visualisation",actuatorMomentArrow);
}

void ActuatorPropertyDialog::toWidget(Element *element) {
  LinkPropertyDialog::toWidget(element);
  static_cast<Actuator*>(element)->forceDir.toWidget(forceDir);
  static_cast<Actuator*>(element)->momentDir.toWidget(momentDir);
  static_cast<Actuator*>(element)->frameOfReference.toWidget(frameOfReference);
  static_cast<Actuator*>(element)->connections.toWidget(connections);
  static_cast<Actuator*>(element)->inputSignal.toWidget(inputSignal);
  static_cast<Actuator*>(element)->actuatorForceArrow.toWidget(actuatorForceArrow);
  static_cast<Actuator*>(element)->actuatorMomentArrow.toWidget(actuatorMomentArrow);
}

void ActuatorPropertyDialog::fromWidget(Element *element) {
  LinkPropertyDialog::fromWidget(element);
  static_cast<Actuator*>(element)->forceDir.fromWidget(forceDir);
  static_cast<Actuator*>(element)->momentDir.fromWidget(momentDir);
  static_cast<Actuator*>(element)->frameOfReference.fromWidget(frameOfReference);
  static_cast<Actuator*>(element)->connections.fromWidget(connections);
  static_cast<Actuator*>(element)->inputSignal.fromWidget(inputSignal);
  static_cast<Actuator*>(element)->actuatorForceArrow.fromWidget(actuatorForceArrow);
  static_cast<Actuator*>(element)->actuatorMomentArrow.fromWidget(actuatorMomentArrow);
}

ObserverPropertyDialog::ObserverPropertyDialog(Observer *observer, QWidget * parent, Qt::WindowFlags f) : ElementPropertyDialog(observer,parent,f) {
}

CoordinatesObserverPropertyDialog::CoordinatesObserverPropertyDialog(CoordinatesObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

  addTab("Visualisation",1);

  frame = new ExtWidget("Frame",new FrameOfReferenceWidget(observer,0));
  addToTab("General", frame);

  position = new ExtWidget("OpenMBV position arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",position);

  velocity = new ExtWidget("OpenMBV velocity arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",velocity);

  acceleration = new ExtWidget("OpenMBV acceleration arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",acceleration);
}

void CoordinatesObserverPropertyDialog::toWidget(Element *element) {
  ObserverPropertyDialog::toWidget(element);
  static_cast<CoordinatesObserver*>(element)->frame.toWidget(frame);
  static_cast<CoordinatesObserver*>(element)->position.toWidget(position);
  static_cast<CoordinatesObserver*>(element)->velocity.toWidget(velocity);
  static_cast<CoordinatesObserver*>(element)->acceleration.toWidget(acceleration);
}

void CoordinatesObserverPropertyDialog::fromWidget(Element *element) {
  ObserverPropertyDialog::fromWidget(element);
  static_cast<CoordinatesObserver*>(element)->frame.fromWidget(frame);
  static_cast<CoordinatesObserver*>(element)->position.fromWidget(position);
  static_cast<CoordinatesObserver*>(element)->velocity.fromWidget(velocity);
  static_cast<CoordinatesObserver*>(element)->acceleration.fromWidget(acceleration);
}

CartesianCoordinatesObserverPropertyDialog::CartesianCoordinatesObserverPropertyDialog(CartesianCoordinatesObserver *observer, QWidget *parent, Qt::WindowFlags f) : CoordinatesObserverPropertyDialog(observer,parent,f) {
}

CylinderCoordinatesObserverPropertyDialog::CylinderCoordinatesObserverPropertyDialog(CylinderCoordinatesObserver *observer, QWidget *parent, Qt::WindowFlags f) : CoordinatesObserverPropertyDialog(observer,parent,f) {
}

NaturalCoordinatesObserverPropertyDialog::NaturalCoordinatesObserverPropertyDialog(NaturalCoordinatesObserver *observer, QWidget *parent, Qt::WindowFlags f) : CoordinatesObserverPropertyDialog(observer,parent,f) {
}

KinematicsObserverPropertyDialog::KinematicsObserverPropertyDialog(KinematicsObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

  addTab("Visualisation",1);

  frame = new ExtWidget("Frame",new FrameOfReferenceWidget(observer,0));
  addToTab("General", frame);

  position = new ExtWidget("OpenMBV position arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",position);

  velocity = new ExtWidget("OpenMBV velocity arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",velocity);

  angularVelocity = new ExtWidget("OpenMBV angular velocity arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",angularVelocity);

  acceleration = new ExtWidget("OpenMBV acceleration arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",acceleration);

  angularAcceleration = new ExtWidget("OpenMBV angular acceleration arrow",new OMBVArrowWidget("NOTSET",true),true);
  addToTab("Visualisation",angularAcceleration);
}

void KinematicsObserverPropertyDialog::toWidget(Element *element) {
  ObserverPropertyDialog::toWidget(element);
  static_cast<KinematicsObserver*>(element)->frame.toWidget(frame);
  static_cast<KinematicsObserver*>(element)->position.toWidget(position);
  static_cast<KinematicsObserver*>(element)->velocity.toWidget(velocity);
  static_cast<KinematicsObserver*>(element)->angularVelocity.toWidget(angularVelocity);
  static_cast<KinematicsObserver*>(element)->acceleration.toWidget(acceleration);
  static_cast<KinematicsObserver*>(element)->angularAcceleration.toWidget(angularAcceleration);
}

void KinematicsObserverPropertyDialog::fromWidget(Element *element) {
  ObserverPropertyDialog::fromWidget(element);
  static_cast<KinematicsObserver*>(element)->frame.fromWidget(frame);
  static_cast<KinematicsObserver*>(element)->position.fromWidget(position);
  static_cast<KinematicsObserver*>(element)->velocity.fromWidget(velocity);
  static_cast<KinematicsObserver*>(element)->angularVelocity.fromWidget(angularVelocity);
  static_cast<KinematicsObserver*>(element)->acceleration.fromWidget(acceleration);
  static_cast<KinematicsObserver*>(element)->angularAcceleration.fromWidget(angularAcceleration);
}

AbsoluteKinematicsObserverPropertyDialog::AbsoluteKinematicsObserverPropertyDialog(AbsoluteKinematicsObserver *observer, QWidget *parent, Qt::WindowFlags f) : KinematicsObserverPropertyDialog(observer,parent,f) {
}

RelativeKinematicsObserverPropertyDialog::RelativeKinematicsObserverPropertyDialog(RelativeKinematicsObserver *observer, QWidget *parent, Qt::WindowFlags f) : KinematicsObserverPropertyDialog(observer,parent,f) {

  refFrame = new ExtWidget("Reference frame",new FrameOfReferenceWidget(observer,0));
  addToTab("General", refFrame);
}

void RelativeKinematicsObserverPropertyDialog::toWidget(Element *element) {
  KinematicsObserverPropertyDialog::toWidget(element);
  static_cast<RelativeKinematicsObserver*>(element)->refFrame.toWidget(refFrame);
}

void RelativeKinematicsObserverPropertyDialog::fromWidget(Element *element) {
  KinematicsObserverPropertyDialog::fromWidget(element);
  static_cast<RelativeKinematicsObserver*>(element)->refFrame.fromWidget(refFrame);
}

SignalPropertyDialog::SignalPropertyDialog(Signal *signal, QWidget * parent, Qt::WindowFlags f) : LinkPropertyDialog(signal,parent,f) {
}

SensorPropertyDialog::SensorPropertyDialog(Sensor *sensor, QWidget * parent, Qt::WindowFlags f) : SignalPropertyDialog(sensor,parent,f) {
}

GeneralizedCoordinateSensorPropertyDialog::GeneralizedCoordinateSensorPropertyDialog(GeneralizedCoordinateSensor *sensor, QWidget * parent, Qt::WindowFlags f) : SensorPropertyDialog(sensor,parent,f) {
  object = new ExtWidget("Object of reference",new ObjectOfReferenceWidget(sensor,0));
  addToTab("General", object);

  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"), QStringList(), 0));
  index = new ExtWidget("Index",new ExtPhysicalVarWidget(input));
  addToTab("General", index);
}

void GeneralizedCoordinateSensorPropertyDialog::toWidget(Element *element) {
  SensorPropertyDialog::toWidget(element);
  static_cast<GeneralizedCoordinateSensor*>(element)->object.toWidget(object);
  static_cast<GeneralizedCoordinateSensor*>(element)->index.toWidget(index);
}

void GeneralizedCoordinateSensorPropertyDialog::fromWidget(Element *element) {
  SensorPropertyDialog::fromWidget(element);
  static_cast<GeneralizedCoordinateSensor*>(element)->object.fromWidget(object);
  static_cast<GeneralizedCoordinateSensor*>(element)->index.fromWidget(index);
}

GeneralizedPositionSensorPropertyDialog::GeneralizedPositionSensorPropertyDialog(GeneralizedPositionSensor *sensor, QWidget * parent, Qt::WindowFlags f) : GeneralizedCoordinateSensorPropertyDialog(sensor,parent,f) {
}

GeneralizedVelocitySensorPropertyDialog::GeneralizedVelocitySensorPropertyDialog(GeneralizedVelocitySensor *sensor, QWidget * parent, Qt::WindowFlags f) : GeneralizedCoordinateSensorPropertyDialog(sensor,parent,f) {
}

AbsoluteCoordinateSensorPropertyDialog::AbsoluteCoordinateSensorPropertyDialog(AbsoluteCoordinateSensor *sensor, QWidget * parent, Qt::WindowFlags f) : SensorPropertyDialog(sensor,parent,f) {
  frame = new ExtWidget("Frame of reference",new FrameOfReferenceWidget(sensor,0));
  addToTab("General", frame);

  vector<PhysicalVariableWidget*> input;
  MatColsVarWidget *forceDirection_ = new MatColsVarWidget(3,1,1,3);
  input.push_back(new PhysicalVariableWidget(forceDirection_,noUnitUnits(),1));
  direction = new ExtWidget("Direction",new ExtPhysicalVarWidget(input),true);
  addToTab("General", direction);
}

void AbsoluteCoordinateSensorPropertyDialog::toWidget(Element *element) {
  SensorPropertyDialog::toWidget(element);
  static_cast<AbsoluteCoordinateSensor*>(element)->frame.toWidget(frame);
  static_cast<AbsoluteCoordinateSensor*>(element)->direction.toWidget(direction);
}

void AbsoluteCoordinateSensorPropertyDialog::fromWidget(Element *element) {
  SensorPropertyDialog::fromWidget(element);
  static_cast<AbsoluteCoordinateSensor*>(element)->frame.fromWidget(frame);
  static_cast<AbsoluteCoordinateSensor*>(element)->direction.fromWidget(direction);
}

AbsolutePositionSensorPropertyDialog::AbsolutePositionSensorPropertyDialog(AbsolutePositionSensor *sensor, QWidget * parent, Qt::WindowFlags f) : AbsoluteCoordinateSensorPropertyDialog(sensor,parent,f) {
}

AbsoluteVelocitySensorPropertyDialog::AbsoluteVelocitySensorPropertyDialog(AbsoluteVelocitySensor *sensor, QWidget * parent, Qt::WindowFlags f) : AbsoluteCoordinateSensorPropertyDialog(sensor,parent,f) {
}

FunctionSensorPropertyDialog::FunctionSensorPropertyDialog(FunctionSensor *sensor, QWidget * parent, Qt::WindowFlags f) : SensorPropertyDialog(sensor,parent,f) {
  function = new ExtWidget("Function",new ChoiceWidget2(new FunctionWidgetFactory2));
  addToTab("General", function);
}

void FunctionSensorPropertyDialog::toWidget(Element *element) {
  SensorPropertyDialog::toWidget(element);
  static_cast<FunctionSensor*>(element)->function.toWidget(function);
}

void FunctionSensorPropertyDialog::fromWidget(Element *element) {
  SensorPropertyDialog::fromWidget(element);
  static_cast<FunctionSensor*>(element)->function.fromWidget(function);
}

SignalProcessingSystemSensorPropertyDialog::SignalProcessingSystemSensorPropertyDialog(SignalProcessingSystemSensor *sensor, QWidget * parent, Qt::WindowFlags f) : SensorPropertyDialog(sensor,parent,f) {
  //spsRef = new ExtWidget("Signal processing system",new LinkOfReferenceWidget(sensor,0));
  addToTab("General", spsRef);
}

void SignalProcessingSystemSensorPropertyDialog::toWidget(Element *element) {
  SensorPropertyDialog::toWidget(element);
  static_cast<SignalProcessingSystemSensor*>(element)->spsRef.toWidget(spsRef);
}

void SignalProcessingSystemSensorPropertyDialog::fromWidget(Element *element) {
  SensorPropertyDialog::fromWidget(element);
  static_cast<SignalProcessingSystemSensor*>(element)->spsRef.fromWidget(spsRef);
}

SignalAdditionPropertyDialog::SignalAdditionPropertyDialog(SignalAddition *signal, QWidget * parent, Qt::WindowFlags f) : SignalPropertyDialog(signal,parent,f) {
  signalReferences = new ExtWidget("Signal references",new ListWidget(new SignalAdditionWidgetFactory(signal),"Signal"));
  addToTab("General", signalReferences);
}

void SignalAdditionPropertyDialog::toWidget(Element *element) {
  SignalPropertyDialog::toWidget(element);
  static_cast<SignalAddition*>(element)->signalReferences.toWidget(signalReferences);
}

void SignalAdditionPropertyDialog::fromWidget(Element *element) {
  SignalPropertyDialog::fromWidget(element);
  static_cast<SignalAddition*>(element)->signalReferences.fromWidget(signalReferences);
}

PIDControllerPropertyDialog::PIDControllerPropertyDialog(PIDController *signal, QWidget * parent, Qt::WindowFlags f) : SignalPropertyDialog(signal,parent,f) {
  sRef = new ExtWidget("Input signal",new SignalOfReferenceWidget(signal,0));
  addToTab("General", sRef);
  sdRef = new ExtWidget("Derivative of input signal",new SignalOfReferenceWidget(signal,0));
  addToTab("General", sdRef);
  vector<PhysicalVariableWidget*> input;
  input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"),noUnitUnits(),1));
  P = new ExtWidget("P",new ExtPhysicalVarWidget(input));
  addToTab("General", P);

  input.clear();
  input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
  I = new ExtWidget("I",new ExtPhysicalVarWidget(input));
  addToTab("General", I);

  input.clear();
  input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"),noUnitUnits(),1));
  D = new ExtWidget("D",new ExtPhysicalVarWidget(input));
  addToTab("General", D);

}

void PIDControllerPropertyDialog::toWidget(Element *element) {
  SignalPropertyDialog::toWidget(element);
  static_cast<PIDController*>(element)->sRef.toWidget(sRef);
  static_cast<PIDController*>(element)->sdRef.toWidget(sdRef);
  static_cast<PIDController*>(element)->P.toWidget(P);
  static_cast<PIDController*>(element)->I.toWidget(I);
  static_cast<PIDController*>(element)->D.toWidget(D);
}

void PIDControllerPropertyDialog::fromWidget(Element *element) {
  SignalPropertyDialog::fromWidget(element);
  static_cast<PIDController*>(element)->sRef.fromWidget(sRef);
  static_cast<PIDController*>(element)->sdRef.fromWidget(sdRef);
  static_cast<PIDController*>(element)->P.fromWidget(P);
  static_cast<PIDController*>(element)->I.fromWidget(I);
  static_cast<PIDController*>(element)->D.fromWidget(D);
}

UnarySignalOperationPropertyDialog::UnarySignalOperationPropertyDialog(UnarySignalOperation *signal, QWidget * parent, Qt::WindowFlags f_) : SignalPropertyDialog(signal,parent,f_) {
  sRef = new ExtWidget("Input signal",new SignalOfReferenceWidget(signal,0));
  addToTab("General", sRef);

  f = new ExtWidget("Function",new ChoiceWidget2(new SymbolicFunctionWidgetFactory2(QStringList("x"))));
  addToTab("General", f);
}

void UnarySignalOperationPropertyDialog::toWidget(Element *element) {
  SignalPropertyDialog::toWidget(element);
  static_cast<UnarySignalOperation*>(element)->sRef.toWidget(sRef);
  static_cast<UnarySignalOperation*>(element)->f.toWidget(f);
}

void UnarySignalOperationPropertyDialog::fromWidget(Element *element) {
  SignalPropertyDialog::fromWidget(element);
  static_cast<UnarySignalOperation*>(element)->sRef.fromWidget(sRef);
  static_cast<UnarySignalOperation*>(element)->f.fromWidget(f);
}

BinarySignalOperationPropertyDialog::BinarySignalOperationPropertyDialog(BinarySignalOperation *signal, QWidget * parent, Qt::WindowFlags f_) : SignalPropertyDialog(signal,parent,f_) {
  s1Ref = new ExtWidget("Input 1 signal",new SignalOfReferenceWidget(signal,0));
  addToTab("General", s1Ref);

  s2Ref = new ExtWidget("Input 2 signal",new SignalOfReferenceWidget(signal,0));
  addToTab("General", s2Ref);

  QStringList var;
  var << "x1" << "x2";
  f = new ExtWidget("Function",new ChoiceWidget2(new SymbolicFunctionWidgetFactory2(var)));
  addToTab("General", f);
}

void BinarySignalOperationPropertyDialog::toWidget(Element *element) {
  SignalPropertyDialog::toWidget(element);
  static_cast<BinarySignalOperation*>(element)->s1Ref.toWidget(s1Ref);
  static_cast<BinarySignalOperation*>(element)->s2Ref.toWidget(s2Ref);
  static_cast<BinarySignalOperation*>(element)->f.toWidget(f);
}

void BinarySignalOperationPropertyDialog::fromWidget(Element *element) {
  SignalPropertyDialog::fromWidget(element);
  static_cast<BinarySignalOperation*>(element)->s1Ref.fromWidget(s1Ref);
  static_cast<BinarySignalOperation*>(element)->s2Ref.fromWidget(s2Ref);
  static_cast<BinarySignalOperation*>(element)->f.fromWidget(f);
}
