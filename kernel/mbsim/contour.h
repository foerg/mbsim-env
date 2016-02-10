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
 *          rzander@users.berlios.de
 */

#ifndef _CONTOUR_H_
#define _CONTOUR_H_

#include "mbsim/element.h"
#include "mbsim/contour_pdata.h"
#include "mbsim/frame.h"
#include "mbsim/mbsim_event.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
namespace OpenMBV {
  class RigidBody;
}
#endif

namespace MBSim {

  class Object;
  class ContourPointData;
  class ContactKinematics;

  /**
   * \brief basic class for contour definition for rigid (which do not know about their shape) and flexible (they know how they look like) bodies
   * \author Martin Foerg
   * \date 2009-03-23 some comments (Thorsten Schindler)
   * \date 2009-04-20 RigidContour added (Thorsten Schindler)
   * \date 2009-06-04 not rigid things are in separate files
   * \date 2009-07-16 split from concret contours into new folder contours
   *
   * kinematics is stored in coordinate system class and is individually evaluated in specific contact kinematics
   */
  class Contour : public Element {
    public:
      /**
       * \brief constructor
       * \param name of contour
       */
      Contour(const std::string &name, Frame *R=0);

      /**
       * \brief destructor
       */
      virtual ~Contour();

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "Contour"; }
      virtual void plot(double t, double dt = 1);
      /***************************************************/

      /**
       * \brief TODO
       */
      virtual void init(InitStage stage);
      /***************************************************/

      /**
       * \brief find contact kinematics
       * \author Markus Schneider
       * \date 2010-11-05 initial commit (Markus Schneider)
       */
      virtual ContactKinematics * findContactPairingWith(std::string type0, std::string type1) = 0;

      /* GETTER / SETTER */
      Frame* getFrame() { return R; }
      Frame* getFrameOfReference() { return R; }
      virtual Frame* createContourFrame(const std::string &name="P");

      virtual int gethSize(int i=0) const { return hSize[i]; }
      virtual int gethInd(int i=0) const { return hInd[i]; }
      virtual void sethSize(int size, int i=0) { hSize[i] = size; }
      virtual void sethInd(int ind, int i=0) { hInd[i] = ind; }

      void setFrameOfReference(Frame *frame) { R = frame; }
      void setFrameOfReference(const std::string &frame) { saved_frameOfReference = frame; }

      virtual fmatvec::Vec3 getKrPS(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getKrPS): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getKs(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getKs): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getKt(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getKt): Not implemented.");
        return 0;
      }
      virtual fmatvec::Vec3 getParDer1Ks(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getParDer1Ks): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getParDer2Ks(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getParDer2Ks): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getParDer1Kt(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getParDer1Kt): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getParDer2Kt(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getParDer2Kt): Not implemented.");
        return 0;
      }

      /**
       * \return position in world frame
       * \param contour position
       */
      virtual fmatvec::Vec3 getPosition(double t, const fmatvec::Vec2 &zeta);

      virtual fmatvec::Vec3 getVelocity(double t, const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getVelocity): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getAngularVelocity(double t, const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getAngularVelocity): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getAcceleration(double t, const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getAcceleration): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getAngularAcceleration(double t, const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getAngularAcceleration): Not implemented.");
        return 0;
      }

      virtual fmatvec::Mat3xV getJacobianOfTranslation(double t, const fmatvec::Vec2 &zeta, int j=0) {
        THROW_MBSIMERROR("(Contour::getJacobianOfTranslation): Not implemented.");
        return 0;
      }

      virtual fmatvec::Mat3xV getJacobianOfRotation(double t, const fmatvec::Vec2 &zeta, int j=0) {
        THROW_MBSIMERROR("(Contour::getJacobianOfRotation): Not implemented.");
        return 0;
      }

      virtual fmatvec::MatV getJacobianOfDeformation(double t, const fmatvec::Vec2 &zeta, int j=0) {
        THROW_MBSIMERROR("(Contour::getJacobianOfDeformation): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getGyroscopicAccelerationOfTranslation(double t, const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getGyroscopicAccelerationOfTranslation): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getGyroscopicAccelerationOfRotation(double t, const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getGyroscopicAccelerationOfRotation): Not implemented.");
        return 0;
      }

      /**
       * \return first tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getWu(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return second tangent in world frame
       * \param Lagrangian position
       */
      virtual fmatvec::Vec3 getWv(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return normal in world frame
       * \param contour position
       */
      virtual fmatvec::Vec3 getWn(double t, const fmatvec::Vec2 &zeta);


      virtual fmatvec::Vec3 getParDer1Kn(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getParDer1Kn): Not implemented.");
        return 0;
      }

      virtual fmatvec::Vec3 getParDer2Kn(const fmatvec::Vec2 &zeta) {
        THROW_MBSIMERROR("(Contour::getParDer2Kn): Not implemented.");
        return 0;
      }

      /**
       * \return derivative of first tangent
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer1Ku(const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of first tangent
       * \param t time
       * \param cp contour position
       */

      virtual fmatvec::Vec3 getParDer2Ku(const fmatvec::Vec2 &zeta);
      /**
       * \return derivative of second tangent
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer1Kv(const fmatvec::Vec2 &zeta);

      /**
       * \return derivative second first tangent
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer2Kv(const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of normal in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer1Wn(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of normal in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer2Wn(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of first tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer1Wu(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of first tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer2Wu(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of second tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer1Wv(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of second tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getParDer2Wv(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return derivative of first tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getWrPS(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return first tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getWs(double t, const fmatvec::Vec2 &zeta);

      /**
       * \return second tangent in world frame
       * \param t time
       * \param cp contour position
       */
      virtual fmatvec::Vec3 getWt(double t, const fmatvec::Vec2 &zeta);

      virtual fmatvec::Mat3x2 getWN(double t, const fmatvec::Vec2 &zeta);

      virtual fmatvec::Mat3x2 getWR(double t, const fmatvec::Vec2 &zeta);

      virtual fmatvec::Mat3x2 getWU(double t, const fmatvec::Vec2 &zeta);

      virtual fmatvec::Mat3x2 getWV(double t, const fmatvec::Vec2 &zeta);

      virtual fmatvec::Vec2 getContourParameters(double t, const fmatvec::Vec3 &WrPS) {
        THROW_MBSIMERROR("(Contour::getContourParameters): Not implemented.");
        return 0;
      }

      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element);

    protected:
      /**
       * \brief size and index of right hand side for frame JACOBIAN settings
       */
      int hSize[2], hInd[2];

      /**
       * \brief coordinate system of contour
       */
      Frame *R;

      std::string saved_frameOfReference;
  };

  /**
   * \brief basic class for rigid contours
   * \author Thorsten Schindler
   * \date 2009-04-20 initial commit (Thorsten Schindler)
   * \date 2009-07-15 initPlot (Thorsten Schindler)
   */
  class RigidContour : public Contour {
    public:
      /**
       * \brief constructor
       * \param name of point
       */
      RigidContour(const std::string &name, Frame *R=0) : Contour(name,R) {}

      virtual ~RigidContour();

      Frame* createContourFrame(const std::string &name="P");

      /* INHERITED INTERFACE OF ELEMENT */
      std::string getType() const { return "RigidContour"; }
      virtual void plot(double t, double dt = 1);
      virtual void init(InitStage stage);
      /***************************************************/

      /**
       * \brief contact search for RigidContours
       * \author Markus Schneider
       * \date 2010-11-05 initial commit (Markus Schneider)
       */
      ContactKinematics * findContactPairingWith(std::string type0, std::string type1);

#ifdef HAVE_OPENMBVCPPINTERFACE
      boost::shared_ptr<OpenMBV::RigidBody>& getOpenMBVRigidBody() {
        return openMBVRigidBody;
      }

      void setOpenMBVRigidBody(const boost::shared_ptr<OpenMBV::RigidBody> &ombvBody) {
        openMBVRigidBody = ombvBody;
      }
#endif

    protected:
#ifdef HAVE_OPENMBVCPPINTERFACE
      boost::shared_ptr<OpenMBV::RigidBody> openMBVRigidBody;
#endif

  };
}

#endif /* _CONTOUR_H_ */

