/* Copyright (C) 2004-2022 MBSim Development Team
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

#include <config.h>
#include "external_finite_elements_body.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimFlexibleBody {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMFLEX, ExternalFiniteElementsFfrBody)

  void ExternalFiniteElementsFfrBody::importData() {

    int nen = net + ner;
    int nN = u.rows();
    int ng = nN*nen;
    int nr = 0;
    for(int i=0; i<bc.rows(); i++)
      nr += bc(i,2)-bc(i,1)+1;
    int n = ng-nr;

    SymMatV M0(ng);
    Ke0.resize(ng);

    KrKP.resize(nN);
    for(int i=0; i<nN; i++)
      KrKP[i] = u.row(i).T();

    for(int i=0; i<M.rows(); i++)
      M0.e(M(i,0),M(i,1)) = M(i,2);

    for(int i=0; i<K.rows(); i++)
      Ke0.e(K(i,0),K(i,1)) = K(i,2);

    // compute mass and lumped mass matrix
    vector<double> mi(nN);
    double ds = 0;
    for(int i=0; i<nN; i++) {
      ds += M0.e(i*nen,i*nen);
      m += M0.e(i*nen,i*nen);
      for(int j=i+1; j<nN; j++)
	m += 2*M0.e(i*nen,j*nen);
    }
    for(int i=0; i<nN; i++)
      mi[i] = M0.e(i*nen,i*nen)/ds*m;

    vector<int> c;
    for(int i=0; i<bc.rows(); i++) {
      for(int j=(int)bc(i,1); j<=(int)bc(i,2); j++)
	c.push_back(bc(i,0)*nen+j);
    }
    sort(c.begin(), c.end());

    size_t h=0;
    Indices IF;
    Indices IX;
    for(int i=0; i<ng; i++) {
      if(h<c.size() and i==c[h]) {
	h++;
	IX.add(i);
      }
      else
	IF.add(i);
    }

    M0 <<= M0(IF);
    Ke0 <<= Ke0(IF);

    c.clear();
    for(int i=0; i<inodes.size(); i++) {
      int j1=nen;
      int j2=-1;
      for(int k=0; k<bc.rows(); k++) {
	if(inodes(i)==(int)bc(k,0)) {
	  j1=bc(k,1);
	  j2=bc(k,2);
	}
      }
      for(int j=0; j<nen; j++)
	if(j<j1 or j>j2) c.push_back(inodes(i)*nen+j);
    }
    sort(c.begin(), c.end());
    h=0;
    Indices IH, IN;
    for(int i=0; i<IF.size(); i++) {
      if(h<c.size() and IF[i]==c[h]) {
	IH.add(i);
	h++;
      }
      else
	IN.add(i);
    }
    MatV Vsd(n,IH.size()+nmodes.size(),NONINIT);
    if(IH.size()) {
      Indices IJ;
      for(int i=0; i<IH.size(); i++)
	IJ.add(i);
      MatV Vs(IF.size(),IH.size(),NONINIT);
      Vs.set(IN,IJ,-slvLL(Ke0(IN),Ke0(IN,IH)));
      Vs.set(IH,IJ,MatV(IH.size(),IH.size(),Eye()));
      Vsd.set(RangeV(0,n-1),RangeV(0,Vs.cols()-1),Vs);
    }

    if(nmodes.size()) {
      SqrMat V;
      Vec w;
      if(fixedBoundaryNormalModes) {
	eigvec(Ke0(IN),M0(IN),V,w);
	vector<int> imod;
	for(int i=0; i<w.size(); i++) {
	  if(w(i)>pow(2*M_PI*0.1,2))
	    imod.push_back(i);
	}
	if(min(nmodes)<1 or max(nmodes)>(int)imod.size())
	  throwError(string("(ExternalFiniteElementsFfrBody::init): node numbers do not match, must be within the range [1,") + to_string(imod.size()) + "]");
	for(int i=0; i<nmodes.size(); i++) {
	  Vsd.set(IN,IH.size()+i,V.col(imod[nmodes(i)-1]));
	  Vsd.set(IH,IH.size()+i,Vec(IH.size()));
	}
      }
      else {
	eigvec(Ke0,M0,V,w);

	vector<int> imod;
	for(int i=0; i<w.size(); i++) {
	  if(w(i)>pow(2*M_PI*0.1,2))
	    imod.push_back(i);
	}
	if(min(nmodes)<1 or max(nmodes)>(int)imod.size())
	  throwError(string("(ExternalFiniteElementsFfrBody::init): node numbers do not match, must be within the range [1,") + to_string(imod.size()) + "]");
	for(int i=0; i<nmodes.size(); i++)
	  Vsd.set(IH.size()+i,V.col(imod[nmodes(i)-1]));
      }
    }

    if(IH.size()) {
      SqrMat V;
      Vec w;
      eigvec(JTMJ(Ke0,Vsd),JTMJ(M0,Vsd),V,w);
      vector<int> imod;
      for(int i=0; i<w.size(); i++) {
	if(w(i)>pow(2*M_PI*0.1,2))
	  imod.push_back(i);
      }
      MatV Vr(w.size(),imod.size(),NONINIT);
      for(size_t i=0; i<imod.size(); i++)
	Vr.set(i,V.col(imod[i]));
      Vsd <<= Vsd*Vr;
    }

    Phi = vector<Mat3xV>(nN,Mat3xV(Vsd.cols(),NONINIT));
    MatV Vsdg(ng,Vsd.cols(),NONINIT);
    Indices IJ;
    for(int i=0; i<Vsd.cols(); i++)
      IJ.add(i);
    Vsdg.set(IF,IJ,Vsd);
    Vsdg.set(IX,IJ,Mat(IX.size(),IJ.size()));
    for(size_t i=0; i<Phi.size(); i++)
      Phi[i] = Vsdg(RangeV(nen*i,nen*i+net-1),RangeV(0,Vsd.cols()-1));
    Psi.resize(nN,Mat3xV(Vsd.cols(),NONINIT));
    sigmahel.resize(nN,Matrix<General,Fixed<6>,Var,double>(Vsd.cols()));

    Ke0 <<= JTMJ(Ke0,Vsd);

    Pdm.resize(Vsd.cols());
    rPdm.resize(3,Mat3xV(Vsd.cols()));
    PPdm.resize(3,vector<SqrMatV>(3,SqrMatV(Vsd.cols())));
    if(formalism==lumpedMass) {
      // compute integrals
      for(int i=0; i<nN; i++) {
        rdm += mi[i]*KrKP[i];
        rrdm += mi[i]*JTJ(KrKP[i].T());
        Pdm += mi[i]*Phi[i];
      }
      for(int k=0; k<3; k++) {
        for(int i=0; i<nN; i++)
          rPdm[k] += mi[i]*KrKP[i](k)*Phi[i];
        for(int l=0; l<3; l++) {
          for(int i=0; i<nN; i++)
            PPdm[k][l] += mi[i]*Phi[i].row(k).T()*Phi[i].row(l);
        }
      }
    }
    else if(formalism==consistentMass) {
      if((fPrPK and fPrPK->getArg1Size()) or (fAPK and fAPK->getArg1Size()))
        throwError("Translation and rotation is not allowed for consistent mass formalism. Use lumped mass formalism instead.");
      // compute reduced mass matrix
      PPdm[0][0] = JTMJ(M0,Vsd);
    }
    else
      throwError("Formalism unknown.");
  }

  void ExternalFiniteElementsFfrBody::init(InitStage stage, const InitConfigSet &config) {
    if(stage==resolveStringRef)
      importData();
    else if(stage==plotting) {
      if(plotFeature[openMBV] and ombvBody) {
        std::shared_ptr<OpenMBV::FlexibleBody> flexbody = ombvBody->createOpenMBV();
        openMBVBody = flexbody;
        ombvColorRepresentation = static_cast<OpenMBVFlexibleBody::ColorRepresentation>(ombvBody->getColorRepresentation());
      }
    }
    GenericFlexibleFfrBody::init(stage, config);
  }

  void ExternalFiniteElementsFfrBody::initializeUsingXML(DOMElement *element) {
    GenericFlexibleFfrBody::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"numberOfNodalTranslationalDegreesOfFreedom");
    setNumberOfNodalTranslationalDegreesOfFreedom(E(e)->getText<int>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"numberOfNodalRotationalDegreesOfFreedom");
    setNumberOfNodalRotationalDegreesOfFreedom(E(e)->getText<int>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"nodes");
    setNodes(E(e)->getText<MatVx3>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"massMatrix");
    setMassMatrix(E(e)->getText<MatVx3>());
    for(int i=0; i<M.rows(); i++) {
      M(i,0)--; M(i,1)--;
    }
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"stiffnessMatrix");
    setStiffnessMatrix(E(e)->getText<MatVx3>());
    for(int i=0; i<K.rows(); i++) {
      K(i,0)--; K(i,1)--;
    }
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"formalism");
    if(e) {
      string formalismStr=string(X()%E(e)->getFirstTextChild()->getData()).substr(1,string(X()%E(e)->getFirstTextChild()->getData()).length()-2);
      if(formalismStr=="consistentMass") formalism=consistentMass;
      else if(formalismStr=="lumpedMass") formalism=lumpedMass;
      else formalism=unknown;
    }
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"proportionalDamping");
    if(e) setProportionalDamping(E(e)->getText<Vec>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"boundaryConditions");
    if(e) {
      setBoundaryConditions(MBXMLUtils::E(e)->getText<MatVx3>());
      for(int i=0; i<bc.rows(); i++) {
	for(int j=0; j<bc.cols(); j++)
	  bc(i,j)--;
      }
    }
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"interfaceNodeNumbers");
    if(e) {
      setInterfaceNodeNumbers(MBXMLUtils::E(e)->getText<VecVI>());
      for(int i=0; i<inodes.size(); i++)
	  inodes(i)--;
    }
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"normalModeNumbers");
    if(e) setNormalModeNumbers(MBXMLUtils::E(e)->getText<VecVI>());
    e=MBXMLUtils::E(element)->getFirstElementChildNamed(MBSIMFLEX%"fixedBoundaryNormalModes");
    if(e) setFixedBoundaryNormalModes(MBXMLUtils::E(e)->getText<bool>());
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"enableOpenMBV");
    if(e) {
      ombvBody = shared_ptr<OpenMBVFlexibleFfrBeam>(new OpenMBVFlexibleFfrBeam);
      ombvBody->initializeUsingXML(e);
    }
    e=E(element)->getFirstElementChildNamed(MBSIMFLEX%"plotNodeNumbers");
    if(e) setPlotNodeNumbers(E(e)->getText<VecVI>());
  }

}
