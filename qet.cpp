#include "qet.h"

/**
	Permet de convertir une chaine de caracteres ("n", "s", "e" ou "w")
	en orientation. Si la chaine fait plusieurs caracteres, seul le
	premier est pris en compte. En cas d'incoherence, QET::North est
	retourne.
	@param s Chaine de caractere cense representer une orientation
	@return l'orientation designee par la chaine de caractere
*/
QET::Orientation QET::orientationFromString(const QString &s) {
	QChar c = s[0];
	if (c == 'e') return(QET::East);
	else if (c == 's') return(QET::South);
	else if (c == 'w') return (QET::West);
	else return(QET::North);
}

/**
	@param o une orientation
	@return une chaine de caractere representant l'orientation
*/
QString QET::orientationToString(QET::Orientation o) {
	QString ret;
	switch(o) {
		case QET::North: ret = "n"; break;
		case QET::East : ret = "e"; break;
		case QET::South: ret = "s"; break;
		case QET::West : ret = "w"; break;
	}
	return(ret);
}

/**
	Indique si deux orientations de Borne sont sur le meme axe (Vertical / Horizontal).
	@param a La premiere orientation de Borne
	@param b La seconde orientation de Borne
	@return Un booleen a true si les deux orientations de bornes sont sur le meme axe
*/
bool QET::surLeMemeAxe(QET::Orientation a, QET::Orientation b) {
	if ((a == QET::North || a == QET::South) && (b == QET::North || b == QET::South)) return(true);
	else if ((a == QET::East || a == QET::West) && (b == QET::East || b == QET::West)) return(true);
	else return(false);
}

/**
	Indique si une orientation de borne est horizontale (Est / Ouest).
	@param a L'orientation de borne
	@return True si l'orientation de borne est horizontale, false sinon
*/
bool QET::estHorizontale(QET::Orientation a) {
	return(a == QET::East || a == QET::West);
}

/**
	Indique si une orientation de borne est verticale (Nord / Sud).
	@param a L'orientation de borne
	@return True si l'orientation de borne est verticale, false sinon
*/
bool QET::estVerticale(QET::Orientation a) {
	return(a == QET::North || a == QET::South);
}

/**
	Permet de connaitre l'orientation suivante apres celle donnee en parametre.
	Les orientations sont generalement presentees dans l'ordre suivant : Nord,
	Est, Sud, Ouest.
	@param o une orientation
	@return l'orientation suivante
*/
QET::Orientation QET::nextOrientation(QET::Orientation o) {
	if (o < 0 || o > 2) return(QET::North);
	return((QET::Orientation)(o + 1));
}

/**
	Permet de connaitre l'orientation precedant celle donnee en parametre.
	Les orientations sont generalement presentees dans l'ordre suivant : Nord,
	Est, Sud, Ouest.
	@param o une orientation
	@return l'orientation precedente
*/
QET::Orientation QET::previousOrientation(QET::Orientation o) {
	if (o < 0 || o > 3) return(QET::North);
	if (o == QET::North) return(QET::West);
	return((QET::Orientation)(o - 1));
}

/**
	Permet de savoir si l'attribut nom_attribut d'un element XML e est bien un
	entier. Si oui, sa valeur est copiee dans entier.
	@param e Element XML
	@param nom_attribut Nom de l'attribut a analyser
	@param entier Pointeur facultatif vers un entier
	@return true si l'attribut est bien un entier, false sinon
*/
bool QET::attributeIsAnInteger(const QDomElement &e, QString nom_attribut, int *entier) {
	// verifie la presence de l'attribut
	if (!e.hasAttribute(nom_attribut)) return(false);
	// verifie la validite de l'attribut
	bool ok;
	int tmp = e.attribute(nom_attribut).toInt(&ok);
	if (!ok) return(false);
	if (entier != NULL) *entier = tmp;
	return(true);
}

/**
	Permet de savoir si l'attribut nom_attribut d'un element XML e est bien un
	reel. Si oui, sa valeur est copiee dans reel.
	@param e Element XML
	@param nom_attribut Nom de l'attribut a analyser
	@param reel Pointeur facultatif vers un double
	@return true si l'attribut est bien un reel, false sinon
*/
bool QET::attributeIsAReal(const QDomElement &e, QString nom_attribut, double *reel) {
	// verifie la presence de l'attribut
	if (!e.hasAttribute(nom_attribut)) return(false);
	// verifie la validite de l'attribut
	bool ok;
	qreal tmp = e.attribute(nom_attribut).toDouble(&ok);
	if (!ok) return(false);
	if (reel != NULL) *reel = tmp;
	return(true);
}

/**
	Permet de composer rapidement la proposition "x elements et y conducteurs"
	@param elements_count nombre d �lements
	@param conductors_count nombre de conducteurs
	@return la proposition decrivant le nombre d'elements et de conducteurs
*/
QString QET::ElementsAndConductorsSentence(int elements_count, int conductors_count, int texts_count) {
	QString text;
	if (elements_count) {
		text += QString::number(elements_count) + " ";
		text += elements_count > 1 ? QObject::tr("\351l\351ments") : QObject::tr("\351l\351ment");
		if (conductors_count ^ texts_count) text += QObject::tr(" et ");
		else if (conductors_count && texts_count) text += QObject::tr(", ");
	}
	if (conductors_count) {
		text += QString::number(conductors_count) + " ";
		text += conductors_count > 1 ? QObject::tr("conducteurs") : QObject::tr("conducteur");
		if (texts_count) text += QObject::tr(" et ");
	}
	if (texts_count) {
		text += QString::number(texts_count) + " ";
		text += texts_count > 1 ? QObject::tr("champs de texte") : QObject::tr("champ de texte");
	}
	return(text);
}

/**
	Etant donne un element XML e, renvoie la liste de tous les elements
	children imbriques dans les elements parent, eux-memes enfants de l'elememt e
	@param e Element XML a explorer
	@param parent tag XML intermediaire
	@param children tag XML a rechercher
	@return La liste des elements XML children
*/
QList<QDomElement> QET::findInDomElement(const QDomElement &e, const QString &parent, const QString &children) {
	QList<QDomElement> return_list;
	
	// parcours des elements parents
	for (QDomNode enfant = e.firstChild() ; !enfant.isNull() ; enfant = enfant.nextSibling()) {
		// on s'interesse a l'element XML "parent"
		QDomElement parents = enfant.toElement();
		if (parents.isNull() || parents.tagName() != parent) continue;
		// parcours des enfants de l'element XML "parent"
		for (QDomNode node_children = parents.firstChild() ; !node_children.isNull() ; node_children = node_children.nextSibling()) {
			// on s'interesse a l'element XML "children"
			QDomElement n_children = node_children.toElement();
			if (!n_children.isNull() && n_children.tagName() == children) return_list.append(n_children);
		}
	}
	return(return_list);
}
