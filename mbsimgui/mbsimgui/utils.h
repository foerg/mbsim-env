/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

  This library is free software; you can redistribute it and/or 
  modify it under the terms of the GNU Lesser General Public 
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version. 
   
  This library is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  Lesser General Public License for more details. 
   
  You should have received a copy of the GNU Lesser General Public 
  License along with this library; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/

#ifndef _UTILS_H_
#define _UTILS_H_

#include <QIcon>
#include <QIconEngine>
#include <string>
#include <iomanip>
#include <limits>
#include <sstream>
#include <objectfactory.h>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <mbxmlutilshelper/dom.h>

class QTreeWidgetItem;
class QMenu;

namespace MBSimGUI {

  class MainWindow;
  class TreeItemData;

  /** Utilitiy class */
  class Utils {
    public:
      // INITIALIZATION

      /** initialize the Utils class. Must be called before any member is used. */
      static void initialize();



      // HELPER FUNCTIONS

      /** Use QIconCached(filename) instead of QIcon(filename) everywhere
       * to cache the parsing of e.g. SVG files. This lead to a speedup
       * (at app init) by a factor of 11 in my test case. */
      static const QIcon& QIconCached(const QString& filename);

      //    /** Convenienc function to convert cardan angles to a rotation matrix */
      //    static SbRotation cardan2Rotation(const SbVec3f& c);
      //    /** Convenienc function to convert a rotation matrix to cardan angles */
      //    static SbVec3f rotation2Cardan(const SbRotation& r);

    private:
      // INITIALIZATION
      static bool initialized;
  };

  class OverlayIconEngine : public QIconEngine {
    public:
      OverlayIconEngine(const std::string &baseFile, const std::string &overlayFile);
      OverlayIconEngine(const QIcon &baseIcon_, const QIcon &overlayIcon_);
      void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
      QIconEngine* clone() const override;
    private:
      QIcon baseIcon;
      QIcon overlayIcon;
  };

  inline std::string toStr(const std::string &str) {
    return str;
  }

  inline std::string toStr(int i) {
    std::stringstream s;
    s << i;
    return s.str();
  }

  inline std::string toStr(double d) {
    std::stringstream s;
    s << d;
    return s.str();
  }

  template <class AT>
    inline std::string toStr(const std::vector<AT> &x) {
      if(x.size()==0)
        return "[]";
      std::string s;
      s += "[";
      for(int i=0; i<x.size(); i++) {
        s += x[i];
        if(i<x.size()-1)
          s += ";";
      }
      s += "]";
      return s;
    }

  template <class AT>
    inline std::string toStr(const std::vector<std::vector<AT>> &A) {
      if(A.size()==0 || A[0].size()==0)
        return "[]";
      std::string s;
      s += "[";
      for(int i=0; i<A.size(); i++) {
        for(int j=0; j<A[i].size(); j++) {
          s += A[i][j];
          if(j<A[i].size()-1)
            s += ",";
        }
        if(i<A.size()-1)
          s += ";";
      }
      s += "]";
      return s;
    }

  inline QString toQStr(const QString &str) {
    return str;
  }

  inline QString toQStr(int i) {
    return QString::number(i);
  }

  inline QString toQStr(double d) {
    return QString::number(d);
  }

  template <class AT>
    inline QString toQStr(const std::vector<AT> &x) {
      if(x.empty())
        return "[]";
      QString s;
      s += "[";
      for(int i=0; i<x.size(); i++) {
        s += x[i];
        if(i<x.size()-1)
          s += ";";
      }
      s += "]";
      return s;
    }

  template <class AT>
    inline QString toQStr(const std::vector<std::vector<AT>> &A) {
      if(A.empty() || A[0].empty())
        return "[]";
      QString s;
      s += "[";
      for(int i=0; i<A.size(); i++) {
        for(int j=0; j<A[i].size(); j++) {
          s += A[i][j];
          if(j<A[i].size()-1)
            s += ",";
        }
        if(i<A.size()-1)
          s += ";";
      }
      s += "]";
      return s;
    }

  inline std::vector<std::string> extract(const std::string &str, char c) {
    std::vector<int> vi;
    vi.push_back(-1);
    int i=0;
    while(true) {
      i = str.find(c,i); 
      if(i!=-1)
        vi.push_back(i);
      else
        break;
      i+=1;
    } 
    vi.push_back(str.size());
    std::vector<std::string> ret(vi.size()-1);
    for(unsigned int i=0; i<vi.size()-1; i++) {
      ret[i] = str.substr(vi[i]+1,vi[i+1]-vi[i]-1);
    }
    return ret;
  }

  inline std::vector<std::string> strToVec(const std::string &str) {
    if(str.empty() || str=="[]" || str.substr(0,2) == "[;")
      return std::vector<std::string>();
    int pos1 = str.find('['); 
    int pos2 = str.find(']'); 
    std::string str0 = str.substr(pos1+1,pos2-1);
    std::vector<std::string> str1 = extract(str0,';');
    std::vector<std::string> x(str1.size());
    for(unsigned int i=0; i<str1.size(); i++) {
      x[i] = str1[i];
    }
    return x;
  }

  inline std::vector<std::vector<std::string>> strToMat(const std::string &str) {
    if(str.empty() || str=="[]" || str.substr(0,2) == "[;")
      return std::vector<std::vector<std::string>>();
    int pos1 = str.find('['); 
    int pos2 = str.find(']'); 
    std::string str0 = str.substr(pos1+1,pos2-1);
    std::vector<std::string> str1 = extract(str0,';');
    std::vector<std::vector<std::string>> A(str1.size());
    for(unsigned int i=0; i<str1.size(); i++) {
      std::vector<std::string> str2 = extract(str1[i],',');
      A[i].resize(str2.size());
      for(unsigned int j=0; j<str2.size(); j++)
        A[i][j] = str2[j];
    }
    return A;
  }

  inline std::vector<QString> extract(const QString &str, char c) {
    std::vector<int> vi;
    vi.push_back(-1);
    int i=0;
    while(true) {
      i = str.indexOf(c,i); 
      if(i!=-1)
        vi.push_back(i);
      else
        break;
      i+=1;
    } 
    vi.push_back(str.size());
    std::vector<QString> ret(vi.size()-1);
    for(unsigned int i=0; i<vi.size()-1; i++) {
      ret[i] = str.mid(vi[i]+1,vi[i+1]-vi[i]-1);
    }
    return ret;
  }


  inline std::vector<QString> strToVec(const QString &str) {
    if(str=="" || str=="[]" || str.mid(0,2) == "[;")
      return std::vector<QString>();
    int pos1 = str.indexOf("["); 
    int pos2 = str.indexOf("]"); 
    QString str0 = str.mid(pos1+1,pos2-1);
    std::vector<QString> str1 = extract(str0,';');
    std::vector<QString> x(str1.size());
    for(unsigned int i=0; i<str1.size(); i++) {
      x[i] = str1[i];
    }
    return x;
  }

  inline std::vector<std::vector<QString>> strToMat(const QString &str) {
    if(str=="" || str=="[]" || str.mid(0,2) == "[;")
      return std::vector<std::vector<QString>>();
    int pos1 = str.indexOf("["); 
    int pos2 = str.indexOf("]"); 
    QString str0 = str.mid(pos1+1,pos2-1);
    std::vector<QString> str1 = extract(str0,';');
    std::vector<std::vector<QString>> A(str1.size());
    for(unsigned int i=0; i<str1.size(); i++) {
      std::vector<QString> str2 = extract(str1[i],',');
      A[i].resize(str2.size());
      for(unsigned int j=0; j<str2.size(); j++)
        A[i][j] = str2[j];
    }
    return A;
  }

  template <class T>
    void addElementAttributeAndText(xercesc::DOMElement *parent, const std::string& name, const std::string& attribute, const std::string& attributeName, T value) {
      std::ostringstream oss;
      oss << std::setprecision(std::numeric_limits<double>::digits10+1) << toStr(value);
      xercesc::DOMDocument *doc=parent->getOwnerDocument();
      xercesc::DOMElement* ele = MBXMLUtils::D(doc)->createElement(name);
      MBXMLUtils::E(ele)->setAttribute(attribute, attributeName);
      ele->insertBefore(doc->createTextNode(MBXMLUtils::X()%oss.str()), nullptr);
      parent->insertBefore(ele, nullptr);
    }

  std::vector<std::vector<double>> Cardan2AIK(const std::vector<std::vector<double>> &x);
  std::vector<std::vector<double>> AIK2Cardan(const std::vector<std::vector<double>> &AIK);

  template <class T>
    inline T fromMatStr(const std::string &str) {
      return T(str.c_str());
    }

  template <class AT>
    inline std::vector<std::vector<AT>> transpose(const std::vector<std::vector<AT>> &A) {
      std::vector<std::vector<AT>> B(A[0].size());
      for(int i=0; i<B.size(); i++) {
        B[i].resize(A.size());
        for(int j=0; j<B[i].size(); j++) {
          B[i][j] = A[j][i];
        }
      }
      return B;
    }

  template <class AT>
    inline std::vector<AT> getScalars(int m, const AT &d) {
      std::vector<AT> x(m);
      for(int i=0; i<m; i++) {
        x[i] = d;
      }
      return x;
    }

  template <class AT>
    inline std::vector<std::vector<AT>> getScalars(int m, int n, const AT &d) {
      std::vector<std::vector<AT>> A(m);
      for(int i=0; i<m; i++) {
        A[i].resize(n);
        for(int j=0; j<n; j++)
          A[i][j] = d;
      }
      return A;
    }

  template <class AT>
    inline std::vector<std::vector<AT>> getEye(int m, int n, const AT &d, const AT &z) {
      std::vector<std::vector<AT>> A(m);
      for(int i=0; i<m; i++) {
        A[i].resize(n);
        for(int j=0; j<n; j++)
          A[i][j] = z;
        if(i<n)
          A[i][i] = d;
      }
      return A;
    }

  template <class AT>
    inline std::vector<AT> getVec(int m, const AT &d) {
      std::vector<AT> x(m);
      for(int i=0; i<m; i++)
        x[i] = d;
      return x;
    }

  template <class AT>
    inline std::vector<std::vector<AT>> getMat(int m, int n, const AT &d) {
      std::vector<std::vector<AT>> A(m);
      for(int i=0; i<m; i++) {
        A[i].resize(n);
        for(int j=0; j<n; j++)
          A[i][j] = d;
      }
      return A;
    }

  template<class T>
  inline std::vector<QString> VecToQvector(const T &x) {
    std::vector<QString> y(x.size());
    for(int i=0; i<x.size(); i++)
      y[i] = QString::number(x.e(i));
    return y;
  }

  template<class T>
  inline std::vector<std::vector<QString>> MatToQvector(const T &A) {
    std::vector<std::vector<QString>> B(A.rows(),std::vector<QString>(A.cols()));
    for(int i=0; i<A.rows(); i++)
      for(int j=0; j<A.cols(); j++)
        B[i][j] = QString::number(A.e(i,j));
    return B;
  }

  inline QStringList noUnitUnits() {
    QStringList units;
    units << "" << "-" << "%";
    return units;
  }

  inline QStringList timeUnits() {
    QStringList units;
    units << "mus" << "ms" << "s" << "sec" << "min" << "h" << "d";
    return units;
  }

  inline QStringList lengthUnits() {
    QStringList units;
    units << "mum" << "mm" << "cm" << "dm" << "m" << "km";
    return units;
  }

  inline QStringList angleUnits() {
    QStringList units;
    units << "rad" << "degree";
    return units;
  }

  inline QStringList velocityUnits() {
    QStringList units;
    units << "m/s" << "km/h"; 
    return units;
  }

  inline QStringList massUnits() {
    QStringList units;
    units << "mg" << "g" << "kg" << "t";
    return units;
  }

  inline QStringList inertiaUnits() {
    QStringList units;
    units << "g*mm^2" << "kg*mm^2" << "kg*m^2";
    return units;
  }

  inline QStringList accelerationUnits() {
    QStringList units;
    units << "m/s^2"; 
    return units;
  }

  inline QStringList stiffnessUnits() {
    QStringList units;
    units << "N/mm" << "N/m"; 
    return units;
  }

  inline QStringList dampingUnits() {
    QStringList units;
    units << "N*s/m"; 
    return units;
  }

  inline QStringList forceUnits() {
    QStringList units;
    units << "mN" << "N" << "kN";
    return units;
  }

  inline QStringList areaUnits() {
    QStringList units;
    units << "mum^2" << "mm^2" << "cm^2" << "dm^2" << "m^2" << "ar^2" << "ha" << "km^2";
    return units;
  }

  inline QStringList volumeUnits() {
    QStringList units;
    units << "mum^3" << "mm^3" << "cm^3" << "dm^3" << "l" << "m^3" << "km^3";
    return units;
  }

  inline QStringList densityUnits() {
    QStringList units;
    units << "kg/m^3";
    return units;
  }

  inline QStringList bulkModulusUnits() {
    QStringList units;
    units << "N/mm^2" << "N/m^2";
    return units;
  }

  inline QStringList pressureUnits() {
    QStringList units;
    units << "mPa" << "Pa" << "mbar" << "bar";
    return units;
  }

  std::string removeWhiteSpace(const std::string &str);

  QString removeWhiteSpace(const QString &str);

  inline std::vector<QString> getBlueColor() {
    std::vector<QString> c(3);
    c[0] = "0.666667"; c[1] = "1"; c[2] = "1";
    return c;
  }

  inline std::vector<QString> getGreenColor() {
    std::vector<QString> c(3);
    c[0] = "0.333333"; c[1] = "1"; c[2] = "1";
    return c;
  }

  inline std::vector<QString> getRedColor() {
    std::vector<QString> c(3);
    c[0] = "0"; c[1] = "1"; c[2] = "1";
    return c;
  }

  template<class Container>
  void createContextMenuFor(QMenu *self, TreeItemData *item, const QString &prefix="");

}

#endif
