/* Copyright (C) 2004-2015 MBSim Development Team
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

#ifndef _FLEXIBLE_BAND_H_
#define _FLEXIBLE_BAND_H_

#include "mbsimFlexibleBody/contours/contour1s.h"
#include "mbsimFlexibleBody/utils/contact_utils.h"
#include "mbsim/utils/eps.h"

#include <openmbvcppinterface/spineextrusion.h>
#include "mbsim/utils/boost_parameters.h"
#include <mbsim/utils/openmbv_utils.h>

#include <list>

namespace MBSim {

  BOOST_PARAMETER_NAME(numberOfSpinePoints)

}

namespace MBSimFlexibleBody {

  class FlexibleBand : public Contour1s {
    public:
      /**
       * \brief constructor
       * \param name of contour
       */
      FlexibleBand(const std::string& name) : Contour1s(name), width(0), ARK(fmatvec::EYE), sOld(-1e12) { }

      /* INHERITED INTERFACE OF ELEMENT */
      void init(InitStage stage, const MBSim::InitConfigSet &config) override;
     /***************************************************/

       MBSim::ContourFrame* createContourFrame(const std::string &name="P") override;

      /* GETTER / SETTER */
      void setWidth(double width_) { width = width_; }
      double getWidth() const { return width; }

      void setRelativePosition(const fmatvec::Vec2 &r);
      void setRelativeOrientation(double al);

      const fmatvec::Vec3& getRelativePosition() const { return RrRP; }
      const fmatvec::SqrMat3& getRelativeOrientation() const { return ARK; }

       fmatvec::Vec3 evalPosition(const fmatvec::Vec2 &zeta) override { return evalPosition(zeta(0)); }
       fmatvec::Vec3 evalWs(const fmatvec::Vec2 &zeta) override { return evalWs(zeta(0)); }
       fmatvec::Vec3 evalWt(const fmatvec::Vec2 &zeta) override { return evalWt(zeta(0)); }
       fmatvec::Vec3 evalWu(const fmatvec::Vec2 &zeta) override { return evalWs(zeta); }
       fmatvec::Vec3 evalWv(const fmatvec::Vec2 &zeta) override { return evalWt(zeta); }

       bool isZetaOutside(const fmatvec::Vec2 &zeta) override;

      void updatePositions(double s);

      fmatvec::Vec3 evalPosition(double s) { if(fabs(s-sOld)>MBSim::macheps) updatePositions(s); return WrOP; }
      fmatvec::Vec3 evalWs(double s) { if(fabs(s-sOld)>MBSim::macheps) updatePositions(s); return Ws; }
      fmatvec::Vec3 evalWt(double s) { if(fabs(s-sOld)>MBSim::macheps) updatePositions(s); return Wt; }

       void plot() override;

      void setContourOfReference(Contour1s *contour_) { contour = contour_; }

      MBSim::ContactKinematics * findContactPairingWith(const std::type_info &type0, const std::type_info &type1) override { return findContactPairingFlexible(type0, type1); }

      void setNodes(const std::vector<double> &nodes_) { etaNodes = nodes_; }

      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, MBSim::tag, (optional (numberOfSpinePoints,(int),10)(diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0))) {
        openMBVSpineExtrusion = OpenMBV::ObjectFactory::create<OpenMBV::SpineExtrusion>();
        openMBVSpineExtrusion->setNumberOfSpinePoints(numberOfSpinePoints);
      }

      void resetUpToDate() override;

    protected:
      /**
       * \brief width of flexible band
       */
      double width;

      fmatvec::Vec3 RrRP, WrOP, Ws, Wt;
      fmatvec::SqrMat3 ARK;

      Contour1s* contour;

      double sOld;

      std::shared_ptr<OpenMBV::SpineExtrusion> openMBVSpineExtrusion;
  };

}

#endif /* _FLEXIBLE_BAND_H_ */
