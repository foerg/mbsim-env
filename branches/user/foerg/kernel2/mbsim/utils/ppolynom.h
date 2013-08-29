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
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef PPOLYNOM
#define PPOLYNOM

#include "fmatvec/fmatvec.h"
#include "fmatvec/function.h"
#include "mbsim/element.h"
#include "mbsim/utils/utils.h"
#include "mbsim/utils/eps.h"
#include "mbsim/mbsim_event.h"

namespace MBSim {

  /*! 
   * \brief class for piecewise-polynomials and cubic spline interpolation
   * \author Robert Huber
   * \date 2006-08-31 initial commit
   * \date 2006-09-05 verified with matlab /octave (Robert Huber)
   * \date 2008-05-21 C++ beauty treatment, PLinear (Thorsten Schindler)
   * \date 2009-08-11 kernel_dev (Thorsten Schindler)
   * \todo add deletes TODO
   * 
   * Spline / PP-Form info
   * Piecewise polynomial: c0* xloc^n + c1* xloc^(n-1) + c2* xloc^(n-2) + ... + cn
   * with [c0 c1 c2 ... cn] ith row vector of coefs-matrix 
   * breaks(i) << x << breaks(i+1) defines i and xloc = x-breaks(i)

   * Ex. cubic spline with
   * breaks= [0; 0.3; 0.5]
   * coefs= [d1 c1 b1 a1;  
   *       d2 c2 b2 a2]
   * -> S(x) = a1 + b1*xloc + c1*xloc^2 + d1*xloc^3 for x\in[0;0.3] and x_loc = x
   * -> S(x) = a2 + b2*xloc + c2*xloc^2 + d2*xloc^3 for x\in[0.3;0.5] and xloc = x - 0.3;

   * Cubic spline
   * (xi,fi) i=1..N is being interpolated by N-1 piecewise polynomials Si of degree 3 yielding a global C^2 curve
   * for uniqueness TWO additional boundary conditions are necessary (periodic / natural)
   * 
   * Piecewise linear polynomial
   * (xi,fi) i=1..N is being interpolated by N-1 piecewise polynomials Si of degree 1 yielding a globally weak differentiable curve
   * in the context of this class the second derivative is defined to be zero everywhere (which is mathematically wrong)
   */
template<class Ret>
  class PPolynom : public fmatvec::Function<Ret(double)> {

    template<typename Dep>
      struct Row {
        typedef fmatvec::ErrorType type;
      };

    template<typename DepVecShape>
      struct Row<fmatvec::Vector<DepVecShape, double> > {
        typedef fmatvec::RowVector<DepVecShape, double> type;
      };

    template<typename Dep>
      struct Tab {
        typedef fmatvec::ErrorType type;
      };

    template<typename DepVecShape>
      struct Tab<fmatvec::Vector<DepVecShape, double> > {
        typedef fmatvec::Matrix<fmatvec::General, fmatvec::Var, DepVecShape, double> type;
      };

    public:
      PPolynom() : f(this), fd(this), fdd(this) { }

      Ret operator()(const double &x) { return f(x); }
      typename fmatvec::Der<Ret, double>::type parDer(const double &x) { return fd(x); }
      typename fmatvec::Der<typename fmatvec::Der<Ret, double>::type, double>::type parDerParDer(const double &x) { return fdd(x); }

      /*! 
       * \brief set interpolation
       * @param x vector of ordered x values
       * @param f corresponding f(x) values (rowwise)
       * @param InterpolationMethod     'csplinePer' -> cubic Spline with periodic end conditions (two-times continuously differentiable)
       *                                                                                        S(x1) = S(xN) -> f(0)=f(end)
       *                                                                                        S'(x1) = S'(xN)
       *                                                                                        S''(x1) = S''(xN)
       *                                'csplineNat' -> cubic Spline with natural end conditions (two-times continuously differentiable)
       *                                                                                        S''(x1) = S''(xN) = 0
       *                                'plinear'    -> piecewise linear function (weak differentiable)
       */
      void setXF(const fmatvec::VecV &x, const typename Tab<Ret>::type &f, std::string InterpolationMethod) {
        assert(x.size() == f.rows());

        if(InterpolationMethod == "csplinePer") {
          calculateSplinePeriodic(x,f);   
        }
        else if(InterpolationMethod == "csplineNat") { 
          calculateSplineNatural(x,f);   
        }
        else if(InterpolationMethod == "plinear") {
          calculatePLinear(x,f);
        }
        else throw MBSimError("ERROR (PPolynom::setXF): No valid method to calculate pp-form");

        index = 0;
        nPoly = x.size()-1;
        order = coefs.size()-1;
      }

      /*! 
       * \return polynomial coefficients
       */
      std::vector<typename Tab<Ret>::type> getCoefs() { return coefs; }

      /*! 
       * \return interval boundaries
       */
      fmatvec::VecV getBreaks() { return breaks; }

      /*!
       * \brief set piecewise polynomial
       * \param polynomial coefficients
       * \param interval boundaries
       */
      void setPP(const std::vector<typename Tab<Ret>::type> &coefs_u, const fmatvec::Vec &breaks_u) {
        coefs = coefs_u; 
        breaks = breaks_u;
        index = 0;
        nPoly = (coefs[0]).rows();
        order = coefs.size()-1;
      }
        
      /**
       * \brief initialize function with XML code
       * \param XML element
       */
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);

    protected:
      /** 
       * \brief vector of polynomial coefficents
       */
      std::vector<typename Tab<Ret>::type> coefs;

      /**
       * \brief vector of breaks (interval boundaries)
       */
      fmatvec::Vec breaks;

      /**
       * \brief number of defined piecewise polynomials 
       */
      int nPoly;

      /**
       * \brief order of polynomial (3 for cubic polynomials) 
       */
      int order;

      /** 
       * \brief for internal use in ppeval functions 
       */
      int index;

      /*! 
       * \brief calculation of periodic spline by interpolation
       * \param interpolated arguments
       * \param interpolated function values
       */  
      void calculateSplinePeriodic(const fmatvec::VecV &x, const typename Tab<Ret>::type &f);

      /*! 
       * \brief calculation of natural spline by interpolation
       * \param interpolated arguments
       * \param interpolated function values
       */  
      void calculateSplineNatural(const fmatvec::VecV &x, const typename Tab<Ret>::type &f);

      /* 
       * \brief calculation of piecewise linear interpolation
       * \param interpolated arguments
       * \param interpolated function values
       *
       * the first derivative is weak and the second derivative is zero elsewhere although it should be distributionally at the corners
       */
      void calculatePLinear(const fmatvec::VecV &x, const typename Tab<Ret>::type &f);

     /**
       * piecewise polynomial interpolation - zeroth derivative
       */
      class ZerothDerivative {
        public:
          ZerothDerivative(PPolynom<Ret> *polynom) : parent(polynom), xSave(0), ySave(), firstCall(true) {}
          virtual ~ZerothDerivative() {}

          /* INHERITED INTERFACE OF FUNCTION */
          Ret operator()(const double &x);
          /***************************************************/

        private:
          PPolynom<Ret> *parent;
          double xSave;
          Ret ySave;
          bool firstCall;
      };

      /**
       * piecewise polynomial interpolation - first derivative
       */
      class FirstDerivative {
        public:
          FirstDerivative(PPolynom<Ret> *polynom) : parent(polynom), xSave(0), ySave(), firstCall(true) {}
          virtual ~FirstDerivative() {}

          /* INHERITED INTERFACE OF FUNCTION */
          Ret operator()(const double& x);
          /***************************************************/

        private:
          PPolynom<Ret> *parent;
          double xSave;
          Ret ySave;
          bool firstCall;
      };

      /**
       * piecewise polynomial interpolation - second derivative
       */
      class SecondDerivative {
        public:
          SecondDerivative(PPolynom<Ret> *polynom) : parent(polynom), xSave(0), ySave(), firstCall(true) {}
          virtual ~SecondDerivative() {}

          /* INHERITED INTERFACE OF FUNCTION */
          Ret operator()(const double& x);
          /***************************************************/

        private:
          PPolynom<Ret> *parent;
          double xSave;
          Ret ySave;
          bool firstCall;
      };

    private:
      ZerothDerivative f;
      FirstDerivative fd;
      SecondDerivative fdd;
  };

  template<class Ret>
  void PPolynom<Ret>::calculateSplinePeriodic(const fmatvec::VecV &x, const typename Tab<Ret>::type &f) {
    double hi, hii;
    int N = x.size();
    if(nrm2(f.row(0)-f.row(f.rows()-1))>epsroot()) throw MBSimError("ERROR (PPolynom::calculateSplinePeriodic): f(0)= "+numtostr(f.row(0))+"!="+numtostr(f.row(f.rows()-1))+" =f(end)");
    fmatvec::SqrMat C(N-1,fmatvec::INIT,0.0);
    fmatvec::Mat rs(N-1,f.cols(),fmatvec::INIT,0.0);

    // Matrix C and vector rs C*c=rs
    for(int i=0; i<N-3;i++) {
      hi = x(i+1) - x(i);
      hii = x(i+2)-x(i+1);
      C(i,i) = hi;
      C(i,i+1) = 2*(hi+hii);
      C(i,i+2) = hii;
      rs.row(i) = 3.*((f.row(i+2)-f.row(i+1))/hii - (f.row(i+1)-f.row(i))/hi);
    }

    // last but one row
    hi = x(N-2)-x(N-3);
    hii = x(N-1)-x(N-2);
    C(N-3,N-3) = hi;
    C(N-3,N-2)= 2*(hi+hii);
    C(N-3,0)= hii;
    rs.row(N-3) = 3.*((f.row(N-1)-f.row(N-2))/hii - (f.row(N-2)-f.row(N-3))/hi);

    // last row
    double h1 = x(1)-x(0);
    double hN_1 = x(N-1)-x(N-2);
    C(N-2,0) = 2*(h1+hN_1);
    C(N-2,1) = h1;
    C(N-2,N-2)= hN_1;
    rs.row(N-2) = 3.*((f.row(1)-f.row(0))/h1 - (f.row(0)-f.row(N-2))/hN_1);

    // solve C*c = rs -> TODO BETTER: RANK-1-MODIFICATION FOR LINEAR EFFORT (Simeon - Numerik 1)
    fmatvec::Mat c = slvLU(C,rs);
    fmatvec::Mat ctmp(N,f.cols());
    ctmp.row(N-1) = c.row(0); // CN = c1
    ctmp(0,0,N-2,f.cols()-1)= c;

    // vector ordering of the further coefficients
    fmatvec::Mat d(N-1,f.cols(),fmatvec::INIT,0.0);
    fmatvec::Mat b(N-1,f.cols(),fmatvec::INIT,0.0);
    fmatvec::Mat a(N-1,f.cols(),fmatvec::INIT,0.0);

    for(int i=0; i<N-1; i++) {
      hi = x(i+1)-x(i);  
      a.row(i) = f.row(i);
      d.row(i) = (ctmp.row(i+1) - ctmp.row(i) ) / 3. / hi;
      b.row(i) = (f.row(i+1)-f.row(i)) / hi - (ctmp.row(i+1) + 2.*ctmp.row(i) ) / 3. * hi;
    }

    breaks.resize(N);
    breaks = x;
    coefs.push_back(d);
    coefs.push_back(c);
    coefs.push_back(b);
    coefs.push_back(a);
  }

  template<class Ret>
  void PPolynom<Ret>::calculateSplineNatural(const fmatvec::VecV &x, const typename Tab<Ret>::type &f) {
    // first row
    int i=0;
    int N = x.size();
    fmatvec::SqrMat C(N-2,fmatvec::INIT,0.0);
    fmatvec::Mat rs(N-2,f.cols(),fmatvec::INIT,0.0);
    double hi = x(i+1)-x(i);
    double hii = x(i+2)-x(i+1);
    C(i,i) = 2*hi+2*hii;
    C(i,i+1) = hii;
    rs.row(i) = 3.*(f.row(i+2)-f.row(i+1))/hii - 3.*(f.row(i+1)-f.row(i))/hi;    

    // last row
    i = (N-3);
    hi = x(i+1)-x(i);
    hii = x(i+2)-x(i+1);
    C(i,i-1) = hi;
    C(i,i) = 2*hii + 2*hi;
    rs.row(i) = 3.*(f.row(i+2)-f.row(i+1))/hii - 3.*(f.row(i+1)-f.row(i))/hi;

    for(i=1;i<N-3;i++) { 
      hi = x(i+1)-x(i);
      hii = x(i+2)-x(i+1);
      C(i,i-1) = hi;
      C(i,i) = 2*(hi+hii);
      C(i,i+1) = hii;
      rs.row(i) = 3.*(f.row(i+2)-f.row(i+1))/hii - 3.*(f.row(i+1)-f.row(i))/hi;
    }

    // solve C*c = rs with C tridiagonal
    fmatvec::Mat C_rs(N-2,N-1+f.cols(),fmatvec::INIT,0.0);  
    C_rs(0,0,N-3,N-3) = C; // C_rs=[C rs] for Gauss in matrix
    C_rs(0,N-2,N-3,N-2+f.cols()-1) = rs;
    for(i=1; i<N-2; i++) C_rs.row(i) = C_rs.row(i) - C_rs.row(i-1)*C_rs(i,i-1)/C_rs(i-1,i-1); // C_rs -> upper triangular matrix C1 -> C1 rs1
    fmatvec::Mat rs1 = C_rs(0,N-2,N-3,N-2+f.cols()-1);
    fmatvec::Mat C1 = C_rs(0,0,N-3,N-3);
    fmatvec::Mat c(N-2,f.cols(),fmatvec::INIT,0.0);
    for(i=N-3;i>=0 ;i--) { // backward substitution
      typename Row<Ret>::type sum_ciCi(f.cols(),fmatvec::NONINIT); 
      sum_ciCi.init(0.);
      for(int ii=i+1; ii<=N-3; ii++) sum_ciCi = sum_ciCi + C1(i,ii)*c.row(ii);
      c.row(i)= (rs1.row(i) - sum_ciCi)/C1(i,i);
    }
    fmatvec::Mat ctmp(N,f.cols(),fmatvec::INIT,0.0);
    ctmp(1,0,N-2,f.cols()-1) = c; // c1=cN=0 natural splines c=[ 0; c; 0]

    // vector ordering of the further coefficients
    fmatvec::Mat d(N-1,f.cols(),fmatvec::INIT,0.0);
    fmatvec::Mat b(N-1,f.cols(),fmatvec::INIT,0.0);
    fmatvec::Mat a(N-1,f.cols(),fmatvec::INIT,0.0);

    for(i=0; i<N-1; i++) {
      hi = x(i+1)-x(i);  
      a.row(i) = f.row(i);
      d.row(i) = (ctmp.row(i+1) - ctmp.row(i) ) / 3. / hi;
      b.row(i) = (f.row(i+1)-f.row(i)) / hi - (ctmp.row(i+1) + 2.*ctmp.row(i) ) / 3. * hi;
    }

    breaks.resize(N);
    breaks = x;
    coefs.push_back(d);
    coefs.push_back(ctmp(0,0,N-2,f.cols()-1));
    coefs.push_back(b);
    coefs.push_back(a);
  }

  template<class Ret>
  void PPolynom<Ret>::calculatePLinear(const fmatvec::VecV &x, const typename Tab<Ret>::type &f) {
    int N = x.size(); // number of supporting points

    breaks.resize(N);
    breaks = x;

    fmatvec::Mat m(N-1,f.cols(),fmatvec::INIT,0.0);
    fmatvec::Mat a(N-1,f.cols(),fmatvec::INIT,0.0);
    for(int i=1;i<N;i++) {
      m.row(i-1) = (f.row(i)-f.row(i-1))/(x(i)-x(i-1)); // slope
      a.row(i-1) = f.row(i-1);
    }
    coefs.push_back(m);
    coefs.push_back(a);
  }
          
  template<class Ret>
  Ret PPolynom<Ret>::ZerothDerivative::operator()(const double& x) {
    if(x>(parent->breaks)(parent->nPoly)) 
      throw MBSimError("ERROR (PPolynom::operator()): x out of range! x= "+numtostr(x)+", upper bound= "+numtostr((parent->breaks)(parent->nPoly)));
    if(x<(parent->breaks)(0)) 
      throw MBSimError("ERROR (PPolynom::operator()): x out of range! x= "+numtostr(x)+", lower bound= "+numtostr((parent->breaks)(0)));

    if ((fabs(x-xSave)<macheps()) && !firstCall)
      return ySave;
    else {
      firstCall = false;
      if(x<(parent->breaks)(parent->index)) // saved index still OK? otherwise search downwards
        while((parent->index) > 0 && (parent->breaks)(parent->index) > x)
          (parent->index)--;
      else if(x>(parent->breaks)((parent->index)+1)) { // saved index still OK? otherwise search upwards
        while((parent->index) < (parent->nPoly) && (parent->breaks)(parent->index) <= x)
          (parent->index)++;
        (parent->index)--;  
      }

      const double dx = x - (parent->breaks)(parent->index); // local coordinate
      fmatvec::VecV yi = trans(((parent->coefs)[0]).row(parent->index));
      for(int i=1;i<=(parent->order);i++) // Horner scheme
        yi = yi*dx+trans(((parent->coefs)[i]).row(parent->index));
      xSave=x;
      ySave=yi;
      return yi;
    }
  }

  template<class Ret>
  Ret PPolynom<Ret>::FirstDerivative::operator()(const double& x) {
    if(x>(parent->breaks)(parent->nPoly)) throw MBSimError("ERROR (PPolynom::diff1): x out of range! x= "+numtostr(x)+", upper bound= "+numtostr((parent->breaks)(parent->nPoly)));
    if(x<(parent->breaks)(0)) throw MBSimError("ERROR (PPolynom::diff1): x out of range!   x= "+numtostr(x)+" lower bound= "+numtostr((parent->breaks)(0)));

    if ((fabs(x-xSave)<macheps()) && !firstCall)
      return ySave;
    else {
      firstCall = false;
      if(x<(parent->breaks)(parent->index)) // saved index still OK? otherwise search downwards
        while((parent->index) > 0 && (parent->breaks)(parent->index) > x)
          (parent->index)--;
      else if(x>(parent->breaks)((parent->index)+1)) { // saved index still OK? otherwise search upwards
        while((parent->index) < (parent->nPoly) && (parent->breaks)(parent->index) <= x)
          (parent->index)++;
        (parent->index)--;  
      }

      double dx = x - (parent->breaks)(parent->index);
      fmatvec::VecV yi = trans(((parent->coefs)[0]).row(parent->index))*double(parent->order);
      for(int i=1;i<parent->order;i++)
        yi = yi*dx+trans(((parent->coefs)[i]).row(parent->index))*double((parent->order)-i);
      xSave=x;
      ySave=yi;
      return yi;
    }
  }

  template<class Ret>
  Ret PPolynom<Ret>::SecondDerivative::operator()(const double& x) {
    if(x>(parent->breaks)(parent->nPoly)) throw MBSimError("ERROR (PPolynom::diff2): x out of range!   x= "+numtostr(x)+" upper bound= "+numtostr((parent->breaks)(parent->nPoly)));
    if(x<(parent->breaks)(0)) throw MBSimError("ERROR (PPolynom::diff2): x out of range!   x= "+numtostr(x)+" lower bound= "+numtostr((parent->breaks)(0)));

    if ((fabs(x-xSave)<macheps()) && !firstCall)
      return ySave;
    else {
      firstCall = false;
      if(x<(parent->breaks)(parent->index)) // saved index still OK? otherwise search downwards
        while((parent->index) > 0 && (parent->breaks)(parent->index) > x)
          (parent->index)--;
      else if(x>(parent->breaks)((parent->index)+1)) { // saved index still OK? otherwise search upwards
        while((parent->index) < (parent->nPoly) && (parent->breaks)(parent->index) <= x)
          (parent->index)++;
        (parent->index)--;  
      }

      double dx = x - (parent->breaks)(parent->index);
      fmatvec::VecV yi = trans(((parent->coefs)[0]).row(parent->index))*double(parent->order)*double((parent->order)-1);
      for(int i=1;i<=((parent->order)-2);i++)
        yi = yi*dx+trans(((parent->coefs)[i]).row(parent->index))*double((parent->order)-i)*double((parent->order)-i-1);
      xSave=x;
      ySave=yi;
      return yi;
    }
  }

  template<class Ret>
  void PPolynom<Ret>::initializeUsingXML(MBXMLUtils::TiXmlElement * element) {
    MBXMLUtils::TiXmlElement *e;
    fmatvec::VecV x;
    typename Tab<Ret>::type y;
    e=element->FirstChildElement(MBSIMNS"x");
    if (e) {
      x=Element::getVec(e);
      e=element->FirstChildElement(MBSIMNS"y");
      y=Element::getMat(e, x.size(), 0);
    }
    else {
      e=element->FirstChildElement(MBSIMNS"xy");
      typename Tab<Ret>::type xy=Element::getMat(e);
      assert(xy.cols()>1);
      x=xy.col(0);
      y=xy(fmatvec::Index(0, xy.rows()-1), fmatvec::Index(1, xy.cols()-1));
    }
    std::string method;
    if (element->FirstChildElement(MBSIMNS"cSplinePeriodic"))
      method="csplinePer";
    else if (element->FirstChildElement(MBSIMNS"cSplineNatural"))
      method="csplineNat";
    else if (element->FirstChildElement(MBSIMNS"piecewiseLinear"))
      method="plinear";
    setXF(x, y, method);
  }


}

#endif /* PPOLYNOM */

