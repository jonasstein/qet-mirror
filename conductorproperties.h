#ifndef CONDUCTOR_PROPERTIES_H
#define CONDUCTOR_PROPERTIES_H
#include "qet.h"
#include <QtGui>
#include <QtXml>
/**
	Cette classe represente les proprietes specifiques a un conducteur unifilaire
*/
class SingleLineProperties {
	public:
	SingleLineProperties();
	virtual ~SingleLineProperties();
	
	void setPhasesCount(int);
	unsigned short int phasesCount();
	void draw(QPainter *, QET::ConductorSegmentType, const QRectF &);
	void toXml(QDomDocument &, QDomElement &) const;
	void fromXml(QDomElement &);
	
	/// indique si le conducteur unifilaire doit afficher le symbole terre
	bool hasGround;
	/// indique si le conducteur unifilaire doit afficher le symbole neutre
	bool hasNeutral;
	
	int operator==(const SingleLineProperties &) const;
	int operator!=(const SingleLineProperties &) const;
	
	private:
	unsigned short int phases;
	void drawGround (QPainter *, QET::ConductorSegmentType, QPointF, qreal);
	void drawNeutral(QPainter *, QET::ConductorSegmentType, QPointF, qreal);
};

/**
	Cette classe represente les proprietes specifiques a un conducteur,
	en dehors de ses bornes et de son trajet.
*/
class ConductorProperties {
	// constructeurs, destructeur
	public:
	/**
		Constructeur : par defaut, les proprietes font un conducteur
		multifilaire dont le texte est "_"
	*/
	ConductorProperties() : type(Multi), text("_") {
	}
	
	/// Destructeur
	virtual ~ConductorProperties() {
	}
	
	/**
		Represente le type d'un conducteur :
		 * Simple : ni symbole ni champ de texte
		 * Single : symboles unifilaires, pas de champ de texte
		 * Multi : champ de texte, pas de symbole
	*/
	enum ConductorType { Simple, Single, Multi };
	
	// attributs
	/// type du conducteur
	ConductorType type;
	
	/// texte affiche si le conducteur est multifilaire
	QString text;
	
	/// proprietes si le conducteur est unifilaire
	SingleLineProperties singleLineProperties;
	
	// methodes
	void toXml(QDomDocument &, QDomElement &) const;
	void fromXml(QDomElement &);
	static QString typeToString(ConductorType);
	
	// operateurs
	int operator==(const ConductorProperties &);
	int operator!=(const ConductorProperties &);
};
#endif
