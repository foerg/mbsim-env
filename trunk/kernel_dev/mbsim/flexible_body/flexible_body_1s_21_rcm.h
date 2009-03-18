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
 * Contact: rzander@users.berlios.de
 *          thschindler@users.berlios.de
 */

#ifndef _FLEXIBLE_BODY_1S_21_RCM_H_
#define _FLEXIBLE_BODY_1S_21_RCM_H_

#include <mbsim/flexible_body.h>

namespace AMVis { class ElasticBody1s21RCM; }

namespace MBSim {

  class FiniteElement1s21RCM;
  class Contour1sFlexible;

  /**
   * \brief model for planar beams with large deflection using Redundant Coordinate Method (RCM)
   * \author Roland Zander
   * \author Thorsten Schindler
   * \date 18.03.09
   *
   * read:\n
   * Zander, R.; Ulbrich, H.: Reference-free mixed FE-MBS approach for beam structures with constraints, Journal of Nonlinear Dynamics, Kluwer Academic Publishers, 2005 \n
   * Zander, R.; Ulbrich, H.: Impacts on beam structures: Interaction of wave propagationand global dynamics, IUTAM Symposium on Multiscale Problems in Multibody System Contacts Stuttgart, Germany, 2006 \n
   * Zander, R.; Ulbrich, H.: Free plain motion of flexible beams in MBS - A comparison of models, III European Conference on Computational Mechanics Lissbon, Portugal, 2006
   */
  class FlexibleBody1s21RCM : public FlexibleBody1s {
    public:
      /**
       * \brief constructor:
       * \param name of body
       * \param bool to specify open (cantilever) or closed (ring) structure
       */
      FlexibleBody1s21RCM(const string &name, bool openStructure);

      /**
       * \brief destructor
       */
      virtual ~FlexibleBody1s21RCM() {}

      /* INHERITED INTERFACE */
      /* FLEXIBLEBODY */
      virtual void GlobalMatrixContribution(int n);
      virtual Mat computeJacobianMatrix(const ContourPointData &S);
      virtual Frame* computeKinematicsForFrame(const ContourPointData &S_);

      /* OBJECT */
      virtual void init();
      virtual double computePotentialEnergy();

      /* OBJECTINTERFACE */
      virtual void updateKinematics(double t);
      /***************************************************/

      /* GETTER / SETTER */
      /**
       * \brief sets size of positions and velocities
       */
      void setNumberElements(int n); 
      void setLength(double L_) { L = L_; }
      void setEModul(double E_) { E = E_; }
      void setCrossSectionalArea(double A_) { A = A_; }
      void setMomentInertia(double I_) { I = I_; }
      void setDensity(double rho_) { rho = rho_; }
      void setCurlRadius(double r);
      void setMaterialDamping(double d);
      void setLehrDamping(double d);

      /**
       * \param name of frame
       * \param node number of finite element
       */
      void addFrame(const string &name, const int &node);
      /***************************************************/

      /**
       * \param contour parameter
       * \return state (position and velocities) of cross-section
       */
      Vec computeState(const double &s);

      /**
       * \brief initialise beam state concerning a straight cantilever setting
       * \param TODO
       */
      void initRelaxed(double alpha);

#ifdef HAVE_AMVIS
      /**
       * \param radius of cylinder for AMVis visualisation
       */
      void setAMVisCylinder(float r) { boolAMVis=true; AMVisRadius=r; }

      /**
       * \param breadth of cuboid for AMVis visualisation
       * \param height of cuboid for AMVis visualisation
       */
      void setAMVisCuboid(float breadth, float height) { boolAMVis=true; AMVisBreadth=breadth; AMVisHeight=height; }
#endif

    protected:
      /** 
       * \brief number of finite elements used for discretisation
       */
      int Elements;

      /**
       * \brief length of beam
       */ 
      double L;

      /** 
       * \brief length of one finite element
       */
      double l0;

      /** 
       * \brief modulus of linear elasticity
       */
      double E;

      /**
       * \brief cross-section area
       */
      double A;

      /**
       * \brief moment of inertia of cross-section
       */
      double I;

      /** 
       * \brief material density
       */
      double rho;

      /** 
       * \brief curl radius
       */
      double rc;

      /**
       * \brief coefficient of material damping
       */
      double dm;

      /** 
       * \brief coefficient of Lehr-damping
       */
      double dl;

      /** 
       * \brief flag for open (cantilever beam) or closed (rings) structures
       */ 
      bool openStructure;

      /** 
       * \brief true if terms for implicit time integration shall be computed, unused so far
       */ 
      bool implicit;

      int CurrentElement;
      SqrMat Dhq, Dhqp;

      void BuildElements();
      double BuildElement(const double&);


      bool initialized;
      double alphaRelax0, alphaRelax;

      double sTangent;

      /** right and left side contour of body: defined using binormal of contour */
      Contour1sFlexible *contourR, *contourL;

      void updateJh_internal(double t);

#ifdef HAVE_AMVIS
      float AMVisRadius, AMVisBreadth, AMVisHeight;
#endif

  };

}

#endif

