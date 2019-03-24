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

#ifndef _FLEXIBLE_SPATIAL_NURBS_CONTOUR_H_
#define _FLEXIBLE_SPATIAL_NURBS_CONTOUR_H_

#include "mbsimFlexibleBody/contours/flexible_contour.h"

#include "mbsim/utils/boost_parameters.h"
#include <mbsimFlexibleBody/utils/openmbv_utils.h>
#include <mbsim/numerics/nurbs/nurbs_surface.h>

namespace OpenMBV {
  class DynamicNurbsSurface;
}

namespace MBSim {
  class ContourFrame;
}

namespace MBSimFlexibleBody {

  /*!  
   * \brief flexible spatial nurbs contour
   * \author Martin Foerg
   */
  class FlexibleSpatialNurbsContour : public FlexibleContour {
    public:
      /**
       * \brief constructor 
       * \param name of contour
       */
      FlexibleSpatialNurbsContour(const std::string &name="") : FlexibleContour(name) { }

      /**
       * \brief destructor
       */
      ~FlexibleSpatialNurbsContour() override = default;  

      /* INHERITED INTERFACE OF ELEMENT */
      /***************************************************/

      /* INHERITED INTERFACE OF CONTOUR */
      void init(InitStage stage, const MBSim::InitConfigSet &config) override;
      MBSim::ContourFrame* createContourFrame(const std::string &name="P") override;
      double getCurvature(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalWn_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalWs_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalWt_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalWu_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalWv_t(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalPosition(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalWs(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalWt(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParDer1Ws(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalParDer2Ws(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalParDer1Wt(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalParDer2Wt(const fmatvec::Vec2 &zeta);
      fmatvec::Vec3 evalParDer1Wu(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParDer2Wu(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParDer1Wv(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParDer2Wv(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParDer1Wn(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParDer2Wn(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParWvCParEta(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParWvCParXi(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParWuPart(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalParWvPart(const fmatvec::Vec2 &zeta) override;

      void updatePositions(MBSim::ContourFrame *frame) override;
      void updateVelocities(MBSim::ContourFrame *frame) override;
      void updateAccelerations(MBSim::ContourFrame *frame) override;
      void updateJacobians(MBSim::ContourFrame *frame, int j=0) override;
      void updateGyroscopicAccelerations(MBSim::ContourFrame *frame) override;
      /***************************************************/

      /* GETTER / SETTER */
      void setInterpolation(bool interpolation_) { interpolation = interpolation_; }
      void setNodeNumbers(const fmatvec::MatVI &node) { index = node; }
      void setEtaKnotVector(const fmatvec::VecV &uKnot_) { uKnot = uKnot_; }
      void setXiKnotVector(const fmatvec::VecV &vKnot_) { vKnot = vKnot_; }
      void setEtaDegree(int etaDegree_) { etaDegree = etaDegree_; }
      void setXiDegree(int xiDegree_) { xiDegree = xiDegree_; }
      /***************************************************/

      void plot() override;

      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, MBSim::tag, (optional (diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0)(pointSize,(double),0)(lineWidth,(double),0))) {
        OpenMBVDynamicNurbsSurface ombv(diffuseColor,transparency,pointSize,lineWidth);
        openMBVNurbsSurface=ombv.createOpenMBV();
      }
      
      void initializeUsingXML(xercesc::DOMElement *element) override;

//      void setNodes(const std::vector<double> &nodes_) { etaNodes = nodes_; }

      bool isZetaOutside(const fmatvec::Vec2 &zeta) override;

      void setOpenEta(bool openEta_) { openEta = openEta_; }
      void setOpenXi(bool openXi_) { openXi = openXi_; }

      void resetUpToDate() override { updSrfPos = true; updSrfVel = true; updSrfJac = true; updSrfGA = true; }

      void updateSurfacePositions();
      void updateSurfaceVelocities();
      void updateSurfaceJacobians();
      void updateSurfaceGyroscopicAccelerations();

    protected:
      fmatvec::Vec2 continueZeta(const fmatvec::Vec2 &zeta_);
      void updateHessianMatrix(const fmatvec::Vec2 &zeta);
      void updateHessianMatrix_t(const fmatvec::Vec2 &zeta);
      const fmatvec::GeneralMatrix<fmatvec::Vec4>& evalHessianMatrix(const fmatvec::Vec2 &zeta){ if(updSrfPos or fabs(zeta(0)-zetaOld(0))>1e-13 or fabs(zeta(1)-zetaOld(1))>1e-13) updateHessianMatrix(zeta); return hess; }
      const fmatvec::GeneralMatrix<fmatvec::Vec4>& evalHessianMatrix_t(const fmatvec::Vec2 &zeta){ updateHessianMatrix_t(zeta); return hess_t; }

      bool interpolation{false};
      fmatvec::MatVI index;
      fmatvec::VecV uKnot, vKnot;
      int etaDegree{3};
      int xiDegree{3};
      bool openEta{false};
      bool openXi{false};
      MBSim::NurbsSurface srfPos, srfVel, srfGA;
      std::vector<MBSim::NurbsSurface> srfJac;
      fmatvec::Vec2 zetaOld;
      fmatvec::GeneralMatrix<fmatvec::Vec4> hess, hess_t, hessTmp;
      bool updSrfPos{true};
      bool updSrfVel{true};
      bool updSrfJac{true};
      bool updSrfGA{true};

      std::shared_ptr<OpenMBV::DynamicNurbsSurface> openMBVNurbsSurface;
  };

}

#endif
