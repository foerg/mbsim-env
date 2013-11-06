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

#include <config.h>
#include "treeitem.h"
#include <iostream>

using namespace std;

TreeItem::~TreeItem() {
  qDeleteAll(childItems);
}

int TreeItem::childNumber() const {
  if (parentItem)
    return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

  return 0;
}

bool TreeItem::insertChildren(TreeItem *item, int count) {

  ID++;

  for (int row = 0; row < count; ++row)
    childItems.insert(childItems.count(), item);

  return true;
}

bool TreeItem::removeChildren(int position, int count) {
  if (position < 0 || position + count > childItems.size())
    return false;

  for (int row = 0; row < count; ++row)
    delete childItems.takeAt(position);

  return true;
}

PropertyTreeItem::PropertyTreeItem(PropertyTreeItemData *itemData, PropertyTreeItem *parent, int ID_) : itemData(itemData), parentItem(parent), ID(ID_) {
      getData_[0] = &PropertyTreeItem::getData0;
      getData_[1] = &PropertyTreeItem::getData1;
      getData_[2] = &PropertyTreeItem::getData2;
      getData_[3] = &PropertyTreeItem::getData3;
      setData_[0] = &PropertyTreeItem::setData0;
      setData_[1] = &PropertyTreeItem::setData1;
      setData_[2] = &PropertyTreeItem::setData2;
      setData_[3] = &PropertyTreeItem::setData3;
    }
PropertyTreeItem::~PropertyTreeItem() {
  qDeleteAll(childItems);
}

int PropertyTreeItem::childNumber() const {
  if (parentItem)
    return parentItem->childItems.indexOf(const_cast<PropertyTreeItem*>(this));

  return 0;
}

bool PropertyTreeItem::insertChildren(PropertyTreeItem *item, int count) {

  ID++;

  for (int row = 0; row < count; ++row)
    childItems.insert(childItems.count(), item);

  return true;
}

bool PropertyTreeItem::removeChildren(int position, int count) {
  if (position < 0 || position + count > childItems.size())
    return false;

  for (int row = 0; row < count; ++row)
    delete childItems.takeAt(position);

  return true;
}
