#include "partline.h"
#include "lineeditor.h"
#include <cmath>

PartLine::PartLine(QGraphicsItem *parent, QGraphicsScene *scene) : QGraphicsLineItem(parent, scene), CustomElementGraphicPart() {
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
	setAcceptedMouseButtons(Qt::LeftButton);
	informations = new LineEditor(this);
	style_editor -> appendWidget(informations);
}

void PartLine::paint(QPainter *painter, const QStyleOptionGraphicsItem */*q*/, QWidget */*w*/) {
	applyStylesToQPainter(*painter);
	QPen t = painter -> pen();
	if (isSelected()) {
		t.setColor(Qt::red);
		painter -> setPen(t);
	}
	painter -> setBrush(Qt::NoBrush);
	painter -> drawLine(line());
}

const QDomElement PartLine::toXml(QDomDocument &xml_document) const {
	QDomElement xml_element = xml_document.createElement("line");
	QPointF p1(sceneP1());
	QPointF p2(sceneP2());
	xml_element.setAttribute("x1", p1.x());
	xml_element.setAttribute("y1", p1.y());
	xml_element.setAttribute("x2", p2.x());
	xml_element.setAttribute("y2", p2.y());
	stylesToXml(xml_element);
	return(xml_element);
}

void PartLine::fromXml(const QDomElement &qde) {
	stylesFromXml(qde);
	setLine(
		QLineF(
			mapFromScene(
				qde.attribute("x1", "0").toDouble(),
				qde.attribute("y1", "0").toDouble()
			),
			mapFromScene(
				qde.attribute("x2", "0").toDouble(),
				qde.attribute("y2", "0").toDouble()
			)
		)
	);
}

QVariant PartLine::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (scene()) {
		if (change == QGraphicsItem::ItemPositionChange || change == QGraphicsItem::ItemSelectedChange) {
			informations -> updateForm();
		}
	}
	return(QGraphicsLineItem::itemChange(change, value));
}

QPointF PartLine::sceneP1() const {
	return(mapToScene(line().p1()));
}

QPointF PartLine::sceneP2() const {
	return(mapToScene(line().p2()));
}

/*




*/

QPainterPath PartLine::shape() const {
	QList<QPointF> points = fourShapePoints();
	QPainterPath t;
	t.setFillRule(Qt::WindingFill);
	t.moveTo(points.at(0));
	t.lineTo(points.at(1));
	t.lineTo(points.at(2));
	t.lineTo(points.at(3));
	t.lineTo(points.at(0));
	return(t);
}

/*
QRectF PartLine::boundingRect() const {
	QList<QPointF> points = fourShapePoints();
	qreal min_x = points.first().x();
	qreal max_x = points.first().x();
	qreal min_y = points.first().y();
	qreal max_y = points.first().y();
	foreach(QPointF p, points) {
		if (p.x() > max_x) max_x = p.x();
		if (p.x() < min_x) min_x = p.x();
		if (p.y() > max_y) max_y = p.y();
		if (p.y() < min_y) min_y = p.y();
	}
	QRectF r;
	r.setCoords(min_x, min_y, max_x, max_y);
	return(r);
}
*/
/**
	@return une liste contenant les deux points de la droite + les 4 points entourant ces deux points
*/
QList<QPointF> PartLine::fourShapePoints() const {
	// on a donc A(xa , ya) et B(xb, yb)
	QPointF a = line().p1();
	QPointF b = line().p2();
	
	// on calcule le vecteur AB : (xb-xa, yb-ya)
	QPointF v_ab = b - a;
	
	// et la distance AB : racine des coordonnees du vecteur au carre
	qreal ab = sqrt(pow(v_ab.x(), 2) + pow(v_ab.y(), 2));
	
	// ensuite on definit le vecteur u(a, b) qui est egal au vecteur AB divise
	// par sa longueur et multiplie par la longueur de la marge  que tu veux
	// laisser
	QPointF u = v_ab / ab * 2.0;
	
	// on d�finit le vecteur v(-b , a)  qui est perpendiculaire � AB
	QPointF v(-u.y(), u.x());
	QPointF m = -u + v; // on a le vecteur M = -u + v
	QPointF n = -u - v; // et le vecteur N=-u-v
	QPointF h =  a + m; // H = A + M
	QPointF k =  a + n; // K = A + N
	QPointF i =  b - n; // I = B - N
	QPointF j =  b - m; // J = B - M
	
	QList<QPointF> result;
	result << h << i << j << k;
	return(result);
}

QRectF PartLine::boundingRect() const {
	qreal adjust = 1.5;
	QRectF r(QGraphicsLineItem::boundingRect());
	r.adjust(-adjust, -adjust, adjust, adjust);
	return(r);
}