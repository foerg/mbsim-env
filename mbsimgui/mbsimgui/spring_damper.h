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

#ifndef _SPRING_DAMPER__H_
#define _SPRING_DAMPER__H_

#include "link_.h"

namespace MBSimGUI {

  class SpringDamper : public FixedFrameLink {
    public:
      QString getType() const override { return "SpringDamper"; }
      xercesc::DOMElement* processIDAndHref(xercesc::DOMElement* element) override;
      PropertyDialog* createPropertyDialog() override {return new SpringDamperPropertyDialog(this);}
  };

  class DirectionalSpringDamper : public FloatingFrameLink {
    public:
      QString getType() const override { return "DirectionalSpringDamper"; }
      xercesc::DOMElement* processIDAndHref(xercesc::DOMElement* element) override;
      PropertyDialog* createPropertyDialog() override {return new DirectionalSpringDamperPropertyDialog(this);}
  };

  class IsotropicRotationalSpringDamper : public FixedFrameLink {
    public:
      QString getType() const override { return "IsotropicRotationalSpringDamper"; }
      PropertyDialog* createPropertyDialog() override {return new IsotropicRotationalSpringDamperPropertyDialog(this);}
  };

  class GeneralizedSpringDamper : public DualRigidBodyLink {
    public:
      QString getType() const override { return "GeneralizedSpringDamper"; }
      PropertyDialog* createPropertyDialog() override {return new GeneralizedSpringDamperPropertyDialog(this);}
  };

}

#endif
