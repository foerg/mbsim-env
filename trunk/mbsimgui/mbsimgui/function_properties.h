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

#ifndef _FUNCTION_PROPERTIES_H_
#define _FUNCTION_PROPERTIES_H_

#include "function_property.h"
#include "extended_properties.h"

namespace MBSimGUI {

  class ConstantFunction : public Function {
    public:
      ConstantFunction(const std::string &name, Element *parent, int m=1);
      virtual PropertyInterface* clone() const {return new ConstantFunction(*this);}
      inline std::string getType() const { return "ConstantFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);
    protected:
      ExtProperty a0;
  };

  class LinearFunction : public Function {
    public:
      LinearFunction(const std::string &name, Element *parent, int m=1);
      virtual PropertyInterface* clone() const {return new LinearFunction(*this);}
      inline std::string getType() const { return "LinearFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);
    protected:
      ExtProperty a0, a1;
  };

  class QuadraticFunction : public Function {
    public:
      QuadraticFunction(const std::string &name, Element *parent, int m=1);
      virtual PropertyInterface* clone() const {return new QuadraticFunction(*this);}
      inline std::string getType() const { return "QuadraticFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty a0, a1, a2;
  };

  class PolynomFunction : public Function {
    public:
      PolynomFunction(const std::string &name, Element *parent, int m=1);
      virtual PropertyInterface* clone() const {return new PolynomFunction(*this);}
      inline std::string getType() const { return "PolynomFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty a;
  };

  class SinusoidalFunction : public Function {
    public:
      SinusoidalFunction(const std::string &name, Element *parent, int m=1);
      virtual PropertyInterface* clone() const {return new SinusoidalFunction(*this);}
      inline std::string getType() const { return "SinusoidalFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty a, f, p, o;
  };

  //class StepFunction : public Function {
  //}

  //class PositiveValueFunction : public Function {
  //}

  class AbsoluteValueFunction : public Function {
    public:
      AbsoluteValueFunction(const std::string &name, Element *parent) : Function(name,parent) { }
      virtual PropertyInterface* clone() const {return new AbsoluteValueFunction(*this);}
      inline std::string getType() const { return "AbsoluteValueFunction"; }
  };

  class ModuloFunction : public Function {
    public:
      ModuloFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new ModuloFunction(*this);}
      inline std::string getType() const { return "ModuloFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty denom;
  };

  class SignumFunction : public Function {
    public:
      SignumFunction(const std::string &name, Element *parent) : Function(name,parent) { }
      virtual PropertyInterface* clone() const {return new SignumFunction(*this);}
      inline std::string getType() const { return "SignumFunction"; }
  };

  class AdditionFunction : public Function {

    public:
      AdditionFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new AdditionFunction(*this);}
      inline std::string getType() const { return "AdditionFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty f1, f2;
  };

  class MultiplicationFunction : public Function {

    public:
      MultiplicationFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new MultiplicationFunction(*this);}
      inline std::string getType() const { return "MultiplicationFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty f1, f2;
  };

  class VectorValuedFunction : public Function {
    public:
      VectorValuedFunction(const std::string &name, Element *parent, int m=0);
      virtual PropertyInterface* clone() const {return new VectorValuedFunction(*this);}
      inline std::string getType() const { return "VectorValuedFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);
    protected:
      ExtProperty functions;
  };

  class NestedFunction : public Function {
    public:
      NestedFunction(const std::string &name, Element *parent, PropertyFactory *factoryo, PropertyFactory *factoryi);
      virtual PropertyInterface* clone() const {return new NestedFunction(*this);}
      int getArg1Size() const;
      inline std::string getType() const { return "NestedFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);
    protected:
      std::string ext;
      ExtProperty fo, fi;
  };

  class PiecewiseDefinedFunction : public Function {
    public:
      PiecewiseDefinedFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new PiecewiseDefinedFunction(*this);}
      inline std::string getType() const { return "PiecewiseDefinedFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);
    protected:
      ExtProperty functions;
      ExtProperty contDiff;
  };

  class SymbolicFunction : public Function {
    public:
      SymbolicFunction(const std::string &name, Element *parent, const std::string &ext, const std::vector<std::string> &var, int m);
      virtual PropertyInterface* clone() const {return new SymbolicFunction(*this);}
      int getArg1Size() const;
      int getArg2Size() const;
      inline std::string getType() const { return "SymbolicFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);
    protected:
      std::string ext;
      ExtProperty f;
      std::vector<ExtProperty> argname, argdim;
  };

  class TabularFunction : public Function {
    public:
      TabularFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new TabularFunction(*this);}
      inline std::string getType() const { return "TabularFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ChoiceProperty2 choice;
  };

  class TwoDimensionalTabularFunction : public Function {
    public:
      TwoDimensionalTabularFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new TwoDimensionalTabularFunction(*this);}
      inline std::string getType() const { return "TwoDimensionalTabularFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty x, y ,xy;
  };

  class PiecewisePolynomFunction : public Function {
    public:
      PiecewisePolynomFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new PiecewisePolynomFunction(*this);}
      inline std::string getType() const { return "PiecewisePolynomFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ChoiceProperty2 choice;
      ExtProperty method;
  };

  class FourierFunction : public Function {
    public:
      FourierFunction(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new FourierFunction(*this);}
      inline std::string getType() const { return "FourierFunction"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty f,a0,amplitudePhaseAngleForm;
      ChoiceProperty2 choice;
  };

  class LinearSpringDamperForce : public Function {
    public:
      LinearSpringDamperForce(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new LinearSpringDamperForce(*this);}
      inline std::string getType() const { return "LinearSpringDamperForce"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty c, d, l0;
  };

  class NonlinearSpringDamperForce : public Function {
    public:
      NonlinearSpringDamperForce(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new NonlinearSpringDamperForce(*this);}
      inline std::string getType() const { return "NonlinearSpringDamperForce"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    protected:
      ExtProperty g, gd;
  };

  class LinearRegularizedBilateralConstraint: public Function {
    public:
      LinearRegularizedBilateralConstraint(const std::string &name, Element *parent);
      virtual PropertyInterface* clone() const {return new LinearRegularizedBilateralConstraint(*this);}
      std::string getType() const { return "LinearRegularizedBilateralConstraint"; }
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    private:
      ExtProperty c, d;
  };

  class LinearRegularizedUnilateralConstraint: public Function {
    public:
      LinearRegularizedUnilateralConstraint(const std::string &name, Element *parent); 

      virtual PropertyInterface* clone() const {return new LinearRegularizedUnilateralConstraint(*this);}
      virtual std::string getType() const { return "LinearRegularizedUnilateralConstraint"; }

      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    private:
      ExtProperty c, d;
  };

  class LinearRegularizedCoulombFriction: public Function {
    public:
      LinearRegularizedCoulombFriction(const std::string &name, Element *parent); 

      virtual PropertyInterface* clone() const {return new LinearRegularizedCoulombFriction(*this);}
      virtual std::string getType() const { return "LinearRegularizedCoulombFriction"; }

      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);

    private:
      ExtProperty gd, mu;
  };

  class SignalFunction : public Function {
    public:
      SignalFunction(const std::string &name, Element *parent); 

      virtual PropertyInterface* clone() const {return new SignalFunction(*this);}
      virtual std::string getType() const { return "SignalFunction"; }
      virtual MBXMLUtils::NamespaceURI getNameSpace() const { return MBSIMCONTROL; }

      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
      void fromWidget(QWidget *widget);
      void toWidget(QWidget *widget);
      void initialize() { sRef.initialize(); }

    private:
      ExtProperty sRef;
  };

}

#endif

