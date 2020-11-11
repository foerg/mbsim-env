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

#ifndef _COMPOUND_CONTOUR_H_
#define _COMPOUND_CONTOUR_H_

#include "mbsim/contours/rigid_contour.h"

namespace MBSim {

  class FixedRelativeFrame;

  /**
   * \brief contour consisting of primitive contour elements
   * \author Martin Foerg
   * \date 2009-04-20 some comments (Thorsten Schindler) 
   */
  class CompoundContour : public RigidContour {
    public:
      /**
       * \brief constructor
       * \param name of contour
       */
      CompoundContour(const std::string &name="", Frame *R=nullptr);

      /**
       * \brief destructor
       */
      ~CompoundContour() override;

      /* INHERITED INTERFACE OF ELEMENT */
      void plot() override;
      std::shared_ptr<OpenMBV::Group> getOpenMBVGrp() override { return openMBVGroup; }
      std::shared_ptr<OpenMBV::Group> getContoursOpenMBVGrp() override { return getOpenMBVGrp(); }
      H5::GroupBase *getContoursPlotGroup() override { return getPlotGroup(); }
      /***************************************************/

      void init(InitStage stage, const InitConfigSet &config) override;
      Contour* getContourElement(int i) {
        return element[i];
      }
      Contour* getContour(int i) { return element[i]; }
      void addContour(RigidContour* c);
      void addFrame(FixedRelativeFrame* f);
      unsigned int getNumberOfElements() { return element.size(); }

      void resetUpToDate() override;

    protected:
      /*!
       * \brief list of all subelements
       */
      std::vector<RigidContour*> element;

      /*!
       * \brief List of all frames on the contour
       */
      std::vector<FixedRelativeFrame*> frame;

      /*!
       * \brief Orientations of the single elements in the contour frame
       */
      std::vector<fmatvec::SqrMat3> AIK;

      std::shared_ptr<OpenMBV::Group> openMBVGroup;
  };
}

#endif /* _COMPOUND_CONTOUR_H_ */

