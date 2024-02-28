/* Copyright (C) 2004-2014 MBSim Development Team
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

#ifndef _SPATIAL_COULOMB_FRICTION_H_
#define _SPATIAL_COULOMB_FRICTION_H_

#include <mbsim/constitutive_laws/friction_force_law.h>

namespace MBSim {

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
      SpatialCoulombFriction(double mu_=0) : mu(mu_) { }

      /**
       * \brief destructor
       */
      ~SpatialCoulombFriction() override = default;

      /* INHERITED INTERFACE */
      fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r) override;
      fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double r) override;
      fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, double laN) override;
      bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, double laN, double laTol, double gdTol) override;
      fmatvec::Vec dlaTdlaN(const fmatvec::Vec& gd) override;
      int getFrictionDirections() override { return 2; }
      bool isSticking(const fmatvec::Vec& s, double sTol) override { return nrm2(s) <= sTol; }
      bool isSetValued() const override { return true; }
      void initializeUsingXML(xercesc::DOMElement *element) override;
      FrictionImpactLaw* createFrictionImpactLaw() const override;
      /***************************************************/

      void setFrictionCoefficient(double mu_) { mu = mu_; }

    protected:
      double mu;
  };

}

#endif
