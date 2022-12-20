#ifndef _MBSIMFMI_FMI_VARIABLES_H_
#define _MBSIMFMI_FMI_VARIABLES_H_

#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <mbsimControl/extern_signal_source.h>
#include <mbsimControl/extern_signal_sink.h>
#include <fmatvec/toString.h>

namespace MBSim {
  class DynamicSystem;
  class DynamicSystemSolver;
  class Link;
}

// local helper functions
namespace {
  //! convert a MBSim path to a FMI structured variable name
  inline std::string mbsimToFMIName(std::string path, int idx, int size) {
    boost::replace_all(path, "\\", "\\\\"); // escape "\" by "\\"
    boost::replace_all(path, "'", "\'"); // escape "'" by "\'"
    boost::replace_all(path, "/", "."); // replace the MBSim separator / by the FMI seperator .
    boost::replace_all(path, "[", "s.'"); // replace "[" with "s.'"; start of container with container plural + FMI seperator + quote
    boost::replace_all(path, "]", "'"); // replace "]" with "'"; end of container with quote
    return path.substr(1)+(size<=1 ? "" : "["+fmatvec::toString(idx)+"]"); // skip the starting spearotor character
  }

  // some platform dependent file suffixes, directory names, ...
#ifdef _WIN32
  std::string SHEXT(".dll");
  #ifdef _WIN64
  std::string FMIOS("win64");
  #else
  std::string FMIOS("win32");
  #endif
#else
  std::string SHEXT(".so");
  #ifdef __x86_64__
  std::string FMIOS("linux64");
  #else
  std::string FMIOS("linux32");
  #endif
#endif
}

namespace MBSimFMI {

class Variable;

//! enumeration for PredefinedParameterStruct::plotMode
enum PlotMode {
  EverynthCompletedStep            = 1,
  NextCompletedStepAfterSampleTime = 2,
  SampleTime                       = 3
};
//! Struct holding all predefined FMI parameters (parameters which are not part of the MBSim dss)
struct PredefinedParameterStruct {
  std::string outputDir; // the MBSim output directory
  int plotMode;          // the MBSim plotting mode
  int plotEachNStep;     // plot at each n-th completed integrator step
  double plotStepSize;   // plot in equidistand time steps
  double gMax;           // tolerance for position constraints 
  double gdMax;          // tolerance for velocity constraints 
};

//! add all FMI predefined parameters to var.
//! The values of all predefined parameters are stored in the struct PredefinedParameterStruct.
void addPredefinedParameters(bool cosim, std::vector<std::shared_ptr<Variable> > &var,
                             PredefinedParameterStruct &predefinedParameterStruct,
                             bool setToDefaultValue);

//! add all FMI input/outputs used by the MBSim dss to var.
void addModelInputOutputs(std::vector<std::shared_ptr<Variable> > &var,
                          const MBSim::DynamicSystemSolver *dss);

//! Type of the variable
enum Type {
  Parameter,
  Input,
  Output
};

//! Abstract base class for all FMI variables
class Variable {
  public:
    using EnumListCont = std::vector<std::pair<std::string, std::string>>;
    using EnumList = std::shared_ptr<EnumListCont>;

    //! ctor
    Variable(std::string name_, std::string desc_, Type type_, char datatypeChar_, EnumList enumList_=EnumList()) :
      name(std::move(name_)), desc(std::move(desc_)), type(type_), datatypeChar(datatypeChar_), enumList(std::move(enumList_)) {}

    //! FMI name
    std::string getName() { return name; }
    //! FMI description
    std::string getDescription() { return desc; }
    //! Variable type
    Type getType() { return type; }
    //! FMI variable datatype
    char getDatatypeChar() { return datatypeChar; }
    //! Enumeration list.
    //! Only allowed for datatype int (='i'). A empty pointer means integer type a non empty pointer a enumeration type.
    const EnumList& getEnumerationList() { return enumList; }

    //! get the current value as a string (usable to add it to e.g. XML)
    virtual std::string getValueAsString()=0;

    // value setter

    //! set FMI variable of type real. The default implementation throws, if not overloaded.
    virtual void setValue(const double &v) {
      throw std::runtime_error("Setting this variable is not allowed or is not of type real.");
    }
    //! set FMI variable of type integer. The default implementation throws, if not overloaded.
    virtual void setValue(const int &v) {
      throw std::runtime_error("Setting this variable is not allowed or is not of type integer.");
    }
    //! set FMI variable of type boolean. The default implementation throws, if not overloaded.
    virtual void setValue(const bool &v) {
      throw std::runtime_error("Setting this variable is not allowed or is not of type boolean.");
    }
    //! set FMI variable of type string. The default implementation throws, if not overloaded.
    virtual void setValue(const std::string &v) {
      throw std::runtime_error("Setting this variable is not allowed or is not of type string.");
    }

    // value getter

    //! get FMI variable of type real. The default implementation throws, if not overloaded.
    //! Note: the argument just exists to be able to overload all type with the name function name.
    virtual const double& getValue(const double&) {
      throw std::runtime_error("This variable is not of type real.");
    }
    //! get FMI variable of type integer. The default implementation throws, if not overloaded.
    //! Note: the argument just exists to be able to overload all type with the name function name.
    virtual const int& getValue(const int&) {
      throw std::runtime_error("This variable is not of type integer.");
    }
    //! get FMI variable of type boolean. The default implementation throws, if not overloaded.
    //! Note: the argument just exists to be able to overload all type with the name function name.
    virtual const bool& getValue(const bool&) {
      throw std::runtime_error("This variable is not of type boolean.");
    }
    //! get FMI variable of type string. The default implementation throws, if not overloaded.
    //! Note: the argument just exists to be able to overload all type with the name function name.
    virtual const std::string& getValue(const std::string&) {
      throw std::runtime_error("This variable is not of type string.");
    }

  protected:
    std::string name, desc;
    Type type;
    char datatypeChar;
    EnumList enumList;
};



//! map c++ type to FMI datatype character
template<typename Datatype> struct MapDatatypeToFMIDatatypeChar;
template<> struct MapDatatypeToFMIDatatypeChar<double     > { static const char value='r'; };
template<> struct MapDatatypeToFMIDatatypeChar<int        > { static const char value='i'; };
template<> struct MapDatatypeToFMIDatatypeChar<bool       > { static const char value='b'; };
template<> struct MapDatatypeToFMIDatatypeChar<std::string> { static const char value='s'; };

//! A FMI parameter which stores the value in a externally provided reference.
//! This is used for predefined parameters.
template<typename Datatype>
class PredefinedParameter : public Variable {
  public:
    PredefinedParameter(const std::string &name_, const std::string &desc_, Datatype &v, const EnumList& enumList=EnumList()) :
      Variable(name_, desc_, Parameter, MapDatatypeToFMIDatatypeChar<Datatype>::value, enumList), value(v) {}
    std::string getValueAsString() override { return boost::lexical_cast<std::string>(value); }
    void setValue(const Datatype &v) override { value=v; }
    const Datatype& getValue(const Datatype&) override { return value; }
  protected:
    Datatype &value;
};

//! FMI input variable for MBSim::ExternSignalSource
class ExternSignalSourceInput : public Variable {
  public:
    ExternSignalSourceInput(MBSimControl::ExternSignalSource *sig_, int idx_) :
      Variable(mbsimToFMIName(sig_->getPath(), idx_, sig_->getSignalSize()),
        "ExternSignalSource", Input, 'r'), sig(sig_), idx(idx_) {}
    std::string getValueAsString() override { return fmatvec::toString(getValue(double())); }
    void setValue(const double &v) override {
      fmatvec::VecV curv = sig->evalSignal();
      curv(idx) = v;
      sig->setSignal(curv);
    }
    const double& getValue(const double &) override {
      return sig->evalSignal()(idx);
    }
  protected:
    MBSimControl::ExternSignalSource *sig;
    int idx;
};

//! FMI output variable for MBSim::ExternSignalSink
class ExternSignalSinkOutput : public Variable {
  public:
    ExternSignalSinkOutput(MBSimControl::ExternSignalSink *sig_, int idx_) :
      Variable(mbsimToFMIName(sig_->getPath(), idx_, sig_->getSignalSize()),
        "ExternSignalSink", Output, 'r'), sig(sig_), idx(idx_) {}
    std::string getValueAsString() override { return fmatvec::toString(getValue(double())); }
    const double& getValue(const double &) override {
      return sig->evalSignal()(idx);
    }
  protected:
    MBSimControl::ExternSignalSink *sig;
    int idx;
};

//! A FMI variable which stores the value internally.
//! This class is uses for ALL FMI variables during the preprocessing phase:
//! between fmiInstantiateModel and fmiInitialize.
//! This class is also used for XML model parameters during mbsimCreateFMU.
template<class Datatype>
class VariableStore : public Variable {
  public:
    VariableStore(const std::string &name_, Type type, Datatype defaultValue=Datatype()) :
      Variable(name_, "", type, MapDatatypeToFMIDatatypeChar<Datatype>::value), value(std::move(defaultValue)) {}
    std::string getValueAsString() override { return boost::lexical_cast<std::string>(value); }
    const Datatype& getValue(const Datatype&) override { return value; }
    void setValue(const Datatype &v) override { value=v; }
  protected:
    Datatype value;
};

}

#endif
