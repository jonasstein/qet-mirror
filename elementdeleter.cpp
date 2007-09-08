#include "elementdeleter.h"

ElementDeleter::ElementDeleter(const QString &elmt_path, QWidget *parent) :
	QWidget(parent),
	element_path(elmt_path)
{
}

ElementDeleter::~ElementDeleter() {
}

void ElementDeleter::exec() {
	// verifie l'existence de l'element
	QFile elmt_file(element_path);
	if (!elmt_file.exists()) return;
	
	// confirmation #1
	QMessageBox::StandardButton answer_1 = QMessageBox::question(
		this,
		tr("Supprimer l'\351l\351ment ?"),
		tr("\312tes-vous s\373r de vouloir supprimer cet \351l\351ment ?\n"),
		QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel
	);
	if (answer_1 != QMessageBox::Yes) return;
	
	// supprime l'element
	if (!elmt_file.remove()) {
		QMessageBox::warning(
			this,
			tr("Suppression de l'\351l\351ment"),
			tr("La suppression de l'\351l\351ment a \351chou\351.\n"
			"V\351rifiez vos droits sur le fichier ") + element_path + tr(".")
		);
	}
}