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

#ifndef _FUNCTION_WIDGETS_H_
#define _FUNCTION_WIDGETS_H_

#include "widget.h"

class ExtPhysicalVarWidget;
class ExtWidget;
class QVBoxLayout;
class QComboBox;
class ChoiceWidget;
class QStackedWidget;
class QListWidget;
class QSpinBox;

class Function1Widget : public Widget {
  Q_OBJECT
  public:
    Function1Widget() {}
    virtual ~Function1Widget() {}
  public slots:
    virtual void resize_(int m, int n) {}
};

class Function2Widget : public Widget {
  Q_OBJECT
  public:
    Function2Widget() {}
    virtual ~Function2Widget() {}
  public slots:
    virtual void resize_(int m, int n) {}
};

class SymbolicFunction1Widget : public Function1Widget {

  friend class SymbolicFunction1Property;

  public:
    SymbolicFunction1Widget(const QString &var, int max=99);
    int getArgDim() const;
  protected:
    ExtWidget *f;
    std::vector<ExtWidget*> argname, argdim;
};

class DifferentiableFunction1Widget : public Function1Widget {
  public:
    DifferentiableFunction1Widget() : Function1Widget(), order(0) {}
    //virtual ~DifferentiableFunction1() { delete derivatives[0]; derivatives.erase(derivatives.begin()); }
    const Function1Widget& getDerivative(int degree) const { return *(derivatives[degree]); }
    Function1Widget& getDerivative(int degree) { return *(derivatives[degree]); }
    void addDerivative(Function1Widget *diff) { derivatives.push_back(diff); }
    void setDerivative(Function1Widget *diff,size_t degree);

    void setOrderOfDerivative(int i) { order=i; }


  protected:
    std::vector<Function1Widget*> derivatives;
    int order;
};

class ConstantFunction1Widget : public Function1Widget {

  friend class ConstantFunction1Property;

  public:
    ConstantFunction1Widget(bool vec, int n);
    void resize_(int m, int n);
  protected:
    ExtWidget *c;
};

class QuadraticFunction1Widget : public DifferentiableFunction1Widget {

  friend class QuadraticFunction1Property;

  public:
    QuadraticFunction1Widget(int n);
    void resize_(int m, int n);

  protected:
    ExtWidget *a0, *a1, *a2;
};

class SinusFunction1Widget : public DifferentiableFunction1Widget {

  friend class SinusFunction1Property;

  public:
    SinusFunction1Widget(int n);
    void resize_(int m, int n);

  protected:
    ExtWidget *a, *f, *p, *o;
};

class TabularFunction1Widget : public Function1Widget {

  friend class TabularFunction1Property;

  public:
    TabularFunction1Widget(int n);

  protected:
    ChoiceWidget* choice;
};

class SummationFunction1Widget : public Function1Widget {
  Q_OBJECT

  friend class SummationFunction1Property;

  public:
    SummationFunction1Widget(int n);
    void resize_(int m, int n);

  protected:
    QStackedWidget *stackedWidget; 
    QListWidget *functionList; 
    int n;

  protected slots:
    void updateList();
    void addFunction();
    void removeFunction();
    void openContextMenu(const QPoint &pos);
    void changeCurrent(int idx);
  signals:
    void resize_();
};

class SymbolicFunction2Widget : public Function2Widget {

  friend class SymbolicFunction2Property;

  public:
    SymbolicFunction2Widget(const QStringList &var, int max=99);
    int getArgDim(int i) const;
  protected:
    ExtWidget *f;
    std::vector<ExtWidget*> argname, argdim;
};

class LinearSpringDamperForceWidget : public Function2Widget {

  friend class LinearSpringDamperForceProperty;

  public:
    LinearSpringDamperForceWidget();
  protected:
    ExtWidget *c, *d, *l0;
};

class LinearRegularizedBilateralConstraintWidget: public Function2Widget {

  friend class LinearRegularizedBilateralConstraintProperty;

  public:
    LinearRegularizedBilateralConstraintWidget(); 


  private:
    ExtWidget *c, *d;
};

class LinearRegularizedUnilateralConstraintWidget: public Function2Widget {

  friend class LinearRegularizedUnilateralConstraintProperty;

  public:
    LinearRegularizedUnilateralConstraintWidget(); 


  private:
    ExtWidget *c, *d;
};

class LinearRegularizedCoulombFrictionWidget: public Function2Widget {

  friend class LinearRegularizedCoulombFrictionProperty;

  public:
    LinearRegularizedCoulombFrictionWidget(); 


  private:
    ExtWidget *gd, *mu;
};

#endif
