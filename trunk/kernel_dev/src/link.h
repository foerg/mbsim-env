/* Copyright (C) 2004-2006  Martin Förg
 
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
 *
 */

#ifndef _LINK_H_
#define _LINK_H_

#include <vector>
#include "element.h"
#include "contour_pdata.h"

#ifdef HAVE_AMVIS
namespace AMVis {class Arrow;}
#endif

namespace MBSim {
  class Object;
  class CoordinateSystem;
  class Contour;
  class MultiBodySystem;
  class HitSphereLink;
  class UserFunction;
  class Subsystem;
  struct ContourPointData;
  /*! 
   *  \brief This is a general link to one or more objects.
   * 
   * */
  class Link : public Element {

    protected:
      //Subsystem* parent;

      /** Internal integrable State Variables */
      Vec x;
      /** Internal integrable State Variables velocities ud \see updatedu(double t, double dt), updateud(double t) */
      Vec xd;
      Vec x0;

      int xSize;
      int xInd;

      int svSize;
      Vec sv;
      int svInd;
      Vector<int> jsv;

      bool setValued;

      int gSize, gInd;
      int gdSize, gdInd;
      int laSize, laInd;
      int laIndMBS;

      vector<Vec> WF, WM;
      vector<Mat> fF, fM;

      int rFactorSize, rFactorInd;
      Vec g, gd, gdn, la, s, res;

      Vec rFactor;
      Vector<int> rFactorUnsure;

      //bool active;
      Index Ig, Ila;
      Vec la0;

      double scaleTolQ,scaleTolp;
      double gdTol, gddTol, laTol, LaTol, rMax;

      HitSphereLink* HSLink;
      bool checkHSLink;

      vector<Mat> W;
      vector<Mat> V;
      vector<Vec> h;
      vector<Vec> r;
      Vec b;

      /*! Array in which all ports are listed, connecting bodies via a Link.
      */
      vector<CoordinateSystem*> port;

      /** Array in which all contours linked by LinkContour are managed.*/
      vector<Contour*> contour;

      vector<ContourPointData> cpData;

#ifdef HAVE_AMVIS
      vector<AMVis::Arrow*> arrowAMVis;
      vector<double> arrowAMVisScale;
      vector<int> arrowAMVisID;
      vector<bool> arrowAMVisMoment;
      vector<UserFunction*> arrowAMVisUserFunctionColor;
#endif

    public:

      virtual void updater(double t);
      virtual void updateb(double t);
      virtual void updateW(double t);
      virtual void updateV(double t) {}
      virtual void updateh(double t);
      virtual void updateWRef(const Mat& ref);
      virtual void updateVRef(const Mat& ref);
      virtual void updatehRef(const Vec &ref);
      virtual void updaterRef(const Vec &ref);
      virtual void updatebRef(const Vec &ref);
      virtual void updatefRef(const Vec &ref) {};

      Link(const string &name, bool setValued);
      ~Link();

      //void setParent(Subsystem *parent_) {parent = parent_;}

      //string getFullName() const; 

      const vector<Mat>& getW() const {return W;}
      const vector<Mat>& getV() const {return V;}
      const vector<Vec>& geth() const {return h;}

     // Object* getObject(int id) { return object[id]; }
     // void addObject(Object* obj) { object.push_back(obj); }
     // int getNumObjects() const { return object.size(); }

      void setxInd(int xInd_) {xInd = xInd_;};
      void setsvInd(int svInd_) {svInd = svInd_;};

      int getlaIndMBS() const {return laIndMBS;}
      void setlaIndMBS(int laIndParent) { laIndMBS = laInd + laIndParent; }

      int getxSize() const {return xSize;}
      int getsvSize() const {return svSize;}

      /*! activate HitSphereLink-check for this Link before updateStage1(), only usefull for Contacts
       * 	\param checkHSLink_, true for checks, false for no check
       */
      void setHitSphereCheck(bool checkHSLink_) {checkHSLink=checkHSLink_;}
      /*! \return HSLink for MultiBodySystem
       *  */
      bool getHitSphereCheck() {return checkHSLink;}

      virtual void calcxSize() {xSize = 0;}
      virtual void calcsvSize() {svSize = 0;}
      virtual void calclaSize() {laSize = 0;}
      virtual void calcgSize() {gSize = 0;}
      virtual void calcgdSize() {gdSize = 0;}
      virtual void calcrFactorSize() {rFactorSize = 0;}

      virtual void init();
      virtual void initz();

      virtual void updateg(double t) = 0;
      virtual void updategd(double t) = 0; 

      /*! compute potential energy, holding every potential!!!
      */
      virtual double computePotentialEnergy() {return 0;}

      /*! Supplies time variation of x to a fixed step solver.*/
      virtual void updatedx(double t, double dt) {};
      /*! Supplies the time derivative of x to a variable step solver.*/
      virtual void updatexd(double t) {};

      virtual void updateStopVector(double t) {}

      const Vec& getx() const {return x;}
      const Vec& getxd() const {return xd;}
      /*! Sets the internal states of a Link.*/
      void setx(const Vec &x_) {x = x_;}

      void plot(double t, double dt=1);
      void initPlotFiles();

      //bool isSetValued() const {return setValued;} 
      bool isSetValued() const; 

      const Vec& getla() const {return la;}
      Vec& getla() {return la;}
      const Vec& getg() const {return g;}
      Vec& getg() {return g;}
      //void setg(const Vec& g_) {g = g_;}
      //void setgd(const Vec& gd_) {gd = gd_;}
      int getgSize() const {return gSize;} 
      int getgdSize() const {return gdSize;} 
      int getlaSize() const {return laSize;} 
      int getgdInd() const {return gdInd;} 
      int getlaInd() const {return laInd;} 
      int getrFactorSize() const {return rFactorSize;} 
      const Index& getgIndex() const {return Ig;}
      const Index& getlaIndex() const {return Ila;}
      virtual bool isActive() const = 0;
      //virtual bool activeConstraintsChanged() = 0;
      virtual bool gActiveChanged() = 0;
      //virtual bool activeHolonomicConstraintsChanged() = 0;
      //virtual bool activeNonHolonomicConstraintsChanged() = 0;

      void savela();
      void initla();

      const Vector<int>& getrFactorUnsure() const {return rFactorUnsure;}

      virtual void updatexRef(const Vec& ref);
      virtual void updatexdRef(const Vec& ref);
      virtual void updatelaRef(const Vec& ref);
      virtual void updategRef(const Vec& ref);
      virtual void updategdRef(const Vec& ref);
      virtual void updatesRef(const Vec& ref);
      virtual void updateresRef(const Vec& ref);
      virtual void updaterFactorRef(const Vec& ref);
      virtual void updatesvRef(const Vec &sv);
      virtual void updatejsvRef(const Vector<int> &jsvParent);

      void setgInd(int gInd_) {gInd = gInd_;Ig=Index(gInd,gInd+gSize-1);} 
      void setgdInd(int gdInd_) {gdInd = gdInd_;} 
      void setlaInd(int laInd_) {laInd = laInd_;Ila=Index(laInd,laInd+laSize-1); } 
      void setrFactorInd(int rFactorInd_) {rFactorInd = rFactorInd_; } 

      virtual void projectJ(double dt) { cout << "\nprojectJ not implemented." << endl; throw 50; }
      virtual void projectGS(double dt) { cout << "\nprojectGS not implemented." << endl; throw 50; }
      virtual void solveGS(double dt) { cout << "\nsolveGS not implemented." << endl; throw 50; }
      virtual void solveGS() { cout << "\nsolveGS not implemented." << endl; throw 50; }

      virtual void residualProj(double dt) { cout << "\nresidualProj not implemented." << endl; throw 50; }
      virtual void checkForTermination(double dt) { cout << "\ncheckForTermination not implemented." << endl; throw 50; }
      virtual std::string getTerminationInfo(double dt) {return ("No Convergence within " + getName());}
      virtual void residualProjJac(double dt) { cout << "\nresidualProjJac not implemented." << endl; throw 50; }

      virtual void updaterFactors() { cout << "\nupdaterFactors not implemented." << endl; throw 50; }
      void decreaserFactors();
      virtual void checkForTermination() { cout << "\ncheckForTermination not implemented." << endl; throw 50; }

      /*! Defines the maximum error radius lambdas have to match. */  
      virtual void setlaTol(double tol) {laTol = tol;}
      virtual void setgdTol(double tol) {gdTol = tol;}
      virtual void setScaleTolQ(double scaleTolQ_) {scaleTolQ = scaleTolQ_;}
      virtual void setScaleTolp(double scaleTolp_) {scaleTolp = scaleTolp_;}

      /*! Defines the maximal r-factor. */  
      virtual void setrMax(double rMax_) {rMax = rMax_;}

      virtual void connect(CoordinateSystem *port1, int id);

      /*! Adds contours of other bodies, as constraints for ports connected to a LinkContour. */
      virtual void connect(Contour *port1, int id);

      const ContourPointData& getContourPointData(int id) const { return cpData[id]; }

      string getType() const {return "Link";}

      void load(const string& path, ifstream &inputfile);
      void save(const string &path, ofstream &outputfile);


      //virtual MultiBodySystem* getMultiBodySystem(); 

      virtual void checkActiveg() {};
      virtual void checkActivegd() {};
      virtual void checkActivegdn() {}
      virtual void checkActivegdd() {}
      virtual void checkAllgd() {};
      virtual void updateCondition() {};

      //virtual int getNumberOfConstraints() const {return laSize;} 
      
      /*! \brief Set AMVisbody Arrow do display the link load (fore or Moment)
       * @param scale scalefactor (default=1) scale=1 means 1KN or 1KNM is equivalent to arrowlength one
       * @param ID ID of load and corresponding CoordinateSystem/Contour (ID=0 or 1)
       * @param funcColor Userfunction to manipulate Color of Arrow at each TimeStep
       * default: Red arrow for Forces and green one for Moments
       * */
#ifdef HAVE_AMVIS
      virtual void addAMVisForceArrow(AMVis::Arrow *arrow,double scale=1, int ID=0, UserFunction *funcColor=0);
      virtual void addAMVisMomentArrow(AMVis::Arrow *arrow,double scale=1, int ID=0, UserFunction *funcColor=0);
#endif
  };
}

#endif
