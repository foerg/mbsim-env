/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _UTILS_H_
#define _UTILS_H_

#include <QIcon>
#include <string>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <mbxmlutilshelper/dom.h>

class QTreeWidgetItem;

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

    static std::map<std::string, std::string>& getMBSimNamespacePrefixMapping();


  private:
    // INITIALIZATION
    static bool initialized;
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
inline std::string toStr(const std::vector<std::vector<AT> > &A) {
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
  if(x.size()==0)
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
inline QString toQStr(const std::vector<std::vector<AT> > &A) {
  if(A.size()==0 || A[0].size()==0)
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
  if(str=="" || str=="[]" || str.substr(0,2) == "[;")
    return std::vector<std::string>();
  int pos1 = str.find("["); 
  int pos2 = str.find("]"); 
  std::string str0 = str.substr(pos1+1,pos2-1);
  std::vector<std::string> str1 = extract(str0,';');
  std::vector<std::string> x(str1.size());
  for(unsigned int i=0; i<str1.size(); i++) {
    x[i] = str1[i];
  }
  return x;
}

inline std::vector<std::vector<std::string> > strToMat(const std::string &str) {
  if(str=="" || str=="[]" || str.substr(0,2) == "[;")
    return std::vector<std::vector<std::string> >();
  int pos1 = str.find("["); 
  int pos2 = str.find("]"); 
  std::string str0 = str.substr(pos1+1,pos2-1);
  std::vector<std::string> str1 = extract(str0,';');
  std::vector<std::vector<std::string> > A(str1.size());
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

inline std::vector<std::vector<QString> > strToMat(const QString &str) {
  if(str=="" || str=="[]" || str.mid(0,2) == "[;")
    return std::vector<std::vector<QString> >();
  int pos1 = str.indexOf("["); 
  int pos2 = str.indexOf("]"); 
  QString str0 = str.mid(pos1+1,pos2-1);
  std::vector<QString> str1 = extract(str0,';');
  std::vector<std::vector<QString> > A(str1.size());
  for(unsigned int i=0; i<str1.size(); i++) {
    std::vector<QString> str2 = extract(str1[i],',');
    A[i].resize(str2.size());
    for(unsigned int j=0; j<str2.size(); j++)
      A[i][j] = str2[j];
  }
  return A;
}

inline std::vector<std::string> toStdVec(const std::vector<QString> &x) {
  std::vector<std::string> y(x.size());
  for(unsigned int i=0; i<x.size(); i++)
    y[i] = x[i].toStdString();
  return y;
}

inline std::vector<QString> fromStdVec(const std::vector<std::string> &x) {
  std::vector<QString> y(x.size());
  for(unsigned int i=0; i<x.size(); i++)
    y[i] = QString::fromStdString(x[i]);
  return y;
}

inline std::vector<std::vector<std::string> > toStdMat(const std::vector<std::vector<QString> > &A) {
  std::vector<std::vector<std::string> > B(A.size());
  for(unsigned int i=0; i<A.size(); i++) {
    B[i].resize(A[i].size());
    for(unsigned int j=0; j<A[i].size(); j++)
      B[i][j] = A[i][j].toStdString();
  }
  return B;
}

inline std::vector<std::vector<QString> > fromStdMat(const std::vector<std::vector<std::string> > &A) {
  std::vector<std::vector<QString> > B(A.size());
  for(unsigned int i=0; i<A.size(); i++) {
    B[i].resize(A[i].size());
    for(unsigned int j=0; j<A[i].size(); j++)
      B[i][j] = QString::fromStdString(A[i][j]);
  }
  return B;
}

template <class T>
void addElementText(xercesc::DOMElement *parent, std::string name, T value) {
  std::ostringstream oss;
  oss << std::setprecision(std::numeric_limits<double>::digits10+1) << toStr(value);
  xercesc::DOMDocument *doc=parent->getOwnerDocument();
  parent->insertBefore(MBXMLUtils::D(doc)->createElement(name), NULL)->insertBefore(MBXMLUtils::D(doc)->createElement(oss.str()), NULL);
}

template <class T>
void addElementAttributeAndText(xercesc::DOMElement *parent, std::string name, std::string attribute, std::string attributeName, T value) {
  std::ostringstream oss;
  oss << std::setprecision(std::numeric_limits<double>::digits10+1) << toStr(value);
  xercesc::DOMDocument *doc=parent->getOwnerDocument();
  xercesc::DOMElement* ele = MBXMLUtils::D(doc)->createElement(name);
  MBXMLUtils::E(ele)->setAttribute(attribute, attributeName);
  ele->insertBefore(doc->createTextNode(MBXMLUtils::X()%oss.str()), NULL);
  parent->insertBefore(ele, NULL);
}

std::vector<std::vector<double> > Cardan2AIK(const std::vector<std::vector<double> > &x);
std::vector<std::vector<double> > AIK2Cardan(const std::vector<std::vector<double> > &x);

template<class T>
inline std::string funcExt() {
  return "V";
}

template < >
inline std::string funcExt<double>() {
  return "S";
}

template <class T>
inline T fromMatStr(const std::string &str) {
  return T(str.c_str());
}
 
QTreeWidgetItem* getChild(QTreeWidgetItem *parentItem, const QString &str);

template <class AT>
inline std::vector<std::vector<AT> > transpose(const std::vector<std::vector<AT> > &A) {
 std::vector<std::vector<AT> > B(A[0].size());
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
inline std::vector<std::vector<AT> > getScalars(int m, int n, const AT &d) {
  std::vector<std::vector<AT> > A(m);
  for(int i=0; i<m; i++) {
    A[i].resize(n);
    for(int j=0; j<n; j++)
      A[i][j] = d;
  }
  return A;
}

template <class AT>
inline std::vector<std::vector<AT> > getEye(int m, int n, const AT &d, const AT &z) {
  std::vector<std::vector<AT> > A(m);
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
inline std::vector<std::vector<AT> > getMat(int m, int n, const AT &d) {
  std::vector<std::vector<AT> > A(m);
  for(int i=0; i<m; i++) {
    A[i].resize(n);
    for(int j=0; j<n; j++)
      A[i][j] = d;
  }
  return A;
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

std::string removeWhiteSpace(const std::string &str);

QString removeWhiteSpace(const QString &str);

//template<class T> 
//T max(T x1, T x2) {
//  return x1>=x2?x1:x2;
//}

#endif
