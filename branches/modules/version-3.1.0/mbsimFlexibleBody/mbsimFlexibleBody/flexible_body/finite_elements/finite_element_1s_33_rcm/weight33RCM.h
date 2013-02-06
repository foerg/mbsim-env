/* Copyright (C) 2004-2011 MBSim Development Team
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
 * Contact: thorsten.schindler@mytum.de
 */

#ifndef WEIGHT33RCM_H_
#define WEIGHT33RCM_H_

#include "fmatvec.h"
#include <mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_33_rcm/trafo33RCM.h>
#include "mbsim/mbsim_event.h"

namespace MBSimFlexibleBody {

  /**
   * \brief integrals of bending parametrisation for FiniteElement1s33RCM
   * \author Thorsten Schindler
   * \date 2009-04-24 initial commit (Thorsten Schindler) 
   *
   * \todo: use fmatvec2.0
   */
  class Weight33RCM {
    public:
      /**
       * \brief constructor
       */
      Weight33RCM(double l0_,double l0h2_,double l0h3_,Trafo33RCMPtr tf_);
      /**
       * \brief Destructor */
      virtual ~Weight33RCM();

      /* GETTER / SETTER */
      /**
       * \param first curvature
       * \param second curvature
       */
      void setCurvature(double k10_,double k20_);
      
      /**
       * \param number of Gauss points
       */
      void setGauss(int nGauss);

      const fmatvec::RowVec4& getvxvt() const;
      const fmatvec::Vec4& getvxvtH() const;
      const fmatvec::RowVec4& getxvxvt() const;
      const fmatvec::Vec4& getxvxvtH() const;
      const fmatvec::SymMat4& getvxvtwxwt() const;
      const fmatvec::RowVec4& getvvt() const;
      const fmatvec::Vec4& getvvtH() const;
      const fmatvec::RowVec4& getxvvt() const;
      const fmatvec::Vec4& getxvvtH() const;
      const fmatvec::SymMat4& getvvtwwt() const;
      
      const double& getIwh1() const;
      const double& getIwh2() const;
      const double& getIwh1t() const;	
      const double& getIwh2t() const;
      const double& getIxwh1() const;
      const double& getIxwh2() const;
      const double& getIxwh1t() const;
      const double& getIxwh2t() const;		
      const double& getIwh1twh1() const;
      const double& getIwh1twh2() const;
      const double& getIwh1twh1t() const;
      const double& getIwh1wh1() const;
      const double& getIwh1wh2t() const;		
      const double& getIwh1wh2() const;
      const double& getIwh2twh2t() const;
      const double& getIwh2twh2() const;
      const double& getIwh2wh2() const;
      const double& getIwh1twh2t() const;		
      const double& getIwh1xwh1x() const;
      const double& getIwh2xwh2x() const;
      const double& getIwh1xxwh1xx() const;
      const double& getIwh2xxwh2xx() const;
      const fmatvec::RowVec4& getIwh1xxwxxwt() const;
      const fmatvec::RowVec4& getIwh2xxwxxwt() const;
      const fmatvec::RowVec4& getIwh1wwt() const;
      const fmatvec::RowVec4& getIwh2wwt() const;
      const fmatvec::Vec4& getIwh1wwtH() const;
      const fmatvec::Vec4& getIwh2wwtH() const;
      const fmatvec::RowVec4& getIwh1twwt() const;
      const fmatvec::RowVec4& getIwh2twwt() const;
      const fmatvec::Vec4& getIwh1twwtH() const;
      const fmatvec::Vec4& getIwh2twwtH() const;
      const fmatvec::RowVec4& getIwh1xwxwt() const;
      const fmatvec::RowVec4& getIwh2xwxwt() const;

      const fmatvec::Vec4& getw1coef() const;
      const fmatvec::Vec4& getw2coef() const;
      const fmatvec::Vec4& getw1tcoef() const;
      const fmatvec::Vec4& getw2tcoef() const;
      const fmatvec::Mat4x16& getw1coefqI() const;
      const fmatvec::Mat4x16& getw2coefqI() const;
      const fmatvec::Vec4& getwh1coef() const;
      const fmatvec::Vec4& getwh2coef() const;
      const fmatvec::Vec4& getwh1tcoef() const;
      const fmatvec::Vec4& getwh2tcoef() const;
      const fmatvec::Mat4x16& getwh1coefqI() const;
      const fmatvec::Mat4x16& getwh2coefqI() const;
      fmatvec::Mat16x4 getwh1coefqIH() const;
      fmatvec::Mat16x4 getwh2coefqIH() const;
      const fmatvec::Mat4x16& getwh1tcoefqI() const;
      const fmatvec::Mat4x16& getwh2tcoefqI() const;
      const fmatvec::Mat16x4& getwh1coefqInunutH() const;
      const fmatvec::Mat16x4& getwh2coefqInunutH() const;

      const fmatvec::Mat3x16& gettSqI() const;
      const fmatvec::Mat3x16& getnSqI() const;
      const fmatvec::Mat3x16& getbSqI() const;
      const fmatvec::Mat16x3& getnSqIH() const;
      const fmatvec::Mat16x3& getbSqIH() const;
      const fmatvec::Mat3x16& gettStqI() const;
      const fmatvec::Mat3x16& getnStqI() const;
      const fmatvec::Mat3x16& getbStqI() const;

      const double& getTtil() const;
      const fmatvec::RowVec16& getTtilqI() const;
      const fmatvec::SymMat16& getTtilqItqIt() const;
      const fmatvec::Vec16& getTtilqItqIqIt() const;

      const fmatvec::Mat3x16& getdpS() const;
      const fmatvec::Mat16x3& getdpSH() const;
      /***************************************************/

      /* BASIC INTEGRALS */
      double intv(const fmatvec::Vec4& vt) const;
      double intvx(const fmatvec::Vec4& vt) const;
      double intxv(const fmatvec::Vec4& vt) const;
      double intxvx(const fmatvec::Vec4& vt) const;
      double intvw(const fmatvec::Vec4& vt,const fmatvec::Vec4& wt) const;
      double intvxwx(const fmatvec::Vec4& vt,const fmatvec::Vec4& wt) const;
      double intvxxvxx(const fmatvec::Vec4& vt,double C) const;
      void intvvt();		
      void intvvtH();
      void intvxvt();		
      void intvxvtH();		
      void intxvvt();		
      void intxvvtH();
      void intxvxvt();		
      void intxvxvtH();		
      fmatvec::RowVec4 intvwwt(const fmatvec::Vec4& vt) const;
      fmatvec::RowVec4 intvxwxwt(const fmatvec::Vec4& vt) const;
      fmatvec::RowVec4 intvxxwxxwt(const fmatvec::Vec4& vt,double C) const;
      void intvvtwwt();
      void intvxvtwxwt();
      /***************************************************/

      /**
       * \brief computes the integrals of bending polynomials 
       * \param global coordinates
       * \param global velocities
       */
      void computeint(const fmatvec::Vec16& qG,const fmatvec::Vec16& qGt);
      
      /**
       * \brief computes the vector integrals of bending polynomials 
       * \param global coordinates
       * \param global velocities
       */
      void computeintD(const fmatvec::Vec16& qG,const fmatvec::Vec16& qGt);
      
      /**
       * \brief computes the coefficients of bending polynomials w 
       * \param global coordinates
       */
      void computewcoefPos(const fmatvec::Vec16& qG);
      
      /**
       * \brief computes the time differentiated coefficients of bending polynomials w 
       * \param global coordinates
       * \param global velocities
       */
      void computewcoefVel(const fmatvec::Vec16& qG,const fmatvec::Vec16& qGt);
      
      /**
       * \brief computes the coefficients of bending polynomials w and wh
       * \param global coordinates
       */
      void computewhcoefPos(const fmatvec::Vec16& qG);
      
      /**
       * \brief computes the coefficients of bending polynomials w and wh and their time derivatives 
       * \param global coordinates
       * \param global velocities
       */
      void computewhcoefVel(const fmatvec::Vec16& qG,const fmatvec::Vec16& qGt);
      
      /**
       * \brief computes the derivative of w-coefficients with respect to bending coordinates
       */
      void computewcoefPosD();
      
      /**
       * \brief computes bending polynomial values on position level
       * \param global coordinates
       */
      void computewhcoefPosD(const fmatvec::Vec16& qG);
      
      /**
       * \brief computes the coefficients of w and wt
       * \param left translational deflection
       * \param right translational deflection
       * \param left rotational deflection
       * \param right rotational deflection
       */
      fmatvec::Vec4 computewcoef(double dL,double dR,double bL,double bR) const;
      
      /**
       * \brief evaluates the bending polynomial and its x-derivative
       * \param point of evaluation 
       */
      fmatvec::Vec2 computew(const fmatvec::Vec4& wt,double x) const;

    private:
      /** 
       * \brief Trafo-Object
       */
      Trafo33RCMPtr tf;

      /** 
       * \brief length of FEM1s33RCM
       */
      double l0, l0h2, l0h3, l0h4, l0h5, l0h7, l0h9, l0h11;

      /** 
       * \brief predefined bendings
       */
      double k10, k20;

      /** 
       * \brief general integrals
       */
      fmatvec::RowVec4 Ivvt, Ivxvt, Ixvvt, Ixvxvt;
      fmatvec::SymMat4 Ivvtwwt, Ivxvtwxwt;
      fmatvec::Vec4 IvvtH, IvxvtH, IxvvtH, IxvxvtH;

      /**
       * \brief special integrals
       */
      double Iwh1, Iwh2, Iwh1t, Iwh2t, Ixwh1, Ixwh2, Ixwh1t, Ixwh2t;
      double Iwh1twh1, Iwh1twh2, Iwh1twh1t, Iwh1wh1, Iwh1wh2t, Iwh1wh2, Iwh2twh2t;	
      double Iwh2twh2, Iwh2wh2, Iwh1twh2t;
      fmatvec::RowVec4 Iwh1wwt, Iwh1twwt, Iwh2wwt, Iwh2twwt;
      fmatvec::Vec4 Iwh1wwtH, Iwh1twwtH, Iwh2wwtH, Iwh2twwtH;

      double Iwh1xwh1x, Iwh2xwh2x, Iwh1xxwh1xx, Iwh2xxwh2xx;
      fmatvec::RowVec4 Iwh1xwxwt, Iwh2xwxwt;
      fmatvec::RowVec4 Iwh1xxwxxwt, Iwh2xxwxxwt;

      /** 
       * \brief bending coefficients
       */
      fmatvec::Vec4 w1coef, w2coef, w1tcoef, w2tcoef;
      fmatvec::Vec4 wh1coef, wh2coef, wh1tcoef, wh2tcoef;
      fmatvec::Mat4x16 w1coefqI, w2coefqI;
      fmatvec::Mat4x16 wh1coefqI, wh2coefqI, wh1tcoefqI, wh2tcoefqI;
      fmatvec::Mat16x4 wh1coefqInunutH, wh2coefqInunutH;

      /**
       * \brief COSY 
       */
      fmatvec::Mat3x16 tSqI, nSqI, bSqI;
      fmatvec::Mat3x16 tStqI, nStqI, bStqI;
      fmatvec::Mat16x3 nSqIH, bSqIH;
      fmatvec::Mat3x16 ntilSqI, btilSqI;
      fmatvec::RowVec16 xintilqI, xibtilqI, etantilqI, etabtilqI;

      /** 
       * \brief omgtS
       */
      double omgt;
      fmatvec::RowVec16 omgtqI, omgtqIt;
      fmatvec::Vec16 omgtqItqIqIt;

      /**
       * \brief rotational kinetic energy 
       */
      double Ttil;
      fmatvec::RowVec16 TtilqI;
      fmatvec::SymMat16 TtilqItqIt;
      fmatvec::Vec16 TtilqItqIqIt;

      /** 
       * \brief Gauss integration
       */
      fmatvec::Vec gp, xip; // cannot be initialised in constructor
      double bam;

      /** 
       * \brief delta matrix for pS
       */
      fmatvec::Mat3x16 dpS;
      fmatvec::Mat16x3 dpSH;

      /**
       * \brief computes the integrals of bending polynomials 
       */
      void computeint();
      
      /**
       * \brief computes the coefficients of bending polynomials w 
       */
      void computewcoefPos();
      
      /**
       * \brief computes the time differentiated coefficients of bending polynomials w 
       */
      void computewcoefVel();
      
      /**
       * \brief computes the coefficients of bending polynomials w and wh
       */
      void computewhcoefPos();
      
      /**
       * \brief computes the coefficients of bending polynomials w and wh and their time derivatives
       */
      void computewhcoefVel();
      
      /**
       * \brief computes bending polynomial values on position level
       */
      void computewhcoefPosD();
      
      /**
       * \brief computes bending polynomial values on velocity level
       */
      void computewhcoefVelD();

      /**
       * \brief compute angular velocity around tangent */
      void computeomgt(double x);
      
      /**
       * \brief compute rotational kinetic energy
       */
      void computeT();
      
      /**
       * \brief compute delta matrix for CP with respect to rotation 
       */
      void computedpS();			
  };

  inline void Weight33RCM::setCurvature(double k10_,double k20_) { k10 = k10_; k20 = k20_; }
  inline void Weight33RCM::intvvtH() { IvvtH = Ivvt.T(); }
  inline void Weight33RCM::intvxvtH() { IvxvtH = Ivxvt.T(); }
  inline void Weight33RCM::intxvvtH() { IxvvtH = Ixvvt.T(); }
  inline void Weight33RCM::intxvxvtH() { IxvxvtH = Ixvxvt.T(); }

  inline const fmatvec::RowVec4& Weight33RCM::getvxvt() const { return Ivxvt; }
  inline const fmatvec::Vec4& Weight33RCM::getvxvtH() const { return IvxvtH; }
  inline const fmatvec::RowVec4& Weight33RCM::getxvxvt() const { return Ixvxvt; }
  inline const fmatvec::Vec4& Weight33RCM::getxvxvtH() const { return IxvxvtH; }
  inline const fmatvec::SymMat4& Weight33RCM::getvxvtwxwt() const { return Ivxvtwxwt; }
  inline const fmatvec::RowVec4& Weight33RCM::getvvt() const { return Ivvt; }
  inline const fmatvec::Vec4& Weight33RCM::getvvtH() const { return IvvtH; }
  inline const fmatvec::RowVec4& Weight33RCM::getxvvt() const { return Ixvvt; }
  inline const fmatvec::Vec4& Weight33RCM::getxvvtH() const { return IxvvtH; }
  inline const fmatvec::SymMat4& Weight33RCM::getvvtwwt() const { return Ivvtwwt; }

  inline const double& Weight33RCM::getIwh1() const { return Iwh1; }
  inline const double& Weight33RCM::getIwh2() const { return Iwh2; }
  inline const double& Weight33RCM::getIwh1t() const { return Iwh1t; }
  inline const double& Weight33RCM::getIwh2t() const { return Iwh2t; }
  inline const double& Weight33RCM::getIxwh1() const { return Ixwh1; }
  inline const double& Weight33RCM::getIxwh2() const { return Ixwh2; }
  inline const double& Weight33RCM::getIxwh1t() const { return Ixwh1t; }
  inline const double& Weight33RCM::getIxwh2t() const { return Ixwh2t; }
  inline const double& Weight33RCM::getIwh1twh1() const { return Iwh1twh1; }
  inline const double& Weight33RCM::getIwh1twh2() const { return Iwh1twh2; }
  inline const double& Weight33RCM::getIwh1twh1t() const { return Iwh1twh1t; }
  inline const double& Weight33RCM::getIwh1wh1() const { return Iwh1wh1; }
  inline const double& Weight33RCM::getIwh1wh2t() const { return Iwh1wh2t; }
  inline const double& Weight33RCM::getIwh1wh2() const { return Iwh1wh2; }
  inline const double& Weight33RCM::getIwh2twh2t() const { return Iwh2twh2t; }
  inline const double& Weight33RCM::getIwh2twh2() const { return Iwh2twh2; }
  inline const double& Weight33RCM::getIwh2wh2() const { return Iwh2wh2; }
  inline const double& Weight33RCM::getIwh1twh2t() const { return Iwh1twh2t; }
  inline const double& Weight33RCM::getIwh1xwh1x() const { return Iwh1xwh1x; }
  inline const double& Weight33RCM::getIwh2xwh2x() const { return Iwh2xwh2x; }
  inline const double& Weight33RCM::getIwh1xxwh1xx() const { return Iwh1xxwh1xx; }
  inline const double& Weight33RCM::getIwh2xxwh2xx() const { return Iwh2xxwh2xx; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh1xxwxxwt() const { return Iwh1xxwxxwt; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh2xxwxxwt() const { return Iwh2xxwxxwt; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh1wwt() const { return Iwh1wwt; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh2wwt() const { return Iwh2wwt; }
  inline const fmatvec::Vec4& Weight33RCM::getIwh1wwtH() const { return Iwh1wwtH; }
  inline const fmatvec::Vec4& Weight33RCM::getIwh2wwtH() const { return Iwh2wwtH; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh1twwt() const { return Iwh1twwt; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh2twwt() const { return Iwh2twwt; }
  inline const fmatvec::Vec4& Weight33RCM::getIwh1twwtH() const { return Iwh1twwtH; }
  inline const fmatvec::Vec4& Weight33RCM::getIwh2twwtH() const { return Iwh2twwtH; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh1xwxwt() const { return Iwh1xwxwt; }
  inline const fmatvec::RowVec4& Weight33RCM::getIwh2xwxwt() const { return Iwh2xwxwt; }

  inline const fmatvec::Vec4& Weight33RCM::getw1coef() const { return w1coef; }
  inline const fmatvec::Vec4& Weight33RCM::getw2coef() const { return w2coef; }
  inline const fmatvec::Vec4& Weight33RCM::getw1tcoef() const { return w1tcoef; }
  inline const fmatvec::Vec4& Weight33RCM::getw2tcoef() const { return w2tcoef; }
  inline const fmatvec::Mat4x16& Weight33RCM::getw1coefqI() const { return w1coefqI; }
  inline const fmatvec::Mat4x16& Weight33RCM::getw2coefqI() const { return w2coefqI; }
  inline const fmatvec::Vec4& Weight33RCM::getwh1coef() const { return wh1coef; }
  inline const fmatvec::Vec4& Weight33RCM::getwh2coef() const { return wh2coef; }
  inline const fmatvec::Vec4& Weight33RCM::getwh1tcoef() const { return wh1tcoef; }
  inline const fmatvec::Vec4& Weight33RCM::getwh2tcoef() const { return wh2tcoef; }
  inline const fmatvec::Mat4x16& Weight33RCM::getwh1coefqI() const { return wh1coefqI; }
  inline const fmatvec::Mat4x16& Weight33RCM::getwh2coefqI() const { return wh2coefqI; }
  inline fmatvec::Mat16x4 Weight33RCM::getwh1coefqIH() const { return wh1coefqI.T(); }
  inline fmatvec::Mat16x4 Weight33RCM::getwh2coefqIH() const { return wh2coefqI.T(); }
  inline const fmatvec::Mat4x16& Weight33RCM::getwh1tcoefqI() const { return wh1tcoefqI; }
  inline const fmatvec::Mat4x16& Weight33RCM::getwh2tcoefqI() const { return wh2tcoefqI; }
  inline const fmatvec::Mat16x4& Weight33RCM::getwh1coefqInunutH() const { return wh1coefqInunutH; }
  inline const fmatvec::Mat16x4& Weight33RCM::getwh2coefqInunutH() const { return wh2coefqInunutH; }

  inline const fmatvec::Mat3x16& Weight33RCM::gettSqI() const { return tSqI; }
  inline const fmatvec::Mat3x16& Weight33RCM::getnSqI() const { return nSqI; }
  inline const fmatvec::Mat3x16& Weight33RCM::getbSqI() const { return bSqI; }
  inline const fmatvec::Mat16x3& Weight33RCM::getnSqIH() const { return nSqIH; }
  inline const fmatvec::Mat16x3& Weight33RCM::getbSqIH() const { return bSqIH; }
  inline const fmatvec::Mat3x16& Weight33RCM::gettStqI() const { return tStqI; }
  inline const fmatvec::Mat3x16& Weight33RCM::getnStqI() const { return nStqI; }
  inline const fmatvec::Mat3x16& Weight33RCM::getbStqI() const { return bStqI; }

  inline const double& Weight33RCM::getTtil() const { return Ttil; }
  inline const fmatvec::RowVec16& Weight33RCM::getTtilqI() const { return TtilqI; }
  inline const fmatvec::SymMat16& Weight33RCM::getTtilqItqIt() const { return TtilqItqIt; }
  inline const fmatvec::Vec16& Weight33RCM::getTtilqItqIqIt() const { return TtilqItqIqIt; }

  inline const fmatvec::Mat3x16& Weight33RCM::getdpS() const { return dpS; }
  inline const fmatvec::Mat16x3& Weight33RCM::getdpSH() const { return dpSH; }
  /*******************************************************************/



}

#endif /*WEIGHT33RCM_H_*/

