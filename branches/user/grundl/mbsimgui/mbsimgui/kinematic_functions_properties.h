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

#ifndef _KINEMATIC_FUNCTIONS_PROPERTIES_H_
#define _KINEMATIC_FUNCTIONS_PROPERTIES_H_

#include "function_property.h"
#include "extended_properties.h"

class TranslationAlongXAxisProperty: public FunctionProperty {
  public:
    TranslationAlongXAxisProperty() { }
    virtual Property* clone() const {return new TranslationAlongXAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "TranslationAlongXAxis"; }
};

class TranslationAlongYAxisProperty: public FunctionProperty {
  public:
    TranslationAlongYAxisProperty() { }
    virtual Property* clone() const {return new TranslationAlongYAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "TranslationAlongYAxis"; }
};

class TranslationAlongZAxisProperty: public FunctionProperty {
  public:
    TranslationAlongZAxisProperty() { }
    virtual Property* clone() const {return new TranslationAlongZAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "TranslationAlongZAxis"; }
};

class TranslationAlongAxesXYProperty: public FunctionProperty {
  public:
    TranslationAlongAxesXYProperty() { }
    virtual Property* clone() const {return new TranslationAlongAxesXYProperty(*this);}
    int getArg1Size() const {return 2;}
    inline std::string getType() const { return "TranslationAlongAxesXY"; }
};

class TranslationAlongAxesYZProperty: public FunctionProperty {
  public:
    TranslationAlongAxesYZProperty() { }
    virtual Property* clone() const {return new TranslationAlongAxesYZProperty(*this);}
    int getArg1Size() const {return 2;}
    inline std::string getType() const { return "TranslationAlongAxesYZ"; }
};

class TranslationAlongAxesXZProperty: public FunctionProperty {
  public:
    TranslationAlongAxesXZProperty() { }
    virtual Property* clone() const {return new TranslationAlongAxesXZProperty(*this);}
    int getArg1Size() const {return 2;}
    inline std::string getType() const { return "TranslationAlongAxesXZ"; }
};

class TranslationAlongAxesXYZProperty: public FunctionProperty {
  public:
    TranslationAlongAxesXYZProperty() { }
    virtual Property* clone() const {return new TranslationAlongAxesXYZProperty(*this);}
    int getArg1Size() const {return 3;}
    inline std::string getType() const { return "TranslationAlongAxesXYZ"; }
};

class TranslationAlongFixedAxisProperty : public FunctionProperty {
  public:
    TranslationAlongFixedAxisProperty();
    virtual Property* clone() const {return new TranslationAlongFixedAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "TranslationAlongFixedAxis"; }
    xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
    xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    ExtProperty a;
};

class LinearTranslationProperty : public FunctionProperty {
  public:
    LinearTranslationProperty(int m=1, int n=1);
    virtual Property* clone() const {return new LinearTranslationProperty(*this);}
    int getArg1Size() const;
    inline std::string getType() const { return "LinearTranslation"; }
    xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
    xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    ExtProperty A, b;
};

class RotationAboutXAxisProperty : public FunctionProperty {
  public:
    RotationAboutXAxisProperty() { }
    virtual Property* clone() const {return new RotationAboutXAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "RotationAboutXAxis"; }
};

class RotationAboutYAxisProperty : public FunctionProperty {
  public:
    RotationAboutYAxisProperty() { }
    virtual Property* clone() const {return new RotationAboutYAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "RotationAboutYAxis"; }
};

class RotationAboutZAxisProperty : public FunctionProperty {
  public:
    RotationAboutZAxisProperty() { }
    virtual Property* clone() const {return new RotationAboutZAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "RotationAboutZAxis"; }
};

class RotationAboutAxesXYProperty : public FunctionProperty {
  public:
    RotationAboutAxesXYProperty() { }
    virtual Property* clone() const {return new RotationAboutAxesXYProperty(*this);}
    int getArg1Size() const {return 2;}
    inline std::string getType() const { return "RotationAboutAxesXY"; }
};

class RotationAboutAxesYZProperty : public FunctionProperty {
  public:
    RotationAboutAxesYZProperty() { }
    virtual Property* clone() const {return new RotationAboutAxesYZProperty(*this);}
    int getArg1Size() const {return 2;}
    inline std::string getType() const { return "RotationAboutAxesYZ"; }
};

class RotationAboutAxesXZProperty : public FunctionProperty {
  public:
    RotationAboutAxesXZProperty() { }
    virtual Property* clone() const {return new RotationAboutAxesXZProperty(*this);}
    int getArg1Size() const {return 2;}
    inline std::string getType() const { return "RotationAboutAxesXZ"; }
};

class RotationAboutAxesXYZProperty : public FunctionProperty {
  public:
    RotationAboutAxesXYZProperty() { }
    virtual Property* clone() const {return new RotationAboutAxesXYZProperty(*this);}
    int getArg1Size() const {return 3;}
    inline std::string getType() const { return "RotationAboutAxesXYZ"; }
};

class RotationAboutFixedAxisProperty : public FunctionProperty {
  public:
    RotationAboutFixedAxisProperty();
    virtual Property* clone() const {return new RotationAboutFixedAxisProperty(*this);}
    int getArg1Size() const {return 1;}
    inline std::string getType() const { return "RotationAboutFixedAxis"; }
    xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
    xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
  protected:
    ExtProperty a;
};

class TCardanAnglesProperty : public FunctionProperty {
  public:
    TCardanAnglesProperty() { }
    virtual Property* clone() const {return new TCardanAnglesProperty(*this);}
    int getArg1Size() const {return 3;}
    inline std::string getType() const { return "TCardanAngles"; }
};

#endif
