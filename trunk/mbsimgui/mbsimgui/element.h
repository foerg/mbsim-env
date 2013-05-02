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

#ifndef _ELEMENT__H_
#define _ELEMENT__H_

#include "treeitemdata.h"
#include "basic_properties.h"
#include "extended_properties.h"
#include "element_property_dialog.h"
#include "element_context_menu.h"
#include "parameter.h"

class Element;
class Frame;
class Contour;
class Group;
class Object;
class Link;
class ExtraDynamic;
class Observer;
class TextWidget;
namespace MBXMLUtils {
  class TiXmlElement;
  class TiXmlNode;
}

class Element : public TreeItemData {
  friend class ElementPropertyDialog;
  protected:
    Element *parent;
    static int IDcounter;
    std::string ID;
    ExtProperty name, embed;
  public:
    Element(const std::string &name, Element *parent);
    virtual ~Element();
    virtual std::string getPath();
    std::string getXMLPath(Element *ref=0, bool rel=false);
    virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);
    virtual void initializeUsingXMLEmbed(MBXMLUtils::TiXmlElement *element);
    virtual MBXMLUtils::TiXmlElement* writeXMLFileEmbed(MBXMLUtils::TiXmlNode *element);
    virtual void writeXMLFile(const std::string &name);
    virtual void writeXMLFile() { writeXMLFile(getName()); }
    virtual void initialize();
    const std::string& getName() const {return static_cast<const TextProperty*>(name.getProperty())->getText();}
    void setName(const std::string &str) {static_cast<TextProperty*>(name.getProperty())->setText(str);}
    virtual std::string getType() const { return "Element"; }
    virtual std::string getNameSpace() const { return MBSIMNS; }
    std::string getValue() const { return getType(); }
    //std::string newName(const std::string &type);
    virtual std::string getFileExtension() const { return ".xml"; }
    template<class T> T* getByPath(std::string path);
    virtual Element* getByPathSearch(std::string path) {return 0; }
    virtual int getNumberOfFrames() {return 0;}
    virtual int getNumberOfContours() {return 0;}
    virtual int getNumberOfGroups() {return 0;}
    virtual int getNumberOfObjects() {return 0;}
    virtual int getNumberOfLinks() {return 0;}
    virtual int getNumberOfObservers() {return 0;}
    virtual Frame* getFrame(int i) {return 0;}
    virtual Contour* getContour(int i) {return 0;}
    virtual Group* getGroup(int i) {return 0;}
    virtual Object* getObject(int i) {return 0;}
    virtual Link* getLink(int i) {return 0;}
    virtual Observer* getObserver(int i) {return 0;}
    virtual Frame* getFrame(const std::string &name) {return 0;}
    //Contour* getContour(const std::string &name);
    //Object* getObject(const std::string &name);
    //Group* getGroup(const std::string &name);
    //Link* getLink(const std::string &name);
    //Observer* getObserver(const std::string &name);
    virtual void addFrame(Frame *frame) {}
    virtual void addContour(Contour *contour) {}
    virtual void addGroup(Group *group) {}
    virtual void addObject(Object *object) {}
    virtual void addLink(Link *link) {}
    virtual void addObserver(Observer *observer) {}
    virtual void removeElement(Element *element) {}
    const std::string& getID() const { return ID; }
    virtual Element* getParent() {return parent;}
    virtual void setParent(Element* parent_) {parent = parent_;}
    virtual ElementPropertyDialog* createPropertyDialog() {return new ElementPropertyDialog(this);}
    virtual ElementContextMenu* createContextMenu() {return new ElementContextMenu;}
    Element* getRoot() {return parent?parent->getRoot():this;}
    bool isEmbedded() const {return embed.isActive();}
    ParameterList getParameterList(bool addCounter=true) const;
};

template<class T>
T* Element::getByPath(std::string path) {
  Element * e = getByPathSearch(path);
  return dynamic_cast<T*>(e);
}

#endif
