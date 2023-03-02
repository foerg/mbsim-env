/* Copyright (C) 2004-2019 MBSim Development Team
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

#ifndef _NODE_FRAME_H__
#define _NODE_FRAME_H__

#include "mbsimFlexibleBody/frames/node_based_frame.h"

namespace MBSimFlexibleBody {

  /**
   * \brief cartesian frame on nodes of flexible bodies
   * \author Kilian Grundl
   */
  class NodeFrame : public NodeBasedFrame {

    public:
      NodeFrame(const std::string &name = "dummy", int node_ = 1) : NodeBasedFrame(name), node(node_) { }

      void setNodeNumber(int node_) { node = node_; }
      int getNodeNumber() const { return node; }

      void updatePositions() override;
      void updateVelocities() override;
      void updateAccelerations() override;
      void updateJacobians(int j=0) override;
      void updateGyroscopicAccelerations() override;

      void init(InitStage stage, const MBSim::InitConfigSet &config) override;
      void initializeUsingXML(xercesc::DOMElement *element) override;

    protected:
      /*!
       * \brief node number
       */
      int node;
  };

}

#endif
