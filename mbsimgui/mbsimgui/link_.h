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

#ifndef _LINK__H_
#define _LINK__H_

#include "element.h"

namespace MBSimGUI {

  class Link : public Element {
    public:
      QString getType() const override { return "Link"; }
      QMenu* createContextMenu() override { return new LinkContextMenu(this); }
  };

  class UnknownLink : public Link {
    public:
      QString getType() const override { return "UnknownLink"; }
      ElementPropertyDialog* createPropertyDialog() override {return new UnknownElementPropertyDialog(this);}
  };

  class MechanicalLink : public Link {
  };

  class FrameLink : public MechanicalLink {
  };

  class FixedFrameLink : public FrameLink {
  };

  class FloatingFrameLink : public FrameLink {
  };

  class RigidBodyLink : public MechanicalLink {
  };

  class DualRigidBodyLink : public RigidBodyLink {
  };

}

#endif
