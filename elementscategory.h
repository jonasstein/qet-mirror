#ifndef ELEMENTS_CATEGORY_H
#define ELEMENTS_CATEGORY_H
#include <QtCore>
#include "nameslist.h"
/**
	Cette classe represente une categorie d'elements.
	Une categorie d'elements est en fait un dossier avec un fichier
	qet_directory contenant ses caracteristiques (pour le moment : ses noms).
*/
class ElementsCategory : public QDir {
	// constructeurs, destructeur
	public:
	ElementsCategory(const QString & = QString());
	virtual ~ElementsCategory();
	
	private:
	ElementsCategory(const ElementsCategory &);
	
	// attributs
	private:
	NamesList category_names;
	
	// methodes
	public:
	QString name() const;
	NamesList categoryNames() const;
	void clearNames();
	void addName(const QString &, const QString &);
	bool write() const;
	bool remove() const;
	bool isWritable() const;
	//bool move(const QString &new_parent);
	
	private:
	bool rmdir(const QString &) const;
	void loadNames();
};
#endif