/* Copyright (C) 2004-2008  Martin Förg
 
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
#include "tree.h"
#include "object.h"
#include "link.h"
#include "coordinate_system.h"
#include "extra_dynamic_interface.h"

namespace MBSim {

  void Node::addChild(Node* child_) {
    child.push_back(child_);
  }

  void Node::updateKinematics(double t) {
   obj->updateKinematics(t);
   for(unsigned int i=0; i<child.size(); i++)
     child[i]->updateKinematics(t);
  }

  int Node::sethSize(int hSize) {

    for(int i=child.size()-1; i>=0; i--)
     hSize = child[i]->sethSize(hSize);

    obj->sethSize(hSize);
    return hSize - obj->getuSize();
  }

  void Node::calcqSize(int &qSize) {

    obj->calcqSize();
    obj->setqInd(qSize);
    qSize += obj->getqSize();

    for(unsigned int i=0; i<child.size(); i++)
      child[i]->calcqSize(qSize);
  }

  void Node::calcuSize(int &uSize) {

    obj->calcuSize();
    obj->setuInd(uSize);
    uSize += obj->getuSize();

    for(unsigned int i=0; i<child.size(); i++)
      child[i]->calcuSize(uSize);
  }

  Tree::Tree(const string &projectName) : Subsystem(projectName) {
  }

  Tree::~Tree() {
  }

  Node* Tree::addObject(Node* tree, Object* obj) {
    if(getObject(obj->getName(),false)) {
      cout << "Error: The Subsystem " << name << " can only comprise one Object by the name " <<  obj->getName() << "!" << endl;
      assert(getObject(obj->getName(),false) == NULL); 
    }
    object.push_back(obj);

    Node *treeEle = new Node(obj);
    if(tree)
      tree->addChild(treeEle);
    else
      root = treeEle;
    return treeEle;
  }

  Node* Tree::addSubsystem(Node* tree, Subsystem *sys, const Vec &RrRS, const SqrMat &ARS, const CoordinateSystem* refCoordinateSystem) {
    // ADDOBJECT adds an subsystem
    if(getSubsystem(sys->getName(),false)) {
      cout << "Error: The Subsystem " << name << " can only comprise one Object by the name " <<  sys->getName() << "!" << endl;
      assert(getSubsystem(sys->getName(),false) == NULL); 
    }
    subsystem.push_back(sys);

    int i = 0;
    if(refCoordinateSystem)
      i = portIndex(refCoordinateSystem);

    IrOS.push_back(IrOK[i] + AIK[i]*RrRS);
    AIS.push_back(AIK[i]*ARS);

    Node *treeEle = new Node(sys);
    if(tree)
      tree->addChild(treeEle);
    else
      root = treeEle;
    return treeEle;
  }

  void Tree::calcqSize() {
    qSize = 0;
    root->calcqSize(qSize);
  }

  void Tree::calcuSize() {
    uSize = 0;
    root->calcuSize(uSize);
  }

  void Tree::sethSize(int hSize_) {

    cout << getName() << endl;
    hSize = hSize_;
    root->sethSize(hSize);
  } 

  void Tree::updateKinematics(double t) {
    root->updateKinematics(t);
    //   for(vector<Subsystem*>::iterator i = subsystem.begin(); i != subsystem.end(); ++i) 
    //     (*i)->updateKinematics(t);

    //   for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
    //     (*i)->updateKinematics(t);
  }

  void Tree::updatedu(double t, double dt) {

    ud = slvLLFac(LLM, h*dt+r);
  }

  void Tree::updatezd(double t) {

    qd = T*u;
    ud =  slvLLFac(LLM, h+r);

    for(vector<Subsystem*>::iterator i = subsystem.begin(); i != subsystem.end(); ++i) 
      (*i)->updatexd(t);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i)
      (**i).updatexd(t);

    for(vector<ExtraDynamicInterface*>::iterator i = EDI.begin(); i!= EDI.end(); ++i) 
      (**i).updatexd(t);
  }

  void Tree::facLLM() {
    // FACLLM computes Cholesky decomposition of the mass matrix
    //cout << M << endl;
    //cout << qSize << endl;
    //cout << uSize << endl;
    LLM = facLL(M); 
  }

  double Tree::computePotentialEnergy() {
    // double Vtemp = 0.0;
    // if(root) {
    //   Vtemp = computePotentialEnergyBranch(root);
    // }
    // return Vtemp;
    return -1;
  }

  //double Tree::computePotentialEnergyBranch(Object* body) {
  //  double Vtemp = 0.0;

  //  // Koerper selbst
  //  Vtemp += body->computePotentialEnergy();
  //  //    cout << body->getFullName() << endl;

  //  // ... und seine Nachfolger
  //  for(unsigned int i=0; i<body->successor.size(); i++) {
  //    Vtemp += computePotentialEnergyBranch(body->successor[i]);
  //  }
  //  return Vtemp;
  //}

}
