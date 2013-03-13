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
#include "constraint.h"
#include "frame.h"
#include "rigidbody.h"
#include "basic_properties.h"
#include "kinetics_properties.h"
#include "string_widgets.h"
#include "function_widgets.h"
#include "kinetics_widgets.h"

using namespace std;

Constraint::Constraint(const QString &str, QTreeWidgetItem *parentItem, int ind) : Object(str, parentItem, ind) {
}

Constraint::~Constraint() {
}

KinematicConstraint::KinematicConstraint(const QString &str, QTreeWidgetItem *parentItem, int ind) : Constraint(str, parentItem, ind), refBody(0) {

  setText(1,getType());

//  properties->addTab("Kinetics");

  dependentBody = new ExtWidget("Dependent body",new RigidBodyOfReferenceWidget(this,0));
  connect((RigidBodyOfReferenceWidget*)dependentBody->getWidget(),SIGNAL(bodyChanged()),this,SLOT(updateReferenceBody()));
  properties->addToTab("General", dependentBody);

  kinematicFunction = new ExtWidget("Kinematic function",new Function1ChoiceWidget,true);
  properties->addToTab("General", kinematicFunction);
  connect((Function1ChoiceWidget*)kinematicFunction->getWidget(),SIGNAL(resize()),this,SLOT(resizeVariables()));

  firstDerivativeOfKinematicFunction = new ExtWidget("First derivative of kinematic function",new Function1ChoiceWidget(MBSIMNS"firstDerivativeOfKinematicFunction"),true);
  properties->addToTab("General", firstDerivativeOfKinematicFunction);
  connect((Function1ChoiceWidget*)firstDerivativeOfKinematicFunction->getWidget(),SIGNAL(resize()),this,SLOT(resizeVariables()));

  secondDerivativeOfKinematicFunction = new ExtWidget("Second derivative of kinematic function",new Function1ChoiceWidget(MBSIMNS"secondDerivativeOfKinematicFunction"),true);
  properties->addToTab("General", secondDerivativeOfKinematicFunction);
  connect((Function1ChoiceWidget*)secondDerivativeOfKinematicFunction->getWidget(),SIGNAL(resize()),this,SLOT(resizeVariables()));

  properties->addStretch();
}

KinematicConstraint::~KinematicConstraint() {
}

void KinematicConstraint::resizeVariables() {
  int size = refBody?refBody->getUnconstrainedSize():0;
  ((Function1ChoiceWidget*)kinematicFunction->getWidget())->resize(size,1);
  ((Function1ChoiceWidget*)firstDerivativeOfKinematicFunction->getWidget())->resize(size,1);
  ((Function1ChoiceWidget*)secondDerivativeOfKinematicFunction->getWidget())->resize(size,1);
}

void KinematicConstraint::updateReferenceBody() {
  if(refBody) {
    refBody->setConstrained(false);
    refBody->resizeGeneralizedPosition();
    refBody->resizeGeneralizedVelocity();
  }
  refBody = ((RigidBodyOfReferenceWidget*)dependentBody->getWidget())->getBody();
  refBody->setConstrained(true);
  refBody->resizeGeneralizedPosition();
  refBody->resizeGeneralizedVelocity();
  connect(refBody,SIGNAL(sizeChanged()),this,SLOT(resizeVariables()));
  resizeVariables();
}

void KinematicConstraint::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e, *ee;
  blockSignals(true);
  Constraint::initializeUsingXML(element);
  dependentBody->initializeUsingXML(element);
  kinematicFunction->initializeUsingXML(element);
  firstDerivativeOfKinematicFunction->initializeUsingXML(element);
  secondDerivativeOfKinematicFunction->initializeUsingXML(element);
  blockSignals(false);
}

TiXmlElement* KinematicConstraint::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = Constraint::writeXMLFile(parent);

  dependentBody->writeXMLFile(ele0);
  kinematicFunction->writeXMLFile(ele0);
  firstDerivativeOfKinematicFunction->writeXMLFile(ele0);
  secondDerivativeOfKinematicFunction->writeXMLFile(ele0);

  return ele0;
}

JointConstraint::JointConstraint(const QString &str, QTreeWidgetItem *parentItem, int ind) : Constraint(str, parentItem, ind), force(0,false), moment(0,false) {

  setText(1,getType());

  independentBody.setProperty(new RigidBodyOfReferenceProperty(0,this,MBSIMNS"independentRigidBody"));

  DependenciesProperty *dependentBodiesFirstSide_ = new DependenciesProperty(this, MBSIMNS"dependentRigidBodiesFirstSide");
  dependentBodiesFirstSide.setProperty(dependentBodiesFirstSide_);

  DependenciesProperty *dependentBodiesSecondSide_ = new DependenciesProperty(this, MBSIMNS"dependentRigidBodiesSecondSide");
  dependentBodiesSecondSide.setProperty(dependentBodiesSecondSide_);

  connections.setProperty(new ConnectFramesProperty(2,this));

  force.setProperty(new GeneralizedForceDirectionProperty(MBSIMNS"forceDirection"));

  moment.setProperty(new GeneralizedForceDirectionProperty(MBSIMNS"momentDirection"));
}

JointConstraint::~JointConstraint() {
}

void JointConstraint::initialize() {
  Constraint::initialize();
  independentBody.initialize();
  dependentBodiesFirstSide.initialize();
  dependentBodiesSecondSide.initialize();
  connections.initialize();
}

void JointConstraint::initializeDialog() {
  Constraint::initializeDialog();

  dialog->addTab("Kinetics",1);

  independentBodyWidget = new ExtWidget("Independent body",new RigidBodyOfReferenceWidget(this,0));
  dialog->addToTab("General", independentBodyWidget);

  DependenciesWidget *dependentBodiesFirstSide_ = new DependenciesWidget(this);
  dependentBodiesFirstSideWidget = new ExtWidget("Dependendent bodies first side",dependentBodiesFirstSide_);
  dialog->addToTab("General", dependentBodiesFirstSideWidget);
  connect(dependentBodiesFirstSide_,SIGNAL(bodyChanged()),this,SLOT(resizeVariables()));

  DependenciesWidget *dependentBodiesSecondSide_ = new DependenciesWidget(this);
  dependentBodiesSecondSideWidget = new ExtWidget("Dependendent bodies second side",dependentBodiesSecondSide_);
  dialog->addToTab("General", dependentBodiesSecondSideWidget);
  connect(dependentBodiesSecondSide_,SIGNAL(bodyChanged()),this,SLOT(resizeVariables()));

  connectionsWidget = new ExtWidget("Connections",new ConnectFramesWidget(2,this));
  dialog->addToTab("Kinetics", connectionsWidget);

  forceWidget = new ExtWidget("Force",new GeneralizedForceDirectionWidget,true);
  dialog->addToTab("Kinetics", forceWidget);

  momentWidget = new ExtWidget("Moment",new GeneralizedForceDirectionWidget,true);
  dialog->addToTab("Kinetics", momentWidget);
}

void JointConstraint::toWidget() {
  Constraint::toWidget();
  independentBody.toWidget(independentBodyWidget);
  dependentBodiesFirstSide.toWidget(dependentBodiesFirstSideWidget);
  dependentBodiesSecondSide.toWidget(dependentBodiesSecondSideWidget);
  connections.toWidget(connectionsWidget);
  force.toWidget(forceWidget);
  moment.toWidget(momentWidget);
}

void JointConstraint::fromWidget() {
  Constraint::fromWidget();
  independentBody.fromWidget(independentBodyWidget);
  dependentBodiesFirstSide.fromWidget(dependentBodiesFirstSideWidget);
  dependentBodiesSecondSide.fromWidget(dependentBodiesSecondSideWidget);
  connections.fromWidget(connectionsWidget);
  force.fromWidget(forceWidget);
  moment.fromWidget(momentWidget);
}

void JointConstraint::resizeGeneralizedPosition() {
  int size = 0;
  for(int i=0; i<((DependenciesWidget*)dependentBodiesFirstSideWidget->getWidget())->getSize(); i++)
    if(((DependenciesWidget*)dependentBodiesFirstSideWidget->getWidget())->getBody(i))
    size += ((DependenciesWidget*)dependentBodiesFirstSideWidget->getWidget())->getBody(i)->getUnconstrainedSize();
  for(int i=0; i<((DependenciesWidget*)dependentBodiesSecondSideWidget->getWidget())->getSize(); i++)
    if(((DependenciesWidget*)dependentBodiesSecondSideWidget->getWidget())->getBody(i))
      size += ((DependenciesWidget*)dependentBodiesSecondSideWidget->getWidget())->getBody(i)->getUnconstrainedSize();
  if(q0->size() != size)
    q0->resize(size);
}

void JointConstraint::initializeUsingXML(TiXmlElement *element) {
  Constraint::initializeUsingXML(element);
  dependentBodiesFirstSide.initializeUsingXML(element);
  dependentBodiesSecondSide.initializeUsingXML(element);
  independentBody.initializeUsingXML(element);

  force.initializeUsingXML(element);
  moment.initializeUsingXML(element);
  connections.initializeUsingXML(element);
}

TiXmlElement* JointConstraint::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = Constraint::writeXMLFile(parent);

  dependentBodiesFirstSide.writeXMLFile(ele0);
  dependentBodiesSecondSide.writeXMLFile(ele0);

  independentBody.writeXMLFile(ele0);

  force.writeXMLFile(ele0);
  moment.writeXMLFile(ele0);

  connections.writeXMLFile(ele0);

  return ele0;
}
