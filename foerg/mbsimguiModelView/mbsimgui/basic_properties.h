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

#ifndef _BASIC_PROPERTIES_H_
#define _BASIC_PROPERTIES_H_

#include <string>
#include "utils.h"
#include "extended_properties.h"

class Element;
class Frame;
class TiXmlElement;
class TiXmlNode;

class LocalFrameOfReferenceProperty : public Property {
  protected:
    Frame *frame;
    Element* element;
    std::string xmlName;
  public:
    LocalFrameOfReferenceProperty(Frame* frame_=0, Element* element_=0, const std::string &xmlName_="") : frame(frame_), element(element_), xmlName(xmlName_) {}
    Frame* getFrame() const {return frame;}
    void setFrame(Frame *frame_) {frame = frame_;}
    TiXmlElement* initializeUsingXML(TiXmlElement *element);
    TiXmlElement* writeXMLFile(TiXmlNode *element); 
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
};

class ParentFrameOfReferenceProperty : public Property {
  protected:
    Frame *frame;
    Element* element;
    std::string xmlName;
    QString saved_frameOfReference;

  public:
    ParentFrameOfReferenceProperty(Frame* frame_=0, Element* element_=0, const std::string &xmlName_="") : frame(frame_), element(element_), xmlName(xmlName_) {}

    void initialize();
    Frame* getFrame() {return frame;}
    void setFrame(Frame *frame_) {frame = frame_;}
    virtual TiXmlElement* initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    void setSavedFrameOfReference(const QString &str) {saved_frameOfReference = str;}
    const QString& getSavedFrameOfReference() const {return saved_frameOfReference;}
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
};

class FrameOfReferenceProperty : public Property {
  protected:
    Frame *frame;
    Element* element;
    std::string xmlName;
    QString saved_frameOfReference;
  public:
    FrameOfReferenceProperty(Frame* frame_=0, Element* element_=0, const std::string &xmlName_="") : frame(frame_), element(element_), xmlName(xmlName_) {}
    Frame* getFrame() const {return frame;}
    void setFrame(Frame *frame_) {frame = frame_;}
    void initialize();
    TiXmlElement* initializeUsingXML(TiXmlElement *element);
    TiXmlElement* writeXMLFile(TiXmlNode *element); 
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);
};

class FileProperty : public Property {

  public:
    FileProperty(const std::string &xmlName_) : xmlName(xmlName_) {}
    virtual TiXmlElement* initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    QString fileName;
    std::string xmlName;
    QString absoluteFilePath;
};

class SolverTolerancesProperty : public Property {

  public:
    SolverTolerancesProperty();

    virtual TiXmlElement* initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty projection, g, gd, gdd, la, La;
};

class SolverParametersProperty : public Property {

  public:
    SolverParametersProperty();

    virtual TiXmlElement* initializeUsingXML(TiXmlElement *element);
    virtual TiXmlElement* writeXMLFile(TiXmlNode *element);
    void fromWidget(QWidget *widget);
    void toWidget(QWidget *widget);

  protected:
    ExtProperty tolerances;
};

#endif

