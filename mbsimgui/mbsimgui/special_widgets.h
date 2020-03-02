/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2016 Martin Förg

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

#ifndef _SPECIAL_WIDGETS_H_
#define _SPECIAL_WIDGETS_H_

#include "widget.h"
#include "custom_widgets.h"

namespace MBSimGUI {

  class ExtWidget;
  class CustomSpinBox;

  class OneDimVecArrayWidget : public Widget {
    Q_OBJECT
    protected:
      std::vector<ExtWidget*> ele;
      CustomSpinBox* sizeCombo;
      int m;
    public:
      OneDimVecArrayWidget(int size=0, int m_=0, bool var=false);
      const std::vector<ExtWidget*>& getArray() const { return ele; }
      void resize_(int size, int m, int n);
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    public slots:
      void currentIndexChanged(int);
  };

  class OneDimMatArrayWidget : public Widget {
    protected:
      std::vector<ExtWidget*> ele;
    public:
      OneDimMatArrayWidget(int size=0, int m=0, int n=0);
      const std::vector<ExtWidget*>& getArray() const { return ele; }
      void resize_(int size, int m, int n);
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
  };

  class TwoDimMatArrayWidget: public Widget {
    protected:
      std::vector<std::vector<ExtWidget*>> ele;
    public:
      TwoDimMatArrayWidget(int size=0, int m=0, int n=0);
      const std::vector<std::vector<ExtWidget*>>& getArray() const { return ele; }
      void resize_(int rsize, int csize, int m, int n);
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
  };

  class OneDimVecArrayWidgetFactory : public WidgetFactory {
    public:
      OneDimVecArrayWidgetFactory(const MBXMLUtils::FQN &xmlBase, int size_=0, int m_=0, bool var_=false);
      QWidget* createWidget(int i=0) override;
      QString getName(int i=0) const override { return name[i]; }
      MBXMLUtils::FQN getXMLName(int i=0) const override { return xmlName[i]; }
      int getSize() const override { return name.size(); }
    protected:
      std::vector<QString> name;
      std::vector<MBXMLUtils::FQN> xmlName;
      int size, m;
      bool var;
  };

  class OneDimMatArrayWidgetFactory : public WidgetFactory {
    public:
      OneDimMatArrayWidgetFactory(const MBXMLUtils::FQN &xmlBase, int size_=0, int m_=0, int n_=0);
      QWidget* createWidget(int i=0) override;
      QString getName(int i=0) const override { return name[i]; }
      MBXMLUtils::FQN getXMLName(int i=0) const override { return xmlName[i]; }
      int getSize() const override { return name.size(); }
    protected:
      std::vector<QString> name;
      std::vector<MBXMLUtils::FQN> xmlName;
      int size, m, n;
  };

  class TwoDimMatArrayWidgetFactory : public WidgetFactory {
    public:
      TwoDimMatArrayWidgetFactory(const MBXMLUtils::FQN &xmlBase, int size_=0, int m_=0, int n_=0);
      QWidget* createWidget(int i=0) override;
      QString getName(int i=0) const override { return name[i]; }
      MBXMLUtils::FQN getXMLName(int i=0) const override { return xmlName[i]; }
      int getSize() const override { return name.size(); }
    protected:
      std::vector<QString> name;
      std::vector<MBXMLUtils::FQN> xmlName;
      int size, m, n;
  };

}

#endif
