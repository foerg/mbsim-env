/* Copyright (C) 2004-2013 MBSim Development Team
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

#ifndef _KINETICEXCITATION_H_
#define _KINETICEXCITATION_H_

#include <mbsim/links/floating_frame_link.h>
#include <mbsim/functions/function.h>

#include "mbsim/utils/boost_parameters.h"
#include "mbsim/utils/openmbv_utils.h"

namespace MBSim {

  /**
   * \brief kinetic excitations given by time dependent functions
   * \author Markus Friedrich
   * \date 2009-08-11 some comments (Thorsten Schindler)
   * \date 2013-01-09 second frame for action-reaction law (Martin Förg)
   */
  class KineticExcitation : public FloatingFrameLink {
    public:
      /**
       * \brief constructor
       * \param name of link machanics
       */
      KineticExcitation(const std::string &name="");

      /**
       * \brief destructor
       */
      ~KineticExcitation() override;

      void updateGeneralizedPositions() override { updrrel = false; }
      void updateGeneralizedVelocities() override { updvrel = false; }
      void updatelaF() override;
      void updatelaM() override;

      void init(InitStage stage, const InitConfigSet &config) override;

      bool isActive() const override { return true; }
      bool isSingleValued() const override { return true; }
      bool gActiveChanged() override { return false; }
      
      /**
       * \param local force direction represented in first frame
       */
      void setForceDirection(const fmatvec::Mat3xV& fd) { forceDir <<= fd; }

      /**
       * \param local moment direction represented in first frame
       */
      void setMomentDirection(const fmatvec::Mat3xV& md) { momentDir <<= md; }

      /** \brief Set the force excitation.
       * forceDir*func(t) is the applied force vector in space.
       * This force vector is given in the frame set by setFrameOfReference.
       */
      void setForceFunction(Function<fmatvec::VecV(double)> *func);

      /** \brief see setForce */
      void setMomentFunction(Function<fmatvec::VecV(double)> *func);

      using FloatingFrameLink::connect;

      /**
       * \param frame to connect
       */
      void connect(MBSim::Frame *frame_) { frame[1] = frame_; }

      void plot() override;

      void initializeUsingXML(xercesc::DOMElement *element) override;

     BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, tag, (optional (sideOfInteraction,(OpenMBVInteractionArrow::SideOfInteraction),OpenMBVInteractionArrow::action)(scaleLength,(double),1)(scaleSize,(double),1)(referencePoint,(OpenMBVArrow::ReferencePoint),OpenMBVArrow::toPoint)(colorRepresentation,(OpenMBVArrow::ColorRepresentation),OpenMBVArrow::none)(minimalColorValue,(double),0)(maximalColorValue,(double),1)(diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0)(pointSize,(double),0)(lineWidth,(double),0))) {
        ombvArrow = std::shared_ptr<OpenMBVInteractionArrow>(new OpenMBVInteractionArrow(sideOfInteraction,scaleLength,scaleSize,OpenMBVArrow::toHead,referencePoint,colorRepresentation,minimalColorValue,maximalColorValue,diffuseColor,transparency,pointSize,lineWidth));
      }

    protected:
      /**
       * \brief portions of the force / moment in the specific directions
       */
      Function<fmatvec::VecV(double)> *F, *M;

      std::shared_ptr<OpenMBVInteractionArrow> ombvArrow;
      std::vector<std::shared_ptr<OpenMBV::Arrow>> openMBVForce, openMBVMoment;

#ifndef SWIG
      double (KineticExcitation::*evalOMBVForceColorRepresentation[2])();
      double (KineticExcitation::*evalOMBVMomentColorRepresentation[2])();
#endif
      double evalNone() { return 1; }
      double evalAboluteForce() { return nrm2(evalForce()); }
      double evalAboluteMoment() { return nrm2(evalMoment()); }
  };

}

#endif /* _KINETICEXCITATION_H_ */
