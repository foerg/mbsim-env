#ifndef _MBSIMFMI_UTILS_H_
#define _MBSIMFMI_UTILS_H_

#include <boost/filesystem.hpp>
#include <sstream>

extern "C" {
  #include <3rdparty/fmiModelFunctions.h>
}

// define getFMUSharedLibPath() which returns the FMU shared library path = .../resources/local/[lib|bin]/<name>.[so|dll]
#define MBXMLUTILS_SHAREDLIBNAME FMU
#include <mbxmlutilshelper/getsharedlibpath.h>

namespace MBSimFMI {

  //! stream buffer used by fmatvec::Atom for FMI logging
  class LoggerBuffer : public std::stringbuf {
    public:
      LoggerBuffer(fmiCallbackLogger logger_, fmiComponent c_, const std::string &instanceName_,
                   fmiStatus status_, const std::string &category_) :
          std::stringbuf(std::ios_base::out),
          logger(logger_),
          c(c_),
          instanceName(instanceName_),
          status(status_),
          category(category_) {
      }
    protected:
      fmiCallbackLogger logger;
      fmiComponent c;
      std::string instanceName;
      fmiStatus status;
      std::string category;
  
      int sync(); // overwrite the sync function from stringbuf
  };

}

#endif
