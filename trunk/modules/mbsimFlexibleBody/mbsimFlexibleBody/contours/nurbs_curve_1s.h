/* Copyright (C) 2004-2012 MBSim Development Team
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

#ifndef NURBSCURVE1S_H_
#define NURBSCURVE1S_H_

#include "fmatvec/fmatvec.h"
#include "mbsim/mbsim_event.h"
#include "mbsim/contours/contour1s.h"

#ifdef HAVE_NURBS
#include "nurbs++/nurbs.h"
#include "nurbs++/nurbsS.h"
#include "nurbs++/vector.h"
#endif

namespace MBSimFlexibleBody {

  /*!  
   * \brief contour 1s flexible with NURBS parametrization
   * \author Thorsten Schindler
   * \date 2011-10-16 initial commit (Thorsten Schindler)
   * \date 2012-03-15 updateKinematicsForFrame and contact Jacobians (Cebulla / Schindler)
   * \date 2013-02-04 contour now for 2D and 3D Cosserat beam (Zitzewitz / Cebulla / Schindler)
   */
  class NurbsCurve1s : public MBSim::Contour1s {
    public:
      /**
       * \brief constructor 
       * \param name of contour
       */
      NurbsCurve1s(const std::string &name);

      /**
       * \brief destructor
       */
      virtual ~NurbsCurve1s();  

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "NurbsCurve1s"; }
      /***************************************************/

      /* INHERITED INTERFACE OF CONTOURCONTINUUM */
      virtual void computeRootFunctionPosition(MBSim::ContourPointData &cp) { throw MBSim::MBSimError("ERROR(NurbsCurve1s::computeRootFunctionPosition): Not implemented!"); }
      virtual void computeRootFunctionFirstTangent(MBSim::ContourPointData &cp) { throw MBSim::MBSimError("ERROR(NurbsCurve1s::computeRootFunctionFirstTangent): Not implemented!"); }
      virtual void computeRootFunctionNormal(MBSim::ContourPointData &cp) { throw MBSim::MBSimError("ERROR(NurbsCurve1s::computeRootFunctionNormal): Not implemented!"); }
      virtual void computeRootFunctionSecondTangent(MBSim::ContourPointData &cp) { throw MBSim::MBSimError("ERROR(NurbsCurve1s::computeRootFunctionSecondTangent): Not implemented!"); }
      /***************************************************/

      /* INHERITED INTERFACE OF CONTOUR */
      virtual void updateKinematicsForFrame(MBSim::ContourPointData &cp, MBSim::FrameFeature ff);
      virtual void updateJacobiansForFrame(MBSim::ContourPointData &cp, int j=0);
      virtual MBSim::ContactKinematics *findContactPairingWith(std::string type0, std::string type1) { throw MBSim::MBSimError("ERROR(NurbsCurve1s::findContactPairingWith): Not implemented!"); }
      /***************************************************/

      /* GETTER / SETTER */
#ifdef HAVE_NURBS
      void setNormalRotationGrid(fmatvec::Vec normal_) { normalRotationGrid.x() = normal_(0), normalRotationGrid.y() = normal_(1),normalRotationGrid.z() = normal_(2); }
#endif
      /***************************************************/

#ifdef HAVE_NURBS
      /**
       * \brief initialize NURBS curve 
       * \param stage of initialisation
       */
      void initContourFromBody(MBSim::InitStage stage);
#endif


#ifdef HAVE_NURBS
      /*! 
       * \brief computes the U-Vector
       * \param number of points
       */
      void computeUVector(const int NbPts); 
#endif

#ifdef HAVE_NURBS
      /*! 
       * \brief interpolates the translations with node-data from body
       */
      void computeCurveTranslations();
#endif

#ifdef HAVE_NURBS
      /*!
       * \brief interpolates the velocities with the node-data from the body
       */
      void computeCurveVelocities();
#endif

#ifdef HAVE_NURBS
      /*!
       * \brief interpolates the angular velocities with the node-data from the body
       */
      void computeCurveAngularVelocities();
#endif

#ifdef HAVE_NURBS
      /*!
       * \brief interpolates the Jacobians of translation with the node-data from the body
       * \param interpolate translational jacobian
       * \param interpolate rotational jacobian
       */
      void computeCurveJacobians(bool trans = true, bool rot = true);
#endif

    protected:
      /**
       * \brief number of elements
       */
      int Elements;

      /**
       * \brief number of DOFs
       */
      int qSize;

      /**
       * \brief open or closed beam structure
       */
      bool openStructure;

      /**
       * \brief length of entire curve
       */
      double L;

      /**
       * \brief interpolation degree
       */
      int degU;

#ifdef HAVE_NURBS
      /** 
       * \brief interpolated translations of the contour
       */
      PlNurbsCurved *curveTranslations;

      /** 
       * \brief interpolated velocities of the contour
       */
      PlNurbsCurved *curveVelocities;

      /**
       * \brief interpolated angular velocities of the contour
       */
      PlNurbsCurved *curveAngularVelocities;

      /**
       * \brief closest normal on rotation grid to update direction of normal of nurbs-curve and to avoid jumping
       */
      PLib::Point3Dd normalRotationGrid;

      /**
       * \brief Jacobians of Translation of finite element nodes
       */
      std::vector<MBSim::ContourPointData> jacobiansTrans; // size = number of interpolation points

      /**
       * \brief Jacobians of Rotation of finite element nodes
       */
      std::vector<MBSim::ContourPointData> jacobiansRot; // size = number of interpolation points

      /**
       * \brief interpolated Jacobians of Translation of the contour
       */
      std::vector<PlNurbsCurved> CurveJacobiansOfTranslation; // size = number of generalized coordinates

      /**
       * \brief interpolated Jacobians of Rotation on the contour
       */
      std::vector<PlNurbsCurved> CurveJacobiansOfRotation; // size = number of generalized coordinates

      /**
       * \brief knot vector
       */
      PLib::Vector<double> *uvec; // nurbs++ needs this vector
      PLib::Vector<double> *uVec; // knot-vector
#endif
  };

}

#endif /* NURBSCURVE1S_H_ */

