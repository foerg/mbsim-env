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
 * Contact: markus.ms.schneider@gmail.com
 */

#ifndef  _ELASTIC_LINE_VARIATIONAL_H_
#define  _ELASTIC_LINE_VARIATIONAL_H_

#include "hline.h"

namespace MBSimHydraulics {

  /*! ElasticLineVariational 
    This line model is described in
    J. Makinen, R. Piche and A. Ellman (2000),
    Fluid Transmission Line Modeling Using a Variational Method, 
    ASME Journal of Dynamic Systems, Measurement, and Control,122 (1), 153-162.
    */
  class ElasticLineVariational : public HLine {

    public:

      enum WindowFunction {
        None,
        Hann,
        Hamming,
        Riemann,
        BlackmanHarris
      };
      
      /*! Constructor */
      ElasticLineVariational(const std::string &name="");

      /*! set initial pressure of the pipe fluid*/
      void setp0(double p0_) {p0=p0_; }
      /*! set the fracAir of the pipe (for E-Modulus calculation*/
      void setFracAir(double fracAir_) {fracAir=fracAir_; }
      /*! set diameter*/
      void setDiameter(double d_) {r=d_/2.; }
      /*! set length*/
      void setLength(double l_) {l=l_; }
      /*! select points in the pipe, for which a output should be created*/
      void setRelativePlotPoints(const fmatvec::Vec &rPP) {relPlotPoints=rPP; }
      /*! select window function (default Blackman-Harris) */
      void setWindowFunction(WindowFunction w) {window_function_type=w; }
      /*! number of harmonic ansatz functions (default n=9) */
      void setNumberOfAnsatzFunctions(unsigned int n_=4) {n=2*n_+1; }
      /*! print system state vector */
      void printLineStateSpace(bool print=true) {printStateSpace=print; }

      fmatvec::VecV getInflowFactor() override {return wI; }
      fmatvec::VecV getOutflowFactor() override {return wO; }

      void init(InitStage stage, const MBSim::InitConfigSet &config) override;
      void calcqSize() override {qSize=n-1; }
      void calcuSize(int j) override {uSize[j]=n; }

      void updateQ() override;
      void updateh(int j=0) override;
      void updateT() override {T=Tlocal; }
      void updateM() override {M=Mlocal; }

      void plot() override;
      void plotParameters();

      void initializeUsingXML(xercesc::DOMElement * element) override;

    protected:
      double p0, fracAir, r, l;
      fmatvec::Vec relPlotPoints;
      WindowFunction window_function_type;
      int n;
      bool printStateSpace;

    private:
      fmatvec::VecV wO, wI;
      fmatvec::Vec hq, hu, hp0, cu;
      fmatvec::Mat Tlocal, relPlot;
      fmatvec::SymMat Mlocal;
      
      void doPrintStateSpace();
  };

}


#endif

