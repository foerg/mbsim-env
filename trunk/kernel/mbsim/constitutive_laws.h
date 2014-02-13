/* Copyright (C) 2004-2009 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _CONSTITUTIVE_LAWS_H_
#define _CONSTITUTIVE_LAWS_H_

#include <map>

#include <mbsim/numerics/linear_complementarity_problem/linear_complementarity_problem.h>
#include <mbsim/contour.h>
#include <mbsim/contact.h>
#include <fmatvec/function.h>

namespace MBSim {

  class SingleContact;
  class InfluenceFunction;

  /**
   * \brief basic force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class GeneralizedForceLaw {
    public:
      /**
       * \brief constructor
       */
      GeneralizedForceLaw() : forceFunc(NULL) {};

      GeneralizedForceLaw(fmatvec::Function<double(double,double)> *forceFunc_) : forceFunc(forceFunc_) {};

      /**
       * \brief destructor
       */
      virtual ~GeneralizedForceLaw() { if(forceFunc) delete forceFunc; forceFunc = NULL; };

      /* INTERFACE FOR DERIVED CLASSES */
      /**
       * \param relative distance
       * \param tolerance
       * \return flag, if force law is active
       */
      virtual bool isActive(double g, double gTol) { return true; }
      virtual bool remainsActive(double s, double sTol) { return true; }
      virtual double project(double la, double gdn, double r, double laMin=0) { return 0; }
      virtual fmatvec::Vec diff(double la, double gdn, double r, double laMin=0) { return fmatvec::Vec(2, fmatvec::INIT, 0); }
      virtual double solve(double G, double gdn) { return 0; }

      /**
       * \param contact force parameter
       * \param contact relative velocity
       * \param tolerance for contact force parameters
       * \param tolerance for relative velocity
       * \return flag if the force law is valid given the parameters
       */
      virtual bool isFulfilled(double la,  double gdn, double tolla, double tolgd, double laMin=0) { return true; }

      /**
       * \return flag if the force law is setvalued
       */
      virtual bool isSetValued() const = 0;

      /**
       * \brief initialize the force law using XML
       * \param XML element
       */
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);

      /**
       * \return std::string representation
       */
      virtual std::string getType() const { return "GeneralizedForceLaw"; }
      /***************************************************/
      
      /**
       * \brief
       *
       * \param g          distance of the contact points
       * \param gd         relative velocity in normal direction of contact points
       * \param additional ??
       */
      double operator()(double g, double gd) { assert(forceFunc); return (*forceFunc)(g,gd); }

      /*!
       * \brief computes the normal forces for smooth constitutive law on every contact point
       *
       * \param contacts vector of contacts that are part of the contact law
       */
      virtual void computeSmoothForces(std::vector<std::vector<SingleContact> > & contact) {};

      /** \brief Set the force function for use in regularisized constitutive laws
       * The first input parameter to the force function is g.
       * The second input parameter to the force function is gd.
       * The return value is the force.
       */
      void setForceFunction(fmatvec::Function<double(double,double)> *forceFunc_) { forceFunc=forceFunc_; }

    protected:
      /*!
       * \brief force function for a regularized contact law
       */
      fmatvec::Function<double(double,double)> *forceFunc;
  };

  /**
   * \brief basic unilateral force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class UnilateralConstraint : public GeneralizedForceLaw {
    public:
      /**
       * \brief constructor
       */
      UnilateralConstraint() {};

      /**
       * \brief destructor
       */
      virtual ~UnilateralConstraint() {};

      /* INHERITED INTERFACE */
      virtual bool isActive(double g, double gTol) { return g<=gTol; }
      virtual bool remainsActive(double gd, double gdTol) { return gd<=gdTol; }
      virtual double project(double la, double gdn, double r, double laMin=0);
      virtual fmatvec::Vec diff(double la, double gdn, double r, double laMin=0);
      virtual double solve(double G, double gdn);
      virtual bool isFulfilled(double la,  double gdn, double tolla, double tolgd, double laMin=0);
      virtual bool isSetValued() const { return true; }
      /***************************************************/

      bool remainsClosed(double s, double sTol) { return s<=sTol; }  // s = gd/gdd

      virtual std::string getType() const { return "UnilateralConstraint"; }
  };

  /**
   * \brief basic bilateral force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   * \date 2010-07-06 isSticking added for impact laws (Robert Huber)
   */
  class BilateralConstraint : public GeneralizedForceLaw {
    public:
      /**
       * \brief constructor
       */
      BilateralConstraint() {};

      /**
       * \brief destructor
       */
      virtual ~BilateralConstraint() {};

      /* INHERITED INTERFACE */
      virtual bool isActive(double g, double gTol) { return true; }
      virtual double project(double la, double gdn, double r, double laMin=0);
      virtual fmatvec::Vec diff(double la, double gdn, double r, double laMin=0);
      virtual double solve(double G, double gdn);
      virtual bool isFulfilled(double la,  double gdn, double tolla, double tolgd, double laMin=0);
      virtual bool isSetValued() const { return true; }
      /***************************************************/

      bool remainsClosed(double s, double sTol) { return true; }

      virtual std::string getType() const { return "BilateralConstraint"; }
  };

  /**
   * \brief basic force law on velocity level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class GeneralizedImpactLaw {
    public:
      /**
       * \brief constructor
       */
      GeneralizedImpactLaw() {};

      /**
       * \brief destructor
       */
      virtual ~GeneralizedImpactLaw() {};

      /* INTERFACE FOR DERIVED CLASSES */
      virtual double project(double la, double gdn, double gda, double r, double laMin=0) = 0;
      virtual fmatvec::Vec diff(double la, double gdn, double gda, double r, double laMin=0) = 0;
      virtual double solve(double G, double gdn, double gda) = 0;
      virtual bool isFulfilled(double la,  double gdn, double gda, double tolla, double tolgd, double laMin=0) = 0;
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);

      /**
       * \return std::string representation
       */
      virtual std::string getType() const { return "GeneralizedImpactLaw"; }
      /***************************************************/
  };

  /**
   * \brief basic unilateral force law on velocity level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   * \date 2010-08-18 epsilon is set to zero in a smooth way and elastic contribution only for negative gd
   */
  class UnilateralNewtonImpact : public GeneralizedImpactLaw {
    public:
      /**
       * \brief constructor
       */
      UnilateralNewtonImpact() : epsilon(0), gd_limit(1e-2) {};

      /**
       * \brief constructor
       */
      UnilateralNewtonImpact(double epsilon_) : epsilon(epsilon_), gd_limit(1e-2) {};

      /**
       * \brief constructor
       */
      UnilateralNewtonImpact(double epsilon_, double gd_limit_) : epsilon(epsilon_), gd_limit(gd_limit_) {};

      /**
       * \brief destructor
       */
      virtual ~UnilateralNewtonImpact() {};

      /* INHERITED INTERFACE */
      virtual double project(double la, double gdn, double gda, double r, double laMin=0);
      virtual fmatvec::Vec diff(double la, double gdn, double gda, double r, double laMin=0);
      virtual double solve(double G, double gdn, double gda);
      virtual bool isFulfilled(double la,  double gdn, double gda, double tolla, double tolgd, double laMin=0);
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
      virtual std::string getType() const { return "UnilateralNewtonImpact"; }
      /***************************************************/

    protected:
      double epsilon, gd_limit;
  };

  /**
   * \brief basic bilateral force law on velocity level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class BilateralImpact : public GeneralizedImpactLaw {
    public:
      /**
       * \brief constructor
       */
      BilateralImpact() {};

      /**
       * \brief destructor
       */
      virtual ~BilateralImpact() {};

      /* INHERITED INTERFACE */
      virtual double project(double la, double gdn, double gda, double r, double laMin=0);
      virtual fmatvec::Vec diff(double la, double gdn, double gda, double r, double laMin=0);
      virtual double solve(double G, double gdn, double gda);
      virtual bool isFulfilled(double la,  double gdn, double gda, double tolla, double tolgd, double laMin=0);

      virtual std::string getType() const { return "BilateralImpact"; }
  };

  /**
   * \brief basic friction force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class FrictionForceLaw {
    public:
      /**
       * \brief constructor
       */
      FrictionForceLaw() : frictionForceFunc(NULL) {};

      FrictionForceLaw(fmatvec::Function<fmatvec::Vec(fmatvec::Vec,double)> *frictionForceFunc_) : frictionForceFunc(frictionForceFunc_) {};

      /**
       * \brief destructor
       */
      virtual ~FrictionForceLaw() { if(frictionForceFunc) delete frictionForceFunc; frictionForceFunc = NULL; };

      /* INTERFACE FOR DERIVED CLASSES */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r) { return fmatvec::Vec(2); }
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r) { return fmatvec::Mat(2,2); }
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, double laN) { return fmatvec::Vec(2); }
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double tolla, double tolgd) { return true; }
      virtual fmatvec::Vec dlaTdlaN(const fmatvec::Vec& gd, double laN) { return fmatvec::Vec(2); }
      virtual int getFrictionDirections() = 0;
      virtual bool isSticking(const fmatvec::Vec& s, double sTol) = 0;
      virtual double getFrictionCoefficient(double gd) { return 0; }
      virtual bool isSetValued() const = 0;
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);

      /**
       * \return std::string representation
       */
      virtual std::string getType() const { return "FrictionForceLaw"; }
      /***************************************************/
      
      fmatvec::Vec operator()(const fmatvec::Vec &gd, double laN) { assert(frictionForceFunc); return (*frictionForceFunc)(gd,laN); }

      /** \brief Set the friction force function for use in regularisized constitutive friction laws
       * The first input parameter to the friction force function is gd.
       * The second input parameter to the friction force function is laN.
       * The return value is the force vector.
       */
      void setFrictionForceFunction(fmatvec::Function<fmatvec::Vec(fmatvec::Vec,double)> *frictionForceFunc_) { frictionForceFunc=frictionForceFunc_; }

    protected:
      fmatvec::Function<fmatvec::Vec(fmatvec::Vec,double)> *frictionForceFunc;
  };

  /**
   * \brief basic planar friction force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class PlanarCoulombFriction : public FrictionForceLaw {
    public:
      /**
       * \brief constructor
       */
      PlanarCoulombFriction() : mu(0) {};

      /**
       * \brief constructor
       */
      PlanarCoulombFriction(double mu_) : mu(mu_) {};

      /**
       * \brief destructor
       */
      virtual ~PlanarCoulombFriction() {}

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double tolla, double tolgd);
      virtual fmatvec::Vec dlaTdlaN(const fmatvec::Vec& gd, double laN);
      virtual int getFrictionDirections() { return 1; }
      virtual bool isSticking(const fmatvec::Vec& s, double sTol) { return fabs(s(0)) <= sTol; }
      virtual double getFrictionCoefficient(double gd) { return mu; }
      virtual bool isSetValued() const { return true; }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
      virtual std::string getType() const { return "PlanarCoulombFriction"; }
      /***************************************************/

      void setFrictionCoefficient(double mu_) { mu = mu_; }

    protected:
      double mu;
  };

  /**
   * \brief basic spatial friction force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class SpatialCoulombFriction : public FrictionForceLaw {
    public:
      /**
       * \brief constructor
       */
      SpatialCoulombFriction() : mu(0) {};

      /**
       * \brief constructor
       */
      SpatialCoulombFriction(double mu_) : mu(mu_) {};

      /**
       * \brief destructor
       */
      virtual ~SpatialCoulombFriction() {}

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double tolla, double tolgd);
      virtual fmatvec::Vec dlaTdlaN(const fmatvec::Vec& gd, double laN);
      virtual int getFrictionDirections() { return 2; }
      virtual bool isSticking(const fmatvec::Vec& s, double sTol) { return nrm2(s(0,1)) <= sTol; }
      virtual double getFrictionCoefficient(double gd) { return mu; }
      virtual bool isSetValued() const { return true; }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
      virtual std::string getType() const { return "SpatialCoulombFriction"; }
      /***************************************************/

      void setFrictionCoefficient(double mu_) { mu = mu_; }

    protected:
      double mu;
  };

  /**
   * \brief planar Stribeck friction force law on acceleration level for constraint description
   * \author Thorsten Schindler
   * \date 2009-09-02 inital commit (Thorsten Schindler)
   * \todo high oscillations in normal relative velocity TODO
   */
  class PlanarStribeckFriction : public FrictionForceLaw {
    public:
      /**
       * \brief constructor
       */
      PlanarStribeckFriction() : fmu(0) {};

      /**
       * \brief constructor
       */
      PlanarStribeckFriction(fmatvec::Function<double(double)> *fmu_) : fmu(fmu_) {};

      /**
       * \brief destructor
       */
      virtual ~PlanarStribeckFriction() { if(fmu) delete fmu; fmu=0; }

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double tolla, double tolgd);
      virtual fmatvec::Vec dlaTdlaN(const fmatvec::Vec& gd, double laN);
      virtual int getFrictionDirections() { return 1; }
      virtual bool isSticking(const fmatvec::Vec& s, double sTol) { return fabs(s(0)) <= sTol; }
      virtual double getFrictionCoefficient(double gd) { return (*fmu)(gd); }
      virtual bool isSetValued() const { return true; }
      /***************************************************/

    protected:
      /**
       * friction coefficient function
       */
      fmatvec::Function<double(double)> *fmu;
  };

  /**
   * \brief spatial Stribeck friction force law on acceleration level for constraint description
   * \author Thorsten Schindler
   * \date 2009-09-02 initial commit (Thorsten Schindler)
   * \todo high oscillations in normal relative velocity TODO
   */
  class SpatialStribeckFriction : public FrictionForceLaw {
    public:
      /**
       * \brief constructor
       */
      SpatialStribeckFriction() : fmu(0) {};

      /**
       * \brief constructor
       */
      SpatialStribeckFriction(fmatvec::Function<double(double)> *fmu_) : fmu(fmu_) {};

      /**
       * \brief destructor
       */
      virtual ~SpatialStribeckFriction() { if(fmu) delete fmu; fmu=0; }

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double tolla, double tolgd);
      virtual fmatvec::Vec dlaTdlaN(const fmatvec::Vec& gd, double laN);
      virtual int getFrictionDirections() { return 2; }
      virtual bool isSticking(const fmatvec::Vec& s, double sTol) { return nrm2(s(0,1)) <= sTol; }
      virtual double getFrictionCoefficient(double gd) { return (*fmu)(gd); }
      virtual bool isSetValued() const { return true; }
      /***************************************************/

    protected:
      /**
       * friction coefficient function
       */
      fmatvec::Function<double(double)> *fmu;
  };

  /**
   * \brief basic friction force law on velocity level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class FrictionImpactLaw {
    public:
      /**
       * \brief constructor
       */
      FrictionImpactLaw() {};

      /**
       * \brief destructor
       */
      virtual ~FrictionImpactLaw() {};

      /* INTERFACE FOR DERIVED CLASSES */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r) = 0;
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r) = 0;
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN) = 0;
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double tolla, double tolgd) = 0;
      virtual int isSticking(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double laTol, double gdTol) = 0;
      virtual int getFrictionDirections() = 0;
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);

      /**
       * \return std::string representation
       */
      virtual std::string getType() const { return "FrictionImpactLaw"; }
      /***************************************************/
  };

  /**
   * \brief basic planar friction force law on velocity level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class PlanarCoulombImpact : public FrictionImpactLaw {
    public:
      /**
       * \brief constructor
       */
      PlanarCoulombImpact() : mu(0) {};

      /**
       * \brief constructor
       */
      PlanarCoulombImpact(double mu_) : mu(mu_) {};

      /**
       * \brief destructor
       */
      virtual ~PlanarCoulombImpact() {}

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double tolla, double tolgd);
      virtual int isSticking(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double laTol, double gdTol);
      virtual int getFrictionDirections() { return 1; }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
      virtual std::string getType() const { return "PlanarCoulombImpact"; }
      /***************************************************/

      void setFrictionCoefficient(double mu_) { mu = mu_; }
      double getFrictionCoefficient(double gd) { return mu; }

    protected:
      double mu;
  };

  /**
   * \brief basic spatial friction force law on velocity level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class SpatialCoulombImpact : public FrictionImpactLaw {
    public:
      /**
       * \brief constructor
       */
      SpatialCoulombImpact() : mu(0) {};

      /**
       * \brief constructor
       */
      SpatialCoulombImpact(double mu_) : mu(mu_) {};

      /**
       * \brief destructor
       */
      virtual ~SpatialCoulombImpact() {}

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double tolla, double tolgd);
      virtual int isSticking(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double laTol, double gdTol);
      virtual int getFrictionDirections() { return 2; }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
      virtual std::string getType() const { return "SpatialCoulombImpact"; }
      /***************************************************/

      void setFrictionCoefficient(double mu_) { mu = mu_; }
      double getFrictionCoefficient(double gd) { return mu; }

    protected:
      double mu;
  };

  /**
   * \brief planar Stribeck friction force law on velocity level for constraint description
   * \author Thorsten Schindler
   * \date 2009-09-02 initial commit (Thorsten Schindler)
   * \todo high oscillations in normal relative velocity TODO
   */
  class PlanarStribeckImpact : public FrictionImpactLaw {
    public:
      /**
       * \brief constructor
       */
      PlanarStribeckImpact() : fmu(0) {};

      /**
       * \brief constructor
       */
      PlanarStribeckImpact(fmatvec::Function<double(double)> *fmu_) : fmu(fmu_) {};

      /**
       * \brief destructor
       */
      virtual ~PlanarStribeckImpact() { if(fmu) delete fmu; fmu=0; }

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double tolla, double tolgd);
      virtual int isSticking(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double laTol, double gdTol);
      virtual int getFrictionDirections() { return 1; }
      /***************************************************/

      double getFrictionCoefficient(double gd) { return (*fmu)(gd); }

    protected:
      /**
       * friction coefficient function
       */
      fmatvec::Function<double(double)> *fmu;
  };

  /**
   * \brief spatial Stribeck friction force law on velocity level for constraint description
   * \author Thorsten Schindler
   * \date 2009-09-02 initial commit (Thorsten Schindler)
   * \todo high oscillations in normal relative velocity TODO
   */
  class SpatialStribeckImpact : public FrictionImpactLaw {
    public:
      /**
       * \brief constructor
       */
      SpatialStribeckImpact() : fmu(0) {};

      /**
       * \brief constructor
       */
      SpatialStribeckImpact(fmatvec::Function<double(double)> *fmu_) : fmu(fmu_) {};

      /**
       * \brief destructor
       */
      virtual ~SpatialStribeckImpact() { if(fmu) delete fmu; fmu=0; }

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double tolla, double tolgd);
      virtual int isSticking(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double laTol, double gdTol);
      virtual int getFrictionDirections() { return 2; }
      /***************************************************/

      double getFrictionCoefficient(double gd) { return (*fmu)(gd); }

    protected:
      /**
       * friction coefficient function
       */
      fmatvec::Function<double(double)> *fmu;
  };

  /**
   * \brief basic regularized unilateral force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class RegularizedUnilateralConstraint : public GeneralizedForceLaw {
    public:
      /**
       * \brief constructor
       */
      RegularizedUnilateralConstraint() {};

      RegularizedUnilateralConstraint(fmatvec::Function<double(double,double)> *forceFunc_) : GeneralizedForceLaw(forceFunc_) {};

      /**
       * \brief destructor
       */
      virtual ~RegularizedUnilateralConstraint() {};

      /* INHERITED INTERFACE */
      virtual bool isActive(double g, double gTol) { return g<=gTol; }
      virtual bool remainsActive(double s, double sTol) { return s<=sTol; }
      virtual bool isSetValued() const { return false; }
      virtual void computeSmoothForces(std::vector<std::vector<SingleContact> > & contacts);
      /***************************************************/

      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
  };

  /*!
   * \brief A force law that computes the normal force of many contact kinematics based on the Maxwell-Force-Law
   * \author Kilian Grundl
   * \date 30-07-2012 start of development
   */
  class MaxwellUnilateralConstraint : public GeneralizedForceLaw {
    public:
      /*!
       * \brief constructor
       */
      MaxwellUnilateralConstraint(const double & damping = 0, const double & gapLimit = 0);

      /*!
       * \brief destructor
       */
      virtual ~MaxwellUnilateralConstraint() {};

      /* INHERITED INTERFACE */
      virtual bool isActive(double g, double gTol) { return g < gTol ? true : false; }
      virtual bool remainsActive(double s, double sTol) {return true; }
      virtual bool isSetValued() const { return false; }
      virtual void computeSmoothForces(std::vector<std::vector<SingleContact> > & contacts);
      /***************************************************/

      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);

      /*!
       * \brief initialize all saved contour couplings in the map
       *
       * \todo: pointer to parent class is probably not optimal (friend class maybe?)
       */
      virtual void initializeContourCouplings(Contact* parent);

      /*GETTER - SETTER*/
      void setDebuglevel(int debuglevel) {
        DEBUGLEVEL = debuglevel;
      }
      void setLCPSolvingStrategy(LCPSolvingStrategy strategy) {
        LCP.setStrategy(strategy);
      }
      /*****************/

      /**
       * \brief add a function that represents the coupling between two contours
       * \param name of first contour
       * \param name of second contour
       * \param Function to describe coupling between contours
       *
       * \General Remark: The parameters (LagrangeParameterPositions) of the function have to be in the same order as it was given the add(...)-method
       */
      void addContourCoupling(Contour *contour1, Contour *contour2, InfluenceFunction *fct);

    protected:
      /**
       * \brief saves all possible contacts in a vector
       */
      virtual void updatePossibleContactPoints(const std::vector<std::vector<SingleContact> > & contacts);

      /**
       * \brief updates the influence matrix C
       * \param contours vector of contours that are part of the contact
       * \param cpData   vector of ContourPointDatas
       */
      virtual void updateInfluenceMatrix(std::vector<std::vector<SingleContact> > & contacts);

      /**
       * \brief update the rigid body distances (gaps) for the single contacts
       */
      void updateRigidBodyGap(const std::vector<std::vector<SingleContact> > & contacts);

      /**
       * \brief computes the coupling factor for the influence matrix on one contact point (two contours)
       * \param contours     vector of contours that are part of the contact
       * \param cpData       vector of ContourPointDatas
       * \param contactIndex index pair of contact point
       */
      virtual double computeInfluenceCoefficient(std::vector<std::vector<SingleContact> > & contacts, const std::pair<int, int> & contactIndex);

      /**
       * \brief computes the coupling factor for the influence matrix between two contact points (four contours)
       * \param contours            vector of contours that are part of the contact
       * \param cpData              vector of ContourPointDatas
       * \param contactIndex        index pair of contact point
       * \param coupledContactIndex index pair of coupling contact point
       */
      virtual double computeInfluenceCoefficient(std::vector<std::vector<SingleContact> > & contacts, const std::pair<int, int> & contactIndex, const std::pair<int, int> & couplingContactIndex);

      /*
       * \brief computes the "material constant" to have a good guess for the lambda-vector
       */
      virtual void computeMaterialConstant();

      /**
       * \brief saves the indices of all active contacts in pairs
       *
       * pair.first: number of contact kinematics
       * pair.second: number of subcontact point of contact kinematics
       */
      std::vector<std::pair<int, int> > possibleContactPoints;

      /*!
       * \brief variable for the LCP
       */
      LinearComplementarityProblem LCP;

      /**
       * \brief Influence matrix between contact points
       */
      fmatvec::SymMat C;

      /*
       * \brief vector of rigid body distances(gaps) for the active contacts
       */
      fmatvec::Vec rigidBodyGap;

      /**
       * \brief saves the influence functions for a pair of contours. The key is the pair of contour names
       */
      std::map<std::pair<Contour*, Contour*>, InfluenceFunction*> influenceFunctions;

      /**
       * \brief Solution of the last time, where contact has to be solved (can be used as starting guess for the next algorithm)
       */
      fmatvec::Vec solution0;

      /*!
       * \brief coefficient for possible contact damping
       */
      double dampingCoefficient;

      /*!
       * \brief relative contact point distance limit under which damping is active
       */
      double gLim;

      /**
       * \brief parameter for guessing starting values of contact force (average eigenvalue of influence-matrix)
       */
      double matConst;

      /**
       * \brief parameter to save if matConst has been computed already
       */
      bool matConstSetted;

      /**
       * \brief print INFO output?
       *
       * 0 = no DEBUGOutput
       * 1 = most important information
       * 2 = ...
       * 5 = Matrices and Vectors
       * \todo wouldn't a logger for MBSim be nice
       */
      int DEBUGLEVEL;

      struct xmlInfo {
          InfluenceFunction * function;
          std::string name1;
          std::string name2;
      };
      std::vector<xmlInfo> referenceXML;

  };

  /**
   * \brief basic regularized bilateral force law on acceleration level for constraint description
   * \author Martin Foerg
   * \date 2009-07-29 some comments (Thorsten Schindler)
   */
  class RegularizedBilateralConstraint : public GeneralizedForceLaw {
    public:
      /**
       * \brief constructor
       */
      RegularizedBilateralConstraint() {};

      RegularizedBilateralConstraint(fmatvec::Function<double(double,double)> *forceFunc_) : GeneralizedForceLaw(forceFunc_) {};

      /**
       * \brief destructor
       */
      virtual ~RegularizedBilateralConstraint() {};

      /* INHERITED INTERFACE */
      virtual bool isActive(double g, double gTol) { return true; }
      virtual bool remainsActive(double s, double sTol) { return true; }
      virtual bool isSetValued() const { return false; }
      virtual void computeSmoothForces(std::vector<std::vector<SingleContact> > & contact);
      /***************************************************/

      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);

      virtual std::string getType() const { return "RegularizedBilateralConstraint"; }
  };

  class RegularizedPlanarFriction : public FrictionForceLaw {
    public:
      RegularizedPlanarFriction() {};
      RegularizedPlanarFriction(fmatvec::Function<fmatvec::Vec(fmatvec::Vec,double)> *frictionForceFunc_) : FrictionForceLaw(frictionForceFunc_) {};
      virtual ~RegularizedPlanarFriction() {}
      int getFrictionDirections() { return 1; }
      bool isSticking(const fmatvec::Vec& s, double sTol) { return fabs(s(0)) <= sTol; }
      bool isSetValued() const { return false; }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
  };

  class RegularizedSpatialFriction : public FrictionForceLaw {
    public:
      RegularizedSpatialFriction() {};
      RegularizedSpatialFriction(fmatvec::Function<fmatvec::Vec(fmatvec::Vec,double)> *frictionForceFunc_) : FrictionForceLaw(frictionForceFunc_) {};
      virtual ~RegularizedSpatialFriction() {}
      int getFrictionDirections() { return 2; }
      bool isSticking(const fmatvec::Vec& s, double sTol) { return nrm2(s(0,1)) <= sTol; }
      bool isSetValued() const { return false; }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
  };

}

#endif /* _CONSTITUTIVE_LAWS_H_ */

