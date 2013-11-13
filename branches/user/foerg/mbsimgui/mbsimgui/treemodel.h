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

#ifndef _TREEMODEL_H
#define _TREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class Element;
class Frame;
class Contour;
class Group;
class Object;
class Link;
class Observer;
class Parameter;
class Property;
class TreeItem;
class PropertyTreeItem;

class TreeModel : public QAbstractItemModel {
  public:
    TreeModel(QObject *parent = 0);
    ~TreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &) const {return 2;}

    Qt::ItemFlags flags(const QModelIndex &index) const;
//    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
//    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

    TreeItem *getItem(const QModelIndex &index) const;

 protected:
    TreeItem *rootItem;
};

//class ElementTreeModel : public TreeModel {
//  public:
//    ElementTreeModel(QObject *parent = 0);
//
//    void createFrameItem(Frame *frame, const QModelIndex &parent = QModelIndex());
//    void createContourItem(Contour *contour, const QModelIndex &parent = QModelIndex());
//    void createGroupItem(Group *group, const QModelIndex &parent = QModelIndex());
//    void createObjectItem(Object *object, const QModelIndex &parent = QModelIndex());
//    void createLinkItem(Link *link, const QModelIndex &parent = QModelIndex());
//    void createObserverItem(Observer *observer, const QModelIndex &parent = QModelIndex());
//
//    std::map<std::string, QModelIndex> idEleMap;
//};

class ParameterListModel : public TreeModel {
  public:
    ParameterListModel(QObject *parent = 0);

    void createParameterItem(Parameter *parameter, const QModelIndex &parent = QModelIndex());
};

//class PropertyTreeModel : public TreeModel {
//  public:
//    PropertyTreeModel(QObject *parent = 0);
//
//    void createPropertyItem(Property *property, const QModelIndex &parent = QModelIndex());
//};

class PropertyTreeModel : public QAbstractItemModel {
  public:
    PropertyTreeModel(QObject *parent = 0);
    ~PropertyTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &) const {return 4;}

    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());

    TreeItem *getItem(const QModelIndex &index) const;

    void createPropertyItem(Property *property, const QModelIndex &parent = QModelIndex());
    void createFrameItem(Frame *frame, const QModelIndex &parent = QModelIndex());
    void createContourItem(Contour *contour, const QModelIndex &parent = QModelIndex());
    void createGroupItem(Group *group, const QModelIndex &parent = QModelIndex());
    void createObjectItem(Object *object, const QModelIndex &parent = QModelIndex());
    void createLinkItem(Link *link, const QModelIndex &parent = QModelIndex());
    void createObserverItem(Observer *observer, const QModelIndex &parent = QModelIndex());

    std::map<std::string, QModelIndex> idEleMap;

 protected:
    TreeItem *rootItem;
};

#endif
