/*
   MBSimGUI - A fronted for MBSim.
   Copyright (C) 2012 Martin Förg

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
   */

#include <config.h>
#include "link.h"

using namespace std;

namespace MBSimGUI {

  Link::Link(const QString &str) : Element(str) {
    addPlotFeature("generalizedRelativePosition");
    addPlotFeature("generalizedRelativeVelocity");
    addPlotFeature("generalizedForce");
    addPlotFeature("energy");
  }

  FrameLink::FrameLink(const QString &str) : MechanicalLink(str) {
//    connections.setProperty(new ConnectFramesProperty(2,this));
  }

  FloatingFrameLink::FloatingFrameLink(const QString &str) : FrameLink(str) {

//    refFrameID.setProperty(new IntegerProperty(1,MBSIM%"frameOfReferenceID"));
  }

  RigidBodyLink::RigidBodyLink(const QString &str) : MechanicalLink(str) {
//    support.setProperty(new FrameOfReferenceProperty("",this,MBSIM%"supportFrame"));
  }

  DualRigidBodyLink::DualRigidBodyLink(const QString &str) : RigidBodyLink(str) {
//    connections.setProperty(new ChoiceProperty2(new ConnectRigidBodiesPropertyFactory(this),"",4));
  }

}
