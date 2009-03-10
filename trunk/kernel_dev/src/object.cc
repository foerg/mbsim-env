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
#include<config.h>
#include<stdexcept>
#include "object.h"
#include "coordinate_system.h"
#include "contour.h"
#include "eps.h"
#include "class_factory.h"
#include "subsystem.h"
#include "function.h"

namespace MBSim {

  Object::Object(const string &name) : Element(name), parent(0), qSize(0), qInd(0) {
    uSize[0] = 0;
    uSize[1] = 0;
    hSize[0] = 0;
    hSize[1] = 0;
    uInd[0] = 0;
    uInd[1] = 0;
    hInd[0] = 0;
    hInd[1] = 0;
  } //, parent(0)

  int Object::gethInd(Subsystem* sys ,int i) {
    return (parent == sys) ? hInd[i] : hInd[i] + parent->gethInd(sys,i);
  }

  void Object::writeq() {
//    string fname="PREINTEG/"+fullName+".q0.asc";  
//    ofstream osq(fname.c_str(), ios::out);
//    osq << q;
//    osq.close();
  }
  void Object::readq0() {
//    string fname="PREINTEG/"+fullName+".q0.asc";  
//    ifstream isq(fname.c_str());
//    if(isq) isq >> q0;
//    else {cout << "Object " << name << ": No Preintegration Data q0 available. Run Preintegration first." << endl; throw 50;}
//    isq.close();
  }
  void Object::writeu() {
 //   string fname="PREINTEG/"+fullName+".u0.asc";  
 //   ofstream osu(fname.c_str(), ios::out);
 //   osu << u;
 //   osu.close();
  }

  void Object::readu0() {
 //   string fname="PREINTEG/"+fullName+".u0.asc";  
 //   ifstream isu(fname.c_str());
 //   if(isu) isu >> u0;
 //   else {cout << "Object " << name << ": No Preintegration Data u0 available. Run Preintegration first." << endl; throw 50;}
 //   isu.close();
  }

  void Object::writex() {
 //   string fname="PREINTEG/"+fullName+".x0.asc";  
 //   ofstream osx(fname.c_str(), ios::out);
 //   osx << x;
 //   osx.close();
  }

  void Object::readx0() {
 //   string fname="PREINTEG/"+fullName+".x0.asc";  
 //   ifstream isx(fname.c_str());
 //   if(isx) isx >> x0;
 //   else {cout << "Object " << name << ": No Preintegration Data x0 available. Run Preintegration first." << endl; throw 50;}
 //   isx.close();
  }

  Object::~Object() {
    // Destructs port and contour pointers
    for(vector<CoordinateSystem*>::iterator i = port.begin(); i != port.end(); ++i) 
      delete *i;
    for(vector<Contour*>::iterator i = contour.begin(); i != contour.end(); ++i) 
      delete *i;
  }

  void Object::updateqRef(const Vec &qParent) {
    // UPDATEQREF references to positions of multibody system parent
    q>>qParent(qInd,qInd+qSize-1);
  }

  void Object::updateqdRef(const Vec &qdParent) {
    // UPDATEQDREF references to differentiated positions of multibody system parent
    qd>>qdParent(qInd,qInd+qSize-1);
  }

  void Object::updateuRef(const Vec &uParent) {
    // UPDATEUREF references to velocities of multibody system parent
    u>>uParent(uInd[0],uInd[0]+uSize[0]-1);
  }

  void Object::updateudRef(const Vec &udParent) {
    // UPDATEUDREF references to differentiated velocities of multibody system parent
    ud>>udParent(uInd[0],uInd[0]+uSize[0]-1);
  }

  void Object::updatehRef(const Vec& hParent, int i) {
    // UPDATEHREF references to smooth force vector of multibody system
    h.resize()>>hParent(hInd[i],hInd[i]+hSize[i]-1);
  }

  void Object::updaterRef(const Vec& rParent) {
    // UPDATERREF references to smooth force vector of multibody system
    r>>rParent(uInd[0],uInd[0]+uSize[0]-1);
  }

  void Object::updateMRef(const SymMat &MParent, int i) {
    // UPDATEMREF references to mass matrix of multibody system parent
    M.resize()>>MParent(Index(hInd[i],hInd[i]+hSize[i]-1));
  }

  void Object::updateTRef(const Mat &TParent) {
    // UPDATETREF references to T-matrix of multibody system parent
    T>>TParent(Index(qInd,qInd+qSize-1),Index(uInd[0],uInd[0]+uSize[0]-1));
  }

  void Object::updateLLMRef(const SymMat &LLMParent, int i) {
    // UPDATELLMREF references to cholesky decomposition of mass matrix of multibody system parent
    LLM.resize()>>LLMParent(Index(hInd[i],hInd[i]+hSize[i]-1));
  }

  void Object::initz() {
    // INITZ initialises the Object state
    q = q0;
    u = u0;
  }

  void Object::save(const string &path, ofstream &outputfile) {
    Element::save(path,outputfile);

    // all CoordinateSystem of Object
    outputfile << "# Coordinate systems:" << endl;
    for(vector<CoordinateSystem*>::iterator i = port.begin();  i != port.end();  ++i) {
      outputfile << (**i).getName() << endl;
      string newname = path + "/" + (**i).getFullName() + ".mdl";
      ofstream newoutputfile(newname.c_str(), ios::binary);
      (**i).save(path,newoutputfile);
      newoutputfile.close();
    }
    outputfile << endl;

    // all Contours of Object
    outputfile << "# Contours:" << endl;
    for(vector<Contour*>::iterator i = contour.begin();  i != contour.end();  ++i) {
      outputfile << (**i).getName() << endl;
      string newname = path + "/" + (**i).getFullName() + ".mdl";
      ofstream newoutputfile(newname.c_str(), ios::binary);
      (**i).save(path,newoutputfile);
      newoutputfile.close();
    }
    outputfile << endl;

    outputfile << "# q0:" << endl;
    outputfile << q0 << endl << endl;
    outputfile << "# u0:" << endl;
    outputfile << u0 << endl << endl;
  }

  void Object::load(const string &path, ifstream& inputfile) {
    Element::load(path, inputfile);
    string dummy;

    string basename = path + "/" + getFullName() + ".";

    getline(inputfile,dummy); // # CoSy
    unsigned int no=getNumberOfElements(inputfile);
    for(unsigned int i=0; i<no; i++) {
      getline(inputfile,dummy); // CoSy
      string newname = basename + dummy + ".mdl";
      ifstream newinputfile(newname.c_str(), ios::binary);
      getline(newinputfile,dummy);
      getline(newinputfile,dummy);
      newinputfile.seekg(0,ios::beg);
      if(i>=port.size())
	addCoordinateSystem(new CoordinateSystem("NoName"));
      port[i]->load(path, newinputfile);
      newinputfile.close();
    }
    getline(inputfile,dummy); // # newline

    getline(inputfile,dummy); // # Contour
    no=getNumberOfElements(inputfile);
    for(unsigned int i=0; i<no; i++) {
      getline(inputfile,dummy); // contour
      string newname = basename + dummy + ".mdl";
      ifstream newinputfile(newname.c_str(), ios::binary);
      getline(newinputfile,dummy);
      getline(newinputfile,dummy);
      newinputfile.seekg(0,ios::beg);
      ClassFactory cf;
      if(i>=contour.size())
	addContour(cf.getContour(dummy));
      contour[i]->load(path, newinputfile);
      newinputfile.close();
    }
    getline(inputfile,dummy); // newline

    getline(inputfile,dummy); // # q0
    inputfile >> q0; // # q0
    getline(inputfile,dummy); // Rest of line
    getline(inputfile,dummy); // Newline

    getline(inputfile,dummy); // # u0
    inputfile >> u0; // # q0
    getline(inputfile,dummy); // Rest of line
    getline(inputfile,dummy); // Newline
  }

  void Object::plot(double t, double dt, bool top) {
    if(getPlotFeature(plotRecursive)==enabled) {
      Element::plot(t,dt,false);

      if(getPlotFeature(state)==enabled) {
        for(int i=0; i<qSize; ++i)
          plotVector.push_back(q(i));
        for(int i=0; i<uSize[0]; ++i)
          plotVector.push_back(u(i));
      }
      if(getPlotFeature(stateDerivative)==enabled) {
        for(int i=0; i<qSize; ++i)
          plotVector.push_back(qd(i)/dt);
        for(int i=0; i<uSize[0]; ++i)
          plotVector.push_back(ud(i)/dt);
      }
      if(getPlotFeature(rightHandSide)==enabled) {
        for(int i=0; i<uSize[0]; ++i)
          plotVector.push_back(h(i));
        for(int i=0; i<uSize[0]; ++i)
          plotVector.push_back(r(i)/dt);
      }

      if(top && plotColumns.size()>1)
        plotVectorSerie->append(plotVector);

      for(unsigned int j=0; j<port.size(); j++)
        port[j]->plot(t,dt);
      /*for(unsigned int j=0; j<contour.size(); j++)
        contour[j]->plot(t,dt);*/
    }
  }

  void Object::initPlot(bool top) {
    Element::initPlot(parent, true, false);

    if(getPlotFeature(plotRecursive)==enabled) {
      if(getPlotFeature(state)==enabled) {
        for(int i=0; i<qSize; ++i)
          plotColumns.push_back("q("+numtostr(i)+")");
        for(int i=0; i<uSize[0]; ++i)
          plotColumns.push_back("u("+numtostr(i)+")");
      }
      if(getPlotFeature(stateDerivative)==enabled) {
        for(int i=0; i<qSize; ++i)
          plotColumns.push_back("qd("+numtostr(i)+")");
        for(int i=0; i<uSize[0]; ++i)
          plotColumns.push_back("ud("+numtostr(i)+")");
      }
      if(getPlotFeature(rightHandSide)==enabled) {
        for(int i=0; i<uSize[0]; ++i)
          plotColumns.push_back("h("+numtostr(i)+")");
        for(int i=0; i<getuSize(); ++i)
          plotColumns.push_back("r("+numtostr(i)+")");
      }

      if(top) createDefaultPlot();

      for(unsigned int j=0; j<port.size(); j++)
        port[j]->initPlot();
      /*for(unsigned int j=0; j<contour.size(); j++)
        contour[j]->initPlotFiles();*/
    }
  }

  void Object::closePlot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      for(unsigned int j=0; j<port.size(); j++)
        port[j]->closePlot();
      /*for(unsigned int j=0; j<contour.size(); j++)
        contour[j]->closePlot();*/

      Element::closePlot();
    }
  }

  int Object::portIndex(const CoordinateSystem *port_) const {
    for(unsigned int i=0; i<port.size(); i++) {
      if(port_==port[i])
	return i;
    }
    return -1;
  }

  void Object::addContour(Contour* contour_) {
    if(getContour(contour_->getName(),false)) { //Contourname exists already
      cout << "Error: The Object " << name << " can only comprise one Contour by the name " <<  contour_->getName() << "!" << endl;
      assert(getContour(contour_->getName(),false)==NULL);
    }
    //contour_->setFullName(getFullName()+"."+contour_->getFullName());
    contour.push_back(contour_);
    contour_->setParent(this);
  }

  void Object::addCoordinateSystem(CoordinateSystem* port_) {
    if(getCoordinateSystem(port_->getName(),false)) { //Contourname exists already
      cout << "Error: The Object " << name << " can only comprise one CoordinateSystem by the name " <<  port_->getName() << "!" << endl;
      assert(getCoordinateSystem(port_->getName(),false)==NULL);
    }
    //port_->setFullName(getFullName()+"."+port_->getFullName());
    port.push_back(port_);
    port_->setParent(this);
  }

  CoordinateSystem* Object::getCoordinateSystem(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<port.size(); i++) {
      if(port[i]->getName() == name)
	return port[i];
    }             
    if(check) {
      if(!(i<port.size())) cout << "Error: The object " << this->name <<" comprises no port " << name << "!" << endl; 
      assert(i<port.size());
    }
    return NULL;
  }

  Contour* Object::getContour(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<contour.size(); i++) {
      if(contour[i]->getName() == name)
	return contour[i];
    }
    if(check) {
      if(!(i<contour.size())) cout << "Error: The object " << this->name <<" comprises no contour " << name << "!" << endl; 
      assert(i<contour.size());
    }
    return NULL;
  }

  void Object::calchSize(int j) {  
  //  hSize = uSize;
  }

  void Object::sethSize(int hSize_, int j) {

    hSize[j] = hSize_;
    for(vector<CoordinateSystem*>::iterator i=port.begin(); i!=port.end(); i++)
      (*i)->sethSize(hSize[j],j);
    for(vector<Contour*>::iterator i=contour.begin(); i!=contour.end(); i++) 
      (*i)->sethSize(hSize[j],j);
  }

  void Object::sethInd(int hInd_, int j) {
    hInd[j] = hInd_;
    for(vector<CoordinateSystem*>::iterator i=port.begin(); i!=port.end(); i++) 
      (*i)->sethInd(hInd[j],j);
    for(vector<Contour*>::iterator i=contour.begin(); i!=contour.end(); i++) 
      (*i)->sethInd(hInd[j],j);
  }

  void Object::preinit() {  
    for(vector<CoordinateSystem*>::iterator i=port.begin(); i!=port.end(); i++) 
      (*i)->preinit();
    for(vector<Contour*>::iterator i=contour.begin(); i!=contour.end(); i++) 
      (*i)->preinit();
  }

  void Object::init() {  
    Iu = Index(uInd[0],uInd[0]+uSize[0]-1);
    Ih = Index(hInd[0],hInd[0]+hSize[0]-1);

    for(vector<CoordinateSystem*>::iterator i=port.begin(); i!=port.end(); i++) 
      (*i)->init();
    for(vector<Contour*>::iterator i=contour.begin(); i!=contour.end(); i++) 
      (*i)->init();
  }

  void Object::updatedq(double t, double dt) {
    qd = T*u*dt;
  }

  void Object::updatedu(double t, double dt) {

    ud = slvLLFac(LLM, h*dt+r);
  }

  void Object::updateqd(double t) {

    qd = T*u;
  }

  void Object::updateud(double t) {

    ud =  slvLLFac(LLM, h+r);
  }

  void Object::updatezd(double t) {

    updateqd(t);
    updateud(t);
  }

  void Object::facLLM() {
    // FACLLM computes Cholesky decomposition of the mass matrix
    LLM = facLL(M); 
  }

  double Object::computeKineticEnergy() {
    return 0.5*trans(u)*M*u;
  }

  //  MultiBodySystem* Object::getMultiBodySystem() {
  //    return parent->getMultiBodySystem();
  //  }

  void Object::setMultiBodySystem(MultiBodySystem* sys) {
    Element::setMultiBodySystem(sys);
    for(unsigned i=0; i<port.size(); i++)
      port[i]->setMultiBodySystem(sys);
    for(unsigned i=0; i<contour.size(); i++)
      contour[i]->setMultiBodySystem(sys);
  }

  void Object::setFullName(const string &str) {
    Element::setFullName(str);
    for(unsigned i=0; i<port.size(); i++)
      port[i]->setFullName(getFullName() + "." + port[i]->getName());
    for(unsigned i=0; i<contour.size(); i++)
      contour[i]->setFullName(getFullName() + "." + contour[i]->getName());
  }

}
