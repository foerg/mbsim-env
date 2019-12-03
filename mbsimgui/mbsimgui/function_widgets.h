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

#ifndef _FUNCTION_WIDGETS_H_
#define _FUNCTION_WIDGETS_H_

#include "function_widget.h"

class QVBoxLayout;
class QComboBox;
class QListWidget;
class QSpinBox;

namespace MBSimGUI {

  class ExtPhysicalVarWidget;
  class ExtWidget;
  class ListWidget;
  class ChoiceWidget;
  class ChoiceWidget2;
  class Element;

  class IdentityFunctionWidget : public FunctionWidget {

    public:
      IdentityFunctionWidget(int m=1) { }
      QString getType() const override { return "IdentityFunction"; }
  };

  class ConstantFunctionWidget : public FunctionWidget {

    public:
      ConstantFunctionWidget(int m=1);
      QString getType() const override { return "ConstantFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *a0;
  };

  class LinearFunctionWidget : public FunctionWidget {

    public:
      LinearFunctionWidget(int m=1);
      QString getType() const override { return "LinearFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *a0, *a1;
  };

  class QuadraticFunctionWidget : public FunctionWidget {

    public:
      QuadraticFunctionWidget(int m=1);
      QString getType() const override { return "QuadraticFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *a0, *a1, *a2;
  };

  class PolynomFunctionWidget : public FunctionWidget {

    public:
      PolynomFunctionWidget(int m=1);
      QString getType() const override { return "PolynomFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *a;
  };

  class SinusoidalFunctionWidget : public FunctionWidget {

    public:
      SinusoidalFunctionWidget(int m=1);
      QString getType() const override { return "SinusoidalFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *a, *f, *p, *o;
  };

  class AbsoluteValueFunctionWidget : public FunctionWidget {

    public:
      AbsoluteValueFunctionWidget(int m=0) { }
      QString getType() const override { return "AbsoluteValueFunction"; }
  };

  class ModuloFunctionWidget : public FunctionWidget {

    public:
      ModuloFunctionWidget(int m=0);
      QString getType() const override { return "ModuloFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *denom;
  };

  class BoundedFunctionWidget : public FunctionWidget {

    public:
      BoundedFunctionWidget(int m=0);
      QString getType() const override { return "BoundedFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *lowerBound, *upperBound;
  };

  class SignumFunctionWidget : public FunctionWidget {

    public:
      SignumFunctionWidget(int m=0) { }
      QString getType() const override { return "SignumFunction"; }
  };

  class VectorValuedFunctionWidget : public FunctionWidget {

    public:
      VectorValuedFunctionWidget(Element *element, int m, bool fixedSize, QWidget *parent);
      QString getType() const override { return "VectorValuedFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *functions;
  };

  class CompositeFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      CompositeFunctionWidget(WidgetFactory *factoryo, WidgetFactory *factoryi);
      QString getType() const override { return "CompositeFunction"; }
      int getArg1Size() const override;
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
      void updateWidget() override;
    protected:
      QString ext;
      ExtWidget *fo, *fi;
  };

  class LimitedFunctionWidget : public FunctionWidget {

    public:
      LimitedFunctionWidget(Element *element, QWidget *parent, const QString &var="x");
      QString getType() const override { return "LimitedFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *function, *limit;
  };

  class PiecewiseDefinedFunctionWidget : public FunctionWidget {

    public:
      PiecewiseDefinedFunctionWidget(Element *element, int n, QWidget *parent, const QString &var="x");
      QString getType() const override { return "PiecewiseDefinedFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *functions, *shiftAbscissa, *shiftOrdinate;
  };

  class SymbolicFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      SymbolicFunctionWidget(const QStringList &var, int m, int max, bool fixedSize=true, bool scalar=false);
      QString getType() const override { return "SymbolicFunction"; }
      int getArg1Size() const override;
      int getArg2Size() const override;
      void setArg1Size(int i) override;
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *f;
      std::vector<ExtWidget*> argname, argdim;
  };

  class TabularFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      TabularFunctionWidget(int n);
      QString getType() const override { return "TabularFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ChoiceWidget2* choice;
    protected slots:
      void choiceChanged();
      void updateWidget() override;
  };

  class TwoDimensionalTabularFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      TwoDimensionalTabularFunctionWidget(int n);
      QString getType() const override { return "TwoDimensionalTabularFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ChoiceWidget2* choice;
    protected slots:
      void choiceChanged();
      void updateWidget() override;
  };

  class PiecewisePolynomFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      PiecewisePolynomFunctionWidget(int n);
      QString getType() const override { return "PiecewisePolynomFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ChoiceWidget2* choice;
      ExtWidget *method;
    protected slots:
      void choiceChanged();
      void updateWidget() override;
  };

  class TwoDimensionalPiecewisePolynomFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      TwoDimensionalPiecewisePolynomFunctionWidget(int n);
      QString getType() const override { return "TwoDimensionalPiecewisePolynomFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ChoiceWidget2* choice;
      ExtWidget *method;
    protected slots:
      void choiceChanged();
      void updateWidget() override;
  };

  class FourierFunctionWidget : public FunctionWidget {

    public:
      FourierFunctionWidget(int n);
      QString getType() const override { return "FourierFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *f, *a0, *amplitudePhaseAngleForm;
      ChoiceWidget2* choice;
  };

  class BidirectionalFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      BidirectionalFunctionWidget(QWidget *parent);
      QString getType() const override { return "BidirectionalFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *fn, *fp;
  };

  class ContinuedFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      ContinuedFunctionWidget(WidgetFactory *factoryf, WidgetFactory *factoryr);
      QString getType() const override { return "ContinuedFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *f, *r;
  };

  class LinearSpringDamperForceWidget : public FunctionWidget {

    public:
      LinearSpringDamperForceWidget();
      QString getType() const override { return "LinearSpringDamperForce"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *c, *d;
  };

  class NonlinearSpringDamperForceWidget : public FunctionWidget {

    public:
      NonlinearSpringDamperForceWidget(Element *element, QWidget *parent);
      QString getType() const override { return "NonlinearSpringDamperForce"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *s, *sd;
  };

  class LinearElasticFunctionWidget : public FunctionWidget {
    Q_OBJECT

    public:
      LinearElasticFunctionWidget(bool varSize=false);
      QString getType() const override { return "LinearElasticFunction"; }
      void resize_(int m, int n) override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    private:
      ExtWidget *K, *D;
    private slots:
      void updateWidget();
  };

  class LinearRegularizedBilateralConstraintWidget: public FunctionWidget {

    public:
      LinearRegularizedBilateralConstraintWidget();
      QString getType() const override { return "LinearRegularizedBilateralConstraint"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    private:
      ExtWidget *c, *d;
  };

  class LinearRegularizedUnilateralConstraintWidget: public FunctionWidget {

    public:
      LinearRegularizedUnilateralConstraintWidget();
      QString getType() const override { return "LinearRegularizedUnilateralConstraint"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    private:
      ExtWidget *c, *d;
  };

  class LinearRegularizedCoulombFrictionWidget: public FunctionWidget {

    public:
      LinearRegularizedCoulombFrictionWidget();
      QString getType() const override { return "LinearRegularizedCoulombFriction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    private:
      ExtWidget *gd, *mu;
  };

  class LinearRegularizedStribeckFrictionWidget: public FunctionWidget {

    public:
      LinearRegularizedStribeckFrictionWidget();
      QString getType() const override { return "LinearRegularizedStribeckFriction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    private:
      ExtWidget *gd, *mu;
  };

  class SignalFunctionWidget: public FunctionWidget {

    public:
      SignalFunctionWidget(Element *element, QWidget *parent);
      ~SignalFunctionWidget() override;
      QString getType() const override { return "SignalFunction"; }
      MBXMLUtils::NamespaceURI getNameSpace() const override { return MBSIMCONTROL; }
      void updateWidget() override;
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    private:
      ExtWidget *sRef;
      Element *dummy;
  };

  class PolarContourFunctionWidget : public FunctionWidget {

    public:
      PolarContourFunctionWidget(QWidget *parent, const QString &var="phi");
      QString getType() const override { return "PolarContourFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *radiusFunction;
  };

  class GravityFunctionWidget : public FunctionWidget {

    public:
      GravityFunctionWidget();
      QString getType() const override { return "GravityFunction"; }
      MBXMLUtils::NamespaceURI getNameSpace() const override { return MBSIMPHYSICS; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element) override;
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *parent, xercesc::DOMNode *ref=nullptr) override;
    protected:
      ExtWidget *standardGravity, *meanRadius;
  };

}

#endif
