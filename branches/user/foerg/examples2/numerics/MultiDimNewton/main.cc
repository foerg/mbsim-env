#include <mbsim/numerics/nonlinear_algebra/multi_dimensional_newton_method.h>
#include <mbsim/utils/nonlinear_algebra.h>
#include <mbsim/utils/stopwatch.h>
#include <fmatvec.h>

#include <iostream>

using namespace fmatvec;
using namespace MBSim;
using namespace std;


class TestFunction : public Function<fmatvec::Vec(fmatvec::Vec)> {

  public:
    /**
     * \brief Constructor
     */
  TestFunction(){
  }

    /**
     * \brief Destructor
     */
    virtual ~TestFunction() {
    }

    virtual fmatvec::Vec operator ()(const fmatvec::Vec & vector) {
      Vec result(vector.size(), INIT, 0.);

      for(int i=0; i< result.size(); i++) {
        result(i) = pow(sin(i*2*M_PI/vector.size()) + vector(i), 2);
      }

      return result;
    }

};

int main (int argc, char* argv[]) {

  int dimension = 1000;

  TestFunction * function = new TestFunction();

  MultiDimensionalNewtonMethod newton;
  newton.setFunction(function);
  newton.setJacobianFunction(new NumericalNewtonJacobianFunction());

  map<Index, double> tolerances;
  tolerances.insert(pair<Index, double>(Index(0,dimension/2-1), 1e-10));
  tolerances.insert(pair<Index, double>(Index(dimension/2,dimension-1), 1e-8));



  Vec initialSolution(dimension,INIT,0.0);
  for(int i =0; i< dimension; i++) {
    initialSolution(i) = 5*i;
  }
  Vec test1 = initialSolution.copy();
  Vec test2 = initialSolution.copy();

  StopWatch sw;

  cout << "Solving system of dimension " << dimension << endl;

  /*New Newton*/
  for(int i =0 ; i < 2; i++) {
    if(i==0) {
      cout << "Solving with GlobalResidualCriteriaFunction with tolerance of 1e-10 ... " << endl;
      newton.setCriteriaFunction(new GlobalResidualCriteriaFunction(1e-10));
    }
    else {
      cout << "Solving with LocalResidualCriteriaFunction with tolerance for first half of solution vector of 1e-10 and second half of 1e-8 ... " << endl;
      newton.setCriteriaFunction(new LocalResidualCriteriaFunction(tolerances));
    }
    sw.start();
    test1 = newton.solve(initialSolution);

    cout << "Time = " << sw.stop(true) << endl;
    cout << "Iterations = " << newton.getNumberOfIterations() << endl << endl;
  }

  /*Old newton*/
  cout << "Solving system with old newton algorithm as reference with tolerance of 1e-10 ... " << endl;
  TestFunction * function2 = new TestFunction();
  MultiDimNewtonMethod newton2(function2);
  newton2.setTolerance(1e-10);

  sw.start();

  test2 = newton2.solve(test2);

  cout << "Time = " << sw.stop(true) << endl;
  cout << "Iterations = " << newton2.getNumberOfIterations() << endl;
  cout << "REAMARK: The iterations are counted differently (minus one) with the old newton..." << endl;

  return 0;

}
