/* Copyright (C) 2004-2009 MBSim Development Team
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
 * Contact: martin.o.foerg@gmail.com
 */

#ifndef _ROTATION_ABOUT_FIXED_AXIS_H_
#define _ROTATION_ABOUT_FIXED_AXIS_H_

#include "mbsim/functions/function.h"
#include "mbsim/utils/utils.h"

namespace MBSim {

  template<class Arg> 
  class RotationAboutFixedAxis : public Function<fmatvec::RotMat3(Arg)> {
    using B = fmatvec::Function<fmatvec::RotMat3(Arg)>; 
    private:
      fmatvec::RotMat3 A;
      fmatvec::Vec3 a;
    public:
      RotationAboutFixedAxis() = default;
      RotationAboutFixedAxis(const fmatvec::Vec3 &a_) : a(a_) { }
      int getArgSize() const override { return 1; }
      fmatvec::RotMat3 operator()(const Arg &q) override {
        double alpha = ToDouble<Arg>::cast(q);
        const double cosq=cos(alpha);
        const double sinq=sin(alpha);
        const double onemcosq=1-cosq;
        const double a0a1=a.e(0)*a.e(1);
        const double a0a2=a.e(0)*a.e(2);
        const double a1a2=a.e(1)*a.e(2);
        A.e(0,0) = cosq+onemcosq*a.e(0)*a.e(0);
        A.e(1,0) = onemcosq*a0a1+a.e(2)*sinq;
        A.e(2,0) = onemcosq*a0a2-a.e(1)*sinq;
        A.e(0,1) = onemcosq*a0a1-a.e(2)*sinq;
        A.e(1,1) = cosq+onemcosq*a.e(1)*a.e(1);
        A.e(2,1) = onemcosq*a1a2+a.e(0)*sinq;
        A.e(0,2) = onemcosq*a0a2+a.e(1)*sinq;
        A.e(1,2) = onemcosq*a1a2-a.e(0)*sinq;
        A.e(2,2) = cosq+onemcosq*a.e(2)*a.e(2);
        return A;
      }
      typename B::DRetDArg parDer(const Arg &q) override { return a; }
      typename B::DRetDArg parDerDirDer(const Arg &qd, const Arg &q) override { return typename B::DRetDArg(3); }
      typename B::DDRetDDArg parDerParDer(const Arg &arg) override { this->throwError("parDerParDer is not available for given template parameters."); }
      bool constParDer() const override { return true; }
      const fmatvec::Vec3& getAxisOfRotation() const { return a; }
      void setAxisOfRotation(const fmatvec::Vec3 &a_) { a = a_; }
      void initializeUsingXML(xercesc::DOMElement *element) override {
        xercesc::DOMElement *e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIM%"axisOfRotation");
        a=FromMatStr<fmatvec::Vec3>::cast((MBXMLUtils::X()%MBXMLUtils::E(e)->getFirstTextChild()->getData()).c_str());
      }
  };

  template<>
  inline fmatvec::Vec3 RotationAboutFixedAxis<double>::parDerParDer(const double &arg) { return fmatvec::Vec3(); }

}

#endif
