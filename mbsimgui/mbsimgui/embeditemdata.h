/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2013 Martin Förg

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

#ifndef _EMBEDITEMDATA__H_
#define _EMBEDITEMDATA__H_

#include "treeitemdata.h"
#include "embedding_property_dialog.h"
#include "embedding_context_menu.h"
#include <xercesc/util/XercesDefs.hpp>

namespace XERCES_CPP_NAMESPACE {
  class DOMElement;
  class DOMDocument;
}

namespace MBSimGUI {

  class Parameter;

  class EmbedItemData : public TreeItemData {
    protected:
      std::vector<Parameter*> parameter;
      std::vector<Parameter*> removedParameter;
      xercesc::DOMElement *element{nullptr}, *embed{nullptr};
      bool embeded{false}, embededParam{false};

    public:
      EmbedItemData()  = default;
      ~EmbedItemData() override;
      QString getName() const override { return QString::fromStdString(MBXMLUtils::E(element)->getAttribute("name")); }
      QString getValue() const override { return ""; }
      bool isActive();
      virtual EmbedItemData* getEmbedItemParent() { return nullptr; }
      std::vector<EmbedItemData*> getEmbedItemParents();
      int getNumberOfParameters() const { return parameter.size(); }
      Parameter* getParameter(int i) { return parameter[i]; }
      void setParameter(Parameter *param, int i) { parameter[i] = param; }
      void addParameter(Parameter *param);
      void removeParameter(Parameter *param);
      int getIndexOfParameter(Parameter *param) const;
      xercesc::DOMElement* getXMLElement() { return element; }
      void setXMLElement(xercesc::DOMElement *element_) { element = element_; }
      virtual void removeXMLElements();
      void removeXMLElement(bool removeEmbedding=true);
      xercesc::DOMElement* createParameterXMLElement();
      virtual xercesc::DOMElement* createEmbedXMLElement();
      xercesc::DOMElement* getEmbedXMLElement() { return embed; }
      void setEmbedXMLElement(xercesc::DOMElement *embed_) { embed = embed_; }
      virtual void maybeRemoveEmbedXMLElement();
      bool hasParameterXMLElement() const;
      bool getEmbeded() const { return embeded; }
      virtual void setEmbeded(bool embeded_) { embeded = embeded_; }
      bool getEmbededParameters() const { return embededParam; }
      void setEmbededParameters(bool embededParam_) { embededParam = embededParam_; }
      virtual EmbeddingPropertyDialog* createEmbeddingPropertyDialog() { return new EmbeddingPropertyDialog(this); }
      QMenu* createEmbeddingContextMenu() { return new EmbeddingContextMenu(this); }
      virtual xercesc::DOMElement* processIDAndHref(xercesc::DOMElement* element);
      virtual xercesc::DOMElement* processHref(xercesc::DOMElement* element);
      virtual void updateStatus() { }
  };

}

#endif
