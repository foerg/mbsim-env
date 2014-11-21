#ifndef _MBSIMFMI_FMIINSTANCE_H_
#define _MBSIMFMI_FMIINSTANCE_H_

#include <string>
#include <map>
#include <utils.h>
#include <fmatvec/atom.h>

// fmi function declarations must be included as extern C
extern "C" {
  #include <3rdparty/fmiModelFunctions.h>
}

#include <../general/fmi_variables_impl.h>

namespace MBSim {
  class DynamicSystemSolver;
  class ExternGeneralizedIO;
}

namespace MBSimControl {
  class ExternSignalSource;
  class ExternSignalSink;
}

namespace MBSimFMI {

  class FMIVariablePre {
    public:
      FMIVariablePre(Type type_, char datatype_, const std::string &defaultValue);
      Type getType() { return type; }
      double getValue(double);
      int getValue(int);
      bool getValue(bool);
      const char* getValue(const char *);
      void setValue(double v);
      void setValue(int v);
      void setValue(bool v);
      void setValue(const std::string &v);
    protected:
      Type type;
      char datatype;

      double doubleValue;
      int integerValue;
      bool booleanValue;
      std::string stringValue;
  };

  /*! A MBSim FMI instance */
  class FMIInstance : public fmatvec::Atom {
    public:
      //! ctor used in fmiInstantiateModel
      FMIInstance(fmiString instanceName_, fmiString GUID, fmiCallbackFunctions functions, fmiBoolean loggingOn);

      //! ctor used in fmiFreeModelInstance
      ~FMIInstance();

      //! print exception using FMI logger
      void logException(const std::exception &ex);

      // Wrapper for all other FMI functions
      void setDebugLogging           (fmiBoolean loggingOn);
      void setTime                   (fmiReal time_);
      void setContinuousStates       (const fmiReal x[], size_t nx);
      void completedIntegratorStep   (fmiBoolean* callEventUpdate);
      template<typename Type> void setValue(const fmiValueReference vr[], size_t nvr, const Type value[]);
      void initialize                (fmiBoolean toleranceControlled, fmiReal relativeTolerance, fmiEventInfo* eventInfo);
      void getDerivatives            (fmiReal derivatives[], size_t nx);
      void getEventIndicators        (fmiReal eventIndicators[], size_t ni);
      template<typename Type> void getValue(const fmiValueReference vr[], size_t nvr, Type value[]);
      void eventUpdate               (fmiBoolean intermediateResults, fmiEventInfo* eventInfo);
      void getContinuousStates       (fmiReal states[], size_t nx);
      void getNominalContinuousStates(fmiReal x_nominal[], size_t nx);
      void getStateValueReferences   (fmiValueReference vrx[], size_t nx);
      void terminate                 ();

    private:

      // store FMI instanceName and logger
      std::string instanceName;
      fmiCallbackLogger logger;

      // stream buffers for MBSim objects
      LoggerBuffer infoBuffer;
      LoggerBuffer warnBuffer;
      LoggerBuffer debugBuffer;

      // XML parser
      boost::shared_ptr<MBXMLUtils::DOMParser> parser;

      // the system
      boost::shared_ptr<MBSim::DynamicSystemSolver> dss;

      // system time
      double time;
      // system state
      fmatvec::Vec z;
      // system state derivative
      fmatvec::Vec zd;
      // system stop vector
      fmatvec::Vec sv, svLast;
      // system stop vector indicator (0 = no shift in this index; 1 = shift in this index)
      fmatvec::VecInt jsv;

      // variables store before fmiInitialize
      std::vector<boost::shared_ptr<FMIVariablePre> > vrMapPre;

      // variables store after fmiInitialize
      FMIParameters fmiPar;
      std::vector<boost::shared_ptr<Variable> > vrMap;
  };

}

#endif
