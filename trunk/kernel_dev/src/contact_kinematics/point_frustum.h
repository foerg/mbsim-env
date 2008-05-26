/* Copyright (C) 2007  Martin Förg, Roland Zander
 
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
 * Contact:
 *   mfoerg@users.berlios.de
 *   rzander@users.berlios.de
 *
 */

#ifndef _CONTACT_KINEMATICS_POINT_FRUSTUM_H_
#define _CONTACT_KINEMATICS_POINT_FRUSTUM_H_

#include "contact_kinematics.h"

namespace MBSim {
	
  	class Point; 
  	class Frustum; 

	/*! \brief Pairing Point to Frustum Surface
	 * 
	 * Authors: Martin Foerg, Thorsten Schindler
	 */
	class ContactKinematicsPointFrustum : public ContactKinematics {     
	    public:
	    	  /*! Compute normal distance in contact point */
		      void stage1(Vec &g, vector<ContourPointData> &cpData);
		      /*! Compute tangential directions and normal velocities in contact points */
		      void stage2(const Vec &g, Vec &gd, vector<ContourPointData> &cpData);
		      /*! Treat ordering of contacting bodies in connect-call */
		      void assignContours(const vector<Contour*> &contour);
		      
		private:
		      int ipoint, ifrustum; 
		      Point *point;
		      Frustum *frustum;
	};
}

#endif /* _CONTACT_KINEMATICS_POINT_FRUSTUM_H_ */
