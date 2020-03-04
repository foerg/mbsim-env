/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2017 Martin Förg

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
#include "embedding_context_menu.h"
#include "mainwindow.h"
#include "parameter.h"
#include "embedding_view.h"
#include "embeditemdata.h"

namespace MBSimGUI {

  extern MainWindow *mw;

  EmbeddingContextMenu::EmbeddingContextMenu(EmbedItemData *item_, const QString &title, QWidget *parent) : QMenu(title,parent), item(item_) {
    QAction *action = new QAction(QIcon::fromTheme("document-properties"), "Edit", this);
    connect(action,SIGNAL(triggered()),mw->getEmbeddingView(),SLOT(openEditor()));
    QMenu::addAction(action);
    action=new QAction(QIcon::fromTheme("document-properties"), "View XML", this);
    connect(action,SIGNAL(triggered()),mw,SLOT(viewEmbeddingSource()));
    QMenu::addAction(action);
    addSeparator();
    action = new QAction(QIcon::fromTheme("document-save-as"), "Save as", this);
    action->setEnabled(item->getNumberOfParameters());
    connect(action,SIGNAL(triggered()),mw,SLOT(saveEmbeddingAs()));
    addAction(action);
    addSeparator();
    action = new QAction(QIcon::fromTheme("edit-paste"), "Paste", this);
    action->setEnabled(mw->getParameterBuffer().first);
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction(QIcon::fromTheme("document-open"), "Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
    action = new QAction("Embed", this);
    action->setDisabled(item->getNumberOfParameters() or item->hasParameterXMLElement());
    connect(action,SIGNAL(triggered()),this,SLOT(embed()));
    addAction(action);
    addSeparator();
    action = new QAction(QIcon::fromTheme("edit-delete"), "Remove", this);
    action->setEnabled(item->getNumberOfParameters() and not(item->getEmbedItemParent() and item->getEmbedItemParent()->getEmbeded()));
    connect(action,SIGNAL(triggered()),this,SLOT(remove()));
    QMenu::addAction(action);
    addSeparator();
    action = new QAction("Add import parameter", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addImportParameter()));
    addAction(action);
    action = new QAction("Add matrix parameter", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addMatrixParameter()));
    addAction(action);
    action = new QAction("Add scalar parameter", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addScalarParameter()));
    addAction(action);
    action = new QAction("Add string parameter", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addStringParameter()));
    addAction(action);
    action = new QAction("Add vector parameter", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addVectorParameter()));
    addAction(action);
  }

  void EmbeddingContextMenu::addAction(QAction *action) {
    if(action->isEnabled()) action->setDisabled((item->getEmbedItemParent() and item->getEmbedItemParent()->getEmbeded()) or item->getEmbededParameters());
    QMenu::addAction(action);
  }

  void EmbeddingContextMenu::paste() {
    mw->loadParameter(item, mw->getParameterBuffer().first);
  }

  void EmbeddingContextMenu::load() {
    mw->loadParameter(item);
  }

  void EmbeddingContextMenu::embed() {
    mw->loadParameter(item,nullptr,true);
  }

  void EmbeddingContextMenu::remove() {
    mw->removeParameter(item);
  }

  void EmbeddingContextMenu::addScalarParameter() {
    mw->addParameter(new ScalarParameter, item);
  }

  void EmbeddingContextMenu::addVectorParameter() {
    mw->addParameter(new VectorParameter, item);
  }

  void EmbeddingContextMenu::addMatrixParameter() {
    mw->addParameter(new MatrixParameter, item);
  }

  void EmbeddingContextMenu::addStringParameter() {
    mw->addParameter(new StringParameter, item);
  }

  void EmbeddingContextMenu::addImportParameter() {
    mw->addParameter(new ImportParameter, item);
  }

}
