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

#ifndef _INTEGRATOR__H_
#define _INTEGRATOR__H_

#include "extended_properties.h"
#include "property_widget.h"

class TiXmlElement;
class TiXmlNode;

class Integrator {
  friend class IntegratorPropertyDialog;
  protected:
    ExtProperty startTime, endTime, plotStepSize, initialState;
  public:
    Integrator();
    virtual ~Integrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    static Integrator* readXMLFile(const std::string &filename);
    virtual void writeXMLFile(const std::string &name);
    virtual void writeXMLFile() { writeXMLFile(getType()); }
    virtual std::string getType() const { return "Integrator"; }
    virtual IntegratorPropertyDialog* createPropertyDialog() {return new IntegratorPropertyDialog;}
};

class DOPRI5Integrator : public Integrator {
  friend class DOPRI5IntegratorPropertyDialog;
  public:
    DOPRI5Integrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual std::string getType() const { return "DOPRI5Integrator"; }
    IntegratorPropertyDialog* createPropertyDialog() {return new DOPRI5IntegratorPropertyDialog;}
  protected:
    ExtProperty absTol, relTol, initialStepSize, maximalStepSize, maxSteps;
};

class RADAU5Integrator : public Integrator {
  friend class RADAU5IntegratorPropertyDialog;
  public:
    RADAU5Integrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual std::string getType() const { return "RADAU5Integrator"; }
    IntegratorPropertyDialog* createPropertyDialog() {return new RADAU5IntegratorPropertyDialog;}
  protected:
    ExtProperty absTol, relTol, initialStepSize, maximalStepSize, maxSteps;
};

class LSODEIntegrator : public Integrator {
  friend class LSODEIntegratorPropertyDialog;
  public:
    LSODEIntegrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual std::string getType() const { return "LSODEIntegrator"; }
    IntegratorPropertyDialog* createPropertyDialog() {return new LSODEIntegratorPropertyDialog;}
  protected:
    ExtProperty absTol, relTol, initialStepSize, maximalStepSize, minimalStepSize, maxSteps, stiff;
};

class LSODARIntegrator : public Integrator {
  friend class LSODARIntegratorPropertyDialog;
  public:
    LSODARIntegrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual std::string getType() const { return "LSODARIntegrator"; }
    IntegratorPropertyDialog* createPropertyDialog() {return new LSODARIntegratorPropertyDialog;}
  protected:
    ExtProperty absTol, relTol, initialStepSize, maximalStepSize, minimalStepSize, plotOnRoot;
};

class TimeSteppingIntegrator : public Integrator {
  friend class TimeSteppingIntegratorPropertyDialog;
  public:
    TimeSteppingIntegrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual std::string getType() const { return "TimeSteppingIntegrator"; }
    IntegratorPropertyDialog* createPropertyDialog() {return new TimeSteppingIntegratorPropertyDialog;}
  protected:
    ExtProperty stepSize;
};

class EulerExplicitIntegrator : public Integrator {
  friend class EulerExplicitIntegratorPropertyDialog;
  public:
    EulerExplicitIntegrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual std::string getType() const { return "EulerExplicitIntegrator"; }
    IntegratorPropertyDialog* createPropertyDialog() {return new EulerExplicitIntegratorPropertyDialog;}
  protected:
    ExtProperty stepSize;
};

class RKSuiteIntegrator : public Integrator {
  friend class RKSuiteIntegratorPropertyDialog;
  public:
    RKSuiteIntegrator();
    virtual void initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    virtual std::string getType() const { return "RKSuiteIntegrator"; }
    IntegratorPropertyDialog* createPropertyDialog() {return new RKSuiteIntegratorPropertyDialog;}
  protected:
    ExtProperty type, relTol, threshold, initialStepSize;
};



#endif
