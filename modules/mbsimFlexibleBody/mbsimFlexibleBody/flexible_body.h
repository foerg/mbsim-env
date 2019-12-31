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
 *          martin.o.foerg@googlemail.com
 */

#ifndef _FLEXIBLE_BODY_H_
#define _FLEXIBLE_BODY_H_

#include "mbsimFlexibleBody/node_based_body.h"

namespace MBSim {
  class FixedRelativeFrame;
  class ContourFrame;
}

namespace MBSimFlexibleBody {

  class DiscretizationInterface;

  /**
   * \brief upmost class for flexible body implementation
   * \author Roland Zander
   * \author Thorsten Schindler
   * \date 2009-03-24 changes for new MBSim (Thorsten Schindler)
   * \date 2009-03-25 conflicts solved (Thorsten Schindler)
   * \date 2009-04-05 changed to non-template definition (Schindler / Zander)
   * \date 2009-04-20 frame concept (Thorsten Schindler)
   * \date 2009-06-14 OpenMP (Thorsten Schindler)
   * \date 2009-07-16 splitted link / object right hand side (Thorsten Schindler)
   * \date 2009-07-27 implicit integration (Thorsten Schindler)
   * \date 2010-06-20 revision of doxygen comments: add parameter names (Roland Zander)
   * \todo OpenMP only static scheduling with intelligent reordering of vectors by dynamic test runs TODO
   * \todo mass proportional damping should be distributed on discretization and is not at the correct place (dependence on M) TODO
   */
  class FlexibleBody : public NodeBasedBody {
    public:
      /**
       * \brief constructor
       * \param name of body
       */
      FlexibleBody(const std::string &name);

      /**
       * \brief destructor
       */
      ~FlexibleBody() override;

      /* INHERITED INTERFACE OF OBJECTINTERFACE */
      void updateqd() override { qd = u; }
      void updateh(int k=0) override;
      void updateM() override;
      void updatedhdz() override;

      /* INHERITED INTERFACE OF ELEMENT */
      void initializeUsingXML(xercesc::DOMElement *element) override;
      /***************************************************/

      /* INHERITED INTERFACE OF OBJECT */
      void init(InitStage stage, const MBSim::InitConfigSet &config) override;
      virtual double computeKineticEnergy();
      virtual double computePotentialEnergy();
      void setFrameOfReference(MBSim::Frame *frame) override;
      virtual void setq0(fmatvec::Vec q0_) { if(q0_.size()) MBSim::Body::setGeneralizedInitialPosition(q0_); q <<= q0; }
      virtual void setu0(fmatvec::Vec u0_) { if(u0_.size()) MBSim::Body::setGeneralizedInitialVelocity(u0_); u <<= u0; }
      /***************************************************/

      /* INTERFACE TO BE DEFINED IN DERIVED CLASSES */
      /**
       * \brief references finite element coordinates to assembled coordinates
       */
      virtual void BuildElements() = 0;

      const fmatvec::Vec& getqElement(int i) { if(updEle) BuildElements(); return qElement[i]; }

      const fmatvec::Vec& getuElement(int i) { if(updEle) BuildElements(); return uElement[i]; }

      virtual fmatvec::Vec3 getAngles(int i) { return fmatvec::Vec3(); }
      virtual fmatvec::Vec3 getDerivativeOfAngles(int i) { return fmatvec::Vec3(); }

      /**
       * \brief insert 'local' information in global vectors
       * \param number of finite element
       * \param local vector
       * \param global vector
       */
      virtual void GlobalVectorContribution(int CurrentElement, const fmatvec::Vec &locVec, fmatvec::Vec &gloVec) = 0;

      /**
       * \brief insert 'local' information in global matrices
       * \param CurrentElement number of current finite element
       * \param locMat local matrix
       * \param gloMat global matrix
       */
      virtual void GlobalMatrixContribution(int CurrentElement, const fmatvec::Mat &locMat, fmatvec::Mat &gloMat) = 0;

      /**
       * \brief insert 'local' information in global matrices
       * \param CurrentElement number of current finite element
       * \param locMat local matrix
       * \param gloMat global matrix
       */
      virtual void GlobalMatrixContribution(int CurrentElement, const fmatvec::SymMat &locMat, fmatvec::SymMat &gloMat) = 0;

//      /**
//       * \brief cartesian kinematic for contour or external frame (normal, tangent, binormal) is set by implementation class
//       * \param data contour parameter
//       * \param ff selection of specific calculations for frames
//       * \param frame optional: external frame, otherwise contour parameters are changed
//       */
//      virtual void updateKinematicsForFrame(MBSim::ContourPointData &data, MBSim::Frame::Frame::Feature ff, MBSim::Frame *frame=0) = 0;
//
//      /*!
//       * \brief cartesian kinematic on a node
//       */
//      virtual void updateKinematicsAtNode(NodeFrame *frame, MBSim::Frame::Feature ff) {
//    	  throwError("updateKinematicsAtNode(): Not implemented for " + typid(*this).name()); //TODO: make that interface prettier
//      }
//
//      /**
//       * \brief Jacobians and gyroscopes for contour or external frame are set by implementation class
//       * \param data contour parameter
//       * \param frame: optional external frame, otherwise contour parameters are changed
//       */
//      virtual void updateJacobiansForFrame(MBSim::ContourPointData &data, MBSim::Frame *frame=0) = 0;
//      /***************************************************/

      /* GETTER / SETTER */
      /*!
       * damping matrix computation, updated with changes in mass matrix \f$\vM\f$: \f$\vh_d=-d_{pm}\vM\vu\f$
       * \brief set mass proportional damping
       * \param d_ coefficient \f$d_{pm}\f$
       */
      void setMassProportionalDamping(const double d_) { d_massproportional = d_; }
      /***************************************************/

      using NodeBasedBody::addFrame;

      void addFrame(MBSim::ContourFrame *frame);

      /**
       * \param fixed relative frame that should be added
       */
      void addFrame(MBSim::FixedRelativeFrame *frame);

      void addContour(MBSim::Contour *contour) override;

      /**
       * \brief interpolates the position and optional the velocity coordinates of the flexible body with Nurbs-package and exports the nurbs curve in the specified file
       * \param filenamePos    Name of the exported position curve file
       * \param filenameVel    Name of the exported velocity curve file
       * \param deg            Degree of Nurbs interpolation
       * \param writePsFile A Postscript-file of the curve profile is created
       *
       * Remark: the knot vector is parametrized between [0,L]
       */
      virtual void exportPositionVelocity(const std::string & filenamePos, const std::string & filenameVel = std::string(), const int & deg = 3, const bool &writePsFile = false){throwError("exportPositionVelocity(const std::string& filenamePos, const std::string& filenameVel, const int& deg, const bool& writePsFile) is not implemented for " + boost::core::demangle(typeid(*this).name())) ;}

      /**
       * \brief imports the interpolated position and optional the velocity files (created with exportPositionVelocity) and fits the rigid and flexible coordinate dofs and optional the translatory velocity components of flexible body to the imported nurbs curve
       * \param filenamePos    Name of the imported position curve file
       * \param filenameVel    Name of the imported velocity curve file
       */
      virtual void importPositionVelocity(const std::string& filenamePos, const std::string& filenameVel = std::string()){throwError("importPositionVelocity(const std::string& filenamePos, const std::string& filenameVel) is not implemented for " + boost::core::demangle(typeid(*this).name())) ;}

      void resetUpToDate() override;

    protected:
      /**
       * \brief stl-vector of discretizations/finite elements
       */
      std::vector<DiscretizationInterface*> discretization;

      /**
       * \brief stl-vector of finite element wise positions
       */
      std::vector<fmatvec::Vec> qElement;

      /**
       * \brief stl-vector of finite element wise velocities
       */
      std::vector<fmatvec::Vec> uElement;

      /**
       * \brief damping factor for mass proportion, see BodyFlexible::setMassProportionalDamping()
       */
      double d_massproportional;

      /**
       * \brief vector of contour parameters each describing a frame
       */
//      std::vector<MBSim::ContourPointData> S_Frame;

      // Workaround to free memory of contourFrame in dtor.
      // TODO: provide a consistent solution and remove the following line
//      MBSim::Frame *contourFrame;

      /*!
       * \brief list of all contour frames
       * \todo: actually continous frames should be added to a contour and not to the body?!
       */

      bool updEle;
  };

  /**
   * \brief flexible body entirely described within MBSim holding all informations about continuum approximations
   * \author Roland Zander
   * \author Thorsten Schindler
   * \date 2009-04-05 initial definition (Schindler / Zander)
   */
  template <class AT>
    class FlexibleBodyContinuum : public FlexibleBody {
      public:
        /**
         * \brief constructor
         * \param name of flexible body
         */
        FlexibleBodyContinuum<AT>(const std::string &name) : FlexibleBody(name) {}

        /* INHERITED INTERFACE OF ELEMENT */

        /* GETTER / SETTER */
        void setContourNodes(const std::vector<AT> nodes) { userContourNodes = nodes; }

        void setNodeOffset(const AT nodeOffset_){ nodeOffset = nodeOffset_;}  // TODO:: call this function in the init() of flexible body.
        AT getNodeOffset() const { return nodeOffset;}

      protected:
        /**
         * \brief grid for contact point detection
         */
        std::vector<AT> userContourNodes;

        /**
         * \brief offset of the ROTNODE from the TRANSNODE
         */
        AT nodeOffset;
    };
}

#endif /* _FLEXIBLE_BODY_H_ */
