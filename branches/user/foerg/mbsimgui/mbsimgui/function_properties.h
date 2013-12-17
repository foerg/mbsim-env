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

class VecProperty;
class IntegerProperty;

class ConstantFunctionProperty : public FunctionProperty {
  public:
    ConstantFunctionProperty(int m=1);
    virtual Property* clone() const {return new ConstantFunctionProperty(*this);}
    inline std::string getType() const { return "ConstantFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    ExtProperty a0;
};

class LinearFunctionProperty : public FunctionProperty {
  public:
    LinearFunctionProperty(int m=1);
    virtual Property* clone() const {return new LinearFunctionProperty(*this);}
    inline std::string getType() const { return "LinearFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    ExtProperty a0, a1;
};

class QuadraticFunctionProperty : public FunctionProperty {
  public:
    QuadraticFunctionProperty(int m=1);
    virtual Property* clone() const {return new QuadraticFunctionProperty(*this);}
    inline std::string getType() const { return "QuadraticFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty a0, a1, a2;
};

class PolynomFunctionProperty : public FunctionProperty {
  public:
    PolynomFunctionProperty(int m=1);
    virtual Property* clone() const {return new PolynomFunctionProperty(*this);}
    inline std::string getType() const { return "PolynomFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty a;
};

class SinusoidalFunctionProperty : public FunctionProperty {
  public:
    SinusoidalFunctionProperty(int m=1);
    virtual Property* clone() const {return new SinusoidalFunctionProperty(*this);}
    inline std::string getType() const { return "SinusoidalFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty a, f, p, o;
};

//class StepFunctionProperty : public FunctionProperty {
//}

//class PositiveFunctionProperty : public FunctionProperty {
//}

class ScaledFunctionProperty : public FunctionProperty {
  public:
    ScaledFunctionProperty();
    virtual Property* clone() const {return new ScaledFunctionProperty(*this);}
    int getArgSize(int i=0) const;
    inline std::string getType() const { return "ScaledFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty function, factor;
};

class SummationFunctionProperty : public FunctionProperty {

  public:
    SummationFunctionProperty();
    virtual Property* clone() const {return new SummationFunctionProperty(*this);}
    inline std::string getType() const { return "SummationFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty functions;
};

class VectorValuedFunctionProperty : public FunctionProperty {
  public:
    VectorValuedFunctionProperty(int m=0);
    virtual Property* clone() const {return new VectorValuedFunctionProperty(*this);}
    inline std::string getType() const { return "VectorValuedFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    ExtProperty functions;
};

class NestedFunctionProperty : public FunctionProperty {
  public:
    //NestedFunctionProperty(const std::string &ext, const std::vector<Property*> &property);
    NestedFunctionProperty(const std::string &name);
    virtual Property* clone() const {return new NestedFunctionProperty(*this);}
    int getArgSize(int i=0) const;
    inline std::string getType() const { return "NestedFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    void update();
};

class PiecewiseDefinedFunctionProperty : public FunctionProperty {
  public:
    PiecewiseDefinedFunctionProperty();
    virtual Property* clone() const {return new PiecewiseDefinedFunctionProperty(*this);}
    inline std::string getType() const { return "PiecewiseDefinedFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    ExtProperty functions;
    ExtProperty contDiff;
};

class SymbolicFunctionProperty : public FunctionProperty {
  friend class SymbolicFunctionWidget;
  public:
    SymbolicFunctionProperty(const std::string &name, const std::vector<std::string> &var, int m);
    virtual Property* clone() const {return new SymbolicFunctionProperty(*this);}
    int getArgSize(int i=0) const;
    void resizeArg(int i, int size);
    void resizeRet(int size);
    const std::string& getValue() const {return f->getValue();}
    inline std::string getType() const { return "SymbolicFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
    //Widget* createWidget() { return f->createWidget(); }
    Widget* createWidget();
  protected:
    VecProperty *f;
    std::vector<std::string> argname;
    std::vector<IntegerProperty*> argdim;
};

class TabularFunctionProperty : public FunctionProperty {
  public:
    TabularFunctionProperty();
    virtual Property* clone() const {return new TabularFunctionProperty(*this);}
    inline std::string getType() const { return "TabularFunction"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ChoiceProperty2 choice;
};

class LinearSpringDamperForceProperty : public FunctionProperty {
  public:
    LinearSpringDamperForceProperty();
    virtual Property* clone() const {return new LinearSpringDamperForceProperty(*this);}
    inline std::string getType() const { return "LinearSpringDamperForce"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty c, d, l0;
};

class LinearRegularizedBilateralConstraintProperty: public FunctionProperty {
  public:
    LinearRegularizedBilateralConstraintProperty();
    virtual Property* clone() const {return new LinearRegularizedBilateralConstraintProperty(*this);}
    std::string getType() const { return "LinearRegularizedBilateralConstraint"; }
    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  private:
    ExtProperty c, d;
};

class LinearRegularizedUnilateralConstraintProperty: public FunctionProperty {
  public:
    LinearRegularizedUnilateralConstraintProperty(); 

    virtual Property* clone() const {return new LinearRegularizedUnilateralConstraintProperty(*this);}
    virtual std::string getType() const { return "LinearRegularizedUnilateralConstraint"; }

    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  private:
    ExtProperty c, d;
};

class LinearRegularizedCoulombFrictionProperty: public FunctionProperty {
  public:
    LinearRegularizedCoulombFrictionProperty(); 

    virtual Property* clone() const {return new LinearRegularizedCoulombFrictionProperty(*this);}
    virtual std::string getType() const { return "LinearRegularizedCoulombFriction"; }

    MBXMLUtils::TiXmlElement* initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  private:
    ExtProperty gd, mu;
};

#endif

