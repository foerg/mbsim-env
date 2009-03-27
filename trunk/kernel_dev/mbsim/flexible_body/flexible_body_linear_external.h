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
 * Contact:
 *   rzander@users.berlios.de
 *
 */

#ifndef _BODY_FLEXIBLE_LINEAR_EXTERNAL_H_
#define _BODY_FLEXIBLE_LINEAR_EXTERNAL_H_

#include "mbsim/flexible_body.h"
#include "mbsim/mbsim_event.h"
#include <fstream>

namespace MBSim {

  class ContourInterpolation;


  /*! 
   * \brief Linear models from external preprocessing, e.g. Finite %Element model.
   * \author Roland Zander
   * \date 2009-03-26 initial kernel_dev commit (Roland Zander)
   *
   * Systems equation of motion:
   * \f[ \vM{\rm d}\vu = -( \vK \vq + \vD \vu){\rm d}t - \vW{\rm d}\vLambda \f]
   * with constant matrices \f$\vM,\vK,\vD\f$. The model uses n degrees of freedom (dimension of \f$\vq,\vu\f$) and d translational directions (dimension of Jacobi-matrizes \f$n\times d\f$)
   * */
  class FlexibleBodyLinearExternal : public FlexibleBody<int> {

    protected:
      /** number of Contours directly asoziated to body */
      int nContours;
	  
	  /** vector of ContourPointData controlling type of interface: node/interpolation */
      vector<ContourPointData> contourType;

      /** origin \f$\rs{_W}[_K0]{\vr}\f$ of model */
      Vec WrON00;

      /*! update kinematical values\n
       * Call to updateFrames(double t)
       * and updateContours(double t)
       * \param t time
       */
      void updateKinematics(double t);
      /*!
       * Kinematical update of postition and velocities of every Contour:
       * \f[ \vr_{C} = \vr_{K} + \vJ_T(\vr_{P0} + \vW^T\vq) \f]
       * \param t time
       * \todo angular kinematics
       */
      void updateContours(double t);

      /*! create interface in form of ContourPointData based on file
       * \return cpData for refering to Port or Contour added
       * \param jacbifilename file containing interface data
       */
      ContourPointData addInterface(const string &jacbifilename);
      /*! create interface in form of ContourPointData based on Jacobian matrix and undeformed position
       * \return cpData for refering to Port or Contour added
       * \param J Jacobian matrix
       * \param r undeformed position in body coordinate system
       */
      ContourPointData addInterface(const Mat &J, const Vec &r);

      /* empty function since mass, damping and stiffness matrices are constant !!! */
      void updateJh_internal(double t);

    public:
	  /*!
	   * \brief constructor
	   * \param name of body
	   */
      FlexibleBodyLinearExternal(const string &name); 
      /*!
       * \brief destructor
       */
      virtual ~FlexibleBodyLinearExternal() {}

      /* inherited interface */
      /* FlexibleBody */
      virtual string getType() const { return "FlexibleBodyLinearExternal"; }
      virtual void BuildElements() { new MBSimError("BITTE INITIALISIEREN!"); } 
      virtual void GlobalMatrixContribution(int n);
      /*!
       * Kinematical update of postition and velocities of every attached Frame:
       * \f[ \vr_{P} = \vr_{K} + \vJ_T(\vr_{P0} + \vW^T\vq) \f]
       * \param t time
       * \todo angular kinematics
       */
      virtual void updateKinematicsForFrame(ContourPointData &S_, Frame *frame=0);
      virtual void updateJacobiansForFrame(ContourPointData &data, Frame *frame=0);

      /* Object */
      virtual void init() { new MBSimError("BITTE INITIALISIEREN!"); }

      /*! \return true
      */
      bool hasConstMass() const {return true;}

      /*! NULL-function, since mass matrix is constant*/
      void facLLM() {}

      /*! 
       * read mass matrix: style e.g. for 2*2 matrix \n
       * n x n   \n
       *[1.0 0.0 \n
       * 0.0 1.0]
       * \param massfilename name of file holding mass matrix
       */
      void readMassMatrix(const string &massfilename); 

      /*! 
       * set mass matrix: 
       * \param mat mass matrix
       */
      void setMassMatrix(const SymMat &mat); 

      /*!
       * read stiffness matrix form given file readMassMatrix(const string &massfilename)
       * \param stiffnessfilename name of file holding stiffness matrix
       */
      void readStiffnessMatrix(const string &stiffnessfilename); 

      /*!
       * read stiffness matrix
       * \param mat stiffness matrix 
       */
      void setStiffnessMatrix(const SqrMat &mat); 

      /*!
       * set damping \f$\vD\f$ proportional to mass and stiffness
       * \f[ \vD = \alpha * \vM + \beta*\vK \f]
       * \param a_ \f$\alpha\f$
       * \param b_ \f$\beta \f$
       */
	  inline void setProportionalDamping(const double &a, const double &b);

//      /*! plot parameters of "BodyFlexibleLinearExternal"
//      */
//      void plotParameters();

      using FlexibleBody<int>::addFrame;
      /*!
       * add Port using JACOBIAN matrix and port location form given file
       * n x d   \n   [1.0  \n  0.0 ] \n
       * \n
       * d x 1   \n   [1.0  \n  0.0   \n  0.0 ]
       * \param name name of Port create
       * \param jacobifilename name of file holding matrices
       */
//      void addFrame(const string &name, const string &jacobifilename);

      /*!
       * add Port using JACOBIAN matrix and location of reference point
       * \param name of Port to 
       * \param J Jacobian matrix create
       * \param r undeformed reference point in body coordinate system
       */
//      void addFrame(const string &name, const Mat &J_, const Vec &r_);

      /*!
       * add Contour using information given in file for JACOBIAN matrix and location of reference point
       * \param contour Contour to add
       * \param jacobifilename name of file holding matrices
       */
//      void addContour(Contour *contour, const string &jacobifilename);

      /*!
       * add Contour using JACOBIAN matrix and location of reference point
       * \param contour Contour to add
       * \param J Jacobian matrix
       * \param r undeformed reference point in body coordinate system
       */
//      void addContour(Contour *contour, const Mat &J_, const Vec &r_);

      /*! add a ContourInterpolation, no additional information needed */
//      void addContourInterpolation(ContourInterpolation *contour);

      /*! set origin BodyFlexibleLinearExternal::WrON00, \f$\rs{_W}[_K0]{\vr}\f$ of model
      */
      void setWrON00(const Vec &WrON00_) {WrON00 = WrON00_;}

      /* geerbt */
      Mat computeJacobianMatrix(const ContourPointData &CP);
   };

}

#endif
