/* Copyright (C) 2004-2006  Martin Förg
 
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
 *   mfoerg@users.berlios.de
 *
 */

#ifndef _LSODER_INTEGRATOR_H_
#define _LSODER_INTEGRATOR_H_

#include "integrator.h"

namespace MBSim {

  /** \brief ODE-Integrator LSODER
    Integrator with root finding for ODEs.
    This integrator uses LSODE from http://www.netlib.org . */
  class LSODERIntegrator : public Integrator {

    private:

      static void fzdot(int* zSize, double* t, double* z_, double* zd_);
      static void fsv(int* zSize, double* t, double* z_, int* nsv, double* sv_);

      /** maximal step size */
      double dtMax;
      /** minimal step size */
      double dtMin;
      /** Absolute Toleranz */
      fmatvec::Vec aTol;
      /** Relative Toleranz */
      double rTol;
      /** step size for the first step */
      double dt0;

    public:

      LSODERIntegrator();
      ~LSODERIntegrator() {}

      void setdtMax(double dtMax_) {dtMax = dtMax_;}
      void setdtMin(double dtMin_) {dtMin = dtMin_;}
      void setaTol(const Vec &aTol_) {aTol.resize() = aTol_;}
      void setaTol(double aTol_) {aTol.resize() = Vec(1,INIT,aTol_);}
      void setrTol(double rTol_) {rTol = rTol_;}
      void setdt0(double dt0_) {dt0 = dt0_;}

      void integrate(MultiBodySystem& system);

  };

}


#endif
