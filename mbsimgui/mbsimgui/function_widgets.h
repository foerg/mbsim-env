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

    friend class IdentityFunction;

    public:
      IdentityFunctionWidget(int m=1) { }
      std::string getType() const { return "IdentityFunction"; }
  };

  class ConstantFunctionWidget : public FunctionWidget {

    friend class ConstantFunction;

    public:
      ConstantFunctionWidget(int m=1);
      std::string getType() const { return "ConstantFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *a0;
  };

  class LinearFunctionWidget : public FunctionWidget {

    friend class LinearFunction;

    public:
      LinearFunctionWidget(int m=1);
      std::string getType() const { return "LinearFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *a0, *a1;
  };

  class QuadraticFunctionWidget : public FunctionWidget {

    friend class QuadraticFunction;

    public:
      QuadraticFunctionWidget(int m=1);
      std::string getType() const { return "QuadraticFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *a0, *a1, *a2;
  };

  class PolynomFunctionWidget : public FunctionWidget {

    friend class PolynomFunction;

    public:
      PolynomFunctionWidget(int m=1);
      std::string getType() const { return "PolynomFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *a;
  };

  class SinusoidalFunctionWidget : public FunctionWidget {

    friend class SinusoidalFunction;

    public:
      SinusoidalFunctionWidget(int m=1);
      std::string getType() const { return "SinusoidalFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *a, *f, *p, *o;
  };

  class AbsoluteValueFunctionWidget : public FunctionWidget {

    friend class AbsoluteValueFunction;

    public:
      AbsoluteValueFunctionWidget(int m=0) { }
      std::string getType() const { return "AbsoluteValueFunction"; }
  };

  class ModuloFunctionWidget : public FunctionWidget {

    friend class ModuloFunction;

    public:
      ModuloFunctionWidget(int m=0);
      std::string getType() const { return "ModuloFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *denom;
  };

  class BoundedFunctionWidget : public FunctionWidget {

    friend class BoundedFunction;

    public:
      BoundedFunctionWidget(int m=0);
      std::string getType() const { return "BoundedFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *lowerBound, *upperBound;
  };

  class SignumFunctionWidget : public FunctionWidget {

    friend class SignumFunction;

    public:
      SignumFunctionWidget(int m=0) { }
      std::string getType() const { return "SignumFunction"; }
  };

  class VectorValuedFunctionWidget : public FunctionWidget {

    friend class VectorValuedFunction;

    public:
      VectorValuedFunctionWidget(Element *parent, int m=0, bool fixedSize=false);
      std::string getType() const { return "VectorValuedFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *functions;
  };

  class CompositeFunctionWidget : public FunctionWidget {
    Q_OBJECT

    friend class CompositeFunction;

    public:
      CompositeFunctionWidget(WidgetFactory *factoryo, WidgetFactory *factoryi);
      std::string getType() const { return "CompositeFunction"; }
      int getArg1Size() const;
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      QString ext;
      ExtWidget *fo, *fi;
    public slots:
      void resizeVariables();
  };

  class PiecewiseDefinedFunctionWidget : public FunctionWidget {

    friend class PiecewiseDefinedFunction;

    public:
      PiecewiseDefinedFunctionWidget(Element *parent, int n=0);
      std::string getType() const { return "PiecewiseDefinedFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *functions, *shiftAbscissa, *shiftOrdinate;
  };

  class SymbolicFunctionWidget : public FunctionWidget {
    Q_OBJECT

    friend class SymbolicFunction;

    public:
      SymbolicFunctionWidget(const QStringList &var, int m, int max);
      std::string getType() const { return "SymbolicFunction"; }
      int getArg1Size() const;
      int getArg2Size() const;
      void setArg1Size(int i);
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *f;
      std::vector<ExtWidget*> argname, argdim;
    signals:
      void arg1SizeChanged(int i);
  };

  class TabularFunctionWidget : public FunctionWidget {

    friend class TabularFunction;

    public:
      TabularFunctionWidget(int n);
      std::string getType() const { return "TabularFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ChoiceWidget2* choice;
  };

  class TwoDimensionalTabularFunctionWidget : public FunctionWidget {

    friend class TwoDimensionalTabularFunction;

    public:
      TwoDimensionalTabularFunctionWidget(int n);
      std::string getType() const { return "TwoDimensionalTabularFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ChoiceWidget2* choice;
  };

  class PiecewisePolynomFunctionWidget : public FunctionWidget {

    friend class PiecewisePolynomFunction;

    public:
      PiecewisePolynomFunctionWidget(int n);
      std::string getType() const { return "PiecewisePolynomFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ChoiceWidget2* choice;
      ExtWidget *method;
  };

  class TwoDimensionalPiecewisePolynomFunctionWidget : public FunctionWidget {

    friend class TwoDimensionalPiecewisePolynomFunction;

    public:
      TwoDimensionalPiecewisePolynomFunctionWidget(int n);
      std::string getType() const { return "TwoDimensionalPiecewisePolynomFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ChoiceWidget2* choice;
      ExtWidget *method;
  };

  class FourierFunctionWidget : public FunctionWidget {

    friend class FourierFunction;

    public:
      FourierFunctionWidget(int n);
      std::string getType() const { return "FourierFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *f, *a0, *amplitudePhaseAngleForm;
      ChoiceWidget2* choice;
  };

  class BidirectionalFunctionWidget : public FunctionWidget {
    Q_OBJECT

    friend class BidirectionalFunction;

    public:
      BidirectionalFunctionWidget();
      std::string getType() const { return "BidirectionalFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *fn, *fp;
  };

  class ContinuedFunctionWidget : public FunctionWidget {
    Q_OBJECT

    friend class ContinuedFunction;

    public:
      ContinuedFunctionWidget(WidgetFactory *factoryf, WidgetFactory *factoryr);
      std::string getType() const { return "ContinuedFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *f, *r;
  };

  class LinearSpringDamperForceWidget : public FunctionWidget {

    friend class LinearSpringDamperForce;

    public:
      LinearSpringDamperForceWidget();
      std::string getType() const { return "LinearSpringDamperForce"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *c, *d;
  };

  class NonlinearSpringDamperForceWidget : public FunctionWidget {

    friend class NonlinearSpringDamperForce;

    public:
      NonlinearSpringDamperForceWidget(Element *parent);
      std::string getType() const { return "NonlinearSpringDamperForce"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *s, *sd;
  };

  class LinearElasticFunctionWidget : public FunctionWidget {

    friend class LinearElasticFunction;

    public:
      LinearElasticFunctionWidget();
      std::string getType() const { return "LinearElasticFunction"; }
      void resize_(int m, int n);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *K, *D;
  };

  class LinearRegularizedBilateralConstraintWidget: public FunctionWidget {

    friend class LinearRegularizedBilateralConstraint;

    public:
      LinearRegularizedBilateralConstraintWidget();
      std::string getType() const { return "LinearRegularizedBilateralConstraint"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    private:
      ExtWidget *c, *d;
  };

  class LinearRegularizedUnilateralConstraintWidget: public FunctionWidget {

    friend class LinearRegularizedUnilateralConstraint;

    public:
      LinearRegularizedUnilateralConstraintWidget();
      std::string getType() const { return "LinearRegularizedUnilateralConstraint"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    private:
      ExtWidget *c, *d;
  };

  class LinearRegularizedCoulombFrictionWidget: public FunctionWidget {

    friend class LinearRegularizedCoulombFriction;

    public:
      LinearRegularizedCoulombFrictionWidget();
      std::string getType() const { return "LinearRegularizedCoulombFriction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    private:
      ExtWidget *gd, *mu;
  };

  class LinearRegularizedStribeckFrictionWidget: public FunctionWidget {

    friend class LinearRegularizedStribeckFriction;

    public:
      LinearRegularizedStribeckFrictionWidget();
      std::string getType() const { return "LinearRegularizedStribeckFriction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    private:
      ExtWidget *gd, *mu;
  };

  class SignalFunctionWidget: public FunctionWidget {

    friend class SignalFunction;

    public:
      SignalFunctionWidget(Element *element);
      ~SignalFunctionWidget();
      std::string getType() const { return "SignalFunction"; }
      void updateWidget();
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    private:
      ExtWidget *sRef;
      Element *dummy;
  };

  class PolarContourFunctionWidget : public FunctionWidget {

    friend class PolarContourFunction;

    public:
      PolarContourFunctionWidget();
      std::string getType() const { return "PolarContourFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *radiusFunction;
  };

}

#endif
