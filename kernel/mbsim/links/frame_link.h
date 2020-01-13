/* Copyright (C) 2004-2016 MBSim Development Team
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

#ifndef _FRAME_LINK_H_
#define _FRAME_LINK_H_

#include "mbsim/links/mechanical_link.h"

namespace MBSim {

  /** 
   * \brief frame link
   * \author Martin Foerg
   */
  class FrameLink : public MechanicalLink {
    public:
      /**
       * \brief constructor
       * \param name of link
       */
      FrameLink(const std::string &name);

      void init(InitStage stage, const InitConfigSet &config) override;
      void initializeUsingXML(xercesc::DOMElement *element) override;

      void connect(Frame *frame0, Frame *frame1);

      Frame* getFrame(int i) { return frame[i]; }

      void resetUpToDate() override;

      const fmatvec::Mat3xV& evalGlobalForceDirection() { if(updDF) updateForceDirections(); return DF; }
      const fmatvec::Mat3xV& evalGlobalMomentDirection() { if(updDF) updateForceDirections(); return DM; }

      fmatvec::Mat3xV& getGlobalForceDirection(bool check=true) { assert((not check) or (not updDF)); return DF; }
      fmatvec::Mat3xV& getGlobalMomentDirection(bool check=true) { assert((not check) or (not updDF)); return DM; }

      void updatePositions() override;
      void updateForce() override;
      void updateMoment() override;
      void updateR() override;

      const fmatvec::Vec3& evalGlobalRelativePosition() { if(updPos) updatePositions(); return WrP0P1; }
      const fmatvec::SqrMat3& evalGlobalRelativeOrientation() { if(updPos) updatePositions(); return AK0K1; }
      const fmatvec::Vec3& evalGlobalRelativeVelocity() { if(updVel) updateVelocities(); return WvP0P1; }
      const fmatvec::Vec3& evalGlobalRelativeAngularVelocity() { if(updVel) updateVelocities(); return WomK0K1; }

      fmatvec::Vec3& getGlobalRelativePosition(bool check=true) { assert((not check) or (not updPos)); return WrP0P1; }
      fmatvec::SqrMat3& getGlobalRelativeOrientation(bool check=true) { assert((not check) or (not updPos)); return AK0K1; }
      fmatvec::Vec3& getGlobalRelativeVelocity(bool check=true) {  assert((not check) or (not updVel)); return WvP0P1; }
      fmatvec::Vec3& getGlobalRelativeAngularVelocity(bool check=true) { assert((not check) or (not updVel)); return WomK0K1; }

      /* INHERITED INTERFACE OF LINK */
      void updateWRef(fmatvec::Mat& ref, int i=0) override;
      void updateVRef(fmatvec::Mat& ref, int i=0) override;
      void updatehRef(fmatvec::Vec &hRef, int i=0) override;
      virtual void updatedhdqRef(fmatvec::Mat& ref, int i=0);
      virtual void updatedhduRef(fmatvec::SqrMat& ref, int i=0);
      virtual void updatedhdtRef(fmatvec::Vec& ref, int i=0);
      void updaterRef(fmatvec::Vec &ref, int i=0) override;
      /***************************************************/

    protected:
      /**
       * \brief array in which all frames are listed, connecting bodies via a link
       */
      std::vector<Frame*> frame;

      /**
       * \brief difference vector of position, velocity and angular velocity
       */
      fmatvec::Vec3 WrP0P1, WvP0P1, WomK0K1;

      fmatvec::SqrMat3 AK0K1;

//      int nF, nM;

      fmatvec::Mat3xV DF, DM;

      bool updPos, updVel, updDF;

    private:
      std::string saved_ref1, saved_ref2;
  };
}

#endif
