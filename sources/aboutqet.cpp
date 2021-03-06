/*
	Copyright 2006-2013 The QElectroTech Team
	This file is part of QElectroTech.
	
	QElectroTech is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.
	
	QElectroTech is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with QElectroTech.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QtGui>
#include "qettabwidget.h"
#include "aboutqet.h"
#include "qet.h"
#include "qeticons.h"

/**
	Constructeur
	@param parent Le QWidget parent de la boite de dialogue
*/
AboutQET::AboutQET(QWidget *parent) : QDialog(parent) {
	// Titre, taille, comportement...
	setWindowTitle(tr("\300 propos de QElectrotech", "window title"));
	setMinimumWidth(680);
	setMinimumHeight(690);
	setModal(true);
	
	// Trois onglets
	QETTabWidget *tabs = new QETTabWidget(this);
	tabs -> addTab(aboutTab(),        tr("\300 &propos",       "tab title"));
	tabs -> addTab(authorsTab(),      tr("A&uteurs",           "tab title"));
	tabs -> addTab(translatorsTab(),  tr("&Traducteurs",       "tab title"));
	tabs -> addTab(contributorsTab(), tr("&Contributeurs",     "tab title"));
	tabs -> addTab(licenseTab(),      tr("&Accord de licence", "tab title"));
	
	// Un bouton pour fermer la boite de dialogue
	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(accept()));
	
	// Le tout dans une disposition verticale
	QVBoxLayout *vlayout = new QVBoxLayout();
	vlayout -> addWidget(title());
	vlayout -> addWidget(tabs);
	vlayout -> addWidget(buttons);
	setLayout(vlayout);
}

/**
	Destructeur
*/
AboutQET::~AboutQET() {
}

/**
	@return Le titre QElectroTech avec son icone
*/
QWidget *AboutQET::title() const {
	QWidget *icon_and_title = new QWidget();
	// icone
	QLabel *icon = new QLabel();
	icon -> setPixmap(QET::Icons::QETOxygenLogo.pixmap(48, 48));
	// label "QElectroTech"
	QLabel *title = new QLabel("<span style=\"font-weight:0;font-size:16pt;\">QElectroTech v" + QET::displayedVersion + "</span>");
	QString compilation_info = "<br />" + tr("Compilation : ") +  __DATE__ + " "  + __TIME__;
#ifdef __GNUC__
	compilation_info += " - GCC " + QString(__VERSION__);
	compilation_info += " - built with Qt " + QString(QT_VERSION_STR);
	compilation_info += " - run with Qt "+ QString(qVersion());
#endif
	title -> setText(title->text() + compilation_info);
	title -> setTextFormat(Qt::RichText);
	
	QHBoxLayout *hlayout = new QHBoxLayout();
	hlayout -> addWidget(icon);
	hlayout -> addWidget(title);
	hlayout -> addStretch();
	icon_and_title -> setLayout(hlayout);
	return(icon_and_title);
}

/**
	@return Le widget contenu par l'onglet "A propos"
*/
QWidget *AboutQET::aboutTab() const {
	QLabel *about = new QLabel(
		tr("QElectroTech, une application de r\351alisation de sch\351mas \351lectriques.", "about tab, description line") +
		"<br><br>" +
		tr("\251 2006-2013 Les d\351veloppeurs de QElectroTech", "about tab, developers line") +
		"<br><br>"
		"<a href=\"http://qelectrotech.org/\">http://qelectrotech.org/</a>"
		"<br><br>" +
		tr("Contact\240: <a href=\"mailto:qet@lists.tuxfamily.org\">qet@lists.tuxfamily.org</a>", "about tab, contact line")
	);
	about -> setAlignment(Qt::AlignCenter);
	about -> setOpenExternalLinks(true);
	about -> setTextFormat(Qt::RichText);
	return(about);
}

/**
	@return Le widget contenu par l'onglet "Auteurs"
*/
QWidget *AboutQET::authorsTab() const {
	QLabel *authors = new QLabel();
	addAuthor(authors, "Beno\356t Ansieau",  "benoit@qelectrotech.org",     tr("Id\351e originale"));
	addAuthor(authors, "Xavier Guerrin",     "xavier@qelectrotech.org",     tr("D\351veloppement"));
	addAuthor(authors, "Laurent Trinques",   "scorpio@qelectrotech.org",    tr("Collection d'\351l\351ments & D\351veloppement"));
	addAuthor(authors, "Cyril Frausti",      "cyril@qelectrotech.org",      tr("D\351veloppement"));
	addAuthor(authors, "Joshua Claveau",     "Joshua@qelectrotech.org",     tr("D\351veloppement"));

	authors -> setAlignment(Qt::AlignCenter);
	authors -> setOpenExternalLinks(true);
	authors -> setTextFormat(Qt::RichText);
	return(authors);
}

/**
	@return Le widget contenu par l'onglet "Traducteurs"
*/
QWidget *AboutQET::translatorsTab() const {
	QLabel *translators = new QLabel();
	
	addAuthor(translators, "Alfredo Carreto",            "electronicos_mx@yahoo.com.mx",tr("Traduction en espagnol"));
	addAuthor(translators, "Yuriy Litkevich",            "yuriy@qelectrotech.org",      tr("Traduction en russe"));
	addAuthor(translators, "Jos\351 Carlos Martins",     "jose@qelectrotech.org",       tr("Traduction en portugais"));
	addAuthor(translators, "Pavel Fric",                 "pavelfric@seznam.cz",         tr("Traduction en tch\350que"));
	addAuthor(translators, "Pawe&#x0142; &#x015A;miech", "pawel32640@gmail.com",        tr("Traduction en polonais"));
	addAuthor(translators, "Markus Budde",               "markus.budde@msn.com",        tr("Traduction en allemand"));
	addAuthor(translators, "Jonas Stein",                "news@jonasstein.de",          tr("Traduction en allemand"));
	addAuthor(translators, "Noah Braden",                "",                            tr("Traduction en allemand"));
	addAuthor(translators, "Gabi Mandoc",                "gabriel.mandoc@gic.ro",       tr("Traduction en roumain"));
	addAuthor(translators, "Alessandro Conti",           "dr.slump@alexconti.it",       tr("Traduction en italien"));
	addAuthor(translators, "Silvio",                     "silvio@qelectrotech.org",     tr("Traduction en italien"));
	addAuthor(translators, "Mohamed Souabni",            "souabnimohamed@yahoo.fr",     tr("Traduction en arabe"));
	addAuthor(translators, "Antun Marakovi&#x0107;",     "antun.marakovic@lolaribar.hr",tr("Traduction en croate"));
	addAuthor(translators, "Eduard Amor\363s",           "amoros@marmenuda.com",        tr("Traduction en catalan"));
	addAuthor(translators, "Nikos Papadopoylos",         "231036448@freemail.gr",       tr("Traduction en grec"));
	addAuthor(translators, "Yannis Gyftomitros",         "yang@hellug.gr",              tr("Traduction en grec"));
	addAuthor(translators, "Eduard Amoros",              "",                            tr("Traduction en catalan"));



	
	translators -> setOpenExternalLinks(true);
	translators -> setTextFormat(Qt::RichText);
	
	QWidget *translators_widget = new QWidget();
	QHBoxLayout *translators_layout = new QHBoxLayout(translators_widget);
	translators_layout -> addWidget(translators, 0, Qt::AlignCenter);
	return(translators_widget);
}

/**
	@return Le widget contenu par l'onglet "Contributeurs"
*/
QWidget *AboutQET::contributorsTab() const {
	QLabel *contributors = new QLabel();
	
	addAuthor(contributors, "Remi Collet",         "remi@fedoraproject.org",      tr("Paquets Fedora et Red Hat"));
	addAuthor(contributors, "Trem",                "trem@mandriva.org",           tr("Paquets Mandriva"));
	addAuthor(contributors, "Laurent Trinques",    "scorpio@qelectrotech.org",    tr("Paquets Debian"));
	addAuthor(contributors, "Markos Chandras",     "hwoarang@gentoo.org.",        tr("Paquets gentoo"));
	addAuthor(contributors, "Nuno Pinheiro",       "nuno@nuno-icons.com",         tr("Ic\364nes"));

	
	contributors -> setOpenExternalLinks(true);
	contributors -> setTextFormat(Qt::RichText);
	
	QWidget *contributors_widget = new QWidget();
	QHBoxLayout *contributors_layout = new QHBoxLayout(contributors_widget);
	contributors_layout -> addWidget(contributors, 0, Qt::AlignCenter);
	return(contributors_widget);
}

/**
	@return Le widget contenu par l'onglet "Accord de Licence"
*/
QWidget *AboutQET::licenseTab() const {
	QWidget *license = new QWidget();
	// label
	QLabel *title_license = new QLabel(tr("Ce programme est sous licence GNU/GPL."));
	
	// texte de la GNU/GPL dans une zone de texte scrollable non editable
	QTextEdit *text_license = new QTextEdit();
	text_license -> setPlainText(QET::license());
	text_license -> setReadOnly(true);
	
	// le tout dans une disposition verticale
	QVBoxLayout *license_layout = new QVBoxLayout();
	license_layout -> addWidget(title_license);
	license_layout -> addWidget(text_license);
	license -> setLayout(license_layout);
	return(license);
}

/**
	Ajoute une personne a la liste des auteurs
	@param label QLabel auquel sera ajoute la personne
	@param name  Nom de la personne
	@param email Adresse e-mail de la personne
	@param work  Fonction / travail effectue par la personne
*/
void AboutQET::addAuthor(QLabel *label, const QString &name, const QString &email, const QString &work) const {
	QString new_text = label -> text();
	
	QString author_template = "<span style=\"text-decoration: underline;\">%1</span> : %2 &lt;<a href=\"mailto:%3\">%3</a>&gt;&lrm;<br/><br/>";
	
	// ajoute la fonction de la personne
	new_text += author_template.arg(work).arg(name).arg(email);
	label -> setText(new_text);
}
