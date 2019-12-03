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

#ifndef _TRANSLATION_ALONG_Y_AXIS_H_
#define _TRANSLATION_ALONG_Y_AXIS_H_

#include "mbsim/functions/function.h"
#include "mbsim/utils/utils.h"

namespace MBSim {

  template<typename Arg>
  class TranslationAlongYAxis : public Function<fmatvec::Vec3(Arg)> {
    using B = fmatvec::Function<fmatvec::Vec3(Arg)>; 
    private:
      fmatvec::Vec3 r, a, zero;
    public:
      TranslationAlongYAxis() { a.e(1) = 1; }
      int getArgSize() const override { return 1; }
      fmatvec::Vec3 operator()(const Arg &q) override { 
        r.e(1) = ToDouble<Arg>::cast(q);
        return r; 
      }
      typename B::DRetDArg parDer(const Arg &arg) override { return a; }
      typename B::DRetDArg parDerDirDer(const Arg &arg1Dir, const Arg &arg1) override { return zero; }
      bool constParDer() const override { return true; }
  };

}

#endif
