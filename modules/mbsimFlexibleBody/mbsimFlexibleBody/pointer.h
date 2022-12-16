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
 * Contact: thorsten.schindler@mytum.de
 */

#ifndef _POINTER_H_
#define _POINTER_H_

#include <memory>

namespace MBSimFlexibleBody {

class Cardan;
class RevCardan;
class Trafo33RCM;
class Weight33RCM;

using CardanPtr = std::shared_ptr<Cardan>;
using RevCardanPtr = std::shared_ptr<RevCardan>;
using Trafo33RCMPtr = std::shared_ptr<Trafo33RCM>;
using Weight33RCMPtr = std::shared_ptr<Weight33RCM>;

}

#endif /* _POINTER_H_ */
