/* Copyright (C) 2004-2010 MBSim Development Team
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

#ifndef _QUASI_STATIC_INTEGRATOR_H_
#define _QUASI_STATIC_INTEGRATOR_H_

#include "integrator.h"
#include "mbsim/functions/function.h"
#include<mbsim/dynamic_system_solver.h>

namespace MBSimIntegrator {

  /*!
   * \brief calculate h vector according the new q and system boundary conditions
   * \author Zhan Wang
   * \date 2014-11-11 initial commit (Zhan Wang)
   */
  class hgFun : public MBSim::Function<fmatvec::Vec(fmatvec::Vec)> {
    public:
      /*!
       * \brief constructor
       */
      hgFun(MBSim::DynamicSystemSolver* sys_) :
          sys(sys_), t(0) {
      }

      /*!
       * \brief destructor
       */
      virtual ~hgFun() {
      }
      ;

      /* INHERITED INTERFACE */
      fmatvec::Vec operator()(const fmatvec::Vec& qla);

      void setT(double t) {
        this->t = t;
      }

      /*******************************************************/

    private:

      MBSim::DynamicSystemSolver* sys;
      double t;

  };

  /*!
   * \brief function for the dh/dq
   * \author Zhan Wang
   * \date 2014-12-01 initial commit (Zhan Wang)
   */
  class jacFun : public MBSim::Function<fmatvec::SqrMat(fmatvec::Vec)> {
    public:
      /*!
       * \brief constructor
       */
      jacFun(MBSim::DynamicSystemSolver* sys_, double t_, fmatvec::Vec& z_) :
          sys(sys_), t(t_), z(z_) {
      }

      /*!
       * \brief destructor
       */
      virtual ~jacFun() {
      }
      ;

      /* INHERITED INTERFACE */
      fmatvec::SqrMat operator()(const fmatvec::Vec& q);
      /*******************************************************/

    private:

      MBSim::DynamicSystemSolver* sys;
      double t;
      fmatvec::Vec z;

  };
  /** 
   * brief quasi static time-stepping integrator
   * \author Zhan Wang
   * \date 2014-11-11 initial commit (Zhan Wang)
   */
  class QuasiStaticIntegrator : public Integrator {
    public:
      /**
       * \brief constructor
       */
      QuasiStaticIntegrator();

      /**
       * \brief destructor
       */
      virtual ~QuasiStaticIntegrator() {
      }

      void preIntegrate(MBSim::DynamicSystemSolver& system);
      void subIntegrate(MBSim::DynamicSystemSolver& system, double tStop);
      void postIntegrate(MBSim::DynamicSystemSolver& system);

      /* INHERITED INTERFACE OF INTEGRATOR */
      virtual void integrate(MBSim::DynamicSystemSolver& system);
      virtual void initializeUsingXML(xercesc::DOMElement *element);
      /***************************************************/

      /* GETTER / SETTER */
      void setStepSize(double dt_) {
        dt = dt_;
      }
      void setgTolerance(double tolerance_) {
        gTol = tolerance_;
      }
      void sethTolerance(double tolerance_) {
        hTol = tolerance_;
      }
      void setmaxExtraPolate(int value) {
        maxExtraPolate = value;
      }
      void setextraPolateAfter(int value) {
        extraPolateAfter = value;
      }
      void setupdateJacobianEvery(int value) {
        updateJacobianEvery = value;
      }
      /***************************************************/

    private:
      /**
       * \brief step size
       */
      double dt;

      /**
       * \brief time and plot time
       */
      double t, tPlot;

      /*!
       * \brief tolerance for the newton iteration for distances
       */
      double gTol;

      /*!
       * \brief tolerance for newton iteration for forces
       */
      double hTol;

      /**
       * \brief iteration counter for constraints, plots, integration, maximum constraints, cummulation constraint
       */
      int iter, step, integrationSteps, maxIter, sumIter;

      /*!
       * \brief value of how many points in the past should be used to extrapolate for new value
       */
      int maxExtraPolate;

      /*!
       * \brief extrapolate after such and that integration steps
       */
      int extraPolateAfter;

      /*!
       * \brief value of how often the Jacobian should be updated every step
       */
      int updateJacobianEvery;

      /**
       * \brief computing time counter
       */
      double s0, time;

      /**
       * \brief plot step difference
       */
      int stepPlot;

      /**
       * \brief state, position, velocity, order coordinate of dynamical system
       */
      fmatvec::Vec z, q, u, x;

      /**
       * \brief file stream for integration information
       */
      std::ofstream integPlot;
  };

}

#endif /* _QUASI_STATIC_INTEGRATOR_H_ */

