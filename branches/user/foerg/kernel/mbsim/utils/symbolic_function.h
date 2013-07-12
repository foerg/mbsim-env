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

#ifndef SYMBOLIC_FUNCTION_H_
#define SYMBOLIC_FUNCTION_H_

#include <mbsim/utils/function.h>
#include <casadi/symbolic/fx/sx_function.hpp>
#include "casadi/symbolic/matrix/matrix_tools.hpp"
#include "mbxmlutilstinyxml/casadiXML.h"

namespace MBSim {

  template <class Arg>
  class ToCasadi {
  };

  template <>
  class ToCasadi<double> {
    public:
      static double cast(const double &x) {
        return x;
      }
  };

  template <class Col>
  class ToCasadi<fmatvec::Vector<Col,double> > {
    public:
      static std::vector<double> cast(const fmatvec::Vector<Col,double> &x) {
        std::vector<double> y(x.size());
        for(int i=0; i<x.size(); i++)
          y[i] = x.e(i);
        return y; 
      }
  };

  template <class Ret>
  class FromCasadi {
    public:
      static Ret cast(const CasADi::Matrix<double> &x) {
        throw std::runtime_error("FromCasadi::cast not implemented for current type.");
        return Ret();
      }
  };

  template <class Col>
  class FromCasadi<fmatvec::Vector<Col,double> > {
    public:
      static fmatvec::Vector<Col,double> cast(const CasADi::Matrix<double> &x) {
        fmatvec::Vector<Col,double> y(x.size1(),fmatvec::NONINIT);
        for(int i=0; i<x.size1(); i++)
          y.e(i) = x(i,0).toScalar();
        return y;
      }
  };

  template <class Col>
  class FromCasadi<fmatvec::RowVector<Col,double> > {
    public:
      static fmatvec::RowVector<Col,double> cast(const CasADi::Matrix<double> &x) {
        fmatvec::RowVector<Col,double> y(x.size2(),fmatvec::NONINIT);
        for(int i=0; i<x.size2(); i++)
          y.e(i) = x(0,i).toScalar();
        return y;
      }
  };

  template <class Row>
  class FromCasadi<fmatvec::Matrix<fmatvec::General,Row,fmatvec::Var,double> > {
    public:
      static fmatvec::Matrix<fmatvec::General,Row,fmatvec::Var,double> cast(const CasADi::Matrix<double> &A) {
        fmatvec::Matrix<fmatvec::General,Row,fmatvec::Var,double> B(A.size1(),A.size2(),fmatvec::NONINIT);
        for(int i=0; i<A.size1(); i++)
          for(int j=0; j<A.size2(); j++)
            B.e(i,j) = A(i,j).toScalar();
        return B;
      }
  };

  template <>
  class FromCasadi<double> {
    public:
      static double cast(const CasADi::Matrix<double> &x) {
        return x.toScalar();
      }
  };

template<typename Sig>
class SymbolicFunction;

template<typename Ret, typename Arg>
  class SymbolicFunction<Ret(Arg)> : public Function<Ret(Arg)> {
    CasADi::SXFunction f, pd, dd, pddd, pdpd;
    public:
    SymbolicFunction() {}
    SymbolicFunction(const CasADi::SXFunction &f_) : f(f_) {
      init();
    }
//    SymbolicFunction(const CasADi::FX &f_) : f(CasADi::SXFunction(f_)) {
//      f.init();
//      pd = CasADi::SXFunction(f.inputExpr(),f.jac(0));
//      pd.init();
//    }

    void init() {
      f.init();
      pd = CasADi::SXFunction(f.inputExpr(),f.jac());
      pd.init();
      int nq = getArgSize();
      std::vector<CasADi::SX> sqd(nq);
      for(int i=0; i<nq; i++) {
        std::stringstream stream;
        stream << "qd" << i;
        sqd[i] = CasADi::SX(stream.str());
      }
      std::vector<CasADi::SXMatrix> input2(3);
      input2[0] = sqd;
      input2[1] = f.inputExpr(0);
      dd = CasADi::SXFunction(input2,f.jac().mul(sqd));
      dd.init();
      pddd = CasADi::SXFunction(input2,pd.jac().mul(sqd));
      pddd.init();
      pdpd = CasADi::SXFunction(f.inputExpr(),pd.jac());
      pdpd.init();
    }

    CasADi::SXFunction& getSXFunction() {return f;} 

    typename fmatvec::Size<Arg>::type getArgSize() const {
      return f.inputExpr(0).size();
    }

    std::string getType() const { return "SymbolicFunction2"; }

    Ret operator()(const Arg& x) {
      f.setInput(ToCasadi<Arg>::cast(x),0);
      f.evaluate();
      return FromCasadi<Ret>::cast(f.output());
    }

    typename fmatvec::Der<Ret, Arg>::type parDer(const Arg &x) {
      pd.setInput(ToCasadi<Arg>::cast(x),0);
      pd.evaluate();
      return FromCasadi<typename fmatvec::Der<Ret, Arg>::type>::cast(pd.output());
    }

    Ret dirDer(const Arg &xd, const Arg &x) {
      dd.setInput(ToCasadi<Arg>::cast(xd),0);
      dd.setInput(ToCasadi<Arg>::cast(x),1);
      dd.evaluate();
      return FromCasadi<Ret>::cast(dd.output());
    }

    typename fmatvec::Der<Ret, Arg>::type parDerDirDer(const Arg &xd, const Arg &x) {
      pddd.setInput(ToCasadi<Arg>::cast(xd),0);
      pddd.setInput(ToCasadi<Arg>::cast(x),1);
      pddd.evaluate();
      return FromCasadi<typename fmatvec::Der<Ret, Arg>::type>::cast(pddd.output());
    }

    typename fmatvec::Der<typename fmatvec::Der<Ret, Arg>::type, Arg>::type parDerParDer(const Arg &x) {
      pdpd.setInput(ToCasadi<Arg>::cast(x),0);
      pdpd.evaluate();
      return FromCasadi<typename fmatvec::Der<typename fmatvec::Der<Ret, Arg>::type, Arg>::type>::cast(pdpd.output());
    }

    void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
      f=CasADi::createCasADiSXFunctionFromXML(element->FirstChildElement());
      init();
      assert(f.getNumInputs()==1);
      assert(f.getNumOutputs()==1);
    }
  };

template<typename Ret, typename Arg1, typename Arg2>
  class SymbolicFunction<Ret(Arg1, Arg2)> : public Function<Ret(Arg1, Arg2)> {
    CasADi::SXFunction f, pd1, pd2, pd1dd1, pd1pd2, pd2dd1, pd2pd2;
    public:
    SymbolicFunction() {}
    SymbolicFunction(const CasADi::SXFunction &f_) : f(f_) {
      init();
    }
//    SymbolicFunction(const CasADi::FX &f_) : f(CasADi::SXFunction(f_)) {
//      f.init();
//      pd1 = CasADi::SXFunction(f.inputExpr(),f.jac(0));
//      pd1.init();
//      pd2 = CasADi::SXFunction(f.inputExpr(),f.jac(1));
//      pd2.init();
//    }

    void init() {
      f.init();
      pd1 = CasADi::SXFunction(f.inputExpr(),f.jac(0));
      pd1.init();
      pd2 = CasADi::SXFunction(f.inputExpr(),f.jac(1));
      pd2.init();
      int nq = getArg1Size();
      std::vector<CasADi::SX> sqd(nq);
      for(int i=0; i<nq; i++) {
        std::stringstream stream;
        stream << "qd" << i;
        sqd[i] = CasADi::SX(stream.str());
      }
      std::vector<CasADi::SXMatrix> input2(3);
      input2[0] = sqd;
      input2[1] = f.inputExpr(0);
      input2[2] = f.inputExpr(1);
      int n = f.outputExpr(0).size1();
      CasADi::SXMatrix Jd1(n,nq);
      CasADi::SXMatrix Jd2(n,nq);
      for(int j=0; j<nq; j++) {
        Jd1(CasADi::Slice(0,n),CasADi::Slice(j,j+1)) = pd1.jac(0)(CasADi::Slice(j,nq*n,nq),CasADi::Slice(0,nq)).mul(sqd);
        Jd2(CasADi::Slice(0,n),CasADi::Slice(j,j+1)) = pd1.jac(1)(CasADi::Slice(j,nq*n,nq),CasADi::Slice(0,1));
      }
      pd1dd1 = CasADi::SXFunction(input2,Jd1);
      pd1dd1.init();
      pd1pd2 = CasADi::SXFunction(f.inputExpr(),Jd2);
      pd1pd2.init();

      CasADi::SXMatrix djT1 = pd2.jac(0).mul(sqd);
      CasADi::SXMatrix djT2 = pd2.jac(1);
      pd2dd1 = CasADi::SXFunction(input2,djT1);
      pd2dd1.init();
      pd2pd2 = CasADi::SXFunction(f.inputExpr(),djT2);
      pd2pd2.init();
    }

    CasADi::SXFunction& getSXFunction() {return f;} 

    typename fmatvec::Size<Arg1>::type getArg1Size() const {
      return f.inputExpr(0).size();
    }

    typename fmatvec::Size<Arg2>::type getArg2Size() const {
      return f.inputExpr(1).size();
    }

    std::string getType() const { return "SymbolicFunction2"; }

    Ret operator()(const Arg1& x1, const Arg2& x2) {
      f.setInput(ToCasadi<Arg1>::cast(x1),0);
      f.setInput(ToCasadi<Arg2>::cast(x2),1);
      f.evaluate();
      return FromCasadi<Ret>::cast(f.output());
    }

    typename fmatvec::Der<Ret, Arg1>::type parDer1(const Arg1 &x1, const Arg2 &x2) {
      pd1.setInput(ToCasadi<Arg1>::cast(x1),0);
      pd1.setInput(ToCasadi<Arg2>::cast(x2),1);
      pd1.evaluate();
      return FromCasadi<typename fmatvec::Der<Ret, Arg1>::type>::cast(pd1.output());
    }

    typename fmatvec::Der<Ret, Arg2>::type parDer2(const Arg1 &x1, const Arg2 &x2) {
      pd2.setInput(ToCasadi<Arg1>::cast(x1),0);
      pd2.setInput(ToCasadi<Arg2>::cast(x2),1);
      pd2.evaluate();
      return FromCasadi<typename fmatvec::Der<Ret, Arg2>::type>::cast(pd2.output());
    }

    typename fmatvec::Der<Ret, Arg1>::type parDer1DirDer1(const Arg1 &xd1, const Arg1 &x1, const Arg2 &x2) {
      pd1dd1.setInput(ToCasadi<Arg1>::cast(xd1),0);
      pd1dd1.setInput(ToCasadi<Arg1>::cast(x1),1);
      pd1dd1.setInput(ToCasadi<Arg2>::cast(x2),2);
      pd1dd1.evaluate();
      return FromCasadi<typename fmatvec::Der<Ret, Arg1>::type>::cast(pd1dd1.output());
    }

    typename fmatvec::Der<typename fmatvec::Der<Ret, Arg1>::type, Arg2>::type parDer1ParDer2(const Arg1 &x1, const Arg2 &x2) {
      pd1pd2.setInput(ToCasadi<Arg1>::cast(x1),0);
      pd1pd2.setInput(ToCasadi<Arg2>::cast(x2),1);
      pd1pd2.evaluate();
      return FromCasadi<typename fmatvec::Der<typename fmatvec::Der<Ret, Arg1>::type, Arg2>::type>::cast(pd1pd2.output());
    }

    typename fmatvec::Der<Ret, Arg2>::type parDer2DirDer1(const Arg1 &xd1, const Arg1 &x1, const Arg2 &x2) {
      pd2dd1.setInput(ToCasadi<Arg1>::cast(xd1),0);
      pd2dd1.setInput(ToCasadi<Arg1>::cast(x1),1);
      pd2dd1.setInput(ToCasadi<Arg2>::cast(x2),2);
      pd2dd1.evaluate();
      return FromCasadi<typename fmatvec::Der<Ret, Arg2>::type>::cast(pd2dd1.output());
    }

    typename fmatvec::Der<typename fmatvec::Der<Ret, Arg2>::type, Arg2>::type parDer2ParDer2(const Arg1 &x1, const Arg2 &x2) {
      pd2pd2.setInput(ToCasadi<Arg1>::cast(x1),0);
      pd2pd2.setInput(ToCasadi<Arg2>::cast(x2),1);
      pd2pd2.evaluate();
      return FromCasadi<typename fmatvec::Der<typename fmatvec::Der<Ret, Arg2>::type, Arg2>::type>::cast(pd2pd2.output());
    }

    void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {
      f=CasADi::createCasADiSXFunctionFromXML(element->FirstChildElement());
      init();
      assert(f.getNumInputs()==2);
      assert(f.getNumOutputs()==1);
    }
  };

  template <class Ret, class Arg1, class Arg2, class Arg3>
  class SymbolicFunction3 : public Function<Ret(Arg1,Arg2,Arg3)> {
    CasADi::SXFunction f;
    public:
    SymbolicFunction3(const CasADi::SXFunction &f_) : f(f_) {
      f.init();
    }
    SymbolicFunction3(const CasADi::FX &f_) : f(CasADi::SXFunction(f_)) {
      f.init();
    }
    CasADi::SXFunction& getSXFunction() {return f;} 

    std::string getType() const { return "SymbolicFunction3"; }

    Ret operator()(const Arg1& x1, const Arg2& x2, const Arg3& x3, const void * =NULL) {
      f.setInput(ToCasadi<Arg1>::cast(x1),0);
      f.setInput(ToCasadi<Arg2>::cast(x2),1);
      f.setInput(ToCasadi<Arg3>::cast(x3),2);
      f.evaluate();
      return FromCasadi<Ret>::cast(f.output());
    }
  };

  // The following classes may be faster when calculating the function value
  // together with the first and second derivative
 
//  template <class Ret>
//  class SymbolicFunctionDerivatives1 : public Function1<Ret,double> {
//    private:
//      CasADi::SXFunction f;
//      CasADi::FX fder1, fder2;
//    public:
//    SymbolicFunctionDerivatives1(const CasADi::SXFunction &f_) : f(f_) {
//      f.init();
//      fder1 = f.jacobian();
//      fder1.init();
//      fder2 = fder1.jacobian();
//      fder2.init();
//    }
//    std::string getType() const { return "CasadiDiffFunction"; }
//
//    CasADi::FX& getFXFunction() {return fder2;} 
//
//    Ret operator()(const double& x_, const void * =NULL) {
//      fder2.setInput(x_);
//      fder2.evaluate();
//      return FromCasadi<Ret>::cast(fder2.output(2));
//    }
//  };
//
//  template <class Ret>
//  class SymbolicFXFunction : public Function1<Ret,double> {
//    private:
//    CasADi::FX &f;
//    int index;
//    public:
//    SymbolicFXFunction(CasADi::FX &f_, int index_=0) : f(f_), index(index_) {}
//
//    Ret operator()(const double& x_, const void * =NULL) {
//      return FromCasadi<Ret>::cast(f.output(2-index));
//    }
//  };

}

#endif
