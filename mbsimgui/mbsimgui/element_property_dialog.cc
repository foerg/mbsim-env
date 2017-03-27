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
#include "special_widgets.h"
#include "kinematics_widgets.h"
#include "kinetics_widgets.h"
#include "function_widgets.h"
#include "kinematic_functions_widgets.h"
#include "ombv_widgets.h"
#include "extended_widgets.h"
#include "dynamic_system_solver.h"
#include "frame.h"
#include "contour.h"
#include "rigid_body.h"
#include "flexible_body_ffr.h"
#include "constraint.h"
#include "signal_processing_system.h"
#include "linear_transfer_system.h"
#include "kinetic_excitation.h"
#include "spring_damper.h"
#include "joint.h"
#include "contact.h"
#include "observer.h"
#include "parameter.h"
#include "integrator.h"
#include "sensor.h"
#include "function_widget_factory.h"
#include "friction.h"
#include "gear.h"
#include "connection.h"
#include <QPushButton>
#include <mbxmlutilshelper/getinstallpath.h>
#include "mainwindow.h"

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  extern MainWindow *mw;

  class GeneralizedGearConstraintWidgetFactory : public WidgetFactory {
    public:
      GeneralizedGearConstraintWidgetFactory(Element* element_, QWidget *parent_=0) : element(element_), parent(parent_) { }
      Widget* createWidget(int i=0);
    protected:
      Element *element;
      QWidget *parent;
  };

  Widget* GeneralizedGearConstraintWidgetFactory::createWidget(int i) {
    return new GearInputReferenceWidget(element,0);
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

  ElementPropertyDialog::ElementPropertyDialog(Element *element_, QWidget *parent, Qt::WindowFlags f) : PropertyDialog(parent,f), element(element_) {
    addTab("General");
    name = new ExtWidget("Name",new TextWidget(element->getName()));
    name->setToolTip("Set the name of the element");
    addToTab("General", name);
    addTab("Plot");
    plotFeature = new ExtWidget("Plot features",new PlotFeatureStatusWidget(element->getPlotFeatureType()));
    addToTab("Plot", plotFeature);
    for(unsigned int i=0; i<element->getPlotFeatures().size(); i++)
      static_cast<PlotFeatureStatusWidget*>(plotFeature->getWidget())->addFeature(element->getPlotFeatures()[i]);
  }

  DOMElement* ElementPropertyDialog::initializeUsingXML(DOMElement *parent) {
    static_cast<TextWidget*>(name->getWidget())->setText(element->getName());
    plotFeature->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* ElementPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    element->removeXMLElements();
    element->setName(static_cast<TextWidget*>(name->getWidget())->getText());
    E(element->getXMLElement())->setAttribute("name",element->getName().toStdString());
    plotFeature->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  void ElementPropertyDialog::toWidget(Element *element) {
    initializeUsingXML(element->getXMLElement());
  }

  void ElementPropertyDialog::fromWidget(Element *element) {
    writeXMLFile(element->getXMLElement());
  }

  void ElementPropertyDialog::showXMLHelp() {
    // generate url for current element
    string url=(MBXMLUtils::getInstallPath()/"share"/"mbxmlutils"/"doc").string();
    string ns=element->getNameSpace().getNamespaceURI();
    replace(ns.begin(), ns.end(), ':', '_');
    replace(ns.begin(), ns.end(), '.', '_');
    replace(ns.begin(), ns.end(), '/', '_');
    url+="/"+ns+"/index.html#"+element->getType().toStdString();
    // open in XML help dialog
    mw->xmlHelp(QString::fromStdString(url));
  }

  void ElementPropertyDialog::setName(const QString &str) {
    static_cast<TextWidget*>(name->getWidget())->setText(str);
  }

  void ElementPropertyDialog::setReadOnly(bool readOnly) {
    static_cast<TextWidget*>(name->getWidget())->setReadOnly(readOnly);
  }

  FramePropertyDialog::FramePropertyDialog(Frame *frame, QWidget *parent, Qt::WindowFlags f) : ElementPropertyDialog(frame,parent,f) {
    addTab("Visualisation",1);
    visu = new ExtWidget("OpenMBV frame",new FrameMBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    visu->setToolTip("Set the visualisation parameters for the frame");
    addToTab("Visualisation", visu);
  }

  DOMElement* FramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* FramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    visu->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  InternalFramePropertyDialog::InternalFramePropertyDialog(InternalFrame *frame, QWidget *parent, Qt::WindowFlags f) : ElementPropertyDialog(frame,parent,f) {
    addTab("Visualisation",1);
    visu = new ExtWidget("OpenMBV frame",new FrameMBSOMBVWidget("NOTSET"),true,true,frame->getXMLFrameName());
    visu->setToolTip("Set the visualisation parameters for the frame");
    addToTab("Visualisation", visu);
    setReadOnly(true);
    setName(frame->getName());
  }

  DOMElement* InternalFramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    visu->initializeUsingXML(element->getParent()->getXMLElement());
    static_cast<PlotFeatureStatusWidget*>(plotFeature->getWidget())->initializeUsingXML2(element->getParent()->getXMLElement());
    return parent;
  }

  DOMElement* InternalFramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    element->removeXMLElements();
    visu->writeXMLFile(element->getParent()->getXMLElement(),element->getParent()->getXMLFrame());
    static_cast<PlotFeatureStatusWidget*>(plotFeature->getWidget())->writeXMLFile2(element->getParent()->getXMLElement());
    return NULL;
  }

  FixedRelativeFramePropertyDialog::FixedRelativeFramePropertyDialog(FixedRelativeFrame *frame, QWidget *parent, Qt::WindowFlags f) : FramePropertyDialog(frame,parent,f) {
    addTab("Kinematics",1);

    refFrame = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(frame,frame),true,false,MBSIM%"frameOfReference");
    addToTab("Kinematics", refFrame);

    position = new ExtWidget("Relative position",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,lengthUnits()),vector<int>(3,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"relativePosition");
    addToTab("Kinematics", position);

    orientation = new ExtWidget("Relative orientation",new ChoiceWidget2(new RotMatWidgetFactory,QBoxLayout::RightToLeft),true,false,MBSIM%"relativeOrientation");
    addToTab("Kinematics", orientation);
  }

  DOMElement* FixedRelativeFramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    FramePropertyDialog::initializeUsingXML(element->getXMLElement());
    refFrame->initializeUsingXML(element->getXMLElement());
    position->initializeUsingXML(element->getXMLElement());
    orientation->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* FixedRelativeFramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FramePropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    refFrame->writeXMLFile(element->getXMLElement(),NULL);
    position->writeXMLFile(element->getXMLElement(),NULL);
    orientation->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  NodeFramePropertyDialog::NodeFramePropertyDialog(NodeFrame *frame, QWidget *parent, Qt::WindowFlags f) : FramePropertyDialog(frame,parent,f) {

    nodeNumber = new ExtWidget("Node number",new ChoiceWidget2(new ScalarWidgetFactory("1"),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"nodeNumber");
    addToTab("General", nodeNumber);
  }

  DOMElement* NodeFramePropertyDialog::initializeUsingXML(DOMElement *parent) {
    FramePropertyDialog::initializeUsingXML(element->getXMLElement());
    nodeNumber->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* NodeFramePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FramePropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    nodeNumber->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  ContourPropertyDialog::ContourPropertyDialog(Contour *contour, QWidget * parent, Qt::WindowFlags f) : ElementPropertyDialog(contour,parent,f) {
    thickness = new ExtWidget("Thickness",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"thickness");
    addToTab("General", thickness);
  }

  DOMElement* ContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(element->getXMLElement());
    thickness->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* ContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    thickness->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  RigidContourPropertyDialog::RigidContourPropertyDialog(RigidContour *contour, QWidget * parent, Qt::WindowFlags f) : ContourPropertyDialog(contour,parent,f) {
    refFrame = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(contour,NULL),true,false,MBSIM%"frameOfReference");
    addToTab("General", refFrame);
  }

  DOMElement* RigidContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    refFrame->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* RigidContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    refFrame->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  PointPropertyDialog::PointPropertyDialog(Point *point, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(point,parent,f) {
    addTab("Visualisation",1);

    visu = new ExtWidget("Enable openMBV",new PointMBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* PointPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* PointPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  LinePropertyDialog::LinePropertyDialog(Line *line, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(line,parent,f) {
    addTab("Visualisation",1);

    visu = new ExtWidget("Enable openMBV",new LineMBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* LinePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* LinePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  PlanePropertyDialog::PlanePropertyDialog(Plane *plane, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(plane,parent,f) {
    addTab("Visualisation",1);

    visu = new ExtWidget("Enable openMBV",new PlaneMBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* PlanePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* PlanePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  SpherePropertyDialog::SpherePropertyDialog(Sphere *sphere, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(sphere,parent,f) {
    addTab("Visualisation",1);

    radius = new ExtWidget("Radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"radius");
    addToTab("General", radius);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* SpherePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    radius->initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* SpherePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    radius->writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  CirclePropertyDialog::CirclePropertyDialog(Circle *circle, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(circle,parent,f) {
    addTab("Visualisation",1);

    radius = new ExtWidget("Radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"radius");
    addToTab("General", radius);
    solid = new ExtWidget("Solid",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"solid");
    addToTab("General", solid);
    visu = new ExtWidget("Enable openMBV",new MBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* CirclePropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    radius->initializeUsingXML(element->getXMLElement());
    solid->initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* CirclePropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    radius->writeXMLFile(element->getXMLElement(),NULL);
    solid->writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  CuboidPropertyDialog::CuboidPropertyDialog(Cuboid *circle, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(circle,parent,f) {
    addTab("Visualisation",1);

    length = new ExtWidget("Length",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"length");
    addToTab("General", length);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* CuboidPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    length->initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* CuboidPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    length->writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  LineSegmentPropertyDialog::LineSegmentPropertyDialog(LineSegment *line, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(line,parent,f) {
    addTab("Visualisation",1);

    length = new ExtWidget("Length",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"length");
    addToTab("General", length);

    visu = new ExtWidget("Enable openMBV",new MBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* LineSegmentPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    length->initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* LineSegmentPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    length->writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  PlanarContourPropertyDialog::PlanarContourPropertyDialog(PlanarContour *contour, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    nodes = new ExtWidget("Nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIM%"nodes");
    addToTab("General", nodes);

    contourFunction = new ExtWidget("Contour function",new ChoiceWidget2(new PlanarContourFunctionWidgetFactory(contour),QBoxLayout::TopToBottom,0),false,false,MBSIM%"contourFunction");
    addToTab("General", contourFunction);

    open = new ExtWidget("Open",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"open");
    addToTab("General", open);

    visu = new ExtWidget("Enable openMBV",new PlanarContourMBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* PlanarContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    nodes->initializeUsingXML(element->getXMLElement());
    contourFunction->initializeUsingXML(element->getXMLElement());
    open->initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* PlanarContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    nodes->writeXMLFile(element->getXMLElement(),NULL);
    contourFunction->writeXMLFile(element->getXMLElement(),NULL);
    open->writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  SpatialContourPropertyDialog::SpatialContourPropertyDialog(SpatialContour *contour, QWidget *parent, Qt::WindowFlags f) : RigidContourPropertyDialog(contour,parent,f) {
    addTab("Visualisation",1);

    etaNodes = new ExtWidget("Eta nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIM%"etaNodes");
    addToTab("General", etaNodes);

    xiNodes = new ExtWidget("Xi nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIM%"xiNodes");
    addToTab("General", xiNodes);

    contourFunction = new ExtWidget("Contour function",new ChoiceWidget2(new SpatialContourFunctionWidgetFactory(contour),QBoxLayout::TopToBottom,0),false,false,MBSIM%"contourFunction");
    addToTab("General", contourFunction);

    open = new ExtWidget("Open",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"open");
    addToTab("General", open);

    visu = new ExtWidget("Enable openMBV",new SpatialContourMBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", visu);
  }

  DOMElement* SpatialContourPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidContourPropertyDialog::initializeUsingXML(element->getXMLElement());
    etaNodes->initializeUsingXML(element->getXMLElement());
    xiNodes->initializeUsingXML(element->getXMLElement());
    contourFunction->initializeUsingXML(element->getXMLElement());
    open->initializeUsingXML(element->getXMLElement());
    visu->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* SpatialContourPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidContourPropertyDialog::writeXMLFile(element->getXMLElement(),NULL);
    etaNodes->writeXMLFile(element->getXMLElement(),NULL);
    xiNodes->writeXMLFile(element->getXMLElement(),NULL);
    contourFunction->writeXMLFile(element->getXMLElement(),NULL);
    open->writeXMLFile(element->getXMLElement(),NULL);
    visu->writeXMLFile(element->getXMLElement(),NULL);
    return NULL;
  }

  GroupPropertyDialog::GroupPropertyDialog(Group *group, QWidget *parent, Qt::WindowFlags f, bool kinematics) : ElementPropertyDialog(group,parent,f), position(0), orientation(0), frameOfReference(0) {
    if(kinematics) {
      addTab("Kinematics",1);

      frameOfReference = new ExtWidget("Frame of reference",new ParentFrameOfReferenceWidget(group,NULL),true,false,MBSIM%"frameOfReference");
      addToTab("Kinematics", frameOfReference);

      position = new ExtWidget("Position",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,lengthUnits()),vector<int>(3,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"position");
      addToTab("Kinematics", position);

      orientation = new ExtWidget("Orientation",new ChoiceWidget2(new RotMatWidgetFactory,QBoxLayout::RightToLeft),true,false,MBSIM%"orientation");
      addToTab("Kinematics", orientation);
    }
  }

  DOMElement* GroupPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(element->getXMLElement());
    if(position) {
      frameOfReference->initializeUsingXML(element->getXMLElement());
      position->initializeUsingXML(element->getXMLElement());
      orientation->initializeUsingXML(element->getXMLElement());
    }
    return parent;
  }

  DOMElement* GroupPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(parent,ref?ref:element->getXMLFrames());
    if(position) {
      frameOfReference->writeXMLFile(element->getXMLElement(),ref?ref:element->getXMLFrames());
      position->writeXMLFile(element->getXMLElement(),ref?ref:element->getXMLFrames());
      orientation->writeXMLFile(element->getXMLElement(),ref?ref:element->getXMLFrames());
    }
    return NULL;
  }

  DynamicSystemSolverPropertyDialog::DynamicSystemSolverPropertyDialog(DynamicSystemSolver *solver, QWidget *parent, Qt::WindowFlags f) : GroupPropertyDialog(solver,parent,f,false) {
    addTab("Environment",1);
    addTab("Solver parameters",2);
    addTab("Extra");

    environment = new ExtWidget("Acceleration of gravity",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,accelerationUnits())),QBoxLayout::RightToLeft,5),false,false,MBSIM%"accelerationOfGravity");
    addToTab("Environment", environment);

    solverParameters = new ExtWidget("Solver parameters",new DynamicSystemSolverParametersWidget,true); 
    addToTab("Solver parameters",solverParameters);

    inverseKinetics = new ExtWidget("Inverse kinetics",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft),true);
    addToTab("Extra", inverseKinetics);

    initialProjection = new ExtWidget("Initial projection",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft),true);
    addToTab("Extra", initialProjection);

    useConstraintSolverForPlot = new ExtWidget("Use constraint solver for plot",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft),true);
    addToTab("Extra", useConstraintSolverForPlot);
  }

  DOMElement* DynamicSystemSolverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    GroupPropertyDialog::initializeUsingXML(element->getXMLElement());
    environment->initializeUsingXML(static_cast<DynamicSystemSolver*>(element)->getXMLEnvironments()->getFirstElementChild());
    return parent;
  }

  DOMElement* DynamicSystemSolverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    GroupPropertyDialog::writeXMLFile(parent,element->getXMLFrames());
    environment->writeXMLFile(static_cast<DynamicSystemSolver*>(element)->getXMLEnvironments()->getFirstElementChild());
    return NULL;
  }

  ObjectPropertyDialog::ObjectPropertyDialog(Object *object, QWidget *parent, Qt::WindowFlags f) : ElementPropertyDialog(object,parent,f) {
    addTab("Initial conditions");

    q0 = new ExtWidget("Generalized initial position",new ChoiceWidget2(new VecWidgetFactory(0,vector<QStringList>(3,QStringList())),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedInitialPosition");
    addToTab("Initial conditions", q0);

    u0 = new ExtWidget("Generalized initial velocity",new ChoiceWidget2(new VecWidgetFactory(0,vector<QStringList>(3,QStringList())),QBoxLayout::RightToLeft,5),true,false,MBSIM%"generalizedInitialVelocity");
    addToTab("Initial conditions", u0);

    connect(q0, SIGNAL(resize_()), this, SLOT(resizeVariables()));
    connect(u0, SIGNAL(resize_()), this, SLOT(resizeVariables()));
    connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
  }

  DOMElement* ObjectPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(element->getXMLElement());
    q0->initializeUsingXML(element->getXMLElement());
    u0->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* ObjectPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    q0->writeXMLFile(element->getXMLElement(),ref);
    u0->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  BodyPropertyDialog::BodyPropertyDialog(Body *body, QWidget *parent, Qt::WindowFlags f) : ObjectPropertyDialog(body,parent,f) {
    addTab("Kinematics");

    R = new ExtWidget("Frame of reference",new FrameOfReferenceWidget(body,body->getParent()->getFrame(0)),true,false,MBSIM%"frameOfReference");
    addToTab("Kinematics",R);
  }

  DOMElement* BodyPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObjectPropertyDialog::initializeUsingXML(element->getXMLElement());
    R->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* BodyPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObjectPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    R->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  RigidBodyPropertyDialog::RigidBodyPropertyDialog(RigidBody *body_, QWidget *parent, Qt::WindowFlags f) : BodyPropertyDialog(body_,parent,f), body(body_) {
    addTab("Visualisation",3);

    K = new ExtWidget("Frame for kinematics",new LocalFrameOfReferenceWidget(body,0),true,false,MBSIM%"frameForKinematics");
    addToTab("Kinematics",K);

    mass = new ExtWidget("Mass",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,massUnits()),vector<int>(2,2)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"mass");
    addToTab("General",mass);

    inertia = new ExtWidget("Inertia tensor",new ChoiceWidget2(new SymMatWidgetFactory(getEye<QString>(3,3,"0.01","0"),vector<QStringList>(3,inertiaUnits()),vector<int>(3,2)),QBoxLayout::RightToLeft,5),false,false,MBSIM%"inertiaTensor");
    addToTab("General",inertia);

    frameForInertiaTensor = new ExtWidget("Frame for inertia tensor",new LocalFrameOfReferenceWidget(body,0),true,false,MBSIM%"frameForInertiaTensor");
    addToTab("General",frameForInertiaTensor);

    translation = new ExtWidget("Translation",new ChoiceWidget2(new TranslationWidgetFactory4(body),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", translation);
    connect(translation,SIGNAL(resize_()),this,SLOT(resizeVariables()));

    rotation = new ExtWidget("Rotation",new ChoiceWidget2(new RotationWidgetFactory4(body),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", rotation);
    connect(rotation,SIGNAL(resize_()),this,SLOT(resizeVariables()));

    translationDependentRotation = new ExtWidget("Translation dependent rotation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"translationDependentRotation");
    addToTab("Kinematics", translationDependentRotation);

    coordinateTransformationForRotation = new ExtWidget("Coordinate transformation for rotation",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"coordinateTransformationForRotation");
    addToTab("Kinematics", coordinateTransformationForRotation);

    bodyFixedRepresentationOfAngularVelocity = new ExtWidget("Body-fixed representation of angular velocity",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"bodyFixedRepresentationOfAngularVelocity");
    addToTab("Kinematics", bodyFixedRepresentationOfAngularVelocity);

    ombv = new ExtWidget("Body",new ChoiceWidget2(new OMBVRigidBodyWidgetFactory,QBoxLayout::TopToBottom,0),true,true,MBSIM%"openMBVRigidBody");
    addToTab("Visualisation", ombv);

    ombvFrameRef=new ExtWidget("Frame of reference",new LocalFrameOfReferenceWidget(body),true,false,MBSIM%"openMBVFrameOfReference");
    addToTab("Visualisation", ombvFrameRef);
  }

  DOMElement* RigidBodyPropertyDialog::initializeUsingXML(DOMElement *parent) {
    BodyPropertyDialog::initializeUsingXML(element->getXMLElement());
    K->initializeUsingXML(element->getXMLElement());
    mass->initializeUsingXML(element->getXMLElement());
    inertia->initializeUsingXML(element->getXMLElement());
    frameForInertiaTensor->initializeUsingXML(element->getXMLElement());
    translation->initializeUsingXML(element->getXMLElement());
    rotation->initializeUsingXML(element->getXMLElement());
    translationDependentRotation->initializeUsingXML(element->getXMLElement());
    coordinateTransformationForRotation->initializeUsingXML(element->getXMLElement());
    bodyFixedRepresentationOfAngularVelocity->initializeUsingXML(element->getXMLElement());
    ombv->initializeUsingXML(element->getXMLElement());
    ombvFrameRef->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* RigidBodyPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    BodyPropertyDialog::writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    K->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    mass->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    inertia->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    frameForInertiaTensor->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    translation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    rotation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    translationDependentRotation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    coordinateTransformationForRotation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    bodyFixedRepresentationOfAngularVelocity->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    DOMElement *ele =element->getXMLContours()->getNextElementSibling();
    ombv->writeXMLFile(element->getXMLElement(),ele);
    ombvFrameRef->writeXMLFile(element->getXMLElement(),ele);
    return NULL;
  }

  int RigidBodyPropertyDialog::getqRelSize() const {
    int nqT=0, nqR=0;
    if(translation->isActive()) {
      ExtWidget *extWidget = static_cast<ExtWidget*>(static_cast<ChoiceWidget2*>(translation->getWidget())->getWidget());
      ChoiceWidget2 *trans = static_cast<ChoiceWidget2*>(extWidget->getWidget());
      if(static_cast<ChoiceWidget2*>(translation->getWidget())->getIndex()==1)
        nqT = 0;
      else
        nqT = static_cast<FunctionWidget*>(trans->getWidget())->getArg1Size();
    }
    if(rotation->isActive()) {
      ExtWidget *extWidget = static_cast<ExtWidget*>(static_cast<ChoiceWidget2*>(rotation->getWidget())->getWidget());
      ChoiceWidget2 *rot = static_cast<ChoiceWidget2*>(extWidget->getWidget());
      if(static_cast<ChoiceWidget2*>(rotation->getWidget())->getIndex()==1)
        nqR = 0;
      else
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
    q0->resize_(size,1);
    //translation->resize_(3,1);
    //rotation->resize_(3,1);
    }

  void RigidBodyPropertyDialog::resizeGeneralizedVelocity() {
    int size =  body->isConstrained() ? 0 : getuRelSize();
    u0->resize_(size,1);
  }

  FlexibleBodyFFRPropertyDialog::FlexibleBodyFFRPropertyDialog(FlexibleBodyFFR *body_, QWidget *parent, Qt::WindowFlags f) : BodyPropertyDialog(body_,parent,f), body(body_) {
    addTab("Visualisation",3);
    addTab("Nodal data");

    mass = new ExtWidget("Mass",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,massUnits()),vector<int>(2,2)),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"mass");
    addToTab("General",mass);

    pdm = new ExtWidget("Position integral",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,QStringList()),vector<int>(3,0)),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"positionIntegral");
    addToTab("General", pdm);

    ppdm = new ExtWidget("Position position integral",new ChoiceWidget2(new SymMatWidgetFactory(getEye<QString>(3,3,"0","0"),vector<QStringList>(3,inertiaUnits()),vector<int>(3,2)),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"positionPositionIntegral");
    addToTab("General",ppdm);

    Pdm = new ExtWidget("Shape function integral",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"shapeFunctionIntegral");
    addToTab("General",Pdm);

    //rPdm = new ExtWidget("Position shape function integral",new OneDimMatArrayWidget());
    rPdm = new ExtWidget("Position shape function integral",new ChoiceWidget2(new OneDimMatArrayWidgetFactory(3,3,1),QBoxLayout::TopToBottom,5),false,false,MBSIMFLEX%"positionShapeFunctionIntegral");
    addToTab("General",rPdm);

    //PPdm = new ExtWidget("Shape function shape function integral",new TwoDimMatArrayWidget(3,1,1));
    PPdm = new ExtWidget("Shape function shape function integral",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory(3,1,1),QBoxLayout::TopToBottom,5),false,false,MBSIMFLEX%"shapeFunctionShapeFunctionIntegral");
    addToTab("General",PPdm);

    Ke = new ExtWidget("Stiffness matrix",new ChoiceWidget2(new SymMatWidgetFactory(getMat<QString>(1,1,"0")),QBoxLayout::RightToLeft,5),false,false,MBSIMFLEX%"stiffnessMatrix");
    addToTab("General",Ke);

    De = new ExtWidget("Damping matrix",new ChoiceWidget2(new SymMatWidgetFactory(getMat<QString>(1,1,"0")),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"dampingMatrix");
    addToTab("General",De);

    beta = new ExtWidget("Proportional damping",new ChoiceWidget2(new VecWidgetFactory(2),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"proportionalDamping");
    addToTab("General", beta);

    //Knl1 = new ExtWidget("Nonlinear stiffness matrix of first order",new OneDimMatArrayWidget(1,1,1),true);
    Knl1 = new ExtWidget("Nonlinear stiffness matrix of first order",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nonlinearStiffnessMatrixOfFirstOrder");
    addToTab("General",Knl1);

//    Knl2 = new ExtWidget("Nonlinear stiffness matrix of second order",new TwoDimMatArrayWidget(1,1,1),true);
    Knl2 = new ExtWidget("Nonlinear stiffness matrix of second order",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nonlinearStiffnessMatrixOfSecondOrder");
    addToTab("General",Knl2);

    ksigma0 = new ExtWidget("Initial stress integral",new ChoiceWidget2(new VecWidgetFactory(1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"initialStressIntegral");
    addToTab("General", ksigma0);

    ksigma1 = new ExtWidget("Nonlinear initial stress integral",new ChoiceWidget2(new MatWidgetFactory(1,1),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nonlinearInitialStressIntegral");
    addToTab("General", ksigma1);

    //K0t = new ExtWidget("Geometric stiffness matrix due to acceleration",new OneDimMatArrayWidget(3,1,1),true);
    K0t = new ExtWidget("Geometric stiffness matrix due to acceleration",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"geometricStiffnessMatrixDueToAcceleration");
    addToTab("General",K0t);

    //K0r = new ExtWidget("Geometric stiffness matrix due to angular acceleration",new OneDimMatArrayWidget(3,1,1),true);
    K0r = new ExtWidget("Geometric stiffness matrix due to angular acceleration",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"geometricStiffnessMatrixDueToAngularAcceleration");
    addToTab("General",K0r);

    //K0om = new ExtWidget("Geometric stiffness matrix due to angular velocity",new OneDimMatArrayWidget(3,1,1),true);
    K0om = new ExtWidget("Geometric stiffness matrix due to angular velocity",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"geometricStiffnessMatrixDueToAngularVelocity");
    addToTab("General",K0om);

    //r = new ExtWidget("Relative nodal position",new ChoiceWidget2(new VecSizeVarWidgetFactory(3,vector<QStringList>(3)),QBoxLayout::RightToLeft),true);
    r = new ExtWidget("Nodal relative position",new ChoiceWidget2(new OneDimVecArrayWidgetFactory(1,3,true),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalRelativePosition");
    addToTab("Nodal data", r);

    //A = new ExtWidget("Relative nodal orientation",new ChoiceWidget2(new MatWidgetFactory(3,3,vector<QStringList>(3),vector<int>(3,0)),QBoxLayout::RightToLeft),true);
    A = new ExtWidget("Nodal relative orientation",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalRelativeOrientation");
    addToTab("Nodal data", A);

    //Phi = new ExtWidget("Shape matrix of translation",new ChoiceWidget2(new MatWidgetFactory(3,1,vector<QStringList>(3),vector<int>(3,0)),QBoxLayout::RightToLeft),true);
    Phi = new ExtWidget("Nodal shape matrix of translation",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalShapeMatrixOfTranslation");
    addToTab("Nodal data", Phi);

    //Psi = new ExtWidget("Shape matrix of rotation",new ChoiceWidget2(new MatWidgetFactory(3,1,vector<QStringList>(3),vector<int>(3,0)),QBoxLayout::RightToLeft),true);
    Psi = new ExtWidget("Nodal shape matrix of rotation",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalShapeMatrixOfRotation");
    addToTab("Nodal data", Psi);

    //sigmahel = new ExtWidget("Stress matrix",new ChoiceWidget2(new MatWidgetFactory(6,1,vector<QStringList>(3),vector<int>(3,0)),QBoxLayout::RightToLeft),true);
    sigmahel = new ExtWidget("Nodal stress matrix",new ChoiceWidget2(new OneDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalStressMatrix");
    addToTab("Nodal data", sigmahel);

    sigmahen = new ExtWidget("Nodal nonlinear stress matrix",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalNonlinearStressMatrix");
    addToTab("Nodal data", sigmahen);

    sigma0 = new ExtWidget("Nodal initial stress",new ChoiceWidget2(new OneDimVecArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalInitialStress");
    addToTab("Nodal data", sigma0);

    K0F = new ExtWidget("Nodal geometric stiffness matrix due to force",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalGeometricStiffnessMatrixDueToForce");
    addToTab("Nodal data", K0F);

    K0M = new ExtWidget("Nodal geometric stiffness matrix due to moment",new ChoiceWidget2(new TwoDimMatArrayWidgetFactory,QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"nodalGeometricStiffnessMatrixDueToMoment");
    addToTab("Nodal data", K0M);

    translation = new ExtWidget("Translation",new ChoiceWidget2(new TranslationWidgetFactory4(body,MBSIMFLEX),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", translation);
    connect(translation,SIGNAL(resize_()),this,SLOT(resizeVariables()));

    rotation = new ExtWidget("Rotation",new ChoiceWidget2(new RotationWidgetFactory4(body,MBSIMFLEX),QBoxLayout::TopToBottom,3),true,false,"");
    addToTab("Kinematics", rotation);
    connect(rotation,SIGNAL(resize_()),this,SLOT(resizeVariables()));

    translationDependentRotation = new ExtWidget("Translation dependent rotation",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"translationDependentRotation");
    addToTab("Kinematics", translationDependentRotation);

    coordinateTransformationForRotation = new ExtWidget("Coordinate transformation for rotation",new ChoiceWidget2(new BoolWidgetFactory("1"),QBoxLayout::RightToLeft,5),true,false,MBSIMFLEX%"coordinateTransformationForRotation");
    addToTab("Kinematics", coordinateTransformationForRotation);

    ombvEditor = new ExtWidget("Enable openMBV",new FlexibleBodyFFRMBSOMBVWidget("NOTSET"),true,false,MBSIMFLEX%"enableOpenMBV");
    addToTab("Visualisation", ombvEditor);

    connect(Pdm->getWidget(),SIGNAL(widgetChanged()),this,SLOT(resizeVariables()));
    connect(Pdm->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));
    connect(buttonResize,SIGNAL(clicked(bool)),this,SLOT(resizeVariables()));
//    connect(Knl1,SIGNAL(resize_()),this,SLOT(resizeVariables()));
  }

  void FlexibleBodyFFRPropertyDialog::resizeVariables() {
    int size = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(Pdm->getWidget())->getWidget())->cols();
    if(static_cast<ChoiceWidget2*>(rPdm->getWidget())->getIndex()==0)
      rPdm->resize_(3,size);
    else
      rPdm->resize_(9,size);
    if(static_cast<ChoiceWidget2*>(PPdm->getWidget())->getIndex()==0)
      PPdm->resize_(size,size);
    else
      PPdm->resize_(9*size,size);
    Ke->resize_(size,size);
    De->resize_(size,size);
    if(Knl1->isActive()) {
      if(static_cast<ChoiceWidget2*>(Knl1->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Knl1->getWidget())->getWidget())->resize_(size,size,size);
      else
        Knl1->resize_(size*size,size);
    }
    if(Knl2->isActive()) {
      if(static_cast<ChoiceWidget2*>(Knl2->getWidget())->getIndex()==0)
        static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Knl2->getWidget())->getWidget())->resize_(size,size,size,size);
      else
        Knl2->resize_(size*size*size,size);
    }
    ksigma0->resize_(size,1);
    ksigma1->resize_(size,size);
    if(K0t->isActive()) {
      if(static_cast<ChoiceWidget2*>(K0t->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0t->getWidget())->getWidget())->resize_(3,size,size);
      else
        K0t->resize_(3*size,size);
    }
    if(K0r->isActive()) {
      if(static_cast<ChoiceWidget2*>(K0r->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0r->getWidget())->getWidget())->resize_(3,size,size);
      else
        K0r->resize_(3*size,size);
    }
    if(K0om->isActive()) {
      if(static_cast<ChoiceWidget2*>(K0om->getWidget())->getIndex()==0)
        static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0om->getWidget())->getWidget())->resize_(3,size,size);
      else
        K0om->resize_(3*size,size);
    }
    if(r->isActive()) {
      int rsize;
      if(static_cast<ChoiceWidget2*>(r->getWidget())->getIndex()==0)
        rsize = static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->getArray().size();
      else
        rsize = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(static_cast<ChoiceWidget2*>(r->getWidget())->getWidget())->getWidget())->rows()/3;
      if(A->isActive()) {
        if(static_cast<ChoiceWidget2*>(A->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(A->getWidget())->getWidget())->resize_(rsize,3,3);
        else
          A->resize_(3*rsize,3);
      }
      if(Phi->isActive()) {
        if(static_cast<ChoiceWidget2*>(Phi->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Phi->getWidget())->getWidget())->resize_(rsize,3,size);
        else
          Phi->resize_(3*rsize,size);
      }
      if(Psi->isActive()) {
        if(static_cast<ChoiceWidget2*>(Psi->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(Psi->getWidget())->getWidget())->resize_(rsize,3,size);
        else
          Psi->resize_(3*rsize,size);
      }
      if(sigmahel->isActive()) {
        if(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getIndex()==0)
          static_cast<OneDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(sigmahel->getWidget())->getWidget())->resize_(rsize,6,size);
        else
          sigmahel->resize_(6*rsize,size);
      }
      if(sigmahen->isActive()) {
        if(static_cast<ChoiceWidget2*>(sigmahen->getWidget())->getIndex()==0)
          static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(sigmahen->getWidget())->getWidget())->resize_(rsize,size,6,size);
        else
          sigmahen->resize_(6*rsize*size,size);
      }
      if(sigma0->isActive()) {
        if(static_cast<ChoiceWidget2*>(sigma0->getWidget())->getIndex()==0)
          static_cast<OneDimVecArrayWidget*>(static_cast<ChoiceWidget2*>(sigma0->getWidget())->getWidget())->resize_(rsize,6,1);
        else
          sigma0->resize_(6*rsize,1);
      }
      if(K0F->isActive()) {
        if(static_cast<ChoiceWidget2*>(K0F->getWidget())->getIndex()==0)
          static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0F->getWidget())->getWidget())->resize_(rsize,size,size,size);
        else
          K0F->resize_(size*rsize*size,size);
      }
      if(K0M->isActive()) {
        if(static_cast<ChoiceWidget2*>(K0M->getWidget())->getIndex()==0)
          static_cast<TwoDimMatArrayWidget*>(static_cast<ChoiceWidget2*>(K0M->getWidget())->getWidget())->resize_(rsize,size,size,size);
        else
          K0M->resize_(size*rsize*size,size);
      }
    }
  }

  int FlexibleBodyFFRPropertyDialog::getqRelSize() const {
    int nqT=0, nqR=0;
    if(translation->isActive()) {
      ExtWidget *extWidget = static_cast<ExtWidget*>(static_cast<ChoiceWidget2*>(translation->getWidget())->getWidget());
      ChoiceWidget2 *trans = static_cast<ChoiceWidget2*>(extWidget->getWidget());
      if(static_cast<ChoiceWidget2*>(translation->getWidget())->getIndex()==1)
        nqT = 0;
      else
        nqT = static_cast<FunctionWidget*>(trans->getWidget())->getArg1Size();
    }
    if(rotation->isActive()) {
      ExtWidget *extWidget = static_cast<ExtWidget*>(static_cast<ChoiceWidget2*>(rotation->getWidget())->getWidget());
      ChoiceWidget2 *rot = static_cast<ChoiceWidget2*>(extWidget->getWidget());
      if(static_cast<ChoiceWidget2*>(rotation->getWidget())->getIndex()==1)
        nqR = 0;
      else
        nqR = static_cast<FunctionWidget*>(rot->getWidget())->getArg1Size();
    }
    int nq = nqT + nqR;
    return nq;
  }

  int FlexibleBodyFFRPropertyDialog::getuRelSize() const {
    return getqRelSize();
  }

  void FlexibleBodyFFRPropertyDialog::resizeGeneralizedPosition() {
    int size =  getqRelSize();
    q0->resize_(size,1);
//    translation->resize_(3,1);
//    rotation->resize_(3,1);
    }

  void FlexibleBodyFFRPropertyDialog::resizeGeneralizedVelocity() {
    int size =  getuRelSize();
    u0->resize_(size,1);
  }

  DOMElement* FlexibleBodyFFRPropertyDialog::initializeUsingXML(DOMElement *parent) {
    BodyPropertyDialog::initializeUsingXML(element->getXMLElement());
    mass->initializeUsingXML(element->getXMLElement());
    pdm->initializeUsingXML(element->getXMLElement());
    ppdm->initializeUsingXML(element->getXMLElement());
    Pdm->initializeUsingXML(element->getXMLElement());
    rPdm->initializeUsingXML(element->getXMLElement());
    PPdm->initializeUsingXML(element->getXMLElement());
    Ke->initializeUsingXML(element->getXMLElement());
    De->initializeUsingXML(element->getXMLElement());
    beta->initializeUsingXML(element->getXMLElement());
    Knl1->initializeUsingXML(element->getXMLElement());
    Knl2->initializeUsingXML(element->getXMLElement());
    ksigma0->initializeUsingXML(element->getXMLElement());
    ksigma1->initializeUsingXML(element->getXMLElement());
    K0t->initializeUsingXML(element->getXMLElement());
    K0r->initializeUsingXML(element->getXMLElement());
    K0om->initializeUsingXML(element->getXMLElement());
    r->initializeUsingXML(element->getXMLElement());
    A->initializeUsingXML(element->getXMLElement());
    Phi->initializeUsingXML(element->getXMLElement());
    Psi->initializeUsingXML(element->getXMLElement());
    sigmahel->initializeUsingXML(element->getXMLElement());
    sigmahen->initializeUsingXML(element->getXMLElement());
    sigma0->initializeUsingXML(element->getXMLElement());
    K0F->initializeUsingXML(element->getXMLElement());
    K0M->initializeUsingXML(element->getXMLElement());
    translation->initializeUsingXML(element->getXMLElement());
    rotation->initializeUsingXML(element->getXMLElement());
    translationDependentRotation->initializeUsingXML(element->getXMLElement());
    coordinateTransformationForRotation->initializeUsingXML(element->getXMLElement());
    ombvEditor->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* FlexibleBodyFFRPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    BodyPropertyDialog::writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    mass->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    pdm->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    ppdm->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    Pdm->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    rPdm->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    PPdm->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    Ke->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    De->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    beta->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    Knl1->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    Knl2->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    ksigma0->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    ksigma1->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    K0t->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    K0r->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    K0om->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    r->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    A->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    Phi->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    Psi->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    sigmahel->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    sigmahen->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    sigma0->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    K0F->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    K0M->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    translation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    rotation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    translationDependentRotation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    coordinateTransformationForRotation->writeXMLFile(element->getXMLElement(),element->getXMLFrames());
    DOMElement *ele =element->getXMLContours()->getNextElementSibling();
    ombvEditor->writeXMLFile(element->getXMLElement(),ele);
    return NULL;
  }

  ConstraintPropertyDialog::ConstraintPropertyDialog(Constraint *constraint, QWidget *parent, Qt::WindowFlags f) : ElementPropertyDialog(constraint,parent,f) {
  }

  MechanicalConstraintPropertyDialog::MechanicalConstraintPropertyDialog(MechanicalConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : ConstraintPropertyDialog(constraint,parent,f) {
  }

  GeneralizedConstraintPropertyDialog::GeneralizedConstraintPropertyDialog(GeneralizedConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : MechanicalConstraintPropertyDialog(constraint,parent,f) {

    addTab("Visualisation",2);

    support = new ExtWidget("Support frame",new FrameOfReferenceWidget(constraint,0),true);
    addToTab("General",support);
  }

  GeneralizedGearConstraintPropertyDialog::GeneralizedGearConstraintPropertyDialog(GeneralizedGearConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : GeneralizedConstraintPropertyDialog(constraint,parent,f) {

    dependentBody = new ExtWidget("Dependent body",new RigidBodyOfReferenceWidget(constraint,0));
    addToTab("General", dependentBody);

    independentBodies = new ExtWidget("Independent bodies",new ListWidget(new GeneralizedGearConstraintWidgetFactory(constraint,0),"Independent body"));
    addToTab("General",independentBodies);

    connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
  }

//  void GeneralizedGearConstraintPropertyDialog::toWidget(Element *element) {
//    GeneralizedConstraintPropertyDialog::toWidget(element);
//    static_cast<GeneralizedGearConstraint*>(element)->dependentBody.toWidget(dependentBody);
//    static_cast<GeneralizedGearConstraint*>(element)->independentBodies.toWidget(independentBodies);
//  }
//
//  void GeneralizedGearConstraintPropertyDialog::fromWidget(Element *element) {
//    GeneralizedConstraintPropertyDialog::fromWidget(element);
//    RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<GeneralizedGearConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
//    if(body)
//      body->setConstrained(false);
//    static_cast<GeneralizedGearConstraint*>(element)->dependentBody.fromWidget(dependentBody);
//    static_cast<GeneralizedGearConstraint*>(element)->independentBodies.fromWidget(independentBodies);
//    body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<GeneralizedGearConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
//    if(body)
//      body->setConstrained(true);
//  }

  GeneralizedDualConstraintPropertyDialog::GeneralizedDualConstraintPropertyDialog(GeneralizedDualConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : GeneralizedConstraintPropertyDialog(constraint,parent,f) {

    dependentBody = new ExtWidget("Dependent body",new RigidBodyOfReferenceWidget(constraint,0));
    addToTab("General", dependentBody);

    independentBody = new ExtWidget("Independent body",new RigidBodyOfReferenceWidget(constraint,0),true);
    addToTab("General", independentBody);
  }

//  void GeneralizedDualConstraintPropertyDialog::toWidget(Element *element) {
//    GeneralizedConstraintPropertyDialog::toWidget(element);
//    dependentBody->getWidget()->blockSignals(true);
//    static_cast<GeneralizedDualConstraint*>(element)->dependentBody.toWidget(dependentBody);
//    dependentBody->getWidget()->blockSignals(false);
//    static_cast<GeneralizedDualConstraint*>(element)->independentBody.toWidget(independentBody);
//  }
//
//  void GeneralizedDualConstraintPropertyDialog::fromWidget(Element *element) {
//    GeneralizedConstraintPropertyDialog::fromWidget(element);
//    RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<GeneralizedDualConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
//    if(body)
//      body->setConstrained(false);
//    static_cast<GeneralizedDualConstraint*>(element)->dependentBody.fromWidget(dependentBody);
//    static_cast<GeneralizedDualConstraint*>(element)->independentBody.fromWidget(independentBody);
//    body = static_cast<RigidBodyOfReferenceProperty*>(static_cast<GeneralizedDualConstraint*>(element)->dependentBody.getProperty())->getBodyPtr();
//    if(body)
//      body->setConstrained(true);
//  }

  GeneralizedPositionConstraintPropertyDialog::GeneralizedPositionConstraintPropertyDialog(GeneralizedPositionConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {

    constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new FunctionWidgetFactory2(constraint)));
    addToTab("General", constraintFunction);
    connect(constraintFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

    connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
  }

  void GeneralizedPositionConstraintPropertyDialog::resizeVariables() {
//    RigidBody *refBody = static_cast<RigidBodyOfReferenceWidget*>(dependentBody->getWidget())->getSelectedBody();
//    int size = refBody?refBody->getqRelSize():0;
//    constraintFunction->resize_(size,1);
  }

  GeneralizedVelocityConstraintPropertyDialog::GeneralizedVelocityConstraintPropertyDialog(GeneralizedVelocityConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {
    addTab("Initial conditions",1);

    constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new ConstraintWidgetFactory(constraint)));
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
    cout << "GeneralizedVelocityConstraintPropertyDialog::resizeVariables() not yet implemented" << endl;
  }

  GeneralizedAccelerationConstraintPropertyDialog::GeneralizedAccelerationConstraintPropertyDialog(GeneralizedAccelerationConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {
    addTab("Initial conditions",1);

    constraintFunction = new ExtWidget("Constraint function",new ChoiceWidget2(new ConstraintWidgetFactory(constraint)));
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
//    RigidBody *refBody = static_cast<RigidBodyOfReferenceWidget*>(dependentBody->getWidget())->getSelectedBody();
//    int size = refBody?(refBody->getqRelSize()+refBody->getuRelSize()):0;
//    static_cast<ChoiceWidget2*>(constraintFunction->getWidget())->resize_(size,1);
//    if(x0_ && x0_->size() != size)
//      x0_->resize_(size);
//    static_cast<FunctionWidget*>(static_cast<ChoiceWidget2*>(constraintFunction->getWidget())->getWidget())->setArg1Size(size);
  }

  JointConstraintPropertyDialog::JointConstraintPropertyDialog(JointConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : MechanicalConstraintPropertyDialog(constraint,parent,f) {

    addTab("Kinetics",1);
    addTab("Visualisation",2);
    addTab("Initial conditions",2);

    dependentBodiesFirstSide = new ExtWidget("Dependent bodies on first side",new ListWidget(new RigidBodyOfReferenceWidgetFactory(constraint,this),"Body"));
    addToTab("General",dependentBodiesFirstSide);
    connect(dependentBodiesFirstSide->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

    dependentBodiesSecondSide = new ExtWidget("Dependent bodies on second side",new ListWidget(new RigidBodyOfReferenceWidgetFactory(constraint,this),"Body"));
    addToTab("General",dependentBodiesSecondSide);
    connect(dependentBodiesSecondSide->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));

    independentBody = new ExtWidget("Independent body",new RigidBodyOfReferenceWidget(constraint,0));
    addToTab("General", independentBody);

    connections = new ExtWidget("Connections",new ConnectFramesWidget(2,constraint));
    addToTab("Kinetics", connections);

    refFrameID = new ExtWidget("Frame of reference ID",new SpinBoxWidget(1,1,2),true);
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

    input.clear();
    q0_ = new VecWidget(0);
    input.push_back(new PhysicalVariableWidget(q0_,QStringList(),1));
    ExtPhysicalVarWidget *var = new ExtPhysicalVarWidget(input);  
    q0 = new ExtWidget("Initial guess",var,true);
    addToTab("Initial conditions", q0);

    connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
  }

  void JointConstraintPropertyDialog::resizeVariables() {
//    int size = 0;
//    ListWidget *list = static_cast<ListWidget*>(dependentBodiesFirstSide->getWidget());
//    for(int i=0; i<list->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceWidget*>(list->getWidget(i))->getSelectedBody();
//      if(body)
//        size += body->getqRelSize();
//    }
//    list = static_cast<ListWidget*>(dependentBodiesSecondSide->getWidget());
//    for(int i=0; i<list->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceWidget*>(list->getWidget(i))->getSelectedBody();
//      if(body)
//        size += body->getqRelSize();
//    }
//    if(q0_->size() != size)
//      q0_->resize_(size);
  }

//  void JointConstraintPropertyDialog::toWidget(Element *element) {
//    MechanicalConstraintPropertyDialog::toWidget(element);
//    static_cast<JointConstraint*>(element)->independentBody.toWidget(independentBody);
//    dependentBodiesFirstSide->getWidget()->blockSignals(true);
//    static_cast<JointConstraint*>(element)->dependentBodiesFirstSide.toWidget(dependentBodiesFirstSide);
//    dependentBodiesFirstSide->getWidget()->blockSignals(false);
//    dependentBodiesSecondSide->getWidget()->blockSignals(true);
//    static_cast<JointConstraint*>(element)->dependentBodiesSecondSide.toWidget(dependentBodiesSecondSide);
//    dependentBodiesSecondSide->getWidget()->blockSignals(false);
//    static_cast<JointConstraint*>(element)->refFrameID.toWidget(refFrameID);
//    static_cast<JointConstraint*>(element)->force.toWidget(force);
//    static_cast<JointConstraint*>(element)->moment.toWidget(moment);
//    static_cast<JointConstraint*>(element)->connections.toWidget(connections);
//    static_cast<JointConstraint*>(element)->q0.toWidget(q0);
//    resizeVariables();
//  }
//
//  void JointConstraintPropertyDialog::fromWidget(Element *element) {
//    MechanicalConstraintPropertyDialog::fromWidget(element);
//    ListProperty *list1 = static_cast<ListProperty*>(static_cast<JointConstraint*>(element)->dependentBodiesFirstSide.getProperty());
//    for(int i=0; i<list1->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list1->getProperty(i))->getBodyPtr();
//      if(body)
//        body->setConstrained(false);
//    }
//    ListProperty *list2 = static_cast<ListProperty*>(static_cast<JointConstraint*>(element)->dependentBodiesSecondSide.getProperty());
//    for(int i=0; i<list2->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list2->getProperty(i))->getBodyPtr();
//      if(body)
//        body->setConstrained(false);
//    }
//    static_cast<JointConstraint*>(element)->independentBody.fromWidget(independentBody);
//    static_cast<JointConstraint*>(element)->dependentBodiesFirstSide.fromWidget(dependentBodiesFirstSide);
//    static_cast<JointConstraint*>(element)->dependentBodiesSecondSide.fromWidget(dependentBodiesSecondSide);
//    static_cast<JointConstraint*>(element)->refFrameID.fromWidget(refFrameID);
//    static_cast<JointConstraint*>(element)->force.fromWidget(force);
//    static_cast<JointConstraint*>(element)->moment.fromWidget(moment);
//    static_cast<JointConstraint*>(element)->connections.fromWidget(connections);
//    for(int i=0; i<list1->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list1->getProperty(i))->getBodyPtr();
//      if(body)
//        body->setConstrained(true);
//    }
//    for(int i=0; i<list2->getSize(); i++) {
//      RigidBody *body = static_cast<RigidBodyOfReferenceProperty*>(list2->getProperty(i))->getBodyPtr();
//      if(body)
//        body->setConstrained(true);
//    }
//    static_cast<JointConstraint*>(element)->q0.fromWidget(q0);
//  }

  GeneralizedConnectionConstraintPropertyDialog::GeneralizedConnectionConstraintPropertyDialog(GeneralizedConnectionConstraint *constraint, QWidget *parent, Qt::WindowFlags f) : GeneralizedDualConstraintPropertyDialog(constraint,parent,f) {
    connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
  }

  SignalProcessingSystemPropertyDialog::SignalProcessingSystemPropertyDialog(SignalProcessingSystem *sps, QWidget * parent, Qt::WindowFlags f) : LinkPropertyDialog(sps,parent,f) {
    signalRef = new ExtWidget("Input signal",new SignalOfReferenceWidget(sps,0));
    addToTab("General", signalRef);
  }

  LinkPropertyDialog::LinkPropertyDialog(Link *link, QWidget *parent, Qt::WindowFlags f) : ElementPropertyDialog(link,parent,f) {
  }

  MechanicalLinkPropertyDialog::MechanicalLinkPropertyDialog(MechanicalLink *link, QWidget *parent, Qt::WindowFlags f) : LinkPropertyDialog(link,parent,f) {
  }

  FrameLinkPropertyDialog::FrameLinkPropertyDialog(FrameLink *link, QWidget *parent, Qt::WindowFlags f) : MechanicalLinkPropertyDialog(link,parent,f) {
    addTab("Kinetics",1);
    addTab("Visualisation",2);

    connections = new ExtWidget("Connections",new ConnectFramesWidget(2,link),false,false,MBSIM%"connect");
    addToTab("Kinetics", connections);
  }

  DOMElement* FrameLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ElementPropertyDialog::initializeUsingXML(element->getXMLElement());
    connections->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* FrameLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ElementPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    connections->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  FixedFrameLinkPropertyDialog::FixedFrameLinkPropertyDialog(FixedFrameLink *link, QWidget *parent, Qt::WindowFlags f) : FrameLinkPropertyDialog(link,parent,f) {
  }

  FloatingFrameLinkPropertyDialog::FloatingFrameLinkPropertyDialog(FloatingFrameLink *link, QWidget *parent, Qt::WindowFlags f) : FrameLinkPropertyDialog(link,parent,f) {
    refFrameID = new ExtWidget("Frame of reference ID",new SpinBoxWidget(1,1,2),true,false,MBSIM%"frameOfReferenceID");
    addToTab("Kinetics", refFrameID);
  }

  DOMElement* FloatingFrameLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FrameLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    refFrameID->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* FloatingFrameLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FrameLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    refFrameID->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  RigidBodyLinkPropertyDialog::RigidBodyLinkPropertyDialog(RigidBodyLink *link, QWidget *parent, Qt::WindowFlags f) : MechanicalLinkPropertyDialog(link,parent,f) {
    addTab("Visualisation",2);

    support = new ExtWidget("Support frame",new FrameOfReferenceWidget(link,0),true,false,MBSIM%"supportFrame");
    addToTab("General",support);
  }

  DOMElement* RigidBodyLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    MechanicalLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    support->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* RigidBodyLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    MechanicalLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    support->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  DualRigidBodyLinkPropertyDialog::DualRigidBodyLinkPropertyDialog(DualRigidBodyLink *link, QWidget *parent, Qt::WindowFlags f) : RigidBodyLinkPropertyDialog(link,parent,f) {
    addTab("Kinetics",1);

    connections = new ExtWidget("Connections",new ChoiceWidget2(new ConnectRigidBodiesWidgetFactory(link),QBoxLayout::RightToLeft,5),false,false,MBSIM%"connect");
    addToTab("Kinetics",connections);
  }

  DOMElement* DualRigidBodyLinkPropertyDialog::initializeUsingXML(DOMElement *parent) {
    RigidBodyLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    connections->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* DualRigidBodyLinkPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    RigidBodyLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    connections->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  KineticExcitationPropertyDialog::KineticExcitationPropertyDialog(KineticExcitation *kineticExcitation, QWidget *parent, Qt::WindowFlags wf) : FloatingFrameLinkPropertyDialog(kineticExcitation,parent,wf) {

    static_cast<ConnectFramesWidget*>(connections->getWidget())->setDefaultFrame("../Frame[I]");

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics",forceDirection);

    forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new FunctionWidgetFactory2(kineticExcitation),QBoxLayout::TopToBottom,0),true,false,MBSIM%"forceFunction");
    addToTab("Kinetics",forceFunction);

    momentDirection = new ExtWidget("Moment direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"momentDirection");
    addToTab("Kinetics",momentDirection);

    momentFunction = new ExtWidget("Moment function",new ChoiceWidget2(new FunctionWidgetFactory2(kineticExcitation),QBoxLayout::TopToBottom,0),true,false,MBSIM%"momentFunction");
    addToTab("Kinetics",momentFunction);

    arrow = new ExtWidget("OpenMBV arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBV");
    addToTab("Visualisation",arrow);

    connect(forceDirection->getWidget(),SIGNAL(widgetChanged()),this,SLOT(resizeVariables()));
    connect(forceDirection->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));
    connect(forceFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));
    connect(momentDirection->getWidget(),SIGNAL(widgetChanged()),this,SLOT(resizeVariables()));
    connect(momentDirection->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));
    connect(momentFunction->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));
    connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
  }

  void KineticExcitationPropertyDialog::resizeVariables() {
    if(forceDirection->isActive()) {
      int size = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(forceDirection->getWidget())->getWidget())->cols();
      forceFunction->resize_(size,1);
    }
    if(momentDirection->isActive()) {
      int size = static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(momentDirection->getWidget())->getWidget())->cols();
      momentFunction->resize_(size,1);
    }
  }

  DOMElement* KineticExcitationPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    forceDirection->initializeUsingXML(element->getXMLElement());
    forceFunction->initializeUsingXML(element->getXMLElement());
    momentDirection->initializeUsingXML(element->getXMLElement());
    momentFunction->initializeUsingXML(element->getXMLElement());
    arrow->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* KineticExcitationPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    forceDirection->writeXMLFile(element->getXMLElement(),ref);
    forceFunction->writeXMLFile(element->getXMLElement(),ref);
    momentDirection->writeXMLFile(element->getXMLElement(),ref);
    momentFunction->writeXMLFile(element->getXMLElement(),ref);
    arrow->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  SpringDamperPropertyDialog::SpringDamperPropertyDialog(SpringDamper *springDamper, QWidget *parent, Qt::WindowFlags f) : FixedFrameLinkPropertyDialog(springDamper,parent,f) {

    forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new SpringDamperWidgetFactory(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"forceFunction");
    addToTab("Kinetics", forceFunction);

    unloadedLength = new ExtWidget("Unloaded length",new ChoiceWidget2(new ScalarWidgetFactory("1"),QBoxLayout::RightToLeft,5),false,false,MBSIM%"unloadedLength");
    addToTab("General",unloadedLength);

    coilSpring = new ExtWidget("Enable openMBV",new CoilSpringMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", coilSpring);
  }

  DOMElement* SpringDamperPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FixedFrameLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    forceFunction->initializeUsingXML(element->getXMLElement());
    unloadedLength->initializeUsingXML(element->getXMLElement());
    coilSpring->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* SpringDamperPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FixedFrameLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    forceFunction->writeXMLFile(element->getXMLElement(),ref);
    unloadedLength->writeXMLFile(element->getXMLElement(),ref);
    coilSpring->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  DirectionalSpringDamperPropertyDialog::DirectionalSpringDamperPropertyDialog(DirectionalSpringDamper *springDamper, QWidget *parent, Qt::WindowFlags f) : FloatingFrameLinkPropertyDialog(springDamper,parent,f) {

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new VecWidgetFactory(3),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics",forceDirection);

    forceFunction = new ExtWidget("Force function",new ChoiceWidget2(new SpringDamperWidgetFactory(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"forceFunction");
    addToTab("Kinetics", forceFunction);

    unloadedLength = new ExtWidget("Unloaded length",new ChoiceWidget2(new ScalarWidgetFactory("1"),QBoxLayout::RightToLeft,5),false,false,MBSIM%"unloadedLength");
    addToTab("General",unloadedLength);

    coilSpring = new ExtWidget("Enable openMBV",new CoilSpringMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBV");
    addToTab("Visualisation", coilSpring);
  }

  DOMElement* DirectionalSpringDamperPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    forceDirection->initializeUsingXML(element->getXMLElement());
    forceFunction->initializeUsingXML(element->getXMLElement());
    unloadedLength->initializeUsingXML(element->getXMLElement());
    coilSpring->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* DirectionalSpringDamperPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    forceDirection->writeXMLFile(element->getXMLElement(),ref);
    forceFunction->writeXMLFile(element->getXMLElement(),ref);
    unloadedLength->writeXMLFile(element->getXMLElement(),ref);
    coilSpring->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }


  JointPropertyDialog::JointPropertyDialog(Joint *joint, QWidget *parent, Qt::WindowFlags f) : FloatingFrameLinkPropertyDialog(joint,parent,f) {

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics",forceDirection);

    forceLaw = new ExtWidget("Force law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"forceLaw");
    addToTab("Kinetics",forceLaw);

    momentDirection = new ExtWidget("Moment direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"momentDirection");
    addToTab("Kinetics",momentDirection);

    momentLaw = new ExtWidget("Moment law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"momentLaw");
    addToTab("Kinetics",momentLaw);
  }

  DOMElement* JointPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    forceDirection->initializeUsingXML(element->getXMLElement());
    forceLaw->initializeUsingXML(element->getXMLElement());
    momentDirection->initializeUsingXML(element->getXMLElement());
    momentLaw->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* JointPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    forceDirection->writeXMLFile(element->getXMLElement(),ref);
    forceLaw->writeXMLFile(element->getXMLElement(),ref);
    momentDirection->writeXMLFile(element->getXMLElement(),ref);
    momentLaw->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  ElasticJointPropertyDialog::ElasticJointPropertyDialog(ElasticJoint *joint, QWidget *parent, Qt::WindowFlags f) : FloatingFrameLinkPropertyDialog(joint,parent,f) {

    forceDirection = new ExtWidget("Force direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"forceDirection");
    addToTab("Kinetics", forceDirection);

    momentDirection = new ExtWidget("Moment direction",new ChoiceWidget2(new MatColsVarWidgetFactory(3,1,vector<QStringList>(3,noUnitUnits()),vector<int>(3,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"momentDirection");
    addToTab("Kinetics", momentDirection);

    function = new ExtWidget("Generalized force function",new ChoiceWidget2(new SpringDamperWidgetFactory(joint),QBoxLayout::TopToBottom,0),false,false,MBSIM%"generalizedForceFunction");
    addToTab("Kinetics", function);
  }

  DOMElement* ElasticJointPropertyDialog::initializeUsingXML(DOMElement *parent) {
    FloatingFrameLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    forceDirection->initializeUsingXML(element->getXMLElement());
    momentDirection->initializeUsingXML(element->getXMLElement());
    function->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* ElasticJointPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    FloatingFrameLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    forceDirection->writeXMLFile(element->getXMLElement(),ref);
    momentDirection->writeXMLFile(element->getXMLElement(),ref);
    function->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  GeneralizedSpringDamperPropertyDialog::GeneralizedSpringDamperPropertyDialog(DualRigidBodyLink *springDamper, QWidget *parent, Qt::WindowFlags f) : DualRigidBodyLinkPropertyDialog(springDamper,parent,f) {

    function = new ExtWidget("Generalized force function",new ChoiceWidget2(new SpringDamperWidgetFactory(springDamper),QBoxLayout::TopToBottom,0),false,false,MBSIM%"generalizedForceFunction");
    addToTab("Kinetics", function);

    unloadedLength = new ExtWidget("Generalized Unloaded length",new ChoiceWidget2(new ScalarWidgetFactory("0"),QBoxLayout::RightToLeft,5),false,false,MBSIM%"generalizedUnloadedLength");
    addToTab("General",unloadedLength);
  }

  DOMElement* GeneralizedSpringDamperPropertyDialog::initializeUsingXML(DOMElement *parent) {
    DualRigidBodyLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    function->initializeUsingXML(element->getXMLElement());
    unloadedLength->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedSpringDamperPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    DualRigidBodyLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    function->writeXMLFile(element->getXMLElement(),ref);
    unloadedLength->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  GeneralizedFrictionPropertyDialog::GeneralizedFrictionPropertyDialog(DualRigidBodyLink *friction, QWidget *parent, Qt::WindowFlags f) : DualRigidBodyLinkPropertyDialog(friction,parent,f) {

    function = new ExtWidget("Generalized friction force law",new ChoiceWidget2(new FrictionForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"generalizedFrictionForceLaw");
    addToTab("Kinetics", function);

    normalForce = new ExtWidget("Generalized normal force function",new ChoiceWidget2(new FunctionWidgetFactory2(friction),QBoxLayout::TopToBottom,0),true,false,MBSIM%"generalizedNormalForceFunction");
    addToTab("Kinetics",normalForce);
  }

  DOMElement* GeneralizedFrictionPropertyDialog::initializeUsingXML(DOMElement *parent) {
    DualRigidBodyLinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    function->initializeUsingXML(element->getXMLElement());
    normalForce->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* GeneralizedFrictionPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    DualRigidBodyLinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    function->writeXMLFile(element->getXMLElement(),ref);
    normalForce->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }


  GeneralizedGearPropertyDialog::GeneralizedGearPropertyDialog(RigidBodyLink *constraint, QWidget *parent, Qt::WindowFlags f) : RigidBodyLinkPropertyDialog(constraint,parent,f) {
    addTab("Kinetics",1);
    addTab("Visualisation",2);

    gearOutput = new ExtWidget("Gear output",new RigidBodyOfReferenceWidget(constraint,0));
    addToTab("General",gearOutput);

    gearInput = new ExtWidget("Gear inputs",new ListWidget(new GeneralizedGearConstraintWidgetFactory(constraint,0),"Gear input"));
    addToTab("General",gearInput);

    function = new ExtWidget("Generalized force law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"generalizedForceLaw");
    addToTab("Kinetics",function);

    connect(buttonResize, SIGNAL(clicked(bool)), this, SLOT(resizeVariables()));
  }

  GeneralizedElasticConnectionPropertyDialog::GeneralizedElasticConnectionPropertyDialog(DualRigidBodyLink *connection, QWidget *parent, Qt::WindowFlags f) : DualRigidBodyLinkPropertyDialog(connection,parent,f) {

    function = new ExtWidget("Generalized force function",new ChoiceWidget2(new SpringDamperWidgetFactory(connection)));
    addToTab("Kinetics", function);

    connect(function,SIGNAL(resize_()),this,SLOT(resizeVariables()));
    connect(buttonResize,SIGNAL(clicked(bool)),this,SLOT(resizeVariables()));
    connect(connections->getWidget(),SIGNAL(resize_()),this,SLOT(resizeVariables()));
  }

  void GeneralizedElasticConnectionPropertyDialog::resizeVariables() {
//    RigidBodyOfReferenceWidget* widget = static_cast<ConnectRigidBodiesWidget*>(static_cast<ChoiceWidget2*>(connections->getWidget())->getWidget())->getWidget(0);
//    if(not widget->getBody().isEmpty()) {
//      int size = element->getByPath<RigidBody>(widget->getBody().toStdString())->getuRelSize();
//      function->resize_(size,size);
//    }
  }

  ContactPropertyDialog::ContactPropertyDialog(Contact *contact, QWidget *parent, Qt::WindowFlags f) : LinkPropertyDialog(contact,parent,f) {

    addTab("Kinetics",1);
    addTab("Extra");

    connections = new ExtWidget("Connections",new ConnectContoursWidget(2,contact),false,false,MBSIM%"connect");
    addToTab("Kinetics", connections);

    contactForceLaw = new ExtWidget("Normal force law",new ChoiceWidget2(new GeneralizedForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"normalForceLaw");
    addToTab("Kinetics", contactForceLaw);

    contactImpactLaw = new ExtWidget("Normal impact law",new ChoiceWidget2(new GeneralizedImpactLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"normalImpactLaw");
    addToTab("Kinetics", contactImpactLaw);

    frictionForceLaw = new ExtWidget("Tangential force law",new ChoiceWidget2(new FrictionForceLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"tangentialForceLaw");
    addToTab("Kinetics", frictionForceLaw);

    frictionImpactLaw = new ExtWidget("Tangential impact law",new ChoiceWidget2(new FrictionImpactLawWidgetFactory,QBoxLayout::TopToBottom,0),true,false,MBSIM%"tangentialImpactLaw");
    addToTab("Kinetics", frictionImpactLaw);

    searchAllContactPoints = new ExtWidget("Search all contact points",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft,5),true,false,MBSIM%"searchAllContactPoints");
    addToTab("Extra", searchAllContactPoints);

    initialGuess = new ExtWidget("Initial guess",new ChoiceWidget2(new VecSizeVarWidgetFactory(0),QBoxLayout::RightToLeft,5),true,false,MBSIM%"initialGuess");
    addToTab("Extra", initialGuess);
  }

  DOMElement* ContactPropertyDialog::initializeUsingXML(DOMElement *parent) {
    LinkPropertyDialog::initializeUsingXML(element->getXMLElement());
    connections->initializeUsingXML(element->getXMLElement());
    contactForceLaw->initializeUsingXML(element->getXMLElement());
    contactImpactLaw->initializeUsingXML(element->getXMLElement());
    frictionForceLaw->initializeUsingXML(element->getXMLElement());
    frictionImpactLaw->initializeUsingXML(element->getXMLElement());
    searchAllContactPoints->initializeUsingXML(element->getXMLElement());
    initialGuess->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* ContactPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    LinkPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    connections->writeXMLFile(element->getXMLElement(),ref);
    contactForceLaw->writeXMLFile(element->getXMLElement(),ref);
    contactImpactLaw->writeXMLFile(element->getXMLElement(),ref);
    frictionForceLaw->writeXMLFile(element->getXMLElement(),ref);
    frictionImpactLaw->writeXMLFile(element->getXMLElement(),ref);
    searchAllContactPoints->writeXMLFile(element->getXMLElement(),ref);
    initialGuess->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  ObserverPropertyDialog::ObserverPropertyDialog(Observer *observer, QWidget * parent, Qt::WindowFlags f) : ElementPropertyDialog(observer,parent,f) {
  }

  KinematicCoordinatesObserverPropertyDialog::KinematicCoordinatesObserverPropertyDialog(KinematicCoordinatesObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    frame = new ExtWidget("Frame",new FrameOfReferenceWidget(observer,0),true,false,MBSIM%"frame");
    addToTab("General", frame);

    frameOfReference = new ExtWidget("Frame of reference",new FrameOfReferenceWidget(observer,0),true,false,MBSIM%"frameOfReference");
    addToTab("General", frame);

    position = new ExtWidget("OpenMBV position arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVPosition");
    addToTab("Visualisation",position);

    velocity = new ExtWidget("OpenMBV velocity arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVVelocity");
    addToTab("Visualisation",velocity);

    acceleration = new ExtWidget("OpenMBV acceleration arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAcceleration");
    addToTab("Visualisation",acceleration);
  }

  DOMElement* KinematicCoordinatesObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(element->getXMLElement());
    frame->initializeUsingXML(element->getXMLElement());
    frameOfReference->initializeUsingXML(element->getXMLElement());
    position->initializeUsingXML(element->getXMLElement());
    velocity->initializeUsingXML(element->getXMLElement());
    acceleration->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* KinematicCoordinatesObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    frame->writeXMLFile(element->getXMLElement(),ref);
    frameOfReference->writeXMLFile(element->getXMLElement(),ref);
    position->writeXMLFile(element->getXMLElement(),ref);
    velocity->writeXMLFile(element->getXMLElement(),ref);
    acceleration->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  RelativeKinematicsObserverPropertyDialog::RelativeKinematicsObserverPropertyDialog(RelativeKinematicsObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    frame = new ExtWidget("Frame",new FrameOfReferenceWidget(observer,0),true,false,MBSIM%"frame");
    addToTab("General", frame);

    refFrame = new ExtWidget("Frame of reference",new FrameOfReferenceWidget(observer,0),true,false,MBSIM%"frameOfReference");
    addToTab("General", frame);

    position = new ExtWidget("OpenMBV position arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVPosition");
    addToTab("Visualisation",position);

    velocity = new ExtWidget("OpenMBV velocity arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVVelocity");
    addToTab("Visualisation",velocity);

    angularVelocity = new ExtWidget("OpenMBV angular velocity arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAngularVelocity");
    addToTab("Visualisation",angularVelocity);

    acceleration = new ExtWidget("OpenMBV acceleration arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAcceleration");
    addToTab("Visualisation",acceleration);

    angularAcceleration = new ExtWidget("OpenMBV angular acceleration arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAngularAcceleration");
    addToTab("Visualisation",angularAcceleration);
  }

  DOMElement* RelativeKinematicsObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(element->getXMLElement());
    frame->initializeUsingXML(element->getXMLElement());
    refFrame->initializeUsingXML(element->getXMLElement());
    position->initializeUsingXML(element->getXMLElement());
    velocity->initializeUsingXML(element->getXMLElement());
    angularVelocity->initializeUsingXML(element->getXMLElement());
    acceleration->initializeUsingXML(element->getXMLElement());
    angularAcceleration->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* RelativeKinematicsObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    frame->writeXMLFile(element->getXMLElement(),ref);
    refFrame->writeXMLFile(element->getXMLElement(),ref);
    position->writeXMLFile(element->getXMLElement(),ref);
    velocity->writeXMLFile(element->getXMLElement(),ref);
    angularVelocity->writeXMLFile(element->getXMLElement(),ref);
    acceleration->writeXMLFile(element->getXMLElement(),ref);
    angularAcceleration->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  MechanicalLinkObserverPropertyDialog::MechanicalLinkObserverPropertyDialog(MechanicalLinkObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    link = new ExtWidget("Mechanical link",new LinkOfReferenceWidget(observer,0),true,false,MBSIM%"mechanicalLink");
    addToTab("General", link);

    forceArrow = new ExtWidget("OpenMBV force arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVForce");
    addToTab("Visualisation",forceArrow);

    momentArrow = new ExtWidget("OpenMBV moment arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVMoment");
    addToTab("Visualisation",momentArrow);
  }

  DOMElement* MechanicalLinkObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(element->getXMLElement());
    link->initializeUsingXML(element->getXMLElement());
    forceArrow->initializeUsingXML(element->getXMLElement());
    momentArrow->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* MechanicalLinkObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    link->writeXMLFile(element->getXMLElement(),ref);
    forceArrow->writeXMLFile(element->getXMLElement(),ref);
    momentArrow->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  MechanicalConstraintObserverPropertyDialog::MechanicalConstraintObserverPropertyDialog(MechanicalConstraintObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    constraint = new ExtWidget("Mechanical constraint",new ConstraintOfReferenceWidget(observer,0),true,false,MBSIM%"mechanicalConstraint");
    addToTab("General", constraint);

    forceArrow = new ExtWidget("OpenMBV force arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVForce");
    addToTab("Visualisation",forceArrow);

    momentArrow = new ExtWidget("OpenMBV moment arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVMoment");
    addToTab("Visualisation",momentArrow);
  }

  DOMElement* MechanicalConstraintObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(element->getXMLElement());
    constraint->initializeUsingXML(element->getXMLElement());
    forceArrow->initializeUsingXML(element->getXMLElement());
    momentArrow->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* MechanicalConstraintObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    constraint->writeXMLFile(element->getXMLElement(),ref);
    forceArrow->writeXMLFile(element->getXMLElement(),ref);
    momentArrow->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  ContactObserverPropertyDialog::ContactObserverPropertyDialog(ContactObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    link = new ExtWidget("Mechanical link",new LinkOfReferenceWidget(observer,0),true,false,MBSIM%"contact");
    addToTab("General", link);

    forceArrow = new ExtWidget("OpenMBV force arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVForce");
    addToTab("Visualisation",forceArrow);

    momentArrow = new ExtWidget("OpenMBV force arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVMoment");
    addToTab("Visualisation",momentArrow);

    contactPoints = new ExtWidget("OpenMBV contact points",new FrameMBSOMBVWidget("NOTSET"),true,true,MBSIM%"enableOpenMBVContactPoints");
    addToTab("Visualisation",contactPoints);

    normalForceArrow = new ExtWidget("OpenMBV normal force arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVNormalForce");
    addToTab("Visualisation",normalForceArrow);

    frictionArrow = new ExtWidget("OpenMBV tangential force arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVTangentialForce");
    addToTab("Visualisation",frictionArrow);
  }

  DOMElement* ContactObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(element->getXMLElement());
    link->initializeUsingXML(element->getXMLElement());
    forceArrow->initializeUsingXML(element->getXMLElement());
    momentArrow->initializeUsingXML(element->getXMLElement());
    contactPoints->initializeUsingXML(element->getXMLElement());
    normalForceArrow->initializeUsingXML(element->getXMLElement());
    frictionArrow->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* ContactObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    link->writeXMLFile(element->getXMLElement(),ref);
    forceArrow->writeXMLFile(element->getXMLElement(),ref);
    momentArrow->writeXMLFile(element->getXMLElement(),ref);
    contactPoints->writeXMLFile(element->getXMLElement(),ref);
    normalForceArrow->writeXMLFile(element->getXMLElement(),ref);
    frictionArrow->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  FrameObserverPropertyDialog::FrameObserverPropertyDialog(FrameObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    frame = new ExtWidget("Frame",new FrameOfReferenceWidget(observer,0),true,false,MBSIM%"frame");
    addToTab("General", frame);

    position = new ExtWidget("OpenMBV position arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVPosition");
    addToTab("Visualisation",position);

    velocity = new ExtWidget("OpenMBV velocity arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVVelocity");
    addToTab("Visualisation",velocity);

    angularVelocity = new ExtWidget("OpenMBV angular velocity arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAngularVelocity");
    addToTab("Visualisation",angularVelocity);

    acceleration = new ExtWidget("OpenMBV acceleration arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAcceleration");
    addToTab("Visualisation",acceleration);

    angularAcceleration = new ExtWidget("OpenMBV angular acceleration arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAngularAcceleration");
    addToTab("Visualisation",angularAcceleration);
  }

  DOMElement* FrameObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(element->getXMLElement());
    frame->initializeUsingXML(element->getXMLElement());
    position->initializeUsingXML(element->getXMLElement());
    velocity->initializeUsingXML(element->getXMLElement());
    angularVelocity->initializeUsingXML(element->getXMLElement());
    acceleration->initializeUsingXML(element->getXMLElement());
    angularAcceleration->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* FrameObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    frame->writeXMLFile(element->getXMLElement(),ref);
    position->writeXMLFile(element->getXMLElement(),ref);
    velocity->writeXMLFile(element->getXMLElement(),ref);
    angularVelocity->writeXMLFile(element->getXMLElement(),ref);
    acceleration->writeXMLFile(element->getXMLElement(),ref);
    angularAcceleration->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
  }

  RigidBodyObserverPropertyDialog::RigidBodyObserverPropertyDialog(RigidBodyObserver *observer, QWidget *parent, Qt::WindowFlags f) : ObserverPropertyDialog(observer,parent,f) {

    addTab("Visualisation",1);

    body = new ExtWidget("Rigid body",new RigidBodyOfReferenceWidget(observer,0),true,false,MBSIM%"rigidBody");
    addToTab("General", body);

    weight = new ExtWidget("OpenMBV weight arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVWeight");
    addToTab("Visualisation",weight);

    jointForce = new ExtWidget("OpenMBV joint force arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVJointForce");
    addToTab("Visualisation",jointForce);

    jointMoment = new ExtWidget("OpenMBV joint moment arrow",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVJointMoment");
    addToTab("Visualisation",jointMoment);

    axisOfRotation = new ExtWidget("OpenMBV axis of rotation",new ArrowMBSOMBVWidget("NOTSET"),true,false,MBSIM%"enableOpenMBVAxisOfRotation");
    addToTab("Visualisation",axisOfRotation);
  }

  DOMElement* RigidBodyObserverPropertyDialog::initializeUsingXML(DOMElement *parent) {
    ObserverPropertyDialog::initializeUsingXML(element->getXMLElement());
    body->initializeUsingXML(element->getXMLElement());
    weight->initializeUsingXML(element->getXMLElement());
    jointForce->initializeUsingXML(element->getXMLElement());
    jointMoment->initializeUsingXML(element->getXMLElement());
    axisOfRotation->initializeUsingXML(element->getXMLElement());
    return parent;
  }

  DOMElement* RigidBodyObserverPropertyDialog::writeXMLFile(DOMNode *parent, DOMNode *ref) {
    ObserverPropertyDialog::writeXMLFile(element->getXMLElement(),ref);
    body->writeXMLFile(element->getXMLElement(),ref);
    weight->writeXMLFile(element->getXMLElement(),ref);
    jointForce->writeXMLFile(element->getXMLElement(),ref);
    jointMoment->writeXMLFile(element->getXMLElement(),ref);
    axisOfRotation->writeXMLFile(element->getXMLElement(),ref);
    return NULL;
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

  AbsolutePositionSensorPropertyDialog::AbsolutePositionSensorPropertyDialog(AbsolutePositionSensor *sensor, QWidget * parent, Qt::WindowFlags f) : AbsoluteCoordinateSensorPropertyDialog(sensor,parent,f) {
  }

  AbsoluteVelocitySensorPropertyDialog::AbsoluteVelocitySensorPropertyDialog(AbsoluteVelocitySensor *sensor, QWidget * parent, Qt::WindowFlags f) : AbsoluteCoordinateSensorPropertyDialog(sensor,parent,f) {
  }

  AbsoluteAngularPositionSensorPropertyDialog::AbsoluteAngularPositionSensorPropertyDialog(AbsoluteAngularPositionSensor *sensor, QWidget * parent, Qt::WindowFlags f) : AbsoluteCoordinateSensorPropertyDialog(sensor,parent,f) {
  }

  AbsoluteAngularVelocitySensorPropertyDialog::AbsoluteAngularVelocitySensorPropertyDialog(AbsoluteAngularVelocitySensor *sensor, QWidget * parent, Qt::WindowFlags f) : AbsoluteCoordinateSensorPropertyDialog(sensor,parent,f) {
  }

  FunctionSensorPropertyDialog::FunctionSensorPropertyDialog(FunctionSensor *sensor, QWidget * parent, Qt::WindowFlags f) : SensorPropertyDialog(sensor,parent,f) {
    function = new ExtWidget("Function",new ChoiceWidget2(new FunctionWidgetFactory2(sensor)));
    addToTab("General", function);
  }

  SignalProcessingSystemSensorPropertyDialog::SignalProcessingSystemSensorPropertyDialog(SignalProcessingSystemSensor *sensor, QWidget * parent, Qt::WindowFlags f) : SensorPropertyDialog(sensor,parent,f) {
    //spsRef = new ExtWidget("Signal processing system",new LinkOfReferenceWidget(sensor,0));
    addToTab("General", spsRef);
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

  UnarySignalOperationPropertyDialog::UnarySignalOperationPropertyDialog(UnarySignalOperation *signal, QWidget * parent, Qt::WindowFlags f_) : SignalPropertyDialog(signal,parent,f_) {
    sRef = new ExtWidget("Input signal",new SignalOfReferenceWidget(signal,0));
    addToTab("General", sRef);

    f = new ExtWidget("Function",new ChoiceWidget2(new SymbolicFunctionWidgetFactory3(QStringList("x"))));
    addToTab("General", f);
  }

  BinarySignalOperationPropertyDialog::BinarySignalOperationPropertyDialog(BinarySignalOperation *signal, QWidget * parent, Qt::WindowFlags f_) : SignalPropertyDialog(signal,parent,f_) {
    s1Ref = new ExtWidget("Input 1 signal",new SignalOfReferenceWidget(signal,0));
    addToTab("General", s1Ref);

    s2Ref = new ExtWidget("Input 2 signal",new SignalOfReferenceWidget(signal,0));
    addToTab("General", s2Ref);

    QStringList var;
    var << "x1" << "x2";
    f = new ExtWidget("Function",new ChoiceWidget2(new SymbolicFunctionWidgetFactory2(var,signal)));
    addToTab("General", f);
  }

  ExternSignalSourcePropertyDialog::ExternSignalSourcePropertyDialog(ExternSignalSource *signal, QWidget * parent, Qt::WindowFlags f) : SignalPropertyDialog(signal,parent,f) {
    sourceSize = new ExtWidget("Lenght of input vector",new SpinBoxWidget(1, 1, 1000));
    addToTab("General", sourceSize);
  }

  ExternSignalSinkPropertyDialog::ExternSignalSinkPropertyDialog(ExternSignalSink *signal, QWidget * parent, Qt::WindowFlags f) : SignalPropertyDialog(signal,parent,f) {
    inputSignal = new ExtWidget("Signal of reference",new SignalOfReferenceWidget(signal,0));
    addToTab("General", inputSignal);
  }

}
