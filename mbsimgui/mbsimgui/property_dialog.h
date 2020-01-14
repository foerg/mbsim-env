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

#ifndef _PROPERTY_DIALOG_H_
#define _PROPERTY_DIALOG_H_

#include <QScrollArea>
#include <QTabWidget>
#include <QDialog>
#include <map>
#include <xercesc/util/XercesDefs.hpp>
#include <mbxmlutilshelper/dom.h>

namespace XERCES_CPP_NAMESPACE {
  class DOMElement;
  class DOMNode;
}

class QVBoxLayout;
class QDialogButtonBox;
class QAbstractButton;

namespace MBSimGUI {

  class EmbedItemData;

  class PropertyDialog : public QDialog {
    Q_OBJECT

    public:
      PropertyDialog();
      void setParentObject(QObject *obj);
      void addToTab(const QString &name, QWidget* widget_);
      void addTab(const QString &name, int i=-1);
      void addStretch(int s=1);
      void setCancel(bool on);
      bool getCancel() const;
      virtual void toWidget() { }
      virtual void fromWidget() { }
    protected:
      std::map<QString,QVBoxLayout*> layout;
      QTabWidget *tabWidget;
      QDialogButtonBox *buttonBox;
      QPushButton *buttonResize;
      virtual void showXMLHelp() { }
      void showEvent(QShowEvent *event) override;
      void hideEvent(QHideEvent *event) override;
      void closeEvent(QCloseEvent *event) override;
    public slots:
      void clicked(QAbstractButton *button);
      virtual void updateWidget() { }
    signals:
      void apply();
  };

  class EmbedItemPropertyDialog : public PropertyDialog {

    public:
      EmbedItemPropertyDialog(EmbedItemData *item_);
      virtual xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) { return nullptr; }
      virtual xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=nullptr) { return nullptr; }
      EmbedItemData* getItem() const { return item; }
      void toWidget() override;
      void fromWidget() override;
    protected:
      EmbedItemData *item;
  };

}

#endif
