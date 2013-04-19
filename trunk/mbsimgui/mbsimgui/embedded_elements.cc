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

#include <config.h>
#include "embedded_elements.h"
#include "basic_properties.h"

using namespace std;

EmbeddedObject::EmbeddedObject(const string &str, Element *parent) : Object(str,parent) {
  href.setProperty(new FileProperty(""));
}

EmbeddedObject::~EmbeddedObject() {
}

void EmbeddedObject::initializeUsingXML(TiXmlElement *element) {
  string file = element->Attribute("href");
  static_cast<FileProperty*>(href.getProperty())->setFileName(file);
  static_cast<FileProperty*>(href.getProperty())->setAbsoluteFilePath(file);
}

TiXmlElement* EmbeddedObject::writeXMLFile(TiXmlNode *parent) {    
  TiXmlElement *ele0=new TiXmlElement(PVNS+string("embed"));
  ele0->SetAttribute("href", static_cast<FileProperty*>(href.getProperty())->getAbsoluteFilePath());
  if(count != "")
    ele0->SetAttribute("count", count);
  if(counterName != "")
  ele0->SetAttribute("counterName", counterName);
  parent->LinkEndChild(ele0);
  return ele0;
}
