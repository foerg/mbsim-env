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

#ifndef _KINEMATICS_H_
#define _KINEMATICS_H_

#include "fmatvec.h"
#include "function.h"
#include "mbsim/utils/function.h"

namespace MBSim {

  class Translation {
    protected:
      fmatvec::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fr;
      fmatvec::Function<fmatvec::MatV(fmatvec::VecV, double)> *fT;
      fmatvec::Vec3 r, j, jd;
      fmatvec::Mat3xV J, Jd, drdq, dotdrdq;
      fmatvec::MatV T, dotT;

    public:
      Translation(fmatvec::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fr_=0, fmatvec::Function<fmatvec::MatV(fmatvec::VecV, double)> *fT_=0) : fr(fr_), fT(fT_) { }

      virtual ~Translation() {delete fr; delete fT;}

      virtual void init();

      virtual int getqSize() const { return fr->getArg1Size(); }
      virtual int getuSize() const { return (*fT)(fmatvec::Vec3(),0).cols(); }

      virtual bool isIndependent() const { return false; }

      const fmatvec::Vec3 getPosition() const {return r;}
      const fmatvec::Mat3xV getJacobian() const {return J;}
      const fmatvec::Vec3 getGuidingVelocity() const {return j;}
      const fmatvec::Mat3xV getDerivativeOfJacobian() const {return Jd;}
      const fmatvec::Vec3 getDerivativeOfGuidingVelocity() const {return jd;}
      const fmatvec::MatV getT() const {return T;}

      void updateStateDependentVariables(const fmatvec::VecV &q, const double &t);
      void updateStateDerivativeDependentVariables(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t);

      virtual void updatePosition(const fmatvec::VecV &q, const double &t) { r = (*fr)(q,t); }
      virtual void updateJacobian(const fmatvec::VecV &q, const double &t) { drdq = fr->parDer1(q,t); J = drdq*T; }
      virtual void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { j = fr->parDer2(q,t); }
      virtual void updateT(const fmatvec::VecV &q, const double &t) { T = (*fT)(q,t); }
      virtual void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { dotT = fT->dirDer1(qd,q,t) + fT->parDer2(q,t); }
      virtual void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { dotdrdq = fr->parDer1DirDer1(qd,q,t)+fr->parDer1ParDer2(q,t); Jd = dotdrdq*T + drdq*dotT; }
      virtual void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { jd = fr->parDer2DirDer1(qd,q,t) + fr->parDer2ParDer2(q,t); }
  };

  class TranslationTeqI : public Translation {

    public:
      TranslationTeqI(fmatvec::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fr=0) : Translation(fr) { }

      void init();

      int getuSize() const { return getqSize(); }

      void updateJacobian(const fmatvec::VecV &q, const double &t) { drdq = fr->parDer1(q,t); J = drdq; }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { dotdrdq = fr->parDer1DirDer1(qd,q,t)+fr->parDer1ParDer2(q,t); Jd = dotdrdq; }
  };

  class TranslationInXDirection : public TranslationTeqI {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 1;}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r(0) = q(0); }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class TranslationInYDirection : public TranslationTeqI {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 1;}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r(1) = q(0); }
      void updatePartialDerivativeOfPosition(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfPosition(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class TranslationInZDirection : public TranslationTeqI {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 1;}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r(2) = q(0); }
      void updatePartialDerivativeOfPosition(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfPosition(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class TranslationInXYDirection : public TranslationTeqI {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 2;}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r(0) = q(0); r(1) = q(1); }
      void updatePartialDerivativeOfPosition(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfPosition(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class TranslationInXZDirection : public TranslationTeqI {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 2;}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r(0) = q(0); r(2) = q(1); }
      void updatePartialDerivativeOfPosition(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfPosition(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class TranslationInYZDirection : public TranslationTeqI {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 2;}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r(1) = q(0); r(2) = q(1); }
      void updatePartialDerivativeOfPosition(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfPosition(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class TranslationInXYZDirection : public TranslationTeqI {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 3;}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r(0) = q(0); r(1) = q(1); r(2) = q(2); }
      void updatePartialDerivativeOfPosition(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfPosition(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class LinearTranslation : public TranslationTeqI {
    private:
      fmatvec::Mat3xV D;

    public:

      LinearTranslation() { }
      LinearTranslation(const fmatvec::Mat3xV &D_) { D = D_; }

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return D.cols();}

      void updatePosition(const fmatvec::VecV &q, const double &t) { r = J*q; }
      void updatePartialDerivativeOfPosition(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfPosition(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

//  class TimeDependentLinearTranslation: public Translation {
//    public:
//      /**
//       * \brief constructor
//       */
//      TimeDependentLinearTranslation() : pos(NULL) {}
//
//      /**
//       * \brief constructor
//       * \param independent generalized position function
//       * \param independent direction matrix of translation
//       */
//      TimeDependentLinearTranslation(Function1<fmatvec::VecV, double> *pos_, const fmatvec::Mat3xV &PJT_) : PJT(PJT_), pos(pos_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~TimeDependentLinearTranslation() { delete pos; pos = 0; }
//
//      bool isIndependent() const { return true; }
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqTSize() const {return 0;}
//      virtual int getuTSize() const {return 0;}
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) { return PJT*(*pos)(t); }
//      //virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      /***************************************************/
//
//      /* GETTER / SETTER */
//      Function1<fmatvec::VecV, double>* getTranslationalFunction() { return pos; }
//      void setTranslationalFunction(Function1<fmatvec::VecV, double> *pos_) { pos = pos_; }
//      const fmatvec::Mat3xV& getTranslationVectors() const { return PJT; }
//      void setTranslationVectors(const fmatvec::Mat3xV& PJT_) { PJT = PJT_; }
//      /***************************************************/
//
//    private:
//      /**
//       * independent direction matrix of translation
//       */
//      fmatvec::Mat3xV PJT;
//
//      /**
//       * \brief time dependent generalized position
//       */
//      Function1<fmatvec::VecV, double> *pos;
//  };
//
//  /**
//   * \brief class to describe time dependent translations
//   * \author Markus Schneider
//   * \date 2009-12-21 some adaptations (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Translation (Martin Foerg)
//   */
//  class TimeDependentTranslation : public Translation {
//    public:
//      /**
//       * \brief constructor
//       */
//      TimeDependentTranslation() : pos(NULL) {}
//
//      /**
//       * \brief constructor
//       * \param independent translation function
//       */
//      TimeDependentTranslation(Function1<fmatvec::Vec3, double> *pos_) : pos(pos_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~TimeDependentTranslation() { delete pos; pos = 0; }
//
//      bool isIndependent() const { return true; }
//
//      /* INTERFACE OF TRANSLATION */
//      virtual int getqTSize() const {return 0;}
//      virtual int getuTSize() const {return 0;}
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) { return (*pos)(t); }
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      /***************************************************/
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the translation function
//       */
//      Function1<fmatvec::Vec3, double>* getTranslationFunction() { return pos; }
//      void setTranslationFunction(Function1<fmatvec::Vec3, double> *pos_) { pos = pos_; }
//      /***************************************************/
//
//    private:
//      /**
//       * time dependent translation function
//       */
//      Function1<fmatvec::Vec3, double> *pos;
//  };
//
//  class StateDependentTranslation : public Translation {
//    public:
//      StateDependentTranslation() : qSize(0), pos(NULL) {}
//
//      /**
//       * \brief constructor
//       */
//      StateDependentTranslation(int qSize_, Function1<fmatvec::Vec3,fmatvec::VecV> *pos_) : qSize(qSize_), pos(pos_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~StateDependentTranslation() { delete pos; pos = 0; }
//
//      /* INTERFACE FOR DERIVED CLASSES */
//      /**
//       * \return degree of freedom of translation
//       */
//      virtual int getqSize() const {return qSize;}
//
//      /**
//       * \param generalized position
//       * \param time
//       * \return translational vector as a function of generalized position and time, r=r(q,t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) { return (*pos)(q); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; }
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the translation function
//       */
//      Function1<fmatvec::Vec3,fmatvec::VecV>* getTranslationFunction() { return pos; }
//      void setTranslationFunction(Function1<fmatvec::Vec3,fmatvec::VecV> *pos_) { pos = pos_; }
//      /***************************************************/
//
//    private:
//      int qSize;
//      Function1<fmatvec::Vec3,fmatvec::VecV> *pos;
//  };
//
//  class GeneralTranslation : public Translation {
//    public:
//      GeneralTranslation() : qSize(0), pos(NULL) {}
//      /**
//       * \brief constructor
//       */
//      GeneralTranslation(int qSize_, Function2<fmatvec::Vec3,fmatvec::VecV,double> *pos_) : qSize(qSize_), pos(pos_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~GeneralTranslation() { delete pos; pos = 0; }
//
//      /* INTERFACE FOR DERIVED CLASSES */
//      /**
//       * \return degree of freedom of translation
//       */
//      virtual int getqSize() const {return qSize;}
//
//      /**
//       * \param generalized position
//       * \param time
//       * \return translational vector as a function of generalized position and time, r=r(q,t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) { return (*pos)(q,t); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent) { return 0; }
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the translation function
//       */
//      Function2<fmatvec::Vec3,fmatvec::VecV,double>* getTranslationFunction() { return pos; }
//      void setTranslationFunction(Function2<fmatvec::Vec3,fmatvec::VecV,double> *pos_) { pos = pos_; }
//      /***************************************************/
//
//    private:
//      int qSize;
//      Function2<fmatvec::Vec3,fmatvec::VecV,double> *pos;
//  };

  class Rotation {
    protected:
      fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV, double)> *fA;
      fmatvec::Function<fmatvec::MatV(fmatvec::VecV, double)> *fT;
      fmatvec::RotMat3 A;
      fmatvec::Vec3 j, jd;
      fmatvec::Mat3xV J, Jd, dAdq, dotdAdq;
      fmatvec::MatV T, dotT;

    public:
      Rotation(fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV, double)> *fA_=0, fmatvec::Function<fmatvec::MatV(fmatvec::VecV, double)> *fT_=0) : fA(fA_), fT(fT_) { }

      virtual ~Rotation() {delete fA; delete fT;}

      virtual void init();

      virtual int getqSize() const { return fA->getArg1Size(); }
      virtual int getuSize() const { return (*fT)(fmatvec::Vec3(),0).cols(); }

      virtual bool isIndependent() const { return false; }

      const fmatvec::RotMat3 getOrientation() const {return A;}
      const fmatvec::Mat3xV getJacobian() const {return J;}
      const fmatvec::Vec3 getGuidingVelocity() const {return j;}
      const fmatvec::Mat3xV getDerivativeOfJacobian() const {return Jd;}
      const fmatvec::Vec3 getDerivativeOfGuidingVelocity() const {return jd;}
      const fmatvec::MatV getT() const {return T;}

      void updateStateDependentVariables(const fmatvec::VecV &q, const double &t);
      void updateStateDerivativeDependentVariables(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t);

      virtual void updateOrientation(const fmatvec::VecV &q, const double &t) { A = (*fA)(q,t); }
      virtual void updateJacobian(const fmatvec::VecV &q, const double &t) { dAdq = fA->parDer1(q,t); J = dAdq*T; }
      virtual void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { j = fA->parDer2(q,t); }
      virtual void updateT(const fmatvec::VecV &q, const double &t) { T = (*fT)(q,t); }
      virtual void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { dotT = fT->dirDer1(qd,q,t) + fT->parDer2(q,t); }
      virtual void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { dotdAdq = fA->parDer1DirDer1(qd,q,t)+fA->parDer1ParDer2(q,t); Jd = dotdAdq*T + dAdq*dotT; }
      virtual void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { jd = fA->parDer2DirDer1(qd,q,t) + fA->parDer2ParDer2(q,t); }
  };

  class RotationTeqI : public Rotation {

    public:
      RotationTeqI(fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV, double)> *fA=0) : Rotation(fA) { }

      void init();

      int getuSize() const { return getqSize(); }

      void updateJacobian(const fmatvec::VecV &q, const double &t) { dAdq = fA->parDer1(q,t); J = dAdq; }
      void updateT(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { dotdAdq = fA->parDer1DirDer1(qd,q,t)+fA->parDer1ParDer2(q,t); Jd = dotdAdq; }
  };

  class RotationAboutXAxis : public RotationTeqI {
    public:

      bool isIndependent() const { return true; }

      void init();

      int getqSize() const {return 1;}

      void updateOrientation(const fmatvec::VecV &q, const double &t);
      void updatePartialDerivativeOfOrientation(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfOrientation(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class RotationAboutYAxis : public RotationTeqI {
    public:

      bool isIndependent() const { return true; }

      void init();

      int getqSize() const {return 1;}

      void updateOrientation(const fmatvec::VecV &q, const double &t);
      void updatePartialDerivativeOfOrientation(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfOrientation(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class RotationAboutZAxis : public RotationTeqI {
    public:

      bool isIndependent() const { return true; }

      void init();

      int getqSize() const {return 1;}

      void updateOrientation(const fmatvec::VecV &q, const double &t);
      void updatePartialDerivativeOfOrientation(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfOrientation(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {}
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

  class RotationAboutFixedAxis : public RotationTeqI {
    private:
      fmatvec::Vec3 a;

    public:

      RotationAboutFixedAxis(const fmatvec::Vec3 &a_) : a(a_) { }

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 1;}

      void updateOrientation(const fmatvec::VecV &q, const double &t);
      void updatePartialDerivativeOfOrientation(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfPartialDerivativeOfOrientation(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

//  /**
//   * \brief class to describe state dependent rotation about fixed axis
//   */
//  class StateDependentRotationAboutFixedAxis: public Rotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      StateDependentRotationAboutFixedAxis() : qSize(0), rot(new RotationAboutFixedAxis()), angle(NULL) {}
//
//      /**
//       * \brief constructor
//       */
//      StateDependentRotationAboutFixedAxis(int qSize_, Function1<double, fmatvec::VecV> *angle_, const fmatvec::Vec3 &a_) : qSize(qSize_), rot(new RotationAboutFixedAxis(a_)), angle(angle_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~StateDependentRotationAboutFixedAxis() { delete rot; rot = 0; delete angle; angle = 0; }
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqSize() const {return qSize;}
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) {return (*rot)(fmatvec::VecV(1,fmatvec::INIT,(*angle)(q)),t);} 
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      /***************************************************/
//
//      /* GETTER / SETTER */
//      Function1<double, fmatvec::VecV>* getRotationalFunction() { return angle; }
//      void setRotationalFunction(Function1<double, fmatvec::VecV> *angle_) { angle = angle_; }
//      const fmatvec::Vec3& getAxisOfRotation() const { return rot->getAxisOfRotation(); }
//      void setAxisOfRotation(const fmatvec::Vec3& a_) { rot->setAxisOfRotation(a_); }
//      /***************************************************/
//
//    private:
//      int qSize;
//      RotationAboutFixedAxis *rot;
//      Function1<double, fmatvec::VecV> *angle;
//  };
//
//  /**
//   * \brief class to describe time dependent rotation about fixed axis
//   * \author Thorsten Schindler
//   * \date 2009-12-21 initial commit (Thorsten Schindler)
//   * \date 2009-12-22 should be a rotation because otherwise it has some dof (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Rotation (Martin Foerg)
//   */
//  class TimeDependentRotationAboutFixedAxis: public TranslationIndependentRotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      TimeDependentRotationAboutFixedAxis() : rot(new RotationAboutFixedAxis()), angle(NULL) {}
//
//      /**
//       * \brief constructor
//       * \param independent rotation angle function
//       * \param axis of rotation
//       */
//      TimeDependentRotationAboutFixedAxis(Function1<double, double> *angle_, const fmatvec::Vec3 &a_) : rot(new RotationAboutFixedAxis(a_)), angle(angle_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~TimeDependentRotationAboutFixedAxis() { delete rot; rot = 0; delete angle; angle = 0; }
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqRSize() const { return 0; }
//      virtual int getuRSize() const {return 0;}
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) {return (*rot)(fmatvec::VecV(1,fmatvec::INIT,(*angle)(t)),t);}
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      /***************************************************/
//
//      /* GETTER / SETTER */
//      Function1<double, double>* getRotationalFunction() { return angle; }
//      void setRotationalFunction(Function1<double, double> *angle_) { angle = angle_; }
//      const fmatvec::Vec3& getAxisOfRotation() const { return rot->getAxisOfRotation(); }
//      void setAxisOfRotation(const fmatvec::Vec3& a_) { rot->setAxisOfRotation(a_); }
//      /***************************************************/
//
//    private:
//      /**
//       * \brief rotational parametrisation
//       */
//      RotationAboutFixedAxis *rot;
//
//      /**
//       * \brief time dependent rotation angle
//       */
//      Function1<double, double> *angle;
//  };

//  /**
//   * \brief class to describe rotation about axes x and y with basic rotations interpretated in the current coordinate system
//   * \author Martin Foerg
//   * \date 2009-12-21 some localisations (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Rotation (Martin Foerg)
//   */
//  class RotationAboutAxesXY: public TranslationIndependentRotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      RotationAboutAxesXY() {}
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqRSize() const { return 2; }
//      virtual int getuRSize() const { return 2; }
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
//      /***************************************************/
//  };
//  /**
//   * \brief class to describe rotation about axes x and z with basic rotations interpretated in the current coordinate system
//   * \author Martin Foerg
//   */
//  class RotationAboutAxesXZ: public TranslationIndependentRotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      RotationAboutAxesXZ() {}
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqRSize() const { return 2; }
//      virtual int getuRSize() const { return 2; }
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
//      /***************************************************/
//  };
//
//  /**
//   * \brief class to describe rotation about axes y and z with basic rotations interpretated in the current coordinate system
//   * \author Martin Foerg
//   * \date 2009-12-21 some localisations (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Rotation (Martin Foerg)
//   */
//  class RotationAboutAxesYZ: public TranslationIndependentRotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      RotationAboutAxesYZ() {}
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqRSize() const { return 2; }
//      virtual int getuRSize() const { return 2; }
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
//      /***************************************************/
//  };

  class CardanAngles : public Rotation {
    public:

      void init();

      bool isIndependent() const { return true; }

      int getqSize() const {return 3;}
      int getuSize() const {return 3;}

      void updateOrientation(const fmatvec::VecV &q, const double &t);
      void updateJacobian(const fmatvec::VecV &q, const double &t) { }
      void updateGuidingVelocity(const fmatvec::VecV &q, const double &t) { }
      void updateT(const fmatvec::VecV &q, const double &t);
      void updateDerivativeOfT(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfJacobian(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }
      void updateDerivativeOfGuidingVelocity(const fmatvec::VecV &qd, const fmatvec::VecV &q, const double &t) { }

      void initializeUsingXML(MBXMLUtils::TiXmlElement *element) { }
      MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
  };

//  /**
//   * \brief class to describe rotation parametrised by Euler angles
//   * \author Martin Foerg
//   * \date 2010-10-20 first commit (Martin Foerg)
//   */
//  class EulerAngles: public TranslationIndependentRotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      EulerAngles() {}
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqRSize() const { return 3; }
//      virtual int getuRSize() const { return 3; }
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
//      /***************************************************/
//  };
//
//  /**
//   * \brief class to describe rotation parametrised by cardan angles
//   * \author Martin Foerg
//   * \date 2009-04-08 some comments (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Rotation (Martin Foerg)
//   */
//  class RotationAboutAxesXYZ: public TranslationIndependentRotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      RotationAboutAxesXYZ() {}
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqRSize() const { return 3; }
//      virtual int getuRSize() const { return 3; }
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *parent);
//      /***************************************************/
//  };
//
//  /**
//   * \brief class to describe time dependent rotation parametrised by Cardan angles
//   * \author Thorsten Schindler
//   * \date 2009-12-21 initial commit (Thorsten Schindler)
//   * \date 2009-12-22 should be a rotation because otherwise it has some dof (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Rotation (Martin Foerg)
//   */
//  class TimeDependentCardanAngles: public TranslationIndependentRotation {
//    public:
//      /**
//       * \brief constructor
//       */
//      TimeDependentCardanAngles() : rot(new CardanAngles()), angle(NULL) {}
//
//      /**
//       * \brief constructor
//       * \param independent rotation angle function
//       */
//      TimeDependentCardanAngles(Function1<fmatvec::Vec3, double> *angle_) : rot(new CardanAngles()), angle(angle_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~TimeDependentCardanAngles() { delete rot; rot = 0; delete angle; angle = 0; }
//
//      /* INTERFACE OF ROTATION */
//      virtual int getqRSize() const { return 0; }
//      virtual int getuRSize() const { return 0; }
//      virtual fmatvec::SqrMat3 operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      /***************************************************/
//
//      /* GETTER / SETTER */
//      Function1<fmatvec::Vec3, double>* getRotationalFunction() { return angle; }
//      void setRotationalFunction(Function1<fmatvec::Vec3, double> *angle_) { angle = angle_; }
//    private:
//      /**
//       * \brief rotational parametrisation
//       */
//      CardanAngles *rot;
//
//      /**
//       * \brief time dependent rotation angle
//       */
//      Function1<fmatvec::Vec3, double> *angle;
//  };

//  class TMatrix : public Function2<fmatvec::MatV,fmatvec::VecV,double> {
//    public:
//      /**
//       * \brief constructor
//       */
//      TMatrix() {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~TMatrix() {}
//
//      /* INTERFACE FOR DERIVED CLASSES */
//      /**
//       * \return column size of TMatrix
//       */
//      virtual int getqSize() const = 0;
//      virtual int getuSize() const = 0;
//
//      virtual fmatvec::MatV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) = 0;
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      /***************************************************/
//  };
//
//  /**
//   * \brief base class to describe Jacobians along a path
//   * \author Martin Foerg
//   * \date 2009-04-08 some comments (Thorsten Schindler)
//   * \date 2009-04-20 some comments (Thorsten Schindler)
//   * \date 2010-05-23 Jacobian inherits Function2 (Martin Foerg)
//   */
//  class Jacobian : public Function2<fmatvec::Mat3xV,fmatvec::VecV,double> {
//    public:
//      /**
//       * \brief constructor
//       */
//      Jacobian() {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~Jacobian() {}
//
//      /* INTERFACE FOR DERIVED CLASSES */
//      /**
//       * \return column size of Jacobian
//       */
//      virtual int getuSize() const = 0;
//
//      /**
//       * \param generalized position
//       * \param time
//       * \return Jacobian matrix as a function of generalized position and time,
//       * J=J(q,t)
//       */
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) = 0;
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      /***************************************************/
//  };
//
//  /**
//   * \brief class to describe a constant Jacobians
//   * \author Martin Foerg
//   * \date 2009-04-08 some comments (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Jacobian (Martin Foerg)
//   */
//  class ConstantJacobian : public Jacobian {
//    public:
//      /**
//       * \brief constructor
//       */
//      ConstantJacobian(const fmatvec::Mat &J_) { J = J_; }
//
//      ConstantJacobian() {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~ConstantJacobian() {}
//
//      /* INTERFACE OF JACOBIAN */
//      virtual int getuSize() const { return J.cols(); }
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) { return J; }
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief constant Jacobian
//       */
//      fmatvec::Mat3xV J;
//  };
//
//  class StateDependentJacobian : public Jacobian {
//    public:
//      /**
//       * \brief constructor
//       */
//      StateDependentJacobian(int uSize_, Function1<fmatvec::Mat3xV,fmatvec::VecV> *J_) : uSize(uSize_), J(J_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~StateDependentJacobian() { delete J; J = 0; }
//
//      /* INTERFACE FOR DERIVED CLASSES */
//      /**
//       * \return column size of Jacobian
//       */
//      virtual int getuSize() const {return uSize;}
//
//      /**
//       * \param generalized position
//       * \param time
//       * \return Jacobian matrix as a function of generalized position and time,
//       * J=J(q,t)
//       */
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) { return (*J)(q); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the Jacobian function
//       */
//      Function1<fmatvec::Mat3xV,fmatvec::VecV>* getJacobianFunction() { return J; }
//      void setJacobianFunction(Function1<fmatvec::Mat3xV,fmatvec::VecV> *J_) { J = J_; }
//      /***************************************************/
//
//    private:
//      int uSize;
//      Function1<fmatvec::Mat3xV,fmatvec::VecV> *J;
//  };
//
//  class GeneralJacobian : public Jacobian {
//    public:
//      /**
//       * \brief constructor
//       */
//      GeneralJacobian(int uSize_, Function2<fmatvec::Mat3xV,fmatvec::VecV,double> *J_) : uSize(uSize_), J(J_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~GeneralJacobian() { delete J; J = 0; }
//
//      /* INTERFACE FOR DERIVED CLASSES */
//      /**
//       * \return column size of Jacobian
//       */
//      virtual int getuSize() const {return uSize;}
//
//      /**
//       * \param generalized position
//       * \param time
//       * \return Jacobian matrix as a function of generalized position and time,
//       * J=J(q,t)
//       */
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL) { return (*J)(q,t); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the Jacobian function
//       */
//      Function2<fmatvec::Mat3xV,fmatvec::VecV,double>* getJacobianFunction() { return J; }
//      void setJacobianFunction(Function2<fmatvec::Mat3xV,fmatvec::VecV,double> *J_) { J = J_; }
//      /***************************************************/
//
//    private:
//      int uSize;
//      Function2<fmatvec::Mat3xV,fmatvec::VecV,double> *J;
//  };
//
//  /**
//   * \brief Jacobian for rotation about axes x and y
//   * \author Martin Foerg
//   * \date 2010-05-23 update according to change in Jacobian (Martin Foerg)
//   */
//  class JRotationAboutAxesXY : public Jacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JRotationAboutAxesXY(int uSize_) : uSize(uSize_), J(uSize) {}
//
//      /* INTERFACE OF JACOBIAN */
//      int getuSize() const { return uSize; }
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV J;
//  };
//
//  /**
//   * \brief Jacobian for rotation about axes x and z
//   * \author Martin Foerg
//   */
//  class JRotationAboutAxesXZ : public Jacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JRotationAboutAxesXZ(int uSize_) : uSize(uSize_), J(uSize) {}
//
//      /* INTERFACE OF JACOBIAN */
//      int getuSize() const { return uSize; }
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV J;
//  };
//
//  /**
//   * \brief Jacobian for rotation about axes y and z
//   * \author Martin Foerg
//   * \date 2010-05-23 update according to change in Jacobian (Martin Foerg)
//   */
//  class JRotationAboutAxesYZ : public Jacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JRotationAboutAxesYZ(int uSize_) : uSize(uSize_), J(uSize) {}
//
//      /* INTERFACE OF JACOBIAN */
//      int getuSize() const { return uSize; }
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV J;
//  };
//
//  /**
//   * \brief Jacobian for rotation about axes x and y
//   * \author Martin Foerg
//   * \date 2010-05-23 update according to change in Jacobian (Martin Foerg)
//   */
//  class JRotationAboutAxesXYZ : public Jacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JRotationAboutAxesXYZ(int uSize_) : uSize(uSize_), J(uSize) {}
//
//      /* INTERFACE OF JACOBIAN */
//      int getuSize() const { return uSize; }
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV J;
//  };
//
//  /**
//   * \brief standard parametrisation with angular velocity in reference system yields time-dependent mass matrix
//   * \author Martin Foerg
//   * \date 2009-04-08 some comments (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Jacobian (Martin Foerg)
//   */
//  class TCardanAngles : public TMatrix {
//    public:
//      /**
//       * \brief constructor
//       * \param size of positions
//       * \param size of velocities
//       */
//      TCardanAngles(int qSize_, int uSize_) : qSize(qSize_), uSize(uSize_), T(3,3,fmatvec::EYE) {}
//
//      /* INTERFACE OF JACOBIAN */
//      int getqSize() const { return qSize; }
//      int getuSize() const { return uSize; }
//      virtual fmatvec::MatV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int qSize, uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::MatV T;
//  };
//
//  /**
//   * \brief standard parametrisation with angular velocity in reference system yields time-dependent mass matrix
//   * \author Martin Foerg
//   * \date 2010-10-20 first commit (Martin Foerg)
//   */
//  class TEulerAngles : public TMatrix {
//    public:
//      /**
//       * \brief constructor
//       * \param size of positions
//       * \param size of velocities
//       */
//      TEulerAngles(int qSize_, int uSize_) : qSize(qSize_), uSize(uSize_), T(3,3,fmatvec::EYE) {
//        int iq = qSize-1;
//        int iu = uSize-1;
//        T(iq-2,iu) = 1;
//        T(iq,iu) = 0;
//      }
//
//      /* INTERFACE OF JACOBIAN */
//      int getqSize() const { return qSize; }
//      int getuSize() const { return uSize; }
//      virtual fmatvec::MatV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int qSize, uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::MatV T;
//  };
//
//  /**
//   * \brief alternative parametrisation with angular velocity in body frame yields constant mass matrix for absolute coordinates
//   * \author Martin Foerg
//   * \date 2009-04-08 some comments (Thorsten Schindler)
//   * \date 2010-05-23 update according to change in Jacobian (Martin Foerg)
//   */
//  class TCardanAngles2 : public TMatrix {
//    public:
//      /**
//       * \brief constructor
//       * \param size of positions
//       * \param size of velocities
//       */
//      TCardanAngles2(int qSize_, int uSize_) : qSize(qSize_), uSize(uSize_), T(3,3,fmatvec::EYE) {}
//
//      /* INTERFACE OF JACOBIAN */
//      int getqSize() const { return qSize; }
//      int getuSize() const { return uSize; }
//      virtual fmatvec::MatV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int qSize, uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::MatV T;
//  };
//
//  /**
//   * \brief alternative parametrisation with angular velocity in body frame yields constant mass matrix for absolute coordinates
//   * \author Martin Foerg
//   * \date 2010-10-20 first commit (Martin Foerg)
//   */
//  class TEulerAngles2 : public TMatrix {
//    public:
//      /**
//       * \brief constructor
//       * \param size of positions
//       * \param size of velocities
//       */
//      TEulerAngles2(int qSize_, int uSize_) : qSize(qSize_), uSize(uSize_), T(3,3,fmatvec::EYE) {
//      }
//
//      /* INTERFACE OF JACOBIAN */
//      int getqSize() const { return qSize; }
//      int getuSize() const { return uSize; }
//      virtual fmatvec::MatV operator()(const fmatvec::VecV &q, const double &t, const void * =NULL);
//      /***************************************************/
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int qSize, uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::MatV T;
//  };
//
//  /**
//   * \brief base class to describe derivatives of Jacobians
//   * \author Martin Foerg
//   */
//  class DerivativeOfJacobian : public Function3<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV,double> {
//    public:
//      /**
//       * \brief constructor
//       */
//      DerivativeOfJacobian() {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~DerivativeOfJacobian() {}
//
//      /**
//       * \param derivative of generalized position
//       * \param generalized position
//       * \param time
//       * \return derivative of Jacobian matrix as a function of derivative of generalized position, generalized position and time,
//       * Jd=Jd(qd,q,t)
//       */
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &qd, const fmatvec::VecV& q, const double& t, const void* =NULL) = 0;
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      /***************************************************/
//  };
//
//  class StateDependentDerivativeOfJacobian : public DerivativeOfJacobian {
//    public:
//      /**
//       * \brief constructor
//       */
//      StateDependentDerivativeOfJacobian(Function2<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV> *Jd_) : Jd(Jd_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~StateDependentDerivativeOfJacobian() { delete Jd; Jd = 0; }
//
//      /**
//       * \param derivative of generalized position
//       * \param generalized position
//       * \param time
//       * \return derivative of Jacobian matrix as a function of derivative of generalized position, generalized position and time,
//       * Jd=Jd(qd,q,t)
//       */
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &qd, const fmatvec::VecV& q, const double& t, const void* =NULL) { return (*Jd)(qd,q); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the derivative of Jacobian function
//       */
//      Function2<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV>* getDerivativeOfJacobianFunction() { return Jd; }
//      void setDerivativeOfJacobianFunction(Function2<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV> *Jd_) { Jd = Jd_; }
//      /***************************************************/
//
//    private:
//      Function2<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV> *Jd;
//  };
//
//  class GeneralDerivativeOfJacobian : public DerivativeOfJacobian {
//    public:
//      /**
//       * \brief constructor
//       */
//      GeneralDerivativeOfJacobian(Function3<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV,double> *Jd_) : Jd(Jd_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~GeneralDerivativeOfJacobian() { delete Jd; Jd = 0; }
//
//      /**
//       * \param derivative of generalized position
//       * \param generalized position
//       * \param time
//       * \return derivative of Jacobian matrix as a function of derivative of generalized position, generalized position and time,
//       * Jd=Jd(qd,q,t)
//       */
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &qd, const fmatvec::VecV& q, const double& t, const void* =NULL) { return (*Jd)(qd,q,t); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the Jacobian function
//       */
//      Function3<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV,double>* getDerivativeOfJacobianFunction() { return Jd; }
//      void setDerivativeOfJacobianFunction(Function3<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV,double> *Jd_) { Jd = Jd_; }
//      /***************************************************/
//
//    private:
//      Function3<fmatvec::Mat3xV,fmatvec::VecV,fmatvec::VecV,double> *Jd;
//  };
//
//  /**
//   * \brief derivative of Jacobian for rotation about axes x and y
//   * \author Martin Foerg
//   */
//  class JdRotationAboutAxesXY : public DerivativeOfJacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JdRotationAboutAxesXY(int uSize_) : uSize(uSize_), Jd(uSize) {}
//
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &qd, const fmatvec::VecV& q, const double& t, const void* =NULL);
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV Jd;
//  };
//
//  /**
//   * \brief derivative of Jacobian for rotation about axes x and z
//   * \author Martin Foerg
//   */
//  class JdRotationAboutAxesXZ : public DerivativeOfJacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JdRotationAboutAxesXZ(int uSize_) : uSize(uSize_), Jd(uSize) {}
//
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &qd, const fmatvec::VecV& q, const double& t, const void* =NULL);
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV Jd;
//  };
//
//  /**
//   * \brief derivative of Jacobian for rotation about axes y and z
//   * \author Martin Foerg
//   */
//  class JdRotationAboutAxesYZ : public DerivativeOfJacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JdRotationAboutAxesYZ(int uSize_) : uSize(uSize_), Jd(uSize) {}
//
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &qd, const fmatvec::VecV& q, const double& t, const void* =NULL);
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV Jd;
//  };
//
//  /**
//   * \brief derivative of Jacobian for rotation about axes x and y
//   * \author Martin Foerg
//   */
//  class JdRotationAboutAxesXYZ : public DerivativeOfJacobian {
//    public:
//      /**
//       * \brief constructor
//       * \param size of generalized velocity vector
//       */
//      JdRotationAboutAxesXYZ(int uSize_) : uSize(uSize_), Jd(uSize) {}
//
//      virtual fmatvec::Mat3xV operator()(const fmatvec::VecV &qd, const fmatvec::VecV& q, const double& t, const void* =NULL);
//
//    private:
//      /**
//       * \brief size of positions and velocities
//       */
//      int uSize;
//
//      /**
//       * \brief linear relation between differentiated positions and velocities
//       */
//      fmatvec::Mat3xV Jd;
//  };
//
//  /**
//   * \brief base class to describe guiding velocities
//   * \author Martin Foerg
//   */
//  class GuidingVelocity : public Function2<fmatvec::Vec3,fmatvec::VecV,double> {
//    public:
//      /**
//       * \brief constructor
//       */
//      GuidingVelocity() {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~GuidingVelocity() {}
//
//      /**
//       * \param derivative of generalized position
//       * \param generalized position
//       * \param time
//       * \return guiding velocity as a function of generalized position and time,
//       * Jd=Jd(qd,q,t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV& q, const double& t, const void* =NULL) = 0;
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      /***************************************************/
//  };
//
//  class TimeDependentGuidingVelocity : public GuidingVelocity {
//    public:
//      /**
//       * \brief constructor
//       */
//      TimeDependentGuidingVelocity() : j(NULL) {}
//
//      /**
//       * \brief constructor
//       */
//      TimeDependentGuidingVelocity(Function1<fmatvec::Vec3,double> *j_) : j(j_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~TimeDependentGuidingVelocity() { delete j; j = 0; }
//
//      /**
//       * \param generalized position
//       * \param time
//       * \return guiding velocity as a function of time,
//       * j=j(t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV& q, const double& t, const void* =NULL) { return (*j)(t); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the guiding velocity function
//       */
//      Function1<fmatvec::Vec3,double>* getGuidingVelocityFunction() { return j; }
//      void setGuidingVelocityFunction(Function1<fmatvec::Vec3,double> *j_) { j = j_; }
//      /***************************************************/
//
//    private:
//      Function1<fmatvec::Vec3,double> *j;
//  };
//
//  class GeneralGuidingVelocity : public GuidingVelocity {
//    public:
//      /**
//       * \brief constructor
//       */
//      GeneralGuidingVelocity(Function2<fmatvec::Vec3,fmatvec::VecV,double> *j_) : j(j_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~GeneralGuidingVelocity() { delete j; j = 0; }
//
//      /**
//       * \param derivative of generalized position
//       * \param generalized position
//       * \param time
//       * \return guiding velocity as a function of generalized position and time,
//       * j=j(qd,q,t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV& q, const double& t, const void* =NULL) { return (*j)(q,t); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the guiding velocity function
//       */
//      Function2<fmatvec::Vec3,fmatvec::VecV,double>* getGuidingVelocityFunction() { return j; }
//      void setGuidingVelocityFunction(Function2<fmatvec::Vec3,fmatvec::VecV,double> *j_) { j = j_; }
//      /***************************************************/
//
//    private:
//      Function2<fmatvec::Vec3,fmatvec::VecV,double> *j;
//  };
//
//  /**
//   * \brief base class to describe derivatives of guiding velocities
//   * \author Martin Foerg
//   */
//  class DerivativeOfGuidingVelocity : public Function3<fmatvec::Vec3,fmatvec::VecV,fmatvec::VecV,double> {
//    public:
//      /**
//       * \brief constructor
//       */
//      DerivativeOfGuidingVelocity() {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~DerivativeOfGuidingVelocity() {}
//
//      /**
//       * \param derivative of generalized position
//       * \param generalized position
//       * \param time
//       * \return derivative of guiding velocity as a function of derivative of generalized position, generalized position and time,
//       * Jd=Jd(qd,q,t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV& qd, const fmatvec::VecV& q, const double& t, const void* =NULL) = 0;
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//      /***************************************************/
//  };
//
//  class TimeDependentDerivativeOfGuidingVelocity : public DerivativeOfGuidingVelocity {
//    public:
//      /**
//       * \brief constructor
//       */
//      TimeDependentDerivativeOfGuidingVelocity() : jd(NULL) {}
//
//      /**
//       * \brief constructor
//       */
//      TimeDependentDerivativeOfGuidingVelocity(Function1<fmatvec::Vec3,double> *jd_) : jd(jd_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~TimeDependentDerivativeOfGuidingVelocity() { delete jd; jd = 0; }
//
//      /**
//       * \param generalized position
//       * \param time
//       * \return derivative of guiding velocity as a function of time,
//       * jd=jd(t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV& qd, const fmatvec::VecV& q, const double& t, const void* =NULL) { return (*jd)(t); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the derivative of derivative of guiding velocity function
//       */
//      Function1<fmatvec::Vec3,double>* getDerivativeOfGuidingVelocityFunction() { return jd; }
//      void setDerivativeOfGuidingVelocityFunction(Function1<fmatvec::Vec3,double> *jd_) { jd = jd_; }
//      /***************************************************/
//
//    private:
//      Function1<fmatvec::Vec3,double> *jd;
//  };
//
//  class GeneralDerivativeOfGuidingVelocity : public DerivativeOfGuidingVelocity {
//    public:
//      /**
//       * \brief constructor
//       */
//      GeneralDerivativeOfGuidingVelocity(Function3<fmatvec::Vec3,fmatvec::VecV,fmatvec::VecV,double> *jd_) : jd(jd_) {}
//
//      /**
//       * \brief destructor
//       */
//      virtual ~GeneralDerivativeOfGuidingVelocity() { delete jd; jd = 0; }
//
//      /**
//       * \param derivative of generalized position
//       * \param generalized position
//       * \param time
//       * \return derivative of guiding velocity as a function of derivative of generalized position, generalized position and time,
//       * jd=jd(qd,q,t)
//       */
//      virtual fmatvec::Vec3 operator()(const fmatvec::VecV& qd, const fmatvec::VecV& q, const double& t, const void* =NULL) { return (*jd)(qd,q,t); }
//
//      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element) {};
//
//      /* GETTER / SETTER */
//      /**
//       * \brief set the derivative of guiding velocity function
//       */
//      Function3<fmatvec::Vec3,fmatvec::VecV,fmatvec::VecV,double>* getDerivativeOfGuidingVelocityFunction() { return jd; }
//      void setDerivativeOfGuidingVelocityFunction(Function3<fmatvec::Vec3,fmatvec::VecV,fmatvec::VecV,double> *jd_) { jd = jd_; }
//      /***************************************************/
//
//    private:
//      Function3<fmatvec::Vec3,fmatvec::VecV,fmatvec::VecV,double> *jd;
//  };

}

#endif /* _KINEMATICS_H_ */

