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

#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include <QDialog>
#include <QPushButton>
#include <QTreeWidgetItem>
#include "variable_widgets.h"

class Element;
class Object;
class RigidBody;
class Frame;
class Contour;
class Signal;

class ElementItem : public QTreeWidgetItem {
  private:
    Element* element;
  public:
    ElementItem(Element *element_) : element(element_) {}
    Element* getElement() const {return element;}
};

class EvalDialog : public QDialog {
  public:
    EvalDialog();
    void setValue(const std::vector<std::vector<QString> > &A) {var->setMat(A);}
    //QString getValue() const {return var->getValue();}
  protected:
    MatWidget *var;
};

class ObjectBrowser : public QDialog {
  Q_OBJECT

  public:
    ObjectBrowser(Element* element, Object* selection, QWidget *obj);
    ~ObjectBrowser() {}
    QTreeWidget* getObjectList() const {return objectList;}
    void updateWidget(Object *object);
  protected:
    QPushButton *okButton;
    QTreeWidget *objectList;
    Object *selection;
    ElementItem *savedItem;
    Element* element;
    std::string oldID;
    void mbs2ObjectTree(Element* item, QTreeWidgetItem* parentItem);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
  protected slots:
    void checkForObject(QTreeWidgetItem* item_,int);
};

class RigidBodyBrowser : public QDialog {
  Q_OBJECT

  public:
    RigidBodyBrowser(Element* element, RigidBody* selection, QWidget *obj);
    ~RigidBodyBrowser() {}
    QTreeWidget* getRigidBodyList() const {return rigidBodyList;}
    void updateWidget(RigidBody *rigidBody);
  protected:
    QPushButton *okButton;
    QTreeWidget *rigidBodyList;
    RigidBody *selection;
    ElementItem *savedItem;
    Element* element;
    std::string oldID;
    void mbs2RigidBodyTree(Element* item, QTreeWidgetItem* parentItem);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
  protected slots:
    void checkForRigidBody(QTreeWidgetItem* item_,int);
};

class FrameBrowser : public QDialog {
  Q_OBJECT

  public:
    FrameBrowser(Element* element, Frame* selection, QWidget *obj);
    ~FrameBrowser() {}
    QTreeWidget* getFrameList() const {return frameList;}
    void updateWidget(Frame *frame);
  protected:
    QPushButton *okButton;
    QTreeWidget *frameList;
    Frame *selection;
    ElementItem *savedItem;
    Element* element;
    std::string oldID;
    void mbs2FrameTree(Element* item, QTreeWidgetItem* parentItem);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
  protected slots:
    void checkForFrame(QTreeWidgetItem* item_,int);
};

class ContourBrowser : public QDialog {
  Q_OBJECT

  public:
    ContourBrowser(Element* element, Contour* selection, QWidget *obj);
    ~ContourBrowser() {}
    QTreeWidget* getContourList() const {return contourList;}
    void updateWidget(Contour *contour);
  protected:
    QPushButton *okButton;
    QTreeWidget *contourList;
    Contour *selection;
    ElementItem *savedItem;
    Element* element;
    std::string oldID;
    void mbs2ContourTree(Element* item, QTreeWidgetItem* parentItem);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
  protected slots:
    void checkForContour(QTreeWidgetItem* item_,int);
};

class SignalBrowser : public QDialog {
  Q_OBJECT

  public:
    SignalBrowser(Element* element, Signal* selection, QWidget *obj);
    ~SignalBrowser() {}
    QTreeWidget* getSignalList() const {return signalList;}
    void updateWidget(Signal *signal);
  protected:
    QPushButton *okButton;
    QTreeWidget *signalList;
    Signal *selection;
    ElementItem *savedItem;
    Element* element;
    void mbs2SignalTree(Element* item, QTreeWidgetItem* parentItem);
  protected slots:
    void checkForSignal(QTreeWidgetItem* item_,int);
};

#endif
