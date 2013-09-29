/* Copyright (C) 2004-2009 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: markus.ms.schneider@gmail.com
 */

#ifndef _FUNCTION_LIBRARY_H_
#define _FUNCTION_LIBRARY_H_

#include "fmatvec/function.h"
#include "mbsim/objectfactory.h"
#include "mbsim/element.h"
#include "mbsim/utils/eps.h"
#include "mbsim/utils/utils.h"

namespace MBSim {

  template <class Ret>
    class StateDependentFunction : public fmatvec::Function<Ret(fmatvec::VecV,double)> {
      private:
        fmatvec::Function<Ret(fmatvec::VecV)> *f;
      public:
        StateDependentFunction(fmatvec::Function<Ret(fmatvec::VecV)> *f_=NULL) : f(f_) { }
        ~StateDependentFunction() { delete f; }
        typename fmatvec::Size<fmatvec::VecV>::type getArg1Size() const { return f->getArgSize();}
        typename fmatvec::Size<double>::type getArg2Size() const { return 0; }
        Ret operator()(const fmatvec::VecV &arg1, const double &arg2) {return (*f)(arg1); }
        typename fmatvec::Der<Ret, fmatvec::VecV>::type parDer1(const fmatvec::VecV &arg1, const double &arg2) { return f->parDer(arg1); }
        typename fmatvec::Der<Ret, double>::type parDer2(const fmatvec::VecV &arg1, const double &arg2) {return fmatvec::Vec3(); }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDer2ParDer2(const fmatvec::VecV &arg1, const double &arg2) { return fmatvec::Vec3(); }
        typename fmatvec::Der<Ret, double>::type parDer2DirDer1(const fmatvec::VecV &arg1Dir, const fmatvec::VecV &arg1, const double &arg2) { return fmatvec::Vec3(); }
        typename fmatvec::Der<typename fmatvec::Der<Ret, fmatvec::VecV>::type, double>::type parDer1ParDer2(const fmatvec::VecV &arg1, const double &arg2) { return fmatvec::Mat3xV(getArg1Size()); }
        typename fmatvec::Der<Ret, fmatvec::VecV>::type parDer1DirDer1(const fmatvec::VecV &arg1Dir, const fmatvec::VecV &arg1, const double &arg2) { return f->parDerDirDer(arg1Dir,arg1); }
        bool constParDer1() const { return f->constParDer(); }
        bool constParDer2() const { return true; }
        fmatvec::Function<Ret(fmatvec::VecV)>* getFunction() const { return f; }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          f=ObjectFactory<fmatvec::FunctionBase>::createAndInit<fmatvec::Function<Ret(fmatvec::VecV)> >(element->FirstChildElement());
        }
    };

  template <class Ret>
    class TimeDependentFunction : public fmatvec::Function<Ret(fmatvec::VecV,double)> {
      private:
        fmatvec::Function<Ret(double)> *f;
      public:
        TimeDependentFunction(fmatvec::Function<Ret(double)> *f_=NULL) : f(f_) { }
        ~TimeDependentFunction() { delete f; }
        typename fmatvec::Size<fmatvec::VecV>::type getArg1Size() const { return 0;}
        typename fmatvec::Size<double>::type getArg2Size() const { return 1; }
        Ret operator()(const fmatvec::VecV &arg1, const double &arg2) {return (*f)(arg2); }
        typename fmatvec::Der<Ret, fmatvec::VecV>::type parDer1(const fmatvec::VecV &arg1, const double &arg2) { return fmatvec::Mat3xV(); }
        typename fmatvec::Der<Ret, double>::type parDer2(const fmatvec::VecV &arg1, const double &arg2) {return f->parDer(arg2); }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDer2ParDer2(const fmatvec::VecV &arg1, const double &arg2) { return f->parDerParDer(arg2); }
        typename fmatvec::Der<Ret, double>::type parDer2DirDer1(const fmatvec::VecV &arg1Dir, const fmatvec::VecV &arg1, const double &arg2) { return fmatvec::Vec3(); }
        typename fmatvec::Der<typename fmatvec::Der<Ret, fmatvec::VecV>::type, double>::type parDer1ParDer2(const fmatvec::VecV &arg1, const double &arg2) { return fmatvec::Mat3xV(); }
        typename fmatvec::Der<Ret, fmatvec::VecV>::type parDer1DirDer1(const fmatvec::VecV &arg1Dir, const fmatvec::VecV &arg1, const double &arg2) { return fmatvec::Mat3xV(); }
        bool constParDer1() const { return true; }
        bool constParDer2() const { return f->constParDer(); }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          f=ObjectFactory<fmatvec::FunctionBase>::createAndInit<fmatvec::Function<Ret(double)> >(element->FirstChildElement());
        }
    };

  template<typename Sig> class NestedFunction; 

  template<typename Ret, typename Argo, typename Argi> 
    class NestedFunction<Ret(Argo(Argi))> : public fmatvec::Function<Ret(Argi)> {
      public:
       NestedFunction(fmatvec::Function<Ret(Argo)> *fo_=0, fmatvec::Function<Argo(Argi)> *fi_=0) : fo(fo_), fi(fi_) { }
        ~NestedFunction() { delete fo; delete fi; }
       typename fmatvec::Size<Argi>::type getArgSize() const { return fi->getArgSize();}
        Ret operator()(const Argi &arg) {return (*fo)((*fi)(arg));}
        typename fmatvec::Der<Ret, Argi>::type parDer(const Argi &arg) { return fo->parDer((*fi)(arg))*fi->parDer(arg); }
        typename fmatvec::Der<Ret, Argi>::type parDerDirDer(const Argi &argDir, const Argi &arg) { return fo->parDerDirDer(fi->parDer(arg)*argDir,(*fi)(arg))*fi->parDer(arg) + fo->parDer((*fi)(arg))*fi->parDerDirDer(argDir,arg); }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &arg) { return fo->parDerDirDer(fi->parDer(arg),(*fi)(arg))*fi->parDer(arg) + fo->parDer((*fi)(arg))*fi->parDerParDer(arg); }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"outerFunction");
          fo=ObjectFactory<fmatvec::FunctionBase>::createAndInit<fmatvec::Function<Ret(Argo)> >(e->FirstChildElement());
          e=element->FirstChildElement(MBSIMNS"innerFunction");
          fi=ObjectFactory<fmatvec::FunctionBase>::createAndInit<fmatvec::Function<Argo(Argi)> >(e->FirstChildElement());
        }
        MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; } 
      private:
        fmatvec::Function<Ret(Argo)> *fo;
        fmatvec::Function<Argo(Argi)> *fi;
    };

  /**
   * \brief function describing a linear relationship between the input relative distance / velocity and the output for a spring
   * \author Martin Foerg
   * \date 2009-08-31 some comments (Thorsten Schindler)
   * \todo put in function_library TODO
   */
  class LinearSpringDamperForce : public fmatvec::Function<double(double,double)> {
    public:
      /** 
       * \brief constructor
       */
      LinearSpringDamperForce() {}

      /** 
       * \brief constructor
       * \param stiffness
       * \param damping
       * \param undeformed length
       */
      LinearSpringDamperForce(double c_, double d_, double l0_) : c(c_), d(d_), l0(l0_) {}

      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual double operator()(const double& g, const double& gd) { return c*(g-l0) + d*gd; }
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/

      /* GETTER / SETTER */
      void setParameters(double c_, double d_, double l0_) { c=c_; d=d_; l0=l0_; }
      /***************************************************/

    protected:
      /**
       * \brief stiffness, damping, undeformed length
       */
      double c, d, l0;
  };

  /**
   * \brief function describing a nonlinear relationship between the input relative distance / velocity and the output for a spring
   * \author Martin Foerg
   * \date 2009-08-31 some comments (Thorsten Schindler)
   * \todo delete function pointers
   */
  class NonlinearSpringDamperForce : public fmatvec::Function<double(double,double)> {
    public:
      /** 
       * \brief constructor
       */
      NonlinearSpringDamperForce() {}

      /** 
       * \brief constructor
       * \param distance depending force function
       * \param relative velocity depending force function
       */
      NonlinearSpringDamperForce(Function<fmatvec::Vec(double)> * gForceFun_, Function<fmatvec::Vec(double)> * gdForceFun_) : gForceFun(gForceFun_), gdForceFun(gdForceFun_) {}

      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual double operator()(const double& g, const double& gd) { return (*gForceFun)(g)(0) + (*gdForceFun)(gd)(0); }
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/

      /* GETTER / SETTER */
      void setParameters(Function<fmatvec::Vec(double)> * gForceFun_, Function<fmatvec::Vec(double)> * gdForceFun_) { gForceFun=gForceFun_; gdForceFun=gdForceFun_; }
      /***************************************************/

    protected:
      /**
       * \brief distance depending force function
       */
      Function<fmatvec::Vec(double)> * gForceFun;

      /**
       * \brief relative velocity depending force function
       */
      Function<fmatvec::Vec(double)> * gdForceFun;
  };

  /*! 
   * \brief function describing a linear relationship between the input relative distance / velocity and the output for a unilateral constraint
   * \author Martin Foerg
   * \date 2009-08-31 some comments (Thorsten Schindler)
   * \todo put in function_library TODO
   */
  class LinearRegularizedUnilateralConstraint: public fmatvec::Function<double(double,double)> {
    public:
      /**
       * \brief constructor
       */
      LinearRegularizedUnilateralConstraint() : c(0), d(0) {}

      /**
       * \brief constructor
       * \param stiffness
       * \param damping
       */
      LinearRegularizedUnilateralConstraint(double c_, double d_) : c(c_), d(d_) {}

      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual double operator()(const double& g, const double& gd) { 
        if(g>0)
          return 0;
        else if(gd<0) 
          return -c*g - d*gd;
        else
          return -c*g;
      }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/

      /* GETTER / SETTER */
      void setParameter(double c_, double d_) { c=c_; d=d_; }
      /***************************************************/

    private:
      /**
       * \brief stiffness, damping
       */
      double c, d;
  };

  /*! 
   * \brief function describing a linear relationship between the input relative distance / velocity and the output for a bilateral constraint
   * \author Martin Foerg
   * \date 2009-08-31 some comments (Thorsten Schindler)
   * \todo put in function_library TODO
   */
  class LinearRegularizedBilateralConstraint: public fmatvec::Function<double(double,double)> {
    public:
      /**
       * \brief constructor
       */
      LinearRegularizedBilateralConstraint() : c(0), d(0) {}

      /**
       * \brief constructor
       * \param stiffness
       * \param damping
       */
      LinearRegularizedBilateralConstraint(double c_, double d_) : c(c_), d(d_) {}

      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual double operator()(const double& g, const double& gd) { 
        return -c*g - d*gd;
      }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);

      virtual std::string getType() const { return "LinearRegularizedBilateralConstraint"; }
      /***************************************************/

      /* GETTER / SETTER */
      void setParameter(double c_, double d_) { c=c_; d=d_; }
      /***************************************************/

    private:
      /**
       * \brief stiffness, damping
       */
      double c, d;
  };

  /*! 
   * \brief function describing a linear regularized relationship between the input relative velocity and the output for Coulomb friction
   * \author Martin Foerg
   * \date 2009-08-31 some comments (Thorsten Schindler)
   * \date 2010-01-09 beauty correction (Thorsten Schindler)
   * \todo put in function_library TODO
   */
  class LinearRegularizedCoulombFriction : public fmatvec::Function<fmatvec::Vec(fmatvec::Vec,double)> {
    public:
      /**
       * \brief constructor
       */
      LinearRegularizedCoulombFriction() : mu(0), gdLim(0.01) {}

      /**
       * \brief constructor
       * \param friction coefficient
       * \param border with respect to the relative velocity for the linear regularized increase of the friction force
       */
      LinearRegularizedCoulombFriction(double mu_, double gdLim_=0.01) : mu(mu_), gdLim(gdLim_) {}

      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual fmatvec::Vec operator()(const fmatvec::Vec &gd, const double& laN);
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/

      /* GETTER / SETTER */
      void setFrictionCoefficient(double mu_) { mu=mu_; }
      void setMarginalVelocity(double gdLim_) { gdLim=gdLim_; }
      /***************************************************/

    protected:
      /**
       * \brief friction coefficient, border with respect to the relative velocity for the linear regularized increase of the friction force
       */
      double mu, gdLim;
  };

  /**
   * \brief function describing a linear regularized relationship between the input relative velocity and the output for Stribeck friction
   * \author Martin Foerg
   * \date 2009-08-31 some comments (Thorsten Schindler)
   * \date 2010-01-09 beauty correction (Thorsten Schindler)
   * \todo delete function pointer
   */
  class LinearRegularizedStribeckFriction : public fmatvec::Function<fmatvec::Vec(fmatvec::Vec,double)> {
    public:
      /**
       * \brief constructor
       */
      LinearRegularizedStribeckFriction() : fmu(NULL), gdLim(0.01) {}

      /**
       * \brief constructor
       * \param function for friction coefficient depending on norm of relative velocity
       * \param border with respect to the relative velocity for the linear regularized increase of the friction force
       */
      LinearRegularizedStribeckFriction(fmatvec::Function<double(double)> *fmu_, double gdLim_=0.01) : fmu(fmu_), gdLim(gdLim_) {}

      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual fmatvec::Vec operator()(const fmatvec::Vec &gd, const double& laN);
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/

      /* GETTER / SETTER */
      void setFrictionFunction(fmatvec::Function<double(double)> *fmu_) { fmu=fmu_; }
      void setMarginalVelocity(double gdLim_) { gdLim=gdLim_; }
      /***************************************************/

    protected:
      /**
       * \brief friction coefficient function
       */
      Function<double(double)> *fmu;
      
      /**
       * \brief border with respect to the relative velocity for the linear regularized increase of the friction force
       */
      double gdLim;
  };

  /**
   * \brief function describing the influence between the deformations on a body
   */
  class InfluenceFunction : public fmatvec::Function<double(fmatvec::Vec2,fmatvec::Vec2)> {
    public:
      InfluenceFunction(){}
      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual double operator()(const fmatvec::Vec2& firstContourLagrangeParameter, const fmatvec::Vec2& secondContourLagrangeParameter)=0;
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/
  };

  /*
   * \brief Influence function for flexibility of contour with no influence to other contours (or contour points)
   */
  class FlexibilityInfluenceFunction : public InfluenceFunction {
    public:
      FlexibilityInfluenceFunction() : flexibility(0) {
      }
      FlexibilityInfluenceFunction(const std::string& ContourName_, const double & flexibility_) :
          flexibility(flexibility_) {
      }
      virtual ~FlexibilityInfluenceFunction() {}
      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual double operator()(const fmatvec::Vec2& firstContourLagrangeParameter, const fmatvec::Vec2& secondContourLagrangeParameter) {
        if(nrm2(firstContourLagrangeParameter - secondContourLagrangeParameter) < macheps())
          return flexibility;
        else
          return 0;
      }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/

    protected:
      double flexibility;
  };

  /*
   * \brief a class for Influence-Functions with constant coupling
   */
  class ConstantInfluenceFunction : public InfluenceFunction {
    public:
      ConstantInfluenceFunction() : couplingValue(0) {
    }
      ConstantInfluenceFunction(const double & couplingValue_) :
          couplingValue(couplingValue_) {
      }
      virtual ~ConstantInfluenceFunction() {}
      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual double operator()(const fmatvec::Vec2& firstContourLagrangeParameter, const fmatvec::Vec2& secondContourLagrangeParameter) {
        return couplingValue;
      }
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      /***************************************************/

    protected:
      double couplingValue;
  };

  class TwoDimensionalTabularFunction : public fmatvec::Function<double(double,double)> {
    public:
      TwoDimensionalTabularFunction();
      /* INHERITED INTERFACE OF FUNCTION2 */
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual double operator()(const double& x, const double& y);
      /***************************************************/
      /* GETTER / SETTER */
      void setXValues(fmatvec::Vec x_);
      void setYValues(fmatvec::Vec y_);
      void setXYMat(fmatvec::Mat XY_);
      /***************************************************/

    private:
      fmatvec::Vec xVec;
      fmatvec::Vec yVec;
      fmatvec::Mat XY;

      int xSize;
      int ySize;
      int x0Index,x1Index;
      int y0Index,y1Index;

      fmatvec::Vec func_value;
      fmatvec::Vec xy;
      fmatvec::Vec XYval;
      fmatvec::Mat XYfac;

      void calcIndex(const double * x, fmatvec::Vec X, int * xSize, int * xIndexMinus, int * xIndexPlus);
  };

  template<typename Ret>
    class ConstantFunction : public fmatvec::Function<Ret(double)> {
      protected:
        Ret a;
        Ret zeros(const Ret &x) { return Ret(x.size()); }
      public:
        ConstantFunction() {}
        ConstantFunction(const Ret &a_) : a(a_) {}
        typename fmatvec::Size<double>::type getArgSize() const { return 1; }
        Ret operator()(const double &arg) { return a; }
        typename fmatvec::Der<Ret, double>::type parDer(const double &arg) { return zeros(a); }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &arg) { return zeros(a); }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"value");
          a=FromMatStr<Ret>::cast(e->GetText());
        }
        MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; } 
    };

  template<>
    inline double ConstantFunction<double>::zeros(const double &x) { return 0; } 

  template<typename Ret>
    class LinearFunction : public fmatvec::Function<Ret(double)> {
      private:
        typename fmatvec::Der<Ret, double>::type A;
        Ret b;
        Ret zeros(const typename fmatvec::Der<Ret, double>::type &x) { return Ret(x.rows()); }
      public:
        LinearFunction() { }
        LinearFunction(const typename fmatvec::Der<Ret, double>::type &A_, const Ret &b_) : A(A_), b(b_) {}
        LinearFunction(const typename fmatvec::Der<Ret, double>::type &A_) : A(A_), b(zeros(A)) {}
        typename fmatvec::Size<double>::type getArgSize() const { return 1; }
        Ret operator()(const double &arg) { return A*arg+b; }
        typename fmatvec::Der<Ret, double>::type parDer(const double &arg) { return A; }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &arg) { return zeros(A); }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"slope");
          A=FromMatStr<typename fmatvec::Der<Ret, double>::type>::cast(e->GetText());
          e=element->FirstChildElement(MBSIMNS"intercept");
          b=e?FromMatStr<Ret>::cast(e->GetText()):zeros(A);
        }
        MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; } 
        void setSlope(const typename fmatvec::Der<Ret, double>::type &A_) { A = A_; }
        void setIntercept(const Ret &b_) { b = b_; }
    };

  template<>
    inline double LinearFunction<double>::zeros(const double &arg) { return 0; } 

 template<class Ret>
  class QuadraticFunction : public fmatvec::Function<Ret(double)> {
    private:
       Ret a0, a1, a2;
    public:
      QuadraticFunction() { }
      QuadraticFunction(const Ret &a0_, const Ret &a1_, const Ret &a2_) : a0(a0_), a1(a1_), a2(a2_) { }
      Ret operator()(const double &x) {  
        Ret y(a0.size(), fmatvec::NONINIT);
        for (int i=0; i<y.size(); i++)
          y(i)=a0(i)+(a1(i)+a2(i)*x)*x;
        return y;
      }
      typename fmatvec::Der<Ret, double>::type parDer(const double &x) {  
        typename fmatvec::Der<Ret, double>::type y(a0.size(), fmatvec::NONINIT);
        for (int i=0; i<y.size(); i++)
          y(i)=a1(i)+2.*a2(i)*x;
        return y;
      }
      typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &x) {  
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type y(a0.size(), fmatvec::NONINIT);
        for (int i=0; i<y.size(); i++)
          y(i)=2.*a2(i);
        return y;
      }
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
        MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"a0");
        a0=FromMatStr<Ret>::cast(e->GetText());
        e=element->FirstChildElement(MBSIMNS"a1");
        a1=FromMatStr<Ret>::cast(e->GetText());
        e=element->FirstChildElement(MBSIMNS"a2");
        a2=FromMatStr<Ret>::cast(e->GetText());
      }
  };

 template<>
   inline double QuadraticFunction<double>::operator()(const double &x) {  
     return a0+(a1+a2*x)*x;
   }

 template<>
   inline double QuadraticFunction<double>::parDer(const double &x) {  
     return a1+2.*a2*x;
   }

 template<>
   inline double QuadraticFunction<double>::parDerParDer(const double &x) {  
     return 2.*a2;
   }

  template<class Ret>
  class SinusFunction : public fmatvec::Function<Ret(double)> {
    protected:
      Ret A, f, phi0, y0;
      Ret zeros(const Ret &x) { return Ret(x.size()); }
    public:
      SinusFunction() { }
      SinusFunction(const Ret &A_, const Ret &f_, const Ret &phi0_, const Ret &y0_) : A(A_), f(f_), phi0(phi0_), y0(y0_) { }
      Ret operator()(const double &x) {  
        Ret y(A.size(), fmatvec::NONINIT);
        for (int i=0; i<y.size(); i++)
          y(i)=y0(i)+A(i)*sin(2.*M_PI*f(i)*x+phi0(i));
        return y;
      }
      typename fmatvec::Der<Ret, double>::type parDer(const double &x) {  
        typename fmatvec::Der<Ret, double>::type y(A.size(), fmatvec::NONINIT);
        for (int i=0; i<y.size(); i++) {
          double om = 2.*M_PI*f(i);
          y(i)=A(i)*om*cos(om*x+phi0(i));
        }
        return y;
      }
      typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &x) {  
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type y(A.size(), fmatvec::NONINIT);
        for (int i=0; i<y.size(); i++) {
          double om = 2.*M_PI*f(i);
          y(i)=-A(i)*om*om*sin(om*x+phi0(i));
        }
        return y;
      }
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
        MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"amplitude");
        A=FromMatStr<Ret>::cast(e->GetText());
        e=element->FirstChildElement(MBSIMNS"frequency");
        f=FromMatStr<Ret>::cast(e->GetText());
        e=element->FirstChildElement(MBSIMNS"phase");
        phi0=FromMatStr<Ret>::cast(e->GetText());
        e=element->FirstChildElement(MBSIMNS"offset");
        y0=e?FromMatStr<Ret>::cast(e->GetText()):zeros(A);
      }
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) {
        MBXMLUtils::TiXmlElement *ele0 = fmatvec::Function<Ret(double)>::writeXMLFile(parent);
        addElementText(ele0,MBSIMNS"amplitude",A);
        addElementText(ele0,MBSIMNS"frequency",f);
        addElementText(ele0,MBSIMNS"phase",phi0);
        addElementText(ele0,MBSIMNS"offset",y0);
        return ele0;
      }
  };

  template<>
    inline double SinusFunction<double>::operator()(const double &x) {  
      return y0+A*sin(2.*M_PI*f*x+phi0);
    }

  template<>
    inline double SinusFunction<double>::parDer(const double &x) {  
      double om = 2.*M_PI*f;
      return A*om*cos(om*x+phi0);
    }

  template<>
    inline double SinusFunction<double>::parDerParDer(const double &x) {  
      double om = 2.*M_PI*f;
      return -A*om*om*sin(om*x+phi0);
    }

  template<>
    inline double SinusFunction<double>::zeros(const double &x) { return 0; } 

  template<class Ret>
  class PositiveFunction : public fmatvec::Function<Ret(double)> {
    private:
      fmatvec::Function<Ret(double)> *f;
    public:
      PositiveFunction(fmatvec::Function<Ret(double)> *f_=0) : f(f_) { }
      ~PositiveFunction() { delete f; }
      Ret operator()(const double &x) {
        Ret y=(*f)(x);
        for (int i=0; i<y.size(); i++)
          if(y(i)<0)
            y(i)=0;
        return y;
      }
  };

  template<>
    inline double PositiveFunction<double>::operator()(const double &x) {  
      double y=(*f)(x);
      if(y<0) y=0;
      return y;
    }


  template<class Ret>
  class StepFunction : public fmatvec::Function<Ret(double)> {
    private:
      Ret stepTime, stepSize;
    public:
      StepFunction() {}
      StepFunction(const Ret &stepTime_, const Ret &stepSize_) : stepTime(stepTime_), stepSize(stepSize_) { }
      Ret operator()(const double &x) {
        Ret y(stepTime.size());
        for (int i=0; i<y.size(); i++)
          if (x>=stepTime(i))
            y(i)=stepSize(i);
        return y;
      }
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
        MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"time");
        stepTime=FromMatStr<Ret>::cast(e->GetText());
        e=element->FirstChildElement(MBSIMNS"size");
        stepSize=FromMatStr<Ret>::cast(e->GetText());
      }
  };

  template<>
    inline double StepFunction<double>::operator()(const double &x) {  
      return (x>=stepTime)?stepSize:0;
    }

  template<class Ret>
    class LinearCombinationFunction : public fmatvec::Function<Ret(double)> {
      public:
        LinearCombinationFunction() { }
        ~LinearCombinationFunction() { 
          for (unsigned int i=1; i<functions.size(); i++)
            delete functions[i]; 
        }
        void addFunction(fmatvec::Function<Ret(double)> *function, double factor=1.) {
          functions.push_back(function);
          factors.push_back(factor);
        }
        Ret operator()(const double &x) {
          Ret y=factors[0]*(*(functions[0]))(x);
          for (unsigned int i=1; i<functions.size(); i++)
            y+=factors[i]*(*(functions[i]))(x);
          return y;
        }
        typename fmatvec::Der<Ret, double>::type parDer(const double &x) {  
          typename fmatvec::Der<Ret, double>::type y=factors[0]*functions[0]->parDer(x);
          for (unsigned int i=0; i<functions.size(); i++)
            y+=factors[i]*functions[i]->parDer(x);
          return y;
        }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &x) {  
          typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type y=factors[0]*functions[0]->parDerParDer(x);
          for (unsigned int i=0; i<functions.size(); i++)
            y+=factors[i]*functions[i]->parDerParDer(x);
          return y;
        }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"function");
          while (e && e->ValueStr()==MBSIMNS"function") {
            MBXMLUtils::TiXmlElement *ee = e->FirstChildElement();
            fmatvec::Function<Ret(double)> *f=ObjectFactory<fmatvec::FunctionBase>::createAndInit<fmatvec::Function<Ret(double)> >(ee);
            ee=e->FirstChildElement(MBSIMNS"factor");
            double factor=ee?Element::getDouble(ee):1;
            addFunction(f, factor);
            e=e->NextSiblingElement();
          }
        }
      private:
        std::vector<fmatvec::Function<Ret(double)> *> functions;
        std::vector<double> factors;
    };

  template<class Ret>
    class VectorValuedFunction : public fmatvec::Function<Ret(double)> {
      public:
        VectorValuedFunction() { }
        VectorValuedFunction(const std::vector<fmatvec::Function<double(double)> *> &functions_) : functions(functions_) { }
        ~VectorValuedFunction() { 
          for (unsigned int i=1; i<functions.size(); i++)
            delete functions[i]; 
        }
        Ret operator()(const double &x) {
          Ret y(functions.size(),fmatvec::NONINIT);
          for (unsigned int i=0; i<functions.size(); i++)
            y(i)=(*functions[i])(x);
          return y;
        }
        typename fmatvec::Der<Ret, double>::type parDer(const double &x) {  
          typename fmatvec::Der<Ret, double>::type y(functions.size(),fmatvec::NONINIT);
          for (unsigned int i=0; i<functions.size(); i++)
            y(i)=functions[i]->parDer(x);
          return y;
        }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &x) {  
          typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type y(functions.size(),fmatvec::NONINIT);
          for (unsigned int i=0; i<functions.size(); i++)
            y(i)=functions[i]->parDerParDer(x);
          return y;
        }

        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"function");
          while (e && e->ValueStr()==MBSIMNS"function") {
            MBXMLUtils::TiXmlElement *ee = e->FirstChildElement();
            fmatvec::Function<double(double)> *f=ObjectFactory<fmatvec::FunctionBase>::createAndInit<fmatvec::Function<double(double)> >(ee);
            functions.push_back(f);
            e=e->NextSiblingElement();
          }
        }
      private:
        std::vector<fmatvec::Function<double(double)> *> functions;
    };

  template<class Ret>
    class PiecewiseDefinedFunction : public fmatvec::Function<Ret(double)> {
      public:
        PiecewiseDefinedFunction() : contDiff(0) { }
        PiecewiseDefinedFunction(const std::vector<fmatvec::Function<Ret(double)> *> &functions_, const std::vector<double> &a_, int contDiff_=0) : functions(functions_), a(a_), contDiff(contDiff_) { 
          init();
        }
        ~PiecewiseDefinedFunction() { 
          for (unsigned int i=1; i<functions.size(); i++)
            delete functions[i]; 
        }
        Ret zeros(const Ret &x) { return Ret(x.size()); }
        Ret operator()(const double &x) {
          for(unsigned int i=0; i<a.size(); i++)
            if(x<=a[i])
              return (*functions[i])(x);
          if(contDiff==0)
            return yEnd;
          else if(contDiff==1)
            return yEnd+ysEnd*(x-a[a.size()-1]);
          else 
            return yEnd+(ysEnd+0.5*yssEnd*(x-a[a.size()-1]))*(x-a[a.size()-1]);
        }
        typename fmatvec::Der<Ret, double>::type parDer(const double &x) {  
          for(unsigned int i=0; i<a.size(); i++)
            if(x<=a[i])
              return functions[i]->parDer(x);
          if(contDiff==0)
            return zeros(yEnd);
          else if(contDiff==1)
            return ysEnd;
          else 
            return ysEnd+yssEnd*(x-a[a.size()-1]);
        }
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &x) {  
          for(unsigned int i=0; i<a.size(); i++)
            if(x<=a[i])
              return functions[i]->parDerParDer(x);
          if(contDiff==0)
            return zeros(yEnd);
          else if(contDiff==1)
            return zeros(yEnd);
          else 
            return yssEnd;
        }

        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"function");
          while (e && e->ValueStr()==MBSIMNS"function") {
            MBXMLUtils::TiXmlElement *ee = e->FirstChildElement();
            fmatvec::Function<Ret(double)> *f=ObjectFactory<fmatvec::FunctionBase>::createAndInit<fmatvec::Function<Ret(double)> >(ee);
            functions.push_back(f);
            ee=e->FirstChildElement(MBSIMNS"limit");
            a.push_back(Element::getDouble(ee));
            e=e->NextSiblingElement();
          }
          e=element->FirstChildElement(MBSIMNS"continouslyDifferentiable");
          if(e) contDiff=Element::getDouble(e);
          init();
        }
      private:
        std::vector<fmatvec::Function<Ret(double)> *> functions;
        std::vector<double> a;
        int contDiff;
        Ret yEnd, ysEnd, yssEnd;
        void init() {
          yEnd = (*functions[functions.size()-1])(a[a.size()-1]); 
          if(contDiff>0) {
            ysEnd = functions[functions.size()-1]->parDer(a[a.size()-1]); 
            if(contDiff>1)
              yssEnd = functions[functions.size()-1]->parDerParDer(a[a.size()-1]); 
          }
        }
    };

  template<>
    inline double PiecewiseDefinedFunction<double>::zeros(const double &x) { return 0; } 

  template<class Ret>
  class PolynomFunction : public fmatvec::Function<Ret(double)> {
    public:
      PolynomFunction() { }
      PolynomFunction(const std::vector<Ret> &a_) : a(a_) { init(); }
      void init() {
        for(unsigned int i=1; i<a.size(); i++)
          ad.push_back(double(i)*a[i]);
        for(unsigned int i=1; i<ad.size(); i++)
          add.push_back(double(i)*ad[i]);
      }

      Ret operator()(const double &x) {
        Ret value=a[a.size()-1];
        for (int i=int(a.size())-2; i>-1; i--)
          value=value*x+a[i];
        return value;
      }
      typename fmatvec::Der<Ret, double>::type parDer(const double &x) {  
        typename fmatvec::Der<Ret, double>::type value=ad[ad.size()-1];
        for (int i=int(ad.size())-2; i>-1; i--)
          value=value*x+ad[i];
        return value;
      }
      typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &x) {  
        typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type value=add[add.size()-1];
        for (int i=int(add.size())-2; i>-1; i--)
          value=value*x+add[i];
        return value;
      }

      void addCoefficient(const Ret &c) {
        a.push_back(c);
      }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
        MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"coefficient");
        while (e && e->ValueStr()==MBSIMNS"coefficient") {
          addCoefficient(FromMatStr<Ret>::cast(e->GetText()));
          e=e->NextSiblingElement();
        }
        init();
      }

    private:
      std::vector<Ret> a, ad, add;
  };

  template<class Ret>
  class TabularFunction : public fmatvec::Function<Ret(double)> {

    public:
      TabularFunction() : xIndexOld(0) {}
      TabularFunction(const fmatvec::VecV &x_, const fmatvec::MatV &y_) : x(x_), y(y_), xIndexOld(0) {
        init();
      }
      Ret operator()(const double& xVal) {
        int i=xIndexOld;
        if (xVal<=x(0)) {
          xIndexOld=0;
          return FromVecV<Ret>::cast(trans(y.row(0)));
        }
        else if (xVal>=x(xSize-1)) {
          xIndexOld=xSize-1;
          return FromVecV<Ret>::cast(trans(y.row(xSize-1)));
        }
        else if (xVal<=x(i)) {
          while (xVal<x(i))
            i--;
        }
        else {
          do
            i++;
          while (xVal>x(i));
          i--;
        }
        xIndexOld=i;
        return FromVecV<Ret>::cast(trans(y.row(i)+(xVal-x(i))*(y.row(i+1)-y.row(i))/(x(i+1)-x(i))));
      }
      void initializeUsingXML(MBXMLUtils::TiXmlElement * element) {
        MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"x");
        if (e) {
          x=Element::getVec(e);
          e=element->FirstChildElement(MBSIMNS"y");
          y=Element::getMat(e, x.size(), 0);
        }
        e=element->FirstChildElement(MBSIMNS"xy");
        if (e) {
          fmatvec::MatV xy=Element::getMat(e);
          assert(xy.cols()>1);
          x=xy.col(0);
          y=xy(fmatvec::Range<fmatvec::Var,fmatvec::Var>(0, xy.rows()-1), fmatvec::Range<fmatvec::Var,fmatvec::Var>(1, xy.cols()-1));
        }
        init();
      }
    protected:
      fmatvec::VecV x;
      fmatvec::MatV y;
    private:
      int xIndexOld, xSize;
      void init() {
        for (int i=1; i<x.size(); i++)
          assert(x(i)>x(i-1));
        assert(x.size()==y.rows());
        xSize=x.size();
      }
  };

  template<class Ret>
  class PeriodicTabularFunction : public TabularFunction<Ret> {
    public:
      PeriodicTabularFunction() {}
      PeriodicTabularFunction(const fmatvec::VecV &x_, const fmatvec::MatV &y_) : TabularFunction<Ret>(x_, y_) {
        init();
      }
      Ret operator()(const double& xVal) {
        double xValTmp=xVal;
        while (xValTmp<xMin)
          xValTmp+=xDelta;
        while (xValTmp>xMax)
          xValTmp-=xDelta;
        return TabularFunction<Ret>::operator()(xValTmp);
      }
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
        TabularFunction<Ret>::initializeUsingXML(element);
        init();
      }
    private:
      double xMin, xMax, xDelta;
      void init() {
        xMin=TabularFunction<Ret>::x(0);
        xMax=this->x(TabularFunction<Ret>::x.size()-1);
        xDelta=xMax-xMin;
      }
  };

  template<typename Arg>
    class TranslationAlongXAxis : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 r, a;
      public:
        TranslationAlongXAxis() { a.e(0) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::Vec3 operator()(const Arg &q) { 
          r.e(0) = ToDouble<Arg>::cast(q);
          return r; 
        }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return a; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(1); }
        bool constParDer() const { return true; }
    };

  template<typename Arg>
    class TranslationAlongYAxis : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 r, a;
      public:
        TranslationAlongYAxis() { a.e(1) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::Vec3 operator()(const Arg &q) { 
          r.e(1) = ToDouble<Arg>::cast(q);
          return r; 
        }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return a; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(1); }
        bool constParDer() const { return true; }
    };

  template<typename Arg>
    class TranslationAlongZAxis : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 r, a;
      public:
        TranslationAlongZAxis() { a.e(2) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::Vec3 operator()(const Arg &q) { 
          r.e(2) = ToDouble<Arg>::cast(q);
          return r; 
        }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return a; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(1); }
        bool constParDer() const { return true; }
    };

  template<typename Arg>
    class TranslationAlongAxesXY : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 r;
        fmatvec::Mat3xV A;
      public:
        TranslationAlongAxesXY() : A(2) { A.e(0,0) = 1; A.e(1,1) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 2; }
        fmatvec::Vec3 operator()(const Arg &q) { 
          r.e(0) = q.e(0);
          r.e(1) = q.e(1);
          return r; 
        }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return A; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(2); }
        bool constParDer() const { return true; }
    };

  template<typename Arg>
    class TranslationAlongAxesYZ : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 r;
        fmatvec::Mat3xV A;
      public:
        TranslationAlongAxesYZ() : A(2) { A.e(1,1) = 1; A.e(2,2) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 2; }
        fmatvec::Vec3 operator()(const Arg &q) { 
          r.e(1) = q.e(0);
          r.e(2) = q.e(1);
          return r; 
        }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return A; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(2); }
        bool constParDer() const { return true; }
    };

  template<typename Arg>
    class TranslationAlongAxesXZ : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 r;
        fmatvec::Mat3xV A;
      public:
        TranslationAlongAxesXZ() : A(2) { A.e(0,0) = 1; A.e(2,2) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 2; }
        fmatvec::Vec3 operator()(const Arg &q) { 
          r.e(0) = q.e(0);
          r.e(2) = q.e(1);
          return r; 
        }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return A; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(2); }
        bool constParDer() const { return true; }
    };

  template<typename Arg>
    class TranslationAlongAxesXYZ : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 r;
        fmatvec::Mat3xV A;
      public:
        TranslationAlongAxesXYZ() : A(3) { A.e(0,0) = 1; A.e(1,1) = 1; A.e(2,2) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::Vec3 operator()(const Arg &q) { 
          r.e(0) = q.e(0);
          r.e(1) = q.e(1);
          r.e(2) = q.e(2);
          return r; 
        }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return A; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(3); }
        bool constParDer() const { return true; }
    };

  template<class Arg>
    class TranslationAlongFixedAxis : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        fmatvec::Vec3 a;
        fmatvec::Vec3 zeros(const typename fmatvec::Der<fmatvec::Vec3, Arg>::type &x) { return fmatvec::Vec3(x.rows()); }
      public:
        TranslationAlongFixedAxis() { }
        TranslationAlongFixedAxis(const fmatvec::Vec3 &a_) : a(a_) { }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::Vec3 operator()(const Arg &arg) { return a*arg; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return a; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(1); }
        typename fmatvec::Der<typename fmatvec::Der<fmatvec::Vec3, double>::type, double>::type parDerParDer(const double &arg) { throw MBSimError("parDerParDer is not available for given template parameters."); }
        bool constParDer() const { return true; }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"axisOfTranslation");
          a=FromMatStr<fmatvec::Vec3>::cast(e->GetText());
        }
        MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; } 
    };

  template<>
  inline fmatvec::Vec3 TranslationAlongFixedAxis<double>::parDerParDer(const double &arg) { return fmatvec::Vec3(); }

  template<class Arg>
    class LinearTranslation : public fmatvec::Function<fmatvec::Vec3(Arg)> {
      private:
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type A;
        fmatvec::Vec3 b;
        fmatvec::Vec3 zeros(const typename fmatvec::Der<fmatvec::Vec3, Arg>::type &x) { return fmatvec::Vec3(x.rows()); }
      public:
        LinearTranslation() { }
        LinearTranslation(const typename fmatvec::Der<fmatvec::Vec3, Arg>::type &A_) : A(A_) { }
        LinearTranslation(const typename fmatvec::Der<fmatvec::Vec3, Arg>::type &A_, const fmatvec::Vec3 &b_) : A(A_), b(b_) { }
        typename fmatvec::Size<Arg>::type getArgSize() const { return A.cols(); }
        fmatvec::Vec3 operator()(const Arg &arg) { return A*arg+b; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDer(const Arg &arg) { return A; }
        typename fmatvec::Der<fmatvec::Vec3, Arg>::type parDerDirDer(const Arg &arg1Dir, const Arg &arg1) { return typename fmatvec::Der<fmatvec::Vec3, Arg>::type(A.rows(),A.cols()); }
        typename fmatvec::Der<typename fmatvec::Der<fmatvec::Vec3, double>::type, double>::type parDerParDer(const double &arg) { throw MBSimError("parDerParDer is not available for given template parameters."); }
        bool constParDer() const { return true; }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"translationVectors");
          A=FromMatStr<typename fmatvec::Der<fmatvec::Vec3, Arg>::type>::cast(e->GetText());
          e=element->FirstChildElement(MBSIMNS"offset");
          b=e?FromMatStr<fmatvec::Vec3>::cast(e->GetText()):zeros(A);
        }
        MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; } 
        void setSlope(const typename fmatvec::Der<fmatvec::Vec3, Arg>::type &A_) { A = A_; }
        void setIntercept(const fmatvec::Vec3 &b_) { b = b_; }
    };

  template<>
    inline fmatvec::Vec3 LinearTranslation<double>::parDerDirDer(const double &arg1Dir, const double &arg1) { return fmatvec::Vec3(); }
  template<>
    inline fmatvec::Vec3 LinearTranslation<double>::parDerParDer(const double &arg) { return fmatvec::Vec3(); }

  template<class Arg> 
    class RotationAboutXAxis : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Vec3 a;
      public:
        RotationAboutXAxis() { a.e(0) = 1; A.e(0,0) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double alpha = ToDouble<Arg>::cast(q);
          const double cosq=cos(alpha);
          const double sinq=sin(alpha);
          A.e(1,1) = cosq;
          A.e(2,1) = sinq;
          A.e(1,2) = -sinq;
          A.e(2,2) = cosq;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) { return a; }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) { return typename fmatvec::Der<fmatvec::RotMat3, Arg>::type(1); }
        bool constParDer() const { return true; }
    };

  template<class Arg> 
    class RotationAboutYAxis : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Vec3 a;
      public:
        RotationAboutYAxis() { a.e(1) = 1; A.e(1,1) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double alpha = ToDouble<Arg>::cast(q);
          const double cosq=cos(alpha);
          const double sinq=sin(alpha);
          A.e(0,0) = cosq;
          A.e(2,0) = -sinq;
          A.e(0,2) = sinq;
          A.e(2,2) = cosq;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) { return a; }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) { return typename fmatvec::Der<fmatvec::RotMat3, Arg>::type(1); }
        bool constParDer() const { return true; }
    };

  template<class Arg> 
    class RotationAboutZAxis : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Vec3 a;
      public:
        RotationAboutZAxis() { a.e(2) = 1; A.e(2,2) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double alpha = ToDouble<Arg>::cast(q);
          const double cosq=cos(alpha);
          const double sinq=sin(alpha);
          A.e(0,0) = cosq;
          A.e(1,0) = sinq;
          A.e(0,1) = -sinq;
          A.e(1,1) = cosq;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) { return a; }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) { return typename fmatvec::Der<fmatvec::RotMat3, Arg>::type(1); }
        bool constParDer() const { return true; }
    };

  template<class Arg> 
    class RotationAboutFixedAxis : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Vec3 a;
      public:
        RotationAboutFixedAxis() { }
        RotationAboutFixedAxis(const fmatvec::Vec3 &a_) : a(a_) { }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 1; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double alpha = ToDouble<Arg>::cast(q);
          const double cosq=cos(alpha);
          const double sinq=sin(alpha);
          const double onemcosq=1-cosq;
          const double a0a1=a.e(0)*a.e(1);
          const double a0a2=a.e(0)*a.e(2);
          const double a1a2=a.e(1)*a.e(2);
          A.e(0,0) = cosq+onemcosq*a.e(0)*a.e(0);
          A.e(1,0) = onemcosq*a0a1+a.e(2)*sinq;
          A.e(2,0) = onemcosq*a0a2-a.e(1)*sinq;
          A.e(0,1) = onemcosq*a0a1-a.e(2)*sinq;
          A.e(1,1) = cosq+onemcosq*a.e(1)*a.e(1);
          A.e(2,1) = onemcosq*a1a2+a.e(0)*sinq;
          A.e(0,2) = onemcosq*a0a2+a.e(1)*sinq;
          A.e(1,2) = onemcosq*a1a2-a.e(0)*sinq;
          A.e(2,2) = cosq+onemcosq*a.e(2)*a.e(2);
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) { return a; }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) { return typename fmatvec::Der<fmatvec::RotMat3, Arg>::type(1); }
        typename fmatvec::Der<typename fmatvec::Der<fmatvec::RotMat3, Arg>::type, Arg>::type parDerParDer(const Arg &arg) { throw MBSimError("parDerParDer is not available for given template parameters."); }
        bool constParDer() const { return true; }
        const fmatvec::Vec3& getAxisOfRotation() const { return a; }
        void setAxisOfRotation(const fmatvec::Vec3 &a_) { a = a_; }
        void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
          MBXMLUtils::TiXmlElement *e=element->FirstChildElement(MBSIMNS"axisOfRotation");
          a=FromMatStr<fmatvec::Vec3>::cast(e->GetText());
        }
        MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; } 
    };

  template<>
  inline fmatvec::Vec3 RotationAboutFixedAxis<double>::parDerParDer(const double &arg) { return fmatvec::Vec3(); }

  template<class Arg> 
    class RotationAboutAxesXY : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Mat3xV J, Jd;
      public:
        RotationAboutAxesXY() : J(2), Jd(2) { J.e(0,0) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 2; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double a=q.e(0);
          double b=q.e(1);
          double cosa = cos(a);
          double sina = sin(a);
          double cosb = cos(b);
          double sinb = sin(b);

          A.e(0,0) = cosb;
          A.e(1,0) = sina*sinb;
          A.e(2,0) = -cosa*sinb;
          A.e(1,1) = cosa;
          A.e(2,1) = sina;
          A.e(0,2) = sinb;
          A.e(1,2) = -sina*cosb;
          A.e(2,2) = cosa*cosb;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) {
          double a = q.e(0);
          J.e(1,1) = cos(a);
          J.e(2,1) = sin(a);
          return J;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) {
          double a = q.e(0);
          double ad = qd.e(0);
          double bd = qd.e(1);
          Jd.e(1,1) = -sin(a)*ad*bd;
          Jd.e(2,1) = cos(a)*ad*bd;
          return Jd;
        }
    };

  template<class Arg> 
    class RotationAboutAxesYZ : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Mat3xV J, Jd;
      public:
        RotationAboutAxesYZ() : J(2), Jd(2) { J.e(1,0) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 2; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double b=q.e(0);
          double g=q.e(1);
          double cosb = cos(b);
          double sinb = sin(b);
          double cosg = cos(g);
          double sing = sin(g);

          A.e(0,0) = cosb*cosg;
          A.e(1,0) = sing;
          A.e(2,0) = -sinb*cosg;
          A.e(0,1) = -cosb*sing;
          A.e(1,1) = cosg;
          A.e(2,1) = sinb*sing;
          A.e(0,2) = sinb;
          A.e(2,2) = cosb;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) {
          double beta = q.e(0);
          J.e(0,1) = sin(beta);
          J.e(2,1) = cos(beta);
          return J;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) {
          double beta = q.e(0);
          double betad = qd.e(0);
          double gammad = qd.e(1);
          Jd.e(0,1) = cos(beta)*betad*gammad;
          Jd.e(2,1) = -sin(beta)*betad*gammad;
          return Jd;
        }
    };

  template<class Arg> 
    class RotationAboutAxesXZ : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Mat3xV J, Jd;
      public:
        RotationAboutAxesXZ() : J(2), Jd(2) { J.e(0,0) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 2; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double a=q.e(0);
          double b=q.e(1);
          double cosa = cos(a);
          double sina = sin(a);
          double cosb = cos(b);
          double sinb = sin(b);

          A.e(0,0) = cosb;
          A.e(1,0) = cosa*sinb;
          A.e(2,0) = sina*sinb;
          A.e(0,1) = -sinb;
          A.e(1,1) = cosa*cosb;
          A.e(2,1) = sina*cosb;
          A.e(1,2) = -sina;
          A.e(2,2) = cosa;

          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) {
          double a = q.e(0);
          J.e(1,1) = -sin(a);
          J.e(2,1) = cos(a);
          return J;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) {
          double a = q.e(0);
          double ad = qd.e(0);
          double bd = qd.e(1);
          Jd.e(1,1) = -cos(a)*ad*bd;
          Jd.e(2,1) = -sin(a)*ad*bd;
          return Jd;
        }
    };

  template<class Arg> 
    class RotationAboutAxesXYZ : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Mat3xV J, Jd;
      public:
        RotationAboutAxesXYZ() : J(3), Jd(3) { J.e(0,0) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double a=q.e(0);
          double b=q.e(1);
          double g=q.e(2);
          double cosa = cos(a);
          double sina = sin(a);
          double cosb = cos(b);
          double sinb = sin(b);
          double cosg = cos(g);
          double sing = sin(g);
          A.e(0,0) = cosb*cosg;
          A.e(1,0) = sina*sinb*cosg+cosa*sing;
          A.e(2,0) = -cosa*sinb*cosg+sina*sing;
          A.e(0,1) = -cosb*sing;
          A.e(1,1) = -sing*sinb*sina+cosa*cosg;
          A.e(2,1) = cosa*sinb*sing+sina*cosg;
          A.e(0,2) = sinb;
          A.e(1,2) = -sina*cosb;
          A.e(2,2) = cosa*cosb;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) {
          double a = q.e(0);
          double b = q.e(1);
          double cosa = cos(a);
          double sina = sin(a);
          double cosb = cos(b);
          //J.e(0,0) = 1;
          //J.e(0,1) = 0;
          J.e(0,2) = sin(b);
          //J.e(1,0) = 0;
          J.e(1,1) = cosa;
          J.e(1,2) = -sina*cosb;
          //J.e(2,0) = 0;
          J.e(2,1) = sina;
          J.e(2,2) = cosa*cosb;
          return J;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) {
          double a = q.e(0);
          double b = q.e(1);
          double ad = qd.e(0);
          double bd = qd.e(1);
          double cosa = cos(a);
          double sina = sin(a);
          double cosb = cos(b);
          double sinb = sin(b);
          //Jd.e(0,0) = 0;
          //Jd.e(0,1) = 0;
          Jd.e(0,2) = cosb*bd;
          //Jd.e(1,0) = 0;
          Jd.e(1,1) = -sina*ad;
          Jd.e(1,2) = -cosa*cosb*ad + sina*sinb*bd;
          //Jd.e(2,0) = 0;
          Jd.e(2,1) = cosa*ad;
          Jd.e(2,2) = -sina*cosb*ad - cosa*sinb*bd;
          return Jd;
        }
    };

    template<class Arg> 
    class RotationAboutAxesXYZ2 : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Mat3xV J, Jd;
      public:
        RotationAboutAxesXYZ2() : J(3), Jd(3) { J.e(2,2) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double a=q.e(0);
          double b=q.e(1);
          double g=q.e(2);
          double cosa = cos(a);
          double sina = sin(a);
          double cosb = cos(b);
          double sinb = sin(b);
          double cosg = cos(g);
          double sing = sin(g);
          A.e(0,0) = cosb*cosg;
          A.e(1,0) = sina*sinb*cosg+cosa*sing;
          A.e(2,0) = -cosa*sinb*cosg+sina*sing;
          A.e(0,1) = -cosb*sing;
          A.e(1,1) = -sing*sinb*sina+cosa*cosg;
          A.e(2,1) = cosa*sinb*sing+sina*cosg;
          A.e(0,2) = sinb;
          A.e(1,2) = -sina*cosb;
          A.e(2,2) = cosa*cosb;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) {
          double b = q.e(1);
          double g = q.e(2);
          J.e(0,0) = cos(b)*cos(g);
          J.e(0,1) = sin(g);
          //J.e(0,2) = 0;
          J.e(1,0) = -cos(b)*sin(g);
          J.e(1,1) = cos(g);
          //J.e(1,2) = 0;
          J.e(2,0) = sin(b);
          //J.e(2,1) = 0;
          //J.e(2,2) = 1;
          return J;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) {
          double b = q.e(1);
          double g = q.e(2);
          double bd = qd.e(1);
          double gd = qd.e(2);
          Jd.e(0,0) = -sin(b)*cos(g)*bd - cos(b)*sin(g)*gd;
          Jd.e(0,1) = cos(g)*gd;
          //Jd.e(0,2) = 0;
          Jd.e(1,0) = sin(b)*sin(g)*bd - cos(b)*cos(g)*gd;
          Jd.e(1,1) = -sin(g)*gd;
          //Jd.e(1,2) = 0; 
          Jd.e(2,0) = cos(b)*bd;
          //Jd.e(2,1) = 0;
          //Jd.e(2,2) = 0;
          return Jd;
        }
    };
  
  template<class Arg> 
    class RotationAboutAxesZXZ : public fmatvec::Function<fmatvec::RotMat3(Arg)> {
      private:
        fmatvec::RotMat3 A;
        fmatvec::Mat3xV J, Jd;
      public:
        RotationAboutAxesZXZ() : J(3), Jd(3) { }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::RotMat3 operator()(const Arg &q) {
          double psi=q.e(0);
          double theta=q.e(1);
          double phi=q.e(2);
          double spsi = sin(psi);
          double stheta = sin(theta);
          double sphi = sin(phi);
          double cpsi = cos(psi);
          double ctheta = cos(theta);
          double cphi = cos(phi);
          A.e(0,0) = cpsi*cphi-spsi*ctheta*sphi;
          A.e(1,0) = spsi*cphi+cpsi*ctheta*sphi;
          A.e(2,0) = stheta*sphi;
          A.e(0,1) = -cpsi*sphi-spsi*ctheta*cphi;
          A.e(1,1) = -spsi*sphi+cpsi*ctheta*cphi;
          A.e(2,1) = stheta*cphi;
          A.e(0,2) = spsi*stheta;
          A.e(1,2) = -cpsi*stheta;
          A.e(2,2) = ctheta;
          return A;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDer(const Arg &q) {
          throw MBSimError("RotationAboutAxesZXZ::parDer() not yet implemented.");
          return J;
        }
        typename fmatvec::Der<fmatvec::RotMat3, Arg>::type parDerDirDer(const Arg &qd, const Arg &q) {
          throw MBSimError("RotationAboutAxesZXZ::parDerDirDer() not yet implemented.");
          return Jd;
        }
    };

  template<class Arg> 
    class RotationAboutAxesXYZMapping : public fmatvec::Function<fmatvec::MatV(Arg)> {
      private:
        fmatvec::MatV T;
      public:
        RotationAboutAxesXYZMapping() : T(3,3,fmatvec::Eye()) { }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::MatV operator()(const Arg &q) {
          double alpha = q.e(0);
          double beta = q.e(1);
          double cos_beta = cos(beta);
          double sin_beta = sin(beta);
          double cos_alpha = cos(alpha);
          double sin_alpha = sin(alpha);
          double tan_beta = sin_beta/cos_beta;
          T.e(0,1) = tan_beta*sin_alpha;
          T.e(0,2) = -tan_beta*cos_alpha;
          T.e(1,1) = cos_alpha;
          T.e(1,2) = sin_alpha;
          T.e(2,1) = -sin_alpha/cos_beta;
          T.e(2,2) = cos_alpha/cos_beta;
          return T;
        }
    };

  template<class Arg> 
    class RotationAboutAxesXYZMapping2 : public fmatvec::Function<fmatvec::MatV(Arg)> {
      private:
        fmatvec::MatV T;
      public:
        RotationAboutAxesXYZMapping2() : T(3,3,fmatvec::Eye()) { }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::MatV operator()(const Arg &q) {
          double beta = q.e(1);
          double gamma = q.e(2);
          double cos_beta = cos(beta);
          double sin_beta = sin(beta);
          double cos_gamma = cos(gamma);
          double sin_gamma = sin(gamma);
          double tan_beta = sin_beta/cos_beta;
          T.e(0,0) = cos_gamma/cos_beta;
          T.e(0,1) = -sin_gamma/cos_beta;
          T.e(1,0) = sin_gamma;
          T.e(1,1) = cos_gamma;
          T.e(2,0) = -cos_gamma*tan_beta;
          T.e(2,1) = sin_gamma*tan_beta;
          return T;
        }
    };

  template<class Arg> 
    class RotationAboutAxesZXZMapping : public fmatvec::Function<fmatvec::MatV(Arg)> {
      private:
        fmatvec::MatV T;
      public:
        RotationAboutAxesZXZMapping() : T(3,3) { T.e(0,2) = 1; }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::MatV operator()(const Arg &q) {
          double psi = q.e(0);
          double theta = q.e(1);
          double cos_theta = cos(theta);
          double sin_theta = sin(theta);
          double cos_psi = cos(psi);
          double sin_psi = sin(psi);
          double tan_theta = sin_theta/cos_theta;

          T.e(0,0) = -sin_psi/tan_theta;
          T.e(0,1) = cos_psi/tan_theta;
          T.e(1,0) = cos_psi;
          T.e(1,1) = sin_psi;
          T.e(2,0) = sin_psi/sin_theta;
          T.e(2,1) = -cos_psi/sin_theta;

          return T;
        }
    };

  template<class Arg> 
    class RotationAboutAxesZXZMapping2 : public fmatvec::Function<fmatvec::MatV(Arg)> {
      private:
        fmatvec::MatV T;
      public:
        RotationAboutAxesZXZMapping2() : T(3,3,fmatvec::Eye()) { }
        typename fmatvec::Size<Arg>::type getArgSize() const { return 3; }
        fmatvec::MatV operator()(const Arg &q) {
          double theta = q.e(1);
          double phi = q.e(2);
          double cos_theta = cos(theta);
          double sin_theta = sin(theta);
          double cos_phi = cos(phi);
          double sin_phi = sin(phi);
          double tan_theta = sin_theta/cos_theta;

          T.e(0,0) = sin_phi/sin_theta;
          T.e(0,1) = cos_phi/sin_theta;
          T.e(1,0) = cos_phi;
          T.e(1,1) = -sin_phi;
          T.e(2,0) = -sin_phi/tan_theta;
          T.e(2,1) = -cos_phi/tan_theta;

          return T;
        }
    };


}

#endif /* _FUNCTION_LIBRARY_H_ */

