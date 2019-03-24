/* Copyright (C) 2004-2018 MBSim Development Team
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

#ifndef _FLEXIBLE_PLANAR_NURBS_CONTOUR_H_
#define _FLEXIBLE_PLANAR_NURBS_CONTOUR_H_

#include "mbsimFlexibleBody/contours/flexible_contour.h"

#include "mbsim/utils/boost_parameters.h"
#include <mbsimFlexibleBody/utils/openmbv_utils.h>
#include <mbsim/numerics/nurbs/nurbs_curve.h>

namespace OpenMBV {
  class DynamicNurbsCurve;
}

namespace MBSim {
  class ContourFrame;
}

namespace MBSimFlexibleBody {

  /*!  
   * \brief flexible planar nurbs contour
   * \author Martin Foerg
   */
  class FlexiblePlanarNurbsContour : public FlexibleContour {
    public:
      /**
       * \brief constructor 
       * \param name of contour
       */
      FlexiblePlanarNurbsContour(const std::string &name="") : FlexibleContour(name) { }

      /**
       * \brief destructor
       */
      ~FlexiblePlanarNurbsContour() override = default;  

      /* INHERITED INTERFACE OF ELEMENT */
      /***************************************************/

      /* INHERITED INTERFACE OF CONTOUR */
      void init(InitStage stage, const MBSim::InitConfigSet &config) override;
      MBSim::ContourFrame* createContourFrame(const std::string &name="P") override;
      double getCurvature(const fmatvec::Vec2 &zeta);
//      fmatvec::Vec3 evalWn_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalWs_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalWu_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalPosition(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalWs(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalWt(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParDer1Ws(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalParDer1Wu(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParWvCParEta(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParWuPart(const fmatvec::Vec2 &zeta) override;

      void updatePositions(MBSim::ContourFrame *frame) override;
      void updateVelocities(MBSim::ContourFrame *frame) override;
      void updateAccelerations(MBSim::ContourFrame *frame) override;
      void updateJacobians(MBSim::ContourFrame *frame, int j=0) override;
      void updateGyroscopicAccelerations(MBSim::ContourFrame *frame) override;
      /***************************************************/

      /* GETTER / SETTER */
      void setInterpolation(bool interpolation_) { interpolation = interpolation_; }
      void setNodeNumbers(const fmatvec::VecVI &node) { index = node; }
      void setKnotVector(const fmatvec::VecV &knot_) { knot = knot_; }
      void setDegree(int degree_) { degree = degree_; }
      /***************************************************/

      void plot() override;

      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, MBSim::tag, (optional (diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0)(pointSize,(double),0)(lineWidth,(double),0))) {
        OpenMBVDynamicNurbsCurve ombv(diffuseColor,transparency,pointSize,lineWidth);
        openMBVNurbsCurve=ombv.createOpenMBV();
      }

      void initializeUsingXML(xercesc::DOMElement *element) override;

      bool isZetaOutside(const fmatvec::Vec2 &zeta) override;

      void setOpen(bool open_) { open = open_; }

      void resetUpToDate() override { updCrvPos = true; updCrvVel = true; updCrvJac = true; updCrvGA = true; }

      void updateCurvePositions();
      void updateCurveVelocities();
      void updateCurveJacobians();
      void updateCurveGyroscopicAccelerations();

    protected:
      double continueEta(double eta_);
      void updateHessianMatrix(double eta);
      void updateHessianMatrix_t(double eta);
      const fmatvec::MatVx4& evalHessianMatrix(double eta){ if(updCrvPos or fabs(eta-etaOld)>1e-13) updateHessianMatrix(eta); return hess; }
      const fmatvec::MatVx4& evalHessianMatrix_t(double eta){ updateHessianMatrix_t(eta); return hess_t; }

      bool interpolation{false};
      fmatvec::VecVI index;
      fmatvec::VecV knot;
      int degree{3};
      bool open{false};
      MBSim::NurbsCurve crvPos, crvVel, crvGA;
      std::vector<MBSim::NurbsCurve> crvJac;
      double etaOld{-1e10};
      fmatvec::MatVx4 hess, hess_t, hessTmp;
      bool updCrvPos{true};
      bool updCrvVel{true};
      bool updCrvJac{true};
      bool updCrvGA{true};

      std::shared_ptr<OpenMBV::DynamicNurbsCurve> openMBVNurbsCurve;
  };

}

#endif
