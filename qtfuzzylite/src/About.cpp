/* 
 * File:   About.cpp
 * Author: jcrada
 * 
 * Created on 25 January 2013, 6:00 PM
 */

#include "fl/qt/About.h"
#include "fl/qt/config.h"
#include <fl/Headers.h>
namespace fl {
    namespace qt {

        About::About(QWidget* parent, Qt::WindowFlags f)
        : QDialog(parent, f), ui(new Ui::About) {
            setWindowFlags(Qt::Dialog);

        }

        About::~About() {
            disconnect();
            delete ui;
        }

        void About::setup() {
            ui->setupUi(this);

            _generosityTerm.push_back("low");
            _generosityValue.push_back(10);
            _generosityTerm.push_back("Low");
            _generosityValue.push_back(20);
            _generosityTerm.push_back("LOW");
            _generosityValue.push_back(50);

            _generosityTerm.push_back("medium");
            _generosityValue.push_back(100);
            _generosityTerm.push_back("Medium");
            _generosityValue.push_back(200);
            _generosityTerm.push_back("MEDIUM");
            _generosityValue.push_back(500);

            _generosityTerm.push_back("high");
            _generosityValue.push_back(1000);
            _generosityTerm.push_back("High");
            _generosityValue.push_back(2000);
            _generosityTerm.push_back("HIGH");
            _generosityValue.push_back(5000);

            ui->sld_generosity->setMinimum(0);
            ui->sld_generosity->setMaximum(_generosityTerm.size() - 1);
            ui->sld_generosity->setValue((_generosityTerm.size() - 1) / 2);
            ui->sld_generosity->setSingleStep(1);
            ui->sld_generosity->setPageStep(1);
            ui->sld_generosity->setTickInterval(1);

            for (std::size_t i = 0; i < _generosityTerm.size(); ++i) {
                ui->cbx_generosity->addItem("$" + QString::number(_generosityValue.at(i)) + " - " + QString::fromStdString(_generosityTerm.at(i)));
            }

            ui->lbl_donate->setTextFormat(Qt::RichText);
            ui->lbl_donate->setOpenExternalLinks(true);

            layout()->setSizeConstraint(QLayout::SetFixedSize);
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            std::string header =
                    "<html>"
                    "<div align='center'>"
                    "<table>"
                    "<tr >"
                    "<td align='center' valign='middle'>"
                    "<a href='http://www.fuzzylite.com'><img src=':/icons/qtfuzzylite.svg'/></a>"
                    "</td>"
                    "<td align='left' valign='top'><b>qtfuzzylite v."
                    FLQT_VERSION " (" FLQT_DATE ")</b> "
                    "<br>"
                    "<b>fuzzylite v." FL_VERSION " (" FL_DATE ")</b>"
                    "<br>"
                    "</b>"
                    "<a href='http://www.fuzzylite.com'>www.fuzzylite.com</a>"
                    "<br>"
                    "<br>"
                    "Developed by Juan Rada-Vilela"
                    "<br>"
                    "<a href='mailto:jcrada@fuzzylite.com'>jcrada@fuzzylite.com</a></td>"
                    "</tr>"
                    "</table>"
                    "</div>"
                    "</html>";
            ui->lbl_header->setTextFormat(Qt::RichText);
            ui->lbl_header->setOpenExternalLinks(true);
            ui->lbl_header->setText(QString::fromStdString(header));
            connect();
            ui->cbx_generosity->setCurrentIndex((_generosityTerm.size() - 1) / 2);
        }

        void About::connect() {
            QObject::connect(ui->sld_generosity, SIGNAL(valueChanged(int)),
                    this, SLOT(onChangeSliderGenerosity(int)));
            QObject::connect(ui->cbx_generosity, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onChangeComboGenerosity(int)));
        }

        void About::disconnect() {
            QObject::disconnect(ui->sld_generosity, SIGNAL(valueChanged(int)),
                    this, SLOT(onChangeSliderGenerosity(int)));
            QObject::disconnect(ui->cbx_generosity, SIGNAL(currentIndexChanged(int)),
                    this, SLOT(onChangeComboGenerosity(int)));
        }

        void About::onChangeSliderGenerosity(int) {
            ui->cbx_generosity->setCurrentIndex(ui->sld_generosity->value());
        }

        void About::onChangeComboGenerosity(int) {
            if (ui->sld_generosity->value() != ui->cbx_generosity->currentIndex()) {
                ui->sld_generosity->setValue(ui->cbx_generosity->currentIndex());
            }
            changeDonation();
        }

        void About::changeDonation() {
            std::string href = "https://www.paypal.com/cgi-bin/webscr?"
                    "cmd=_s-xclick&hosted_button_id=NEP8FHAW8FJ8S&on0=Generosity";
            std::string param = "&os0=" + _generosityTerm.at(ui->sld_generosity->value());
            ui->lbl_donate->setText(QString::fromStdString(
                    "<qt><a href='" + href + param + "'>"
                    "<img src=':/icons/btn_donateCC_LG.gif'/></a></qt>"));
        }

    }
}