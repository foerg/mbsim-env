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

#ifndef NURBSDISK2S_H_
#define NURBSDISK2S_H_

#include "mbsimFlexibleBody/contours/contour2s.h"
#include "mbsimFlexibleBody/frames/node_frame.h"
#include "mbsimFlexibleBody/utils/contact_utils.h"

#include "openmbvcppinterface/nurbsdisk.h"
#include "mbsim/utils/boost_parameters.h"
#include <mbsim/utils/openmbv_utils.h>

#include "nurbs++/nurbs.h"
#include "nurbs++/nurbsS.h"
#include "nurbs++/vector.h"

namespace MBSim {
  class ContourFrame;
}

namespace MBSimFlexibleBody {

  /*!  
   * \brief 2s flexible
   * \author Kilian Grundl
   * \author Raphael Missel
   * \author Thorsten Schindler
   * \date 2009-05-22 initial commit (Grundl / Missel / Schindler)
   * \date 2009-06-04 separate contour files (Thorsten Schindler)
   * \date 2009-08-16 contour / visualisation (Grundl / Missel / Schindler)
   * \date 2010-04-21 flexible disks with parent (Grundl / Schindler)
   * \todo computeSurfaceJacobians / computeSurfaceVelocities only in contact case TODO
   * \todo angularVelocity TODO
   * \todo flexible body should only parametrise midplane -> other surfaces in contour TODO
   */
  class NurbsDisk2s : public Contour2s {
    public:
      /**
       * \brief constructor 
       * \param name of contour
       */
      NurbsDisk2s(const std::string &name);

      /**
       * \brief destructor
       */
      ~NurbsDisk2s() override;  

      /* INHERITED INTERFACE OF ELEMENT */
      /***************************************************/

      void init(InitStage stage, const MBSim::InitConfigSet &config) override;

      MBSim::ContourFrame* createContourFrame(const std::string &name="P") override;

      fmatvec::Vec3 evalPosition(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalWs(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalWt(const fmatvec::Vec2 &zeta) override;
      fmatvec::Vec3 evalWu(const fmatvec::Vec2 &zeta) override { return evalWs(zeta); }
      fmatvec::Vec3 evalWv(const fmatvec::Vec2 &zeta) override { return evalWt(zeta); }
      fmatvec::Vec3 evalWn(const fmatvec::Vec2 &zeta) override;

      bool isZetaOutside(const fmatvec::Vec2 &zeta) override { return zeta(0) < etaNodes[0] or zeta(0) > etaNodes[etaNodes.size()-1]; }

      void updatePositions(MBSim::ContourFrame *frame);
      void updateVelocities(MBSim::ContourFrame *frame);
      void updateAccelerations(MBSim::ContourFrame *frame);
      void updateJacobians(MBSim::ContourFrame *frame, int j=0);
      void updateGyroscopicAccelerations(MBSim::ContourFrame *frame);

      fmatvec::Vec3 evalPosition();
      fmatvec::SqrMat3 evalOrientation();

      void plot() override;

      MBSim::ContactKinematics * findContactPairingWith(const std::type_info &type0, const std::type_info &type1) override { return findContactPairingFlexible(type0, type1); }

      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, MBSim::tag, (optional (diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0))) {
        openMBVNurbsDisk = OpenMBV::ObjectFactory::create<OpenMBV::NurbsDisk>();
      }

//      /**
//       * \brief initialize NURBS disk
//       * \param stage of initialisation
//       */
//      void initContourFromBody(InitStage stage);

      /*! 
       * \brief transformation cartesian to cylinder system
       * \param cartesian vector in world system
       * \return cylindrical coordinates
       */
      fmatvec::Vec transformCW(const fmatvec::Vec& WrPoint);

      /*! 
       * \return derivates of the surface: (first column: deg-th derivative in radial direction / second column: deg-th derivative in azimuthal-direction)
       * \param radial location
       * \param azimuthal location
       * \param order of derivative
       */
      fmatvec::Mat computeDirectionalDerivatives(const double &radius, const double &phi, const int &deg);

      /*!
       * \return curvature on the surface: (first column: radial direction / second column: azimuthal-direction)
       * \param radial location
       * \param azimuthal location
       */
      fmatvec::Mat computeCurvatures(const double &radius, const double &phi);

      /*! 
       * \brief computes the U vector of the surface for a closed interpolation
       * \param
       */
      void computeUVector(const int NbPts); 

      /*! 
       * \brief computes the V-vector of the surface for an open interpolation
       * \param
       */
      void computeVVector(const int NbPts);

      /*! 
       * \brief interpolates the surface with node-data from body
       */
      void computeSurface();

      /*!
       * \brief interpolates the velocities of the surface with the node-data from the body
       */
      void computeSurfaceVelocities();

      /*! 
       * \brief interpolates the Jacobians of translation of the surface with the node-data from the body
       */
      void computeSurfaceJacobians();

      /*!
       * \return control point
       * \param u location
       * \param v location
       */
      fmatvec::Vec getControlPoints(const int u, const int v);

      /*! 
       * return U-Vector of the surface (azimuthal direction)
       */
      fmatvec::Vec getUVector();

      /*! 
       * return V-Vector of the surface (radial direction)
       */
      fmatvec::Vec getVVector();

      /*! 
       * \return flag, whether the input radius is inside the bounds or the input angle is between 0 and 2 PI
       * \param parametrisation vector
       */
      int testInsideBounds(const fmatvec::Vec &s);

      /*! 
       * \return norm of the difference between two vectors
       * \param first vector
       * \param second vector
       */
      double computeError(const fmatvec::Vec &Vec1, const fmatvec::Vec &Vec2);

    protected:
      /** 
       * \brief number of reference dofs of the flexible body
       */
      int RefDofs;

      /**
       * \brief number of elements in azimuthal and radial direction
       */
      int nj, nr;

      /**
       * \brief interpolation degree azimuthal and radial
       */
      int degU, degV;

      /**
       * \brief inner and outer radius
       */
      double Ri, Ra;

      std::shared_ptr<OpenMBV::NurbsDisk> openMBVNurbsDisk;
      double drawDegree;

      /** 
       * \brief interpolated surface of the contour
       */
      PlNurbsSurfaced *Surface; // an homogenous surface with accuracy of calculation of double (HSurface<double,3>)

      /** 
       * \brief interpolated velocities of the surface-points
       */
      PlNurbsSurfaced *SurfaceVelocities;

      /**
       * \brief interpolated Jacobians of Translation on the surface 
       */
      std::vector<PlNurbsSurfaced> SurfaceJacobiansOfTranslation; // size = number of generalized coordinates

      /**
       * \brief interpolated Jacobians of Rotation on the surface
       */
      std::vector<PlNurbsSurfaced> SurfaceJacobiansOfRotation; // size = number of generalized coordinates

      /**
       * \brief knot vectors, used for the U und V coordinates of the surface
       */
      PLib::Vector<double> *uvec; // nurbs++ needs this vector 
      PLib::Vector<double> *uVec; // knot-vector for azimuthal direction 
      PLib::Vector<double> *vvec; // nurbs++ needs this vector
      PLib::Vector<double> *vVec; // knot-vector for radial direction
  };

}

#endif /* NURBSDISK2S_H_ */
