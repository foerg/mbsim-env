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

#ifndef _PROJECT_VIEW__H_
#define _PROJECT_VIEW__H_

#include <QLineEdit>
#include <QMenu>

namespace MBSimGUI {

  class Project;
  class ProjectPropertyDialog;

  class ProjectViewContextMenu : public QMenu {
    public:
      ProjectViewContextMenu(QWidget * parent = nullptr);
  };

  class ProjectView : public QLineEdit {
    public:
      ProjectView();
      ~ProjectView() override = default;
      bool editorIsOpen() { return editor; }
      Project* getProject() { return project; }
      void setProject(Project *project_) { project = project_; }
      void updateName();
      QMenu* createContextMenu() { return new ProjectViewContextMenu; }
      void openEditor();
    protected:
      Project *project;
      ProjectPropertyDialog *editor{nullptr};
      void openContextMenu();
      void dialogFinished(int result);
      void apply();
  };

  class ProjectMouseEvent : public QObject {
    public:
      ProjectMouseEvent(ProjectView* view_) : QObject(view_), view(view_) { }
    protected:
      ProjectView *view;
      ProjectPropertyDialog *editor;
      bool eventFilter(QObject *obj, QEvent *event) override;
  };

}

#endif
