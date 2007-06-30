#ifndef PART_POLYGON_H
#define PART_POLYGON_H
#include <QtGui>
#include "customelementgraphicpart.h"
class PolygonEditor;
class PartPolygon : public QGraphicsPolygonItem, public CustomElementGraphicPart {
	// constructeurs, destructeur
	public:
	PartPolygon(QGraphicsItem * = 0, QGraphicsScene * = 0);
	virtual ~PartPolygon() {
		qDebug() << "~PartPolygon()";
	}
	
	private:
	PartPolygon(const PartPolygon &);
	
	// attributs
	private:
	bool closed;
	PolygonEditor *informations;
	
	/**
		constructeur
		paint()
		widget bidon pour l'edition
		methode pour poser le polygone :
			-mousePressEvent = pose un nouveau point
			-mouseMoveEvent = deplace ce point
			-mouveReleaseEvent = finalise ce point
		utiliser QPolygonF ; memoriser le point en cours (tout comme le
		partploygon en cours) et ne l'ajouter au qpolygonf que lors du
		mouseReleaseEvent
	*/
	// methodes
	public:
	void fromXml(const QDomElement &);
	const QDomElement toXml(QDomDocument &) const;
	void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
	void setClosed(bool c);
	bool isClosed() const;
	
	protected:
	QVariant itemChange(GraphicsItemChange, const QVariant &);
};
/**
	Specifie si le polygone doit etre ferme
	@param c true pour un polygone ferme, false sinon
*/
inline void PartPolygon::setClosed(bool c) {
	closed = c;
}

/**
	Indique si le polygone est ferme
	@return true si le polygone est ferme, false sinon
*/
inline bool PartPolygon::isClosed() const {
	return(closed);
}

#endif