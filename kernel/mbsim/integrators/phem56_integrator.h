/* Copyright (C) 2004-2018  Martin Förg
 
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
 *   martin.o.foerg@googlemail.com
 *
 */

#ifndef _PHEM56_INTEGRATOR_H_
#define _PHEM56_INTEGRATOR_H_

#include "integrator.h"

namespace MBSimIntegrator {

  /** \brief DAE-Integrator PHEM56
  */

  class PHEM56Integrator : public Integrator {

    public:

      enum LinearAlgebra {
        DEC=0,
        DGETRF,
        unknown
      };

    private:

      static void fprob(int* ifcn, int* nq, int* nv, int* nu, int* nl, int* nzg, int* nzf, int* lrda, int* nblk, int* nmrc, int* npgp, int* npfl, int* indgr, int* indgc, int* indflr, int* indflc,  double* t, double* p, double* v, double* u, double* xl, double* g, double* gp, double* f, double* gpp, double* gt, double* fl, double* qdot, double* udot, double* am);
      static void solout(int* nr, int* nq, int* nv, int* nu, int* nl, int* lrdo, double* q, double* v, double* u, double* a, double* rlam, double* dowk, int* irtrn);

      bool signChangedWRTsvLast(const fmatvec::Vec &svStepEnd) const;

      double tPlot{0};
      double dtOut{0};
      std::ofstream integPlot;
      double s0; 
      double time{0};

      /** linear algebra */
      LinearAlgebra linearAlgebra{DGETRF};
      /** general V */
      bool generalVMatrix{true};
      /** initial projection */
      bool initialProjection{false};
      /** number of steps between projections */
      int numberOfStepsBetweenProjections{0};
      /** project onto index 1 constraint manifold */
      bool projectOntoIndex1ConstraintManifold{false};
      /** Absolute Toleranz */
      fmatvec::Vec aTol;
      /** Relative Toleranz */
      fmatvec::Vec rTol;
      /** step size for the first step */
      double dt0{1e-3};
      /** maximum number of steps */
      int maxSteps{2000000000};
      /** maximale step size */
      double dtMax{0};

      bool plotOnRoot{false};

       /** tolerance for position constraints */
      double gMax{-1};
      /** tolerance for velocity constraints */
      double gdMax{-1};

      fmatvec::Vec svLast;
      bool shift{false};

    public:

      ~PHEM56Integrator() override = default;

      void setAbsoluteTolerance(const fmatvec::Vec &aTol_) { aTol = aTol_; }
      void setAbsoluteTolerance(double aTol_) { aTol = fmatvec::Vec(1,fmatvec::INIT,aTol_); }
      void setRelativeTolerance(const fmatvec::Vec &rTol_) { rTol = rTol_; }
      void setRelativeTolerance(double rTol_) { rTol = fmatvec::Vec(1,fmatvec::INIT,rTol_); }
      void setInitialStepSize(double dt0_) { dt0 = dt0_; }
      void setMaximumStepSize(double dtMax_) { dtMax = dtMax_; }
      void setStepLimit(int maxSteps_) { maxSteps = maxSteps_; }
      void setLinearAlgebra(LinearAlgebra linearAlgebra_) { linearAlgebra = linearAlgebra_; }
      void setGeneralVMatrix(bool generalVMatrix_) { generalVMatrix = generalVMatrix_; }
      void setInitialProjection(bool initialProjection_) { initialProjection = initialProjection_; }
      void setNumberOfStepsBetweenProjections(int numberOfStepsBetweenProjections_) { numberOfStepsBetweenProjections = numberOfStepsBetweenProjections_; }
      void setProjectOntoIndex1ConstraintManifold(bool projectOntoIndex1ConstraintManifold_) { projectOntoIndex1ConstraintManifold = projectOntoIndex1ConstraintManifold_; }
      void setToleranceForPositionConstraints(double gMax_) { gMax = gMax_; }
      void setToleranceForVelocityConstraints(double gdMax_) { gdMax = gdMax_; }
      void setPlotOnRoot(bool b) { plotOnRoot = b; }

      const fmatvec::Vec& getAbsoluteTolerance() const { return aTol; }
      const fmatvec::Vec& getRelativeTolerance() const { return rTol; }
      double getToleranceForPositionConstraints() { return gMax; }
      double getToleranceForVelocityConstraints() { return gdMax; }

      using Integrator::integrate;
      void integrate() override;

      void initializeUsingXML(xercesc::DOMElement *element) override;
  };

}

#endif
