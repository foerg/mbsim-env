#ifndef DIFFERENTIAL_GEAR_H_
#define DIFFERENTIAL_GEAR_H_

#include "mbsim/group.h"
#include "mbsimPowertrain/shaft.h"

namespace MBSimPowertrain {

  class DifferentialGear : public MBSim::Group { 
    public:
      struct Data {
	double massHousing;
	double massInputShaft;
	double massRightOutputShaft;
	double massLeftOutputShaft;
	double massPlanet;
	fmatvec::SymMat inertiaTensorHousing;
	fmatvec::SymMat inertiaTensorInputShaft;
	fmatvec::SymMat inertiaTensorRightOutputShaft;
	fmatvec::SymMat inertiaTensorLeftOutputShaft;
	fmatvec::SymMat inertiaTensorPlanet;
	double lengthInputShaft;
	double lengthRightOutputShaft;
	double lengthLeftOutputShaft;
	double lengthPlanet;
	double radiusInputShaft;
	double radiusLeftOutputShaft;
	double radiusRightOutputShaft;
	double radiusPlanet;
	Data();
      };
    protected:
      Data data;
      Shaft *shaft2, *shaft4, *shaft5;

    public:
      DifferentialGear(const std::string &name, Data data=Data());
      double getRadiusInputShaft() const {return data.radiusInputShaft;}
      Shaft* getInputShaft() {return shaft2;}
      Shaft* getRightOutputShaft() {return shaft5;}
      Shaft* getLeftOutputShaft() {return shaft4;}
  };

}

#endif
