/*
 * neutral_nurbs_velocity_2s.h
 *
 *  Created on: 03.12.2013
 *      Author: zwang
 */

#ifndef _NEUTRAL_NURBS_VELOCITY_2S_H_
#define _NEUTRAL_NURBS_VELOCITY_2S_H_

#include "nurbs_2s.h"

namespace MBSimFlexibleBody {
  
  class NeutralNurbsVelocity2s : public MBSimFlexibleBody::NeutralNurbs2s {
    public:
      NeutralNurbsVelocity2s(MBSim::Element* parent_, const fmatvec::MatVI & nodes, double nodeOffset, int degU_, int degV_, bool openStructure_);
      ~NeutralNurbsVelocity2s() override;
      void update(MBSim::ContourFrame *frame) override;
    protected:
      void buildNodelist() override;
  };

} /* namespace MBSimFlexibleBody */
#endif
