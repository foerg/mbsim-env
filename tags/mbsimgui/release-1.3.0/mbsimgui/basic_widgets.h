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

#ifndef _BASIC_WIDGETS_H_
#define _BASIC_WIDGETS_H_

#include "extended_widgets.h"
#include <QLineEdit>
#include <QSpinBox>

class Element;
class Object;
class RigidBody;
class Frame;
class Contour;
class Parameter;
class Signal;
class ExtraDynamic;
class QComboBox;
class QCheckBox;
class QStackedWidget;
class QListWidget;
class FrameBrowser;
class ContourBrowser;
class RigidBodyBrowser;
class ObjectBrowser;
class SignalBrowser;
class ExtraDynamicBrowser;
class ExtWidget;
class QLabel;

class LocalFrameComboBox : public QComboBox {
  Q_OBJECT
  public:
    LocalFrameComboBox(Element *element, QWidget *parent = 0);
  protected:
    Element *element;
    std::string oldID;
    virtual void showPopup();
    virtual void hidePopup();
  protected slots:
    void highlightObject(const QString &str);
};

class ParentFrameComboBox : public QComboBox {
  Q_OBJECT
  public:
    ParentFrameComboBox(Element *element, QWidget *parent = 0);
  protected:
    Element *element;
    std::string oldID;
    virtual void showPopup();
    virtual void hidePopup();
  protected slots:
    void highlightObject(const QString &str);
};

class LocalFrameOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    LocalFrameOfReferenceWidget(Element* element, Frame* omitFrame=0);

    void updateWidget();
    QString getFrame() const;
    void setFrame(const QString &str, Frame *framePtr);

  protected:
    QComboBox *frame;
    Element* element;
    Frame *selectedFrame, *omitFrame;

  protected slots:
    void setFrame(const QString &str);
};

class ParentFrameOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    ParentFrameOfReferenceWidget(Element* element, Frame* omitFrame=0);

    void updateWidget();
    QString getFrame() const; 
    void setFrame(const QString &str, Frame *framePtr);

  protected:
    QComboBox *frame;
    Element* element;
    Frame *selectedFrame, *omitFrame;

  protected slots:
    void setFrame(const QString &str);
};

class FrameOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    FrameOfReferenceWidget(Element* element, Frame* selectedFrame);

    void updateWidget();
    void setFrame(const QString &str, Frame *framePtr);
    QString getFrame() const;

  protected:
    QLineEdit *frame;
    Element* element;
    FrameBrowser *frameBrowser;
    Frame *selectedFrame;

  public slots:
    void setFrame(); 
};

class ContourOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    ContourOfReferenceWidget(Element* element, Contour* selectedContour);

    void updateWidget();
    void setContour(const QString &str, Contour *contourPtr);
    QString getContour() const;

  protected:
    QLineEdit *contour;
    Element* element;
    ContourBrowser *contourBrowser;
    Contour *selectedContour;

  public slots:
    void setContour();
};

class RigidBodyOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    RigidBodyOfReferenceWidget(Element* element, RigidBody* selectedBody);

    void updateWidget();
    void setBody(const QString &str, RigidBody *bodyPtr);
    QString getBody() const;
    RigidBody* getSelectedBody() {return selectedBody;}

  protected:
    QLineEdit* body;
    Element* element;
    RigidBodyBrowser* bodyBrowser;
    RigidBody* selectedBody;

  public slots:
    void setBody();

  signals:
    void bodyChanged();
};

class ObjectOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    ObjectOfReferenceWidget(Element* element, Object* selectedObject);

    void updateWidget();
    void setObject(const QString &str, Object *objectPtr);
    QString getObject() const;
    Object* getSelectedObject() {return selectedObject;}

  protected:
    QLineEdit* object;
    Element* element;
    ObjectBrowser* objectBrowser;
    Object* selectedObject;

  public slots:
    void setObject();

  signals:
    void objectChanged();
};

class SignalOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    SignalOfReferenceWidget(Element* element, Signal* selectedSignal);

    void updateWidget();
    void setSignal(const QString &str, Signal *signalPtr);
    QString getSignal() const;
    Signal* getSelectedSignal() {return selectedSignal;}

  protected:
    QLineEdit* signal;
    Element* element;
    SignalBrowser* signalBrowser;
    Signal* selectedSignal;

  public slots:
    void setSignal();

  signals:
    void signalChanged();
};

class ExtraDynamicOfReferenceWidget : public Widget {
  Q_OBJECT

  public:
    ExtraDynamicOfReferenceWidget(Element* element, ExtraDynamic* selectedExtraDynamic);

    void updateWidget();
    void setExtraDynamic(const QString &str, ExtraDynamic *edPtr);
    QString getExtraDynamic() const;
    ExtraDynamic* getSelectedExtraDynamic() {return selectedExtraDynamic;}

  protected:
    QLineEdit* ed;
    Element* element;
    ExtraDynamicBrowser* edBrowser;
    ExtraDynamic* selectedExtraDynamic;

  public slots:
    void setExtraDynamic();

  signals:
    void edChanged();
};

class FileWidget : public Widget {
  Q_OBJECT

  public:
    FileWidget(const QString &description, const QString &extensions, int mode=0);
    QString getFile() const {return file;}
    void setFile(const QString &str);

  protected:
    QLineEdit *relativeFilePath;
    QString file, description, extensions;
    bool mode;

  protected slots:
    void selectFile();

  signals:
    void fileChanged(const QString &str);
};

class IntegerWidget : public Widget {

  public:
    virtual int getValue() = 0;
    virtual void setValue(int val) = 0;
};

class SpinBoxWidget : public IntegerWidget {

  public:
    SpinBoxWidget(int val=0, int min=0, int max=99);
    int getValue() {return value->value();}
    void setValue(int val) {value->setValue(val);}

  protected:
    QSpinBox *value;
};

class BasicTextWidget : public Widget {

  public:
    virtual QString getText() const = 0;
    virtual void setText(const QString &text) = 0;
};

class TextWidget : public BasicTextWidget {
  //Q_OBJECT

  public:
    TextWidget(const QString &text="", bool readOnly=false);

    QString getText() const {return text->text();}
    void setText(const QString &text_) {text->setText(text_);}

  //public slots:

  protected:
    QLineEdit *text;
};

class TextChoiceWidget : public BasicTextWidget {

  public:
    TextChoiceWidget(const std::vector<QString> &list, int num);
    QString getText() const {return text->currentText();}
    void setText(const QString &str) {text->setCurrentIndex(text->findText(str));}

  protected:
    QComboBox *text;
};

class ConnectFramesWidget : public Widget {

  friend class ConnectFramesProperty;

  public:
    ConnectFramesWidget(int n, Element* element);

    void updateWidget();

  protected:
    std::vector<FrameOfReferenceWidget*> widget;
    Element* element;
};

class ConnectContoursWidget : public Widget {

  friend class ConnectContoursProperty;

  public:
    ConnectContoursWidget(int n, Element* element);

    void updateWidget();

  protected:
    std::vector<ContourOfReferenceWidget*> widget;
    Element* element;
};

class DependenciesWidget : public Widget {
  Q_OBJECT

  friend class DependenciesProperty;

  public:
    DependenciesWidget(Element* element);

    void updateWidget(); 

    RigidBody* getSelectedBody(int i) {return refBody[i]->getSelectedBody();}
    void addBody(int i, RigidBody* body_);
    int getSize() const {return refBody.size();}
    void setNumberOfBodies(int n);

  protected:
    Element* element;
    std::vector<RigidBody*> selectedBody;
    std::vector<RigidBodyOfReferenceWidget*> refBody;
    QStackedWidget *stackedWidget; 
    QListWidget *bodyList; 

  protected slots:
    void updateList();
    void addDependency();
    void removeDependency();
    void openContextMenu(const QPoint &pos);

  signals:
    void bodyChanged();
};

class SolverChoiceWidget : public Widget {
  friend class SolverChoiceProperty;

  public:
    SolverChoiceWidget();
    QString getSolver() const {return choice->currentText();}
    void setSolver(const QString &str) {choice->setCurrentIndex(choice->findText(str));}

  protected:
    QComboBox *choice;
};

class SolverTolerancesWidget : public Widget {
  friend class SolverTolerancesProperty;

  public:
    SolverTolerancesWidget();

  protected:
    ExtWidget *projection, *g, *gd, *gdd, *la, *La;
};

class SolverParametersWidget : public Widget {

  friend class SolverParametersProperty;

  public:
    SolverParametersWidget();

  protected:
    ExtWidget *constraintSolver, *impactSolver, *numberOfMaximalIterations, *tolerances;
};

class PlotFeature : public Widget {
  public:
    PlotFeature(const QString &name);

  protected:
    QString name;
    QComboBox *status;
};

class GearDependencyWidget : public Widget {

  friend class GearDependencyProperty;

  public:
    GearDependencyWidget(Element* element);
    RigidBody* getSelectedBody() {return refBody->getSelectedBody();}
    RigidBodyOfReferenceWidget* getRigidBodyOfReferenceWidget() {return refBody;}
    void updateWidget() {refBody->updateWidget();}
  protected:
   RigidBodyOfReferenceWidget* refBody;
   ExtWidget *ratio;
};

class GearDependenciesWidget : public Widget {
  Q_OBJECT

  friend class GearDependenciesProperty;

  public:
    GearDependenciesWidget(Element* element);

    void updateWidget(); 

    RigidBody* getSelectedBody(int i) {return refBody[i]->getSelectedBody();}
    void addBody(int i, RigidBody* body_);
    int getSize() const {return refBody.size();}
    void setNumberOfBodies(int n);

  protected:
    Element* element;
    std::vector<RigidBody*> selectedBody;
    std::vector<GearDependencyWidget*> refBody;
    QStackedWidget *stackedWidget; 
    QListWidget *bodyList; 

  protected slots:
    void updateList();
    void addDependency();
    void removeDependency();
    void openContextMenu(const QPoint &pos);

  signals:
    void bodyChanged();
};

class EmbedWidget : public Widget {

  friend class EmbedProperty;

  public:
    EmbedWidget();

  protected:
    ExtWidget *href, *count, *counterName, *parameterList;

};

class SignalReferenceWidget : public Widget {

  friend class SignalReferenceProperty;

  public:
    SignalReferenceWidget(Element* element);
    Signal* getSelectedSignal() {return refSignal->getSelectedSignal();}
    SignalOfReferenceWidget* getSignalOfReferenceWidget() {return refSignal;}
    void updateWidget() {refSignal->updateWidget();}
  protected:
   SignalOfReferenceWidget* refSignal;
   ExtWidget *factor;
};

class SignalReferencesWidget : public Widget {
  Q_OBJECT

  friend class SignalReferencesProperty;

  public:
    SignalReferencesWidget(Element* element);

    void updateWidget(); 

    Signal* getSelectedSignal(int i) {return refSignal[i]->getSelectedSignal();}
    void addSignal(int i, Signal* signal_);
    int getSize() const {return refSignal.size();}
    void setNumberOfSignals(int n);

  protected:
    Element* element;
    std::vector<Signal*> selectedSignal;
    std::vector<SignalReferenceWidget*> refSignal;
    QStackedWidget *stackedWidget; 
    QListWidget *signalList; 

  protected slots:
    void updateList();
    void addReference();
    void removeReference();
    void openContextMenu(const QPoint &pos);
};

#endif
