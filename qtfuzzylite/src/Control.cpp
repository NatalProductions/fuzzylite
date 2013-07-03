/**
    This file is part of qtfuzzylite.

    qtfuzzylite is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    qtfuzzylite is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with qtfuzzylite.  If not, see <http://www.gnu.org/licenses/>.

    Juan Rada-Vilela, 01 February 2013
    jcrada@fuzzylite.com
 **/

/*
 * Control.cpp
 *
 *  Created on: 18/12/2012
 *      Author: jcrada
 */

#include "fl/qt/Control.h"

#include "fl/qt/qtfuzzylite.h"

#include <QMessageBox>
#include <QMenu>
#include <QSignalMapper>
#include <QAction>
#include <QInputDialog>

#include "fl/qt/Model.h"
namespace fl {
    namespace qt {

        Control::Control(QWidget* parent, Qt::WindowFlags f) :
        Viewer(parent, f), _outputIndex(0), _outputView(false),
        _isTakagiSugeno(false), _minOutput(fl::nan), _maxOutput(fl::nan),
        _allowsOutputView(false) { }

        Control::~Control() {
            //Exception on reset?
            //            if (dynamic_cast<InputVariable*> (variable)) {
            //                QObject::disconnect(this, SIGNAL(valueChanged(double)),
            //                        this, SLOT(updateInput(double)));
            //            }
        }

        void Control::setup(const fl::Variable* model) {
            Viewer::setup(model);
            this->variable = const_cast<fl::Variable*> (model);
            if (dynamic_cast<fl::OutputVariable*> (variable)) {
                ui->sld_x->setEnabled(false);
                ui->sbx_x->setVisible(false);
                ui->led_x->setVisible(true);
                ui->lbl_fuzzy_out->setVisible(true);
                ui->lbl_fuzzy->setVisible(false);
                ui->btn_name->setVisible(true);
                ui->btn_name->setIcon(QIcon(":/output.png"));

                _minOutput = variable->getMinimum();
                _maxOutput = variable->getMaximum();

                _isTakagiSugeno = false;
                for (int i = 0; i < variable->numberOfTerms(); ++i) {
                    if (variable->getTerm(i)->className() == Constant().className() or
                            variable->getTerm(i)->className() == Linear().className()) {
                        _isTakagiSugeno = true;
                        break;
                    }
                }
            } else if (dynamic_cast<InputVariable*> (variable)) {
                QObject::connect(this, SIGNAL(valueChanged(double)),
                        this, SLOT(updateInput(double)));
                ui->btn_name->setIcon(QIcon(":/input.png"));
            }
        }

        void Control::setAllowOutputView(bool allow) {
            _allowsOutputView = allow;
            if (_allowsOutputView) {
                fl::OutputVariable* outputVariable = dynamic_cast<fl::OutputVariable*> (variable);
                if (outputVariable) {
                    _outputs = std::vector<scalar>(
                            250,
                            (outputVariable->getMaximum() + outputVariable->getMinimum()) / 2.0);
                    _outputIndex = 0;
                    _minOutput = variable->getMinimum();
                    _maxOutput = variable->getMaximum();
                    if (_isTakagiSugeno) onActionVariableName("output view");
                }
            }
        }

        bool Control::allowsOutputView() const {
            return this->_allowsOutputView;
        }

        void Control::updateInput(double value) {
            fl::InputVariable* inputVariable = dynamic_cast<fl::InputVariable*> (variable);
            if (inputVariable) {
                inputVariable->setInput(value);
            }
        }

        void Control::onChangeSliderValue(int position) {
            //            Viewer::onChangeSliderValue(position);
            if (not _outputView) {
                Viewer::onChangeSliderValue(position);
            } else {
            }
        }

        void Control::onEditInputValue() {
            if (not _outputView) {
                Viewer::onEditInputValue();
            } else {

                scalar min = fl::Op::min(_minOutput, variable->getMinimum());
                scalar max = fl::Op::max(_maxOutput, variable->getMaximum());

                scalar value = ui->sbx_x->value();
                scalar sliderValue = fl::Op::scale(ui->sld_x->value(),
                        ui->sld_x->minimum(), ui->sld_x->maximum(),
                        min, max);

                double tolerance = 1.0 / std::pow(10.0, ui->sbx_x->decimals());
                if (not fl::Op::isEq(value, sliderValue, tolerance)) {
                    int position = (int) fl::Op::scale(value,
                            min, max,
                            ui->sld_x->minimum(), ui->sld_x->maximum());
                    if (position != ui->sld_x->value()) {
                        ui->sld_x->setValue(position);
                    }
                }

                emit valueChanged(value);
            }
        }

        void Control::updateOutput() {
            fl::OutputVariable* outputVariable = dynamic_cast<fl::OutputVariable*> (variable);
            if (not outputVariable) {
                throw fl::Exception("[cast error] trying to cast OutputVariable", FL_AT);
            }

            scalar y = outputVariable->defuzzify();

            if (_outputView) {
                _outputs.at(_outputIndex) = y;
                _outputIndex = (_outputIndex + 1) % _outputs.size();
                if (not (fl::Op::isNan(y) or fl::Op::isInf(y))) {
                    if (y > _maxOutput) _maxOutput = y;
                    if (y < _minOutput) _minOutput = y;
                }
            }
            ui->led_x->setText(QString::fromStdString(
                    fl::Op::str(y, qtfuzzylite::decimals())));
            ui->sbx_x->setValue(y);

            QString fuzzify = QString::fromStdString(
                    outputVariable->fuzzify(y, qtfuzzylite::decimals()));
            ui->lbl_fuzzy_out->setText("&#956;=" + fuzzify);

            refresh();
            if (not outputVariable->output()->isEmpty())
                draw(outputVariable->output());

        }

        void Control::onClickVariableName() {
            QMenu menu(this);
            std::vector<QAction*> actions;
            if (not ui->wdg_canvas->isVisible()) {
                actions.push_back(new QAction("maximize", this));
            } else {
                actions.push_back(new QAction("minimize", this));

                if (allowsOutputView()) {
                    actions.push_back(NULL);

                    if (not _isTakagiSugeno) {
                        QAction* actionView = new QAction("output view", this);
                        if (_outputView) {
                            actionView->setCheckable(true);
                            actionView->setChecked(true);
                        }
                        actions.push_back(actionView);
                    }

                    if (_outputView) {
                        actions.push_back(new QAction("resolution...", this));
                        actions.push_back(NULL);
                        actions.push_back(new QAction("clear", this));
                    }
                }
            }

            QSignalMapper signalMapper(this);
            for (std::size_t i = 0; i < actions.size(); ++i) {
                if (actions.at(i)) {
                    signalMapper.setMapping(actions.at(i), actions.at(i)->text());
                    QObject::connect(actions.at(i), SIGNAL(triggered()),
                            &signalMapper, SLOT(map()));

                    menu.addAction(actions.at(i));
                } else {
                    menu.addSeparator();
                }
            }
            QObject::connect(&signalMapper, SIGNAL(mapped(const QString &)),
                    this, SLOT(onActionVariableName(const QString &)));

            menu.exec(QCursor::pos() + QPoint(1, 0));

            for (std::size_t i = 0; i < actions.size(); ++i) {
                if (actions.at(i)) {
                    actions.at(i)->deleteLater();
                }
            }
            ui->btn_name->setChecked(false);
        }

        void Control::onActionVariableName(const QString& action) {
            if (action == "maximize") {
                ui->wdg_canvas->setVisible(true);
                ui->wdg_out->setVisible(true);

                if (_outputView) {
                    ui->mainLayout->removeWidget(ui->sld_x);
                    ui->sld_x->setOrientation(Qt::Vertical);
                    ui->lyt_canvas->addWidget(ui->sld_x);
                    ui->canvas->setBackgroundBrush(QBrush(Qt::black));
                }
                setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                setMinimumSize(0, 0);
                setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            } else if (action == "minimize") {
                ui->wdg_canvas->setVisible(false);
                ui->wdg_out->setVisible(false);

                if (_outputView) {
                    ui->canvas->setBackgroundBrush(QBrush(Qt::white));
                    ui->lyt_canvas->removeWidget(ui->sld_x);
                    ui->sld_x->setOrientation(Qt::Horizontal);
                    ui->mainLayout->addWidget(ui->sld_x);
                    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                    setFixedHeight(minimumSizeHint().height() + ui->sld_x->sizeHint().height());
                } else {
                    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                    setFixedHeight(minimumSizeHint().height());
                }

            } else if (action == "output view") {
                _outputView = not _outputView;
                if (_outputView) {
                    ui->mainLayout->removeWidget(ui->sld_x);
                    ui->sld_x->setOrientation(Qt::Vertical);
                    ui->lyt_canvas->addWidget(ui->sld_x);
                    ui->canvas->setBackgroundBrush(QBrush(Qt::black));
                } else {
                    ui->canvas->setBackgroundBrush(QBrush(Qt::white));
                    ui->lyt_canvas->removeWidget(ui->sld_x);
                    ui->sld_x->setOrientation(Qt::Horizontal);
                    ui->mainLayout->addWidget(ui->sld_x);
                }
            } else if (action == "resolution...") {
                bool ok;
                int resolution = QInputDialog::getInteger(this,
                        "Resolution of Output View",
                        "How many defuzzified values do you want to show?", 
                        _outputs.size(), 50, 1000, 10, &ok);
                if (ok) { //clear
                    fl::OutputVariable* outputVariable =
                            dynamic_cast<fl::OutputVariable*> (variable);
                    _outputs = std::vector<scalar>(
                            resolution,
                            (outputVariable->getMaximum() + outputVariable->getMinimum()) / 2.0);
                    _outputIndex = 0;
                    _minOutput = outputVariable->getMinimum();
                    _maxOutput = outputVariable->getMaximum();
                    ui->sbx_x->setValue((_maxOutput + _minOutput) / 2.0);
                }
            } else if (action == "clear") {
                fl::OutputVariable* outputVariable =
                        dynamic_cast<fl::OutputVariable*> (variable);
                _outputs = std::vector<scalar>(
                        _outputs.size(),
                        (outputVariable->getMaximum() + outputVariable->getMinimum()) / 2.0);
                _outputIndex = 0;
                _minOutput = outputVariable->getMinimum();
                _maxOutput = outputVariable->getMaximum();
                ui->sbx_x->setValue((_maxOutput + _minOutput) / 2.0);
            }
            if (not (action == "clear" or action == "resolution...")) {
                if (parentWidget()) parentWidget()->adjustSize();
                adjustSize();
            }

            if (action == "maximize") emit signalRefresh();
            else refresh();
        }

        void Control::refresh() {
            if (not _outputView) {
                Viewer::refresh();
                return;
            }
            draw();
        }

        void Control::draw() {
            if (not _outputView) {
                Viewer::draw();
                return;
            }

            fl::OutputVariable* outputVariable =
                    dynamic_cast<fl::OutputVariable*> (variable);

            scalar min = fl::Op::min(_minOutput, outputVariable->getMinimum());
            scalar max = fl::Op::max(_maxOutput, outputVariable->getMaximum());

            ui->lbl_min->setText(QString::fromStdString(
                    fl::Op::str(min, qtfuzzylite::decimals())));
            ui->lbl_max->setText(QString::fromStdString(
                    fl::Op::str(max, qtfuzzylite::decimals())));

            ui->canvas->scene()->clear();
            ui->canvas->scene()->setSceneRect(ui->canvas->viewport()->rect());
            QRect rect = ui->canvas->viewport()->rect();

            QPainterPath path;
            scalar x0ui, y0ui, x1ui, y1ui;
            for (std::size_t i = 0; i + 1 < _outputs.size(); ++i) {
                int ix0 = (_outputIndex + i) % _outputs.size();
                scalar y0 = _outputs.at(ix0);

                int ix1 = (_outputIndex + i + 1) % _outputs.size();
                scalar y1 = _outputs.at(ix1);

                x0ui = fl::Op::scale(i, 0, _outputs.size(),
                        rect.left(), rect.right());
                y0ui = fl::Op::scale(y0, min, max,
                        rect.bottom(), rect.top());

                x1ui = fl::Op::scale(i + 1, 0, _outputs.size(),
                        rect.left(), rect.right());
                y1ui = fl::Op::scale(y1, min, max,
                        rect.bottom(), rect.top());

                if (fl::Op::isNan(y0ui) or fl::Op::isInf(y0ui)) {
                    path.moveTo(x1ui, y1ui);
                } else {
                    path.moveTo(x0ui, y0ui);
                }

                path.lineTo(x1ui, y1ui);
            }

            QPen pen;
            pen.setColor(QColor(0, 210, 0, 200));
            pen.setStyle(Qt::SolidLine);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::MiterJoin);
            pen.setWidth(3);

            ui->canvas->scene()->addPath(path, pen);
        }

        void Control::draw(const fl::Term* term, const QColor& color) {
            if (not _outputView) {
                Viewer::draw(term, color);
                return;
            }
        }

    }
}
