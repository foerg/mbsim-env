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
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _LINE_H_
#define _LINE_H_

#include "mbsim/contour.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <mbsim/utils/openmbv_utils.h>
#endif

namespace MBSim {

  class ContourPointData;

  /**
   * \brief unbounded line with constant normal
   * \author Martin Foerg
   * \date 2009-04-20 some comments (Thorsten Schindler)
   */ 
  class Line : public MBSim::RigidContour {	
    public:
      /**
       * \brief constructor
       * \param name of line
       */
      Line(const std::string& name="", Frame *R=0) : RigidContour(name,R) {}

      /* INHERITED INTERFACE OF ELEMENT */
      std::string getType() const { return "Line"; }
      /***************************************************/

      /* INHERITED INTERFACE OF CONTOUR */
      virtual double computeCurvature(ContourPointData &cp) { return 0; } 
      /***************************************************/

#ifdef HAVE_OPENMBVCPPINTERFACE
      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, tag, (optional (diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0)(length,(double),1))) {       
        OpenMBVLine ombv(length,diffuseColor,transparency);
        openMBVRigidBody=ombv.createOpenMBV(); 
      }
#endif
      
      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
  };      
}

#endif /* _LINE_H_ */

