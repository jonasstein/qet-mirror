#ifndef ELEMENT_SCENE_H
#define ELEMENT_SCENE_H
#include <QtGui>
#include <QtXml>
#include "nameslistwidget.h"
#include "orientationsetwidget.h"
#include "qgimanager.h"
class QETElementEditor;
class PartLine;
class PartEllipse;
class PartCircle;
class PartPolygon;
class PartArc;
class ElementScene : public QGraphicsScene {
	Q_OBJECT
	
	// enum
	enum Behavior { Normal, Line, Circle, Ellipse, Polygon, Text, Terminal, Arc, TextField };
	
	// constructeurs, destructeur
	public:
	ElementScene(QETElementEditor *, QObject * = 0);
	virtual ~ElementScene();
	
	private:
	ElementScene(const ElementScene &);
	
	// attributs
	private:
	/// longueur de l'element en dizaines de pixels
	uint _width;
	/// hauteur de l'element en dizaines de pixels
	uint _height;
	/// position du point de saisie
	QPoint _hotspot;
	/// Liste des noms de l'element
	NamesList _names;
	/// Liste des orientations de l'element
	OrientationSet ori;
	/// Gestionnaire de QGraphicsItem
	QGIManager qgi_manager;
	/// Pile des actions annulables
	QUndoStack undo_stack;
	
	/// Variables relatives a la gestion du dessin des parties sur la scene
	Behavior behavior;
	PartLine *current_line;
	PartEllipse *current_ellipse;
	PartCircle *current_circle;
	PartPolygon *current_polygon;
	PartArc *current_arc;
	QETElementEditor *element_editor;
	
	// methodes
	public:
	void setWidth(const uint& theValue);
	uint width() const;
	void setHeight(const uint& theValue);
	uint height() const;
	void setHotspot(const QPoint &);
	QPoint hotspot() const;
	void setNames(const NamesList);
	NamesList names() const;
	OrientationSet orientations();
	void setOrientations(const OrientationSet &);
	virtual const QDomDocument toXml() const;
	virtual void fromXml(const QDomDocument &);
	QUndoStack &undoStack();
	QGIManager &qgiManager();
	
	protected:
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
	virtual void drawBackground(QPainter *, const QRectF &);
	virtual void drawForeground(QPainter *, const QRectF &);
	
	public slots:
	void slot_move();
	void slot_addLine();
	void slot_addCircle();
	void slot_addEllipse();
	void slot_addPolygon();
	void slot_addText();
	void slot_addArc();
	void slot_addTerminal();
	void slot_addTextField();
	void slot_checkSelectionChanged();
	void slot_selectAll();
	void slot_deselectAll();
	void slot_invertSelection();
	void slot_delete();
	void slot_editSizeHotSpot();
	void slot_editNames();
	void slot_editOrientations();
	
	signals:
	void selectionChanged();
	void needNormalMode();
};

inline void ElementScene::setWidth(const uint &wid) {
	_width = wid;
	while (_width % 10) ++ _width;
	_width /= 10;
}

inline uint ElementScene::width() const {
	return(_width * 10);
}

inline void ElementScene::setHeight(const uint &hei) {
	_height = hei;
	while (_height % 10) ++ _height;
	_height /= 10;
}

inline uint ElementScene::height() const {
	return(_height * 10);
}

inline void ElementScene::setHotspot(const QPoint &hs) {
	_hotspot = hs;
}

inline QPoint ElementScene::hotspot() const {
	return(_hotspot);
}

inline void ElementScene::setNames(const NamesList nameslist) {
	_names = nameslist;
}

inline NamesList ElementScene::names() const {
	return(_names);
}

inline OrientationSet ElementScene::orientations() {
	return(ori);
}

inline void ElementScene::setOrientations(const OrientationSet &orientation_set) {
	ori = orientation_set;
}

#endif