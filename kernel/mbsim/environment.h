/* Copyright (C) 2004-2009 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: friedrich.at.gc@googlemail.com
 */

#ifndef _MBSIM_ENVIRONMENT_H_
#define _MBSIM_ENVIRONMENT_H_

#include "fmatvec/fmatvec.h"
#include "fmatvec/atom.h"
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNode.hpp>

namespace OpenMBV {
  class Object;
}

namespace MBSim {

  /**
   * \brief basic singleton (see GAMMA et al.) class to capsulate environment variables for XML
   * \author Markus Friedrich
   * \date 2009-07-28 some comments (Thorsten Schindler)
   */
  class Environment : virtual public fmatvec::Atom {
    public:
      /* INTERFACE FOR DERIVED CLASSES */
      /**
       * \brief initializes environment variables by XML element
       * \param XML element
       */
      virtual void initializeUsingXML(xercesc::DOMElement *element) {}
      /***************************************************/

    protected:
      /**
       * \brief constructor
       */
      Environment()  {};

      /**
       * \brief destructor
       */
      ~Environment() override = default;
  };

  /**
   * \brief singleton class (see GAMMA et al.) to capsulate environment variables for XML multibody systems
   * \author Markus Friedrich
   * \date 2009-07-28 some comments (Thorsten Schindler)
   */
  class MBSimEnvironment : public Environment {
    public:
      /* INHERITED INTERFACE */
      void initializeUsingXML(xercesc::DOMElement *element) override;
      /***************************************************/

      /* GETTER / SETTER */
      static MBSimEnvironment *getInstance() { return instance.get(); }
      void setAccelerationOfGravity(const fmatvec::Vec3 &grav_) { grav=grav_; }
      const fmatvec::Vec3& getAccelerationOfGravity() const { return grav; }

      void addOpenMBVObject(const std::shared_ptr<OpenMBV::Object> &object);
      std::vector<std::shared_ptr<OpenMBV::Object>> getOpenMBVObjects();
      /***************************************************/
    
    private:
      /**
       * class pointer to ensure singleton status
       */
      static std::unique_ptr<MBSimEnvironment> instance;
      
    protected:
      /**
       * \brief constructor
       */
      MBSimEnvironment()  {}

      /**
       * \brief acceleration of gravity
       */
      fmatvec::Vec3 grav;

      std::vector<std::shared_ptr<OpenMBV::Object>> openMBVObject;
  };

}

#endif /* _MBSIM_ENVIRONMENT_H_ */

