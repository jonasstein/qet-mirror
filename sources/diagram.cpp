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
#include <math.h>
#include "conductor.h"
#include "conductortextitem.h"
#include "customelement.h"
#include "diagram.h"
#include "diagramcommands.h"
#include "diagramcontent.h"
#include "diagramposition.h"
#include "elementtextitem.h"
#include "elementsmover.h"
#include "elementtextsmover.h"
#include "exportdialog.h"
#include "ghostelement.h"
#include "independenttextitem.h"
#include "qetapp.h"
#include "diagramimageitem.h"

const int   Diagram::xGrid  = 10;
const int   Diagram::yGrid  = 10;
const qreal Diagram::margin = 5.0;

/**
	Constructeur
	@param parent Le QObject parent du schema
*/
Diagram::Diagram(QObject *parent) :
	QGraphicsScene(parent),
	draw_grid_(true),
	use_border_(true),
	draw_terminals_(true),
	draw_colored_conductors_(true),
	project_(0),
	read_only_(false),
	diagram_qet_version_(-1)
{
	undo_stack_ = new QUndoStack();
	qgi_manager_ = new QGIManager(this);
	setBackgroundBrush(Qt::white);
	conductor_setter_ = new QGraphicsLineItem(0, 0);
	conductor_setter_ -> setZValue(1000000);
	QPen t;
	t.setColor(Qt::black);
	t.setWidthF(1.5);
	t.setStyle(Qt::DashLine);
	conductor_setter_ -> setPen(t);
	conductor_setter_ -> setLine(QLineF(QPointF(0.0, 0.0), QPointF(0.0, 0.0)));
	
	// initialise les objets gerant les deplacements
	elements_mover_ = new ElementsMover();           // deplacements d'elements/conducteurs/textes
	element_texts_mover_ = new ElementTextsMover();  // deplacements d'ElementTextItem
	
	connect(
		&border_and_titleblock, SIGNAL(needTitleBlockTemplate(const QString &)),
		this, SLOT(setTitleBlockTemplate(const QString &))
	);
	connect(
		&border_and_titleblock, SIGNAL(diagramTitleChanged(const QString &)),
		this, SLOT(titleChanged(const QString &))
	);
}

/**
	Destructeur
*/
Diagram::~Diagram() {
	// suppression de la liste des annulations - l'undo stack fait appel au qgimanager pour supprimer certains elements
	delete undo_stack_;
	// suppression du QGIManager - tous les elements qu'il connait sont supprimes
	delete qgi_manager_;
	// remove of conductor setter
	delete conductor_setter_;
	
	// suppression des objets gerant les deplacements
	delete elements_mover_;
	delete element_texts_mover_;
	
	// recense les items supprimables
	QList<QGraphicsItem *> deletable_items;
	foreach(QGraphicsItem *qgi, items()) {
		if (qgi -> parentItem()) continue;
		if (qgraphicsitem_cast<Conductor *>(qgi)) continue;
		deletable_items << qgi;
	}
	
	// suppression des items supprimables
	foreach(QGraphicsItem *qgi_d, deletable_items) {
		delete qgi_d;
	}
}

/**
	Dessine l'arriere-plan du schema, cad la grille.
	@param p Le QPainter a utiliser pour dessiner
	@param r Le rectangle de la zone a dessiner
*/
void Diagram::drawBackground(QPainter *p, const QRectF &r) {
	p -> save();
	
	// desactive tout antialiasing, sauf pour le texte
	p -> setRenderHint(QPainter::Antialiasing, false);
	p -> setRenderHint(QPainter::TextAntialiasing, true);
	p -> setRenderHint(QPainter::SmoothPixmapTransform, false);
	
	// dessine un fond blanc
	p -> setPen(Qt::NoPen);
	p -> setBrush(Qt::white);
	p -> drawRect(r);
	
	if (draw_grid_) {
		// dessine les points de la grille
		p -> setPen(Qt::black);
		p -> setBrush(Qt::NoBrush);
		qreal limite_x = r.x() + r.width();
		qreal limite_y = r.y() + r.height();
		
		int g_x = (int)ceil(r.x());
		while (g_x % xGrid) ++ g_x;
		int g_y = (int)ceil(r.y());
		while (g_y % yGrid) ++ g_y;
		
		QPolygon points;
		for (int gx = g_x ; gx < limite_x ; gx += xGrid) {
			for (int gy = g_y ; gy < limite_y ; gy += yGrid) {
				points << QPoint(gx, gy);
			}
		}
		p -> drawPoints(points);
	}
	
	if (use_border_) border_and_titleblock.draw(p, margin, margin);
	p -> restore();
}

/**
	Gere les enfoncements de touches du clavier
	@param e QKeyEvent decrivant l'evenement clavier
*/
void Diagram::keyPressEvent(QKeyEvent *e) {
	bool transmit_event = true;
	if (!isReadOnly()) {
		QPointF movement;
		switch(e -> key()) {
			case Qt::Key_Left:  movement = QPointF(-xGrid, 0.0); break;
			case Qt::Key_Right: movement = QPointF(+xGrid, 0.0); break;
			case Qt::Key_Up:    movement = QPointF(0.0, -yGrid); break;
			case Qt::Key_Down:  movement = QPointF(0.0, +yGrid); break;
		}
		if (!movement.isNull() && !focusItem()) {
			beginMoveElements();
			continueMoveElements(movement);
			e -> accept();
			transmit_event = false;
		}
	}
	if (transmit_event) {
		QGraphicsScene::keyPressEvent(e);
	}
}

/**
	Gere les relachements de touches du clavier
	@param e QKeyEvent decrivant l'evenement clavier
*/
void Diagram::keyReleaseEvent(QKeyEvent *e) {
	bool transmit_event = true;
	if (!isReadOnly()) {
		// detecte le relachement d'une touche de direction ( = deplacement d'elements)
		if (
			(e -> key() == Qt::Key_Left || e -> key() == Qt::Key_Right  ||
			 e -> key() == Qt::Key_Up   || e -> key() == Qt::Key_Down)  &&
			!e -> isAutoRepeat()
		) {
			endMoveElements();
			e -> accept();
			transmit_event = false;
		}
	}
	if (transmit_event) {
		QGraphicsScene::keyReleaseEvent(e);
	}
}

/**
	Exporte le schema vers une image
	@return Une QImage representant le schema
*/
bool Diagram::toPaintDevice(QPaintDevice &pix, int width, int height, Qt::AspectRatioMode aspectRatioMode) {
	// determine la zone source =  contenu du schema + marges
	QRectF source_area;
	if (!use_border_) {
		source_area = itemsBoundingRect();
		source_area.translate(-margin, -margin);
		source_area.setWidth (source_area.width () + 2.0 * margin);
		source_area.setHeight(source_area.height() + 2.0 * margin);
	} else {
		source_area = QRectF(
			0.0,
			0.0,
			border_and_titleblock.borderWidth () + 2.0 * margin,
			border_and_titleblock.borderHeight() + 2.0 * margin
		);
	}
	
	// si les dimensions ne sont pas precisees, l'image est exportee a l'echelle 1:1
	QSize image_size = (width == -1 && height == -1) ? source_area.size().toSize() : QSize(width, height);
	
	// prepare le rendu
	QPainter p;
	if (!p.begin(&pix)) return(false);
	
	// rendu antialiase
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::TextAntialiasing, true);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);
	
	// deselectionne tous les elements
	QList<QGraphicsItem *> selected_elmts = selectedItems();
	foreach (QGraphicsItem *qgi, selected_elmts) qgi -> setSelected(false);
	
	// effectue le rendu lui-meme
	render(&p, QRect(QPoint(0, 0), image_size), source_area, aspectRatioMode);
	p.end();
	
	// restaure les elements selectionnes
	foreach (QGraphicsItem *qgi, selected_elmts) qgi -> setSelected(true);
	
	return(true);
}

/**
	Permet de connaitre les dimensions qu'aura l'image generee par la methode toImage()
	@return La taille de l'image generee par toImage()
*/
QSize Diagram::imageSize() const {
	// determine la zone source =  contenu du schema + marges
	qreal image_width, image_height;
	if (!use_border_) {
		QRectF items_rect = itemsBoundingRect();
		image_width  = items_rect.width();
		image_height = items_rect.height();
	} else {
		image_width  = border_and_titleblock.borderWidth();
		image_height = border_and_titleblock.borderHeight();
	}
	
	image_width  += 2.0 * margin;
	image_height += 2.0 * margin;
	
	// renvoie la taille de la zone source
	return(QSizeF(image_width, image_height).toSize());
}

/**
	@return true si le schema est considere comme vide, false sinon.
	Un schema vide ne contient ni element, ni conducteur, ni champ de texte
*/
bool Diagram::isEmpty() const {
	return(!items().count());
}

/**
 * @brief Diagram::potential
 * @return all potential in the diagram
 *each potential are in the QList and each conductors of one potential are in the QSet
 */
QList < QSet <Conductor *> > Diagram::potentials() {
	QList < QSet <Conductor *> > potential_List;
	if (content().conductors().size() == 0) return (potential_List); //return an empty potential
	QList <Conductor *> conductors_list = content().conductors();

	do {
		QSet <Conductor *> one_potential = conductors_list.first() -> relatedPotentialConductors();
		one_potential << conductors_list.takeFirst();
		foreach (Conductor *c, one_potential) conductors_list.removeOne(c);
		potential_List << one_potential;
	} while (!conductors_list.empty());

	return (potential_List);
}

/**
	Exporte tout ou partie du schema 
	@param whole_content Booleen (a vrai par defaut) indiquant si le XML genere doit
	representer l'integralite du schema ou seulement le contenu selectionne
	@return Un Document XML (QDomDocument)
*/
QDomDocument Diagram::toXml(bool whole_content) {
	// document
	QDomDocument document;
	
	// racine de l'arbre XML
	QDomElement racine = document.createElement("diagram");
	
	// add the application version number
	racine.setAttribute("version", QET::version);
	
	// proprietes du schema
	if (whole_content) {
		border_and_titleblock.titleBlockToXml(racine);
		border_and_titleblock.borderToXml(racine);
		
		// type de conducteur par defaut
		QDomElement default_conductor = document.createElement("defaultconductor");
		defaultConductorProperties.toXml(default_conductor);
		racine.appendChild(default_conductor);

		//autonumerotation of conductor
		if (!getNumerotation(Diagram::Conductors).isEmpty()) {
			QDomElement autonum = document.createElement("autonum");
			autonum.appendChild(getNumerotation(Diagram::Conductors).toXml(document, "conductor"));
			racine.appendChild(autonum);
		}
	}
	document.appendChild(racine);
	
	// si le schema ne contient pas d'element (et donc pas de conducteurs), on retourne de suite le document XML
	if (items().isEmpty()) return(document);
	
	// creation de trois listes : une qui contient les elements, une qui contient les conducteurs, une qui contient les champs de texte
	QList<Element *> list_elements;
	QList<Conductor *> list_conductors;
	QList<DiagramTextItem *> list_texts;
	QList<DiagramImageItem *> list_images;
	
	// Determine les elements a "XMLiser"
	foreach(QGraphicsItem *qgi, items()) {
		if (Element *elmt = qgraphicsitem_cast<Element *>(qgi)) {
			if (whole_content) list_elements << elmt;
			else if (elmt -> isSelected()) list_elements << elmt;
		} else if (Conductor *f = qgraphicsitem_cast<Conductor *>(qgi)) {
			if (whole_content) list_conductors << f;
			// lorsqu'on n'exporte pas tout le diagram, il faut retirer les conducteurs non selectionnes
			// et pour l'instant, les conducteurs non selectionnes sont les conducteurs dont un des elements n'est pas selectionne
			else if (f -> terminal1 -> parentItem() -> isSelected() && f -> terminal2 -> parentItem() -> isSelected()) {
				list_conductors << f;
			}
		} else if (IndependentTextItem *iti = qgraphicsitem_cast<IndependentTextItem *>(qgi)) {
			if (whole_content) list_texts << iti;
			else if (iti -> isSelected()) list_texts << iti;
		} else if (DiagramImageItem *dii = qgraphicsitem_cast<DiagramImageItem *>(qgi)) {
			if (whole_content) list_images << dii;
			else if (dii -> isSelected()) list_images << dii;
		}
	}
	
	// table de correspondance entre les adresses des bornes et leurs ids
	QHash<Terminal *, int> table_adr_id;
	
	// enregistrement des elements
	if (!list_elements.isEmpty()) {
		QDomElement elements = document.createElement("elements");
		foreach(Element *elmt, list_elements) {
			elements.appendChild(elmt -> toXml(document, table_adr_id));
		}
		racine.appendChild(elements);
	}
	
	// enregistrement des conducteurs
	if (!list_conductors.isEmpty()) {
		QDomElement conductors = document.createElement("conductors");
		foreach(Conductor *cond, list_conductors) {
			conductors.appendChild(cond -> toXml(document, table_adr_id));
		}
		racine.appendChild(conductors);
	}
	
	// enregistrement des champs de texte
	if (!list_texts.isEmpty()) {
		QDomElement inputs = document.createElement("inputs");
		foreach(DiagramTextItem *dti, list_texts) {
			inputs.appendChild(dti -> toXml(document));
		}
		racine.appendChild(inputs);
	}

	// save of images
	if (!list_images.isEmpty()) {
		QDomElement images = document.createElement("images");
		foreach (DiagramImageItem *dii, list_images) {
			images.appendChild(dii -> toXml(document));
		}
		racine.appendChild(images);
	}
	
	// on retourne le document XML ainsi genere
	return(document);
}

/**
	Importe le schema decrit dans un document XML. Si une position est
	precisee, les elements importes sont positionnes de maniere a ce que le
	coin superieur gauche du plus petit rectangle pouvant les entourant tous
	(le bounding rect) soit a cette position.
	@param document Le document XML a analyser
	@param position La position du schema importe
	@param consider_informations Si vrai, les informations complementaires
	(auteur, titre, ...) seront prises en compte
	@param content_ptr si ce pointeur vers un DiagramContent est different de 0,
	il sera rempli avec le contenu ajoute au schema par le fromXml
	@return true si l'import a reussi, false sinon
*/
bool Diagram::fromXml(QDomDocument &document, QPointF position, bool consider_informations, DiagramContent *content_ptr) {
	QDomElement root = document.documentElement();
	return(fromXml(root, position, consider_informations, content_ptr));
}

/**
	Importe le schema decrit dans un element XML. Cette methode delegue son travail a Diagram::fromXml
	Si l'import reussit, cette methode initialise egalement le document XML
	interne permettant de bien gerer l'enregistrement de ce schema dans le
	projet auquel il appartient.
	@see Diagram::fromXml
	@param document Le document XML a analyser
	@param position La position du schema importe
	@param consider_informations Si vrai, les informations complementaires
	(auteur, titre, ...) seront prises en compte
	@param content_ptr si ce pointeur vers un DiagramContent est different de 0,
	il sera rempli avec le contenu ajoute au schema par le fromXml
	@return true si l'import a reussi, false sinon
	
*/
bool Diagram::initFromXml(QDomElement &document, QPointF position, bool consider_informations, DiagramContent *content_ptr) {
	// import le contenu et les proprietes du schema depuis l'element XML fourni en parametre
	bool from_xml = fromXml(document, position, consider_informations, content_ptr);
	
	// initialise le document XML interne a partir de l'element XML fourni en parametre
	if (from_xml) {
		xml_document_.clear();
		xml_document_.appendChild(xml_document_.importNode(document, true));
		// a ce stade, le document XML interne contient le code XML qui a ete importe, et non pas une version re-exporte par la methode toXml()
	}
	return(from_xml);
}

/**
	Importe le schema decrit dans un element XML. Si une position est
	precisee, les elements importes sont positionnes de maniere a ce que le
	coin superieur gauche du plus petit rectangle pouvant les entourant tous
	(le bounding rect) soit a cette position.
	@param document Le document XML a analyser
	@param position La position du schema importe
	@param consider_informations Si vrai, les informations complementaires
	(auteur, titre, ...) seront prises en compte
	@param content_ptr si ce pointeur vers un DiagramContent est different de 0,
	il sera rempli avec le contenu ajoute au schema par le fromXml
	@return true si l'import a reussi, false sinon
*/
bool Diagram::fromXml(QDomElement &document, QPointF position, bool consider_informations, DiagramContent *content_ptr) {
	QDomElement root = document;
	// le premier element doit etre un schema
	if (root.tagName() != "diagram") return(false);
	
	// lecture des attributs de ce schema
	if (consider_informations) {
		bool conv_ok;
		qreal version_value = root.attribute("version").toDouble(&conv_ok);
		if (conv_ok) {
			diagram_qet_version_ = version_value;
		}
		
		border_and_titleblock.titleBlockFromXml(root);
		border_and_titleblock.borderFromXml(root);
		
		// repere le permier element "defaultconductor"
		QDomElement default_conductor_elmt = root.firstChildElement("defaultconductor");
		if (!default_conductor_elmt.isNull()) {
			defaultConductorProperties.fromXml(default_conductor_elmt);
		}
		// find the first element autonum
		QDomElement num_auto = root.firstChildElement("autonum");
		if (!num_auto.isNull()) {
			QDomElement num_conductor = num_auto.firstChildElement("conductor");
			//set the auto-numerotation of conductor
			if (!num_conductor.isNull()) {
				NumerotationContext nc(num_conductor);
				setNumerotation(Diagram::Conductors, nc);
			}
		}
	}
	
	// si la racine n'a pas d'enfant : le chargement est fini (schema vide)
	if (root.firstChild().isNull()) {
		write(document);
		return(true);
	}
	
	// Backward compatibility: prior to version 0.3, we need to compensate, at
	// diagram-opening time, the rotation of the element for each of its
	// textfields having the "FollowParentRotation" option disabled.
	// After 0.3, elements textfields get userx, usery and userrotation attributes
	// that explicitly specify their position and orientation.
	qreal project_qet_version = declaredQElectroTechVersion(true);
	bool handle_inputs_rotation = (
		project_qet_version != -1 && project_qet_version < 0.3 &&
		project_ -> state() == QETProject::ProjectParsingRunning
	);
	
	// chargement de tous les elements du fichier XML
	QList<Element *> added_elements;
	QHash<int, Terminal *> table_adr_id;
	foreach (QDomElement element_xml, QET::findInDomElement(root, "elements", "element")) {
		if (!Element::valideXml(element_xml)) continue;
		
		// cree un element dont le type correspond a l'id type
		QString type_id = element_xml.attribute("type");
		ElementsLocation element_location = ElementsLocation(type_id);
		if (type_id.startsWith("embed://")) element_location.setProject(project_);
		
		CustomElement *nvel_elmt = new CustomElement(element_location);
		if (nvel_elmt -> isNull()) {
			QString debug_message = QString("Diagram::fromXml() : Le chargement de la description de l'element %1 a echoue avec le code d'erreur %2").arg(element_location.path()).arg(nvel_elmt -> state());
			qDebug() << qPrintable(debug_message);
			delete nvel_elmt;
			
			qDebug() << "Diagram::fromXml() : Utilisation d'un GhostElement en lieu et place de cet element.";
			nvel_elmt = new GhostElement(element_location);
		}
		
		// charge les caracteristiques de l'element
		if (nvel_elmt -> fromXml(element_xml, table_adr_id, handle_inputs_rotation)) {
			// ajout de l'element au schema et a la liste des elements ajoutes
			addElement(nvel_elmt);
			added_elements << nvel_elmt;
		} else {
			delete nvel_elmt;
			qDebug() << "Diagram::fromXml() : Le chargement des parametres d'un element a echoue";
		}
	}
	
	// chargement de tous les textes du fichiers XML
	QList<IndependentTextItem *> added_texts;
	foreach (QDomElement text_xml, QET::findInDomElement(root, "inputs", "input")) {
		IndependentTextItem *iti = new IndependentTextItem(this);
		iti -> fromXml(text_xml);
		addIndependentTextItem(iti);
		added_texts << iti;
	}

	QList<DiagramImageItem *> added_images;
	foreach (QDomElement image_xml, QET::findInDomElement(root, "images", "image")) {
		DiagramImageItem *dii = new DiagramImageItem (this);
		dii -> fromXml(image_xml);
		addItem(dii);
		added_images << dii;
	}
	
	// gere la translation des nouveaux elements et texte si celle-ci est demandee
	if (position != QPointF()) {
		// determine quel est le coin superieur gauche du rectangle entourant les elements ajoutes
		qreal minimum_x = 0, minimum_y = 0;
		bool init = false;
		QList<QGraphicsItem *> added_items;
		foreach (Element *added_element, added_elements) added_items << added_element;
		foreach (DiagramTextItem *added_text, added_texts) added_items << added_text;
		foreach (DiagramImageItem *added_image, added_images) added_items << added_image;
		foreach (QGraphicsItem *item, added_items) {
			QPointF csg = item -> mapToScene(item -> boundingRect()).boundingRect().topLeft();
			qreal px = csg.x();
			qreal py = csg.y();
			if (!init) {
				minimum_x = px;
				minimum_y = py;
				init = true;
			} else {
				if (px < minimum_x) minimum_x = px;
				if (py < minimum_y) minimum_y = py;
			}
		}
		qreal diff_x = position.x() - minimum_x;
		qreal diff_y = position.y() - minimum_y;
		foreach (Element *added_element, added_elements) {
			added_element -> setPos(added_element -> pos().x() + diff_x, added_element -> pos().y() + diff_y);
		}
		foreach (DiagramTextItem *added_text, added_texts) {
			added_text -> setPos(added_text -> pos().x() + diff_x, added_text -> pos().y() + diff_y);
		}
		foreach (DiagramImageItem *added_image, added_images) {
			added_image -> setPos(added_image -> pos().x() + diff_x, added_image -> pos().y() + diff_y);
		}
	}
	
	// chargement de tous les Conducteurs du fichier XML
	QList<Conductor *> added_conductors;
	foreach (QDomElement f, QET::findInDomElement(root, "conductors", "conductor")) {
		if (!Conductor::valideXml(f)) continue;
		// verifie que les bornes que le conducteur relie sont connues
		int id_p1 = f.attribute("terminal1").toInt();
		int id_p2 = f.attribute("terminal2").toInt();
		if (table_adr_id.contains(id_p1) && table_adr_id.contains(id_p2)) {
			// pose le conducteur... si c'est possible
			Terminal *p1 = table_adr_id.value(id_p1);
			Terminal *p2 = table_adr_id.value(id_p2);
			if (p1 != p2) {
				bool can_add_conductor = true;
				bool cia = ((Element *)p2 -> parentItem()) -> internalConnections();
				if (!cia) {
					foreach(QGraphicsItem *item, p2 -> parentItem() -> children()) {
						if (item == p1) can_add_conductor = false;
					}
				}
				if (can_add_conductor) {
					Conductor *c = new Conductor(table_adr_id.value(id_p1), table_adr_id.value(id_p2), this);
					c -> fromXml(f);
					added_conductors << c;
				}
			}
		} else qDebug() << "Diagram::fromXml() : Le chargement du conducteur" << id_p1 << id_p2 << "a echoue";
	}
	
	// remplissage des listes facultatives
	if (content_ptr) {
		content_ptr -> elements         = added_elements.toSet();
		content_ptr -> conductorsToMove = added_conductors.toSet();
		content_ptr -> textFields       = added_texts.toSet();
		content_ptr -> images			= added_images.toSet();
	}
	
	return(true);
}

/**
	Enregistre le schema XML dans son document XML interne et emet le signal
	written().
*/
void Diagram::write() {
	qDebug() << qPrintable(QString("Diagram::write() : saving changes from diagram \"%1\" [%2]").arg(title()).arg(QET::pointerString(this)));
	write(toXml().documentElement());
	undoStack().setClean();
}

/**
	Enregistre un element XML dans son document XML interne et emet le signal
	written().
	@param element xml a enregistrer
*/
void Diagram::write(const QDomElement &element) {
	xml_document_.clear();
	xml_document_.appendChild(xml_document_.importNode(element, true));
	emit(written());
}

/**
	@return true si la fonction write a deja ete appele (pour etre plus exact :
	si le document XML utilise en interne n'est pas vide), false sinon
*/
bool Diagram::wasWritten() const {
	return(!xml_document_.isNull());
}

/**
	@return le schema en XML tel qu'il doit etre enregistre dans le fichier projet
	@param xml_doc document XML a utiliser pour creer l'element
*/
QDomElement Diagram::writeXml(QDomDocument &xml_doc) const {
	// si le schema n'a pas ete enregistre explicitement, on n'ecrit rien
	if (!wasWritten()) return(QDomElement());
	
	QDomElement diagram_elmt = xml_document_.documentElement();
	QDomNode new_node = xml_doc.importNode(diagram_elmt, true);
	return(new_node.toElement());
}

/**
	Ajoute un element sur le schema
	@param element Element a ajouter
*/
void Diagram::addElement(Element *element) {
	if (!element || isReadOnly()) return;
	
	// ajoute l'element au schema
	if (element -> scene() != this) {
		addItem(element);
	}
	
	// surveille les modifications de ses champs de texte
	foreach(ElementTextItem *eti, element -> texts()) {
		connect(
			eti,
			SIGNAL(diagramTextChanged(DiagramTextItem *, const QString &, const QString &)),
			this,
			SLOT(diagramTextChanged(DiagramTextItem *, const QString &, const QString &))
		);
	}
}

/**
	Ajoute un conducteur sur le schema
	@param conductor Conducteur a ajouter
*/
void Diagram::addConductor(Conductor *conductor) {
	if (!conductor || isReadOnly()) return;
	
	// ajoute le conducteur au schema
	if (conductor -> scene() != this) {
		addItem(conductor);
		conductor -> terminal1 -> addConductor(conductor);
		conductor -> terminal2 -> addConductor(conductor);
	}
}

/**
	Aoute un champ de texte independant sur le schema
	@param iti Champ de texte a ajouter
*/
void Diagram::addIndependentTextItem(IndependentTextItem *iti) {
	if (!iti || isReadOnly()) return;
	
	// ajoute le champ de texte au schema
	if (iti -> scene() != this) {
		addItem(iti);
	}
	
	// surveille les modifications apportees au champ de texte
	connect(
		iti,
		SIGNAL(diagramTextChanged(DiagramTextItem *, const QString &, const QString &)),
		this,
		SLOT(diagramTextChanged(DiagramTextItem *, const QString &, const QString &))
	);
}

void Diagram::addDiagramImageItem(DiagramImageItem *dii) {
	if (!dii || isReadOnly()) return;

	//add image at diagram
	if (dii -> scene() != this) {
		addItem(dii);
	}
}

/**
	Enleve un element du schema
	@param element Element a enlever
*/
void Diagram::removeElement(Element *element) {
	if (!element || isReadOnly()) return;
	
	// enleve l'element au schema
	removeItem(element);
	
	// arrete la surveillance des modifications de ses champs de texte
	foreach(ElementTextItem *eti, element -> texts()) {
		disconnect(
			eti,
			SIGNAL(diagramTextChanged(DiagramTextItem *, const QString &, const QString &)),
			this,
			SLOT(diagramTextChanged(DiagramTextItem *, const QString &, const QString &))
		);
	}
}

/**
	Enleve un conducteur du schema
	@param conductor Conducteur a enlever
*/
void Diagram::removeConductor(Conductor *conductor) {
	if (!conductor || isReadOnly()) return;
	
	// detache le conducteur sans le detruire
	conductor -> terminal1 -> removeConductor(conductor);
	conductor -> terminal2 -> removeConductor(conductor);
	
	// enleve le conducteur du schema
	removeItem(conductor);
}

/**
	Enleve un champ de texte independant du schema
	@param iti Champ de texte a enlever
*/
void Diagram::removeIndependentTextItem(IndependentTextItem *iti) {
	if (!iti || isReadOnly()) return;
	
	// enleve le champ de texte au schema
	removeItem(iti);
	
	// arrete la surveillance des modifications apportees au champ de texte
	disconnect(
		iti,
		SIGNAL(diagramTextChanged(DiagramTextItem *, const QString &, const QString &)),
		this,
		SLOT(diagramTextChanged(DiagramTextItem *, const QString &, const QString &))
	);
}

void Diagram::titleChanged(const QString &title) {
	emit(diagramTitleChanged(this, title));
}

/**
	Gere le fait qu'un texte du schema ait ete modifie
	@param text_item Texte modifie
	@param old_text Ancien texte
	@param new_text Nouveau texte
*/
void Diagram::diagramTextChanged(DiagramTextItem *text_item, const QString &old_text, const QString &new_text) {
	if (!text_item) return;
	undo_stack_ -> push(new ChangeDiagramTextCommand(text_item, old_text, new_text));
}

/**
	This slot may be used to inform the diagram object that the given title
	block template has changed. The diagram will thus flush its title
	block-dedicated rendering cache.
	@param template_name Name of the title block template that has changed
*/
void Diagram::titleBlockTemplateChanged(const QString &template_name) {
	if (border_and_titleblock.titleBlockTemplateName() != template_name) return;
	
	border_and_titleblock.titleBlockTemplateChanged(template_name);
	update();
}

/**
	This slot has to be be used to inform this class that the given title block
	template is about to be removed and is no longer accessible. This class
	will either use the provided  optional TitleBlockTemplate or the default
	title block provided by QETApp::defaultTitleBlockTemplate()
	@param template_name Name of the title block template that has changed
	@param new_template (Optional) Name of the title block template to use instead
*/
void Diagram::titleBlockTemplateRemoved(const QString &template_name, const QString &new_template) {
	if (border_and_titleblock.titleBlockTemplateName() != template_name) return;
	
	const TitleBlockTemplate *final_template = project_ -> getTemplateByName(new_template);
	border_and_titleblock.titleBlockTemplateRemoved(template_name, final_template);
	update();
}

/**
	Set the template to use to render the title block of this diagram.
	@param template_name Name of the title block template.
*/
void Diagram::setTitleBlockTemplate(const QString &template_name) {
	if (!project_) return;
	
	QString current_name = border_and_titleblock.titleBlockTemplateName();
	const TitleBlockTemplate *titleblock_template = project_ -> getTemplateByName(template_name);
	border_and_titleblock.titleBlockTemplateRemoved(current_name, titleblock_template);
	
	if (template_name != current_name) {
		emit(usedTitleBlockTemplateChanged(template_name));
	}
}

/**
	Selectionne tous les objets du schema
*/
void Diagram::selectAll() {
	if (items().isEmpty()) return;
	
	blockSignals(true);
	foreach(QGraphicsItem *qgi, items()) qgi -> setSelected(true);
	blockSignals(false);
	emit(selectionChanged());
}

/**
	Deslectionne tous les objets selectionnes
*/
void Diagram::deselectAll() {
	if (items().isEmpty()) return;
	
	clearSelection();
}

/**
	Inverse l'etat de selection de tous les objets du schema
*/
void Diagram::invertSelection() {
	if (items().isEmpty()) return;
	
	blockSignals(true);
	foreach (QGraphicsItem *item, items()) item -> setSelected(!item -> isSelected());
	blockSignals(false);
	emit(selectionChanged());
}

/**
	@return Le rectangle (coordonnees par rapport a la scene) delimitant le bord du schema
*/
QRectF Diagram::border() const {
	return(
		QRectF(
			margin,
			margin,
			border_and_titleblock.borderWidth(),
			border_and_titleblock.borderHeight()
		)
	);
}

/**
	@return le titre du cartouche
*/
QString Diagram::title() const {
	return(border_and_titleblock.title());
}

/**
	@return la liste des elements de ce schema
*/
QList<CustomElement *> Diagram::customElements() const {
	QList<CustomElement *> elements_list;
	foreach(QGraphicsItem *qgi, items()) {
		if (CustomElement *elmt = qgraphicsitem_cast<CustomElement *>(qgi)) {
			elements_list << elmt;
		}
	}
	return(elements_list);
}

/**
	Initialise un deplacement d'elements, conducteurs et champs de texte sur le
	schema.
	@param driver_item Item deplace par la souris et ne necessitant donc pas
	d'etre deplace lors des appels a continueMovement.
	@see ElementsMover
*/
int Diagram::beginMoveElements(QGraphicsItem *driver_item) {
	return(elements_mover_ -> beginMovement(this, driver_item));
}

/**
	Prend en compte un mouvement composant un deplacement d'elements,
	conducteurs et champs de texte
	@param movement mouvement a ajouter au deplacement en cours
	@see ElementsMover
*/
void Diagram::continueMoveElements(const QPointF &movement) {
	elements_mover_ -> continueMovement(movement);
}

/**
	Finalise un deplacement d'elements, conducteurs et champs de texte
	@see ElementsMover
*/
void Diagram::endMoveElements() {
	elements_mover_ -> endMovement();
}

/**
	Initialise un deplacement d'ElementTextItems
	@param driver_item Item deplace par la souris et ne necessitant donc pas
	d'etre deplace lors des appels a continueMovement.
	@see ElementTextsMover
*/
int Diagram::beginMoveElementTexts(QGraphicsItem *driver_item) {
	return(element_texts_mover_ -> beginMovement(this, driver_item));
}

/**
	Prend en compte un mouvement composant un deplacement d'ElementTextItems
	@param movement mouvement a ajouter au deplacement en cours
	@see ElementTextsMover
*/
void Diagram::continueMoveElementTexts(const QPointF &movement) {
	element_texts_mover_ -> continueMovement(movement);
}

/**
	Finalise un deplacement d'ElementTextItems
	@see ElementTextsMover
*/
void Diagram::endMoveElementTexts() {
	element_texts_mover_ -> endMovement();
}

/**
	Permet de savoir si un element est utilise sur un schema
	@param location Emplacement d'un element
	@return true si l'element location est utilise sur ce schema, false sinon
*/
bool Diagram::usesElement(const ElementsLocation &location) {
	foreach(CustomElement *element, customElements()) {
		if (element -> location() == location) {
			return(true);
		}
	}
	return(false);
}

/**
	@param a title block template name
	@return true if the provided template is used by this diagram, false
	otherwise.
*/
bool Diagram::usesTitleBlockTemplate(const QString &name) {
	return(name == border_and_titleblock.titleBlockTemplateName());
}

/**
	Cette methode permet d'appliquer de nouvelles options de rendu tout en
	accedant aux proprietes de rendu en cours.
	@param new_properties Nouvelles options de rendu a appliquer
	@return les options de rendu avant l'application de new_properties
*/
ExportProperties Diagram::applyProperties(const ExportProperties &new_properties) {
	// exporte les options de rendu en cours
	ExportProperties old_properties;
	old_properties.draw_grid               = displayGrid();
	old_properties.draw_border             = border_and_titleblock.borderIsDisplayed();
	old_properties.draw_titleblock         = border_and_titleblock.titleBlockIsDisplayed();
	old_properties.draw_terminals          = drawTerminals();
	old_properties.draw_colored_conductors = drawColoredConductors();
	old_properties.exported_area           = useBorder() ? QET::BorderArea : QET::ElementsArea;
	
	// applique les nouvelles options de rendu
	setUseBorder                  (new_properties.exported_area == QET::BorderArea);
	setDrawTerminals              (new_properties.draw_terminals);
	setDrawColoredConductors      (new_properties.draw_colored_conductors);
	setDisplayGrid                (new_properties.draw_grid);
	border_and_titleblock.displayBorder(new_properties.draw_border);
	border_and_titleblock.displayTitleBlock (new_properties.draw_titleblock);
	
	// retourne les anciennes options de rendu
	return(old_properties);
}

/**
	@param pos Position cartesienne (ex : 10.3, 45.2) a transformer en position
	dans la grille (ex : B2)
	@return la position dans la grille correspondant a pos
*/
DiagramPosition Diagram::convertPosition(const QPointF &pos) {
	// decale la position pour prendre en compte les marges en haut a gauche du schema
	QPointF final_pos = pos - QPointF(margin, margin);
	
	// delegue le calcul au BorderTitleBlock
	DiagramPosition diagram_position = border_and_titleblock.convertPosition(final_pos);
	
	// embarque la position cartesienne
	diagram_position.setPosition(pos);
	
	return(diagram_position);
}

/**
	Definit s'il faut afficher ou non les bornes
	@param dt true pour afficher les bornes, false sinon
*/
void Diagram::setDrawTerminals(bool dt) {
	foreach(QGraphicsItem *qgi, items()) {
		if (Terminal *t = qgraphicsitem_cast<Terminal *>(qgi)) {
			t -> setVisible(dt);
		}
	}
}

/**
	Definit s'il faut respecter ou non les couleurs des conducteurs.
	Si non, les conducteurs sont tous dessines en noir.
	@param dcc true pour respecter les couleurs, false sinon
*/
void Diagram::setDrawColoredConductors(bool dcc) {
	draw_colored_conductors_ = dcc;
}

/**
	@return la liste des conducteurs selectionnes sur le schema
*/
QSet<Conductor *> Diagram::selectedConductors() const {
	QSet<Conductor *> conductors_set;
	foreach(QGraphicsItem *qgi, selectedItems()) {
		if (Conductor *c = qgraphicsitem_cast<Conductor *>(qgi)) {
			conductors_set << c;
		}
	}
	return(conductors_set);
}

/**
	@return la liste de tous les textes selectionnes : les textes independants,
	mais aussi ceux rattaches a des conducteurs ou des elements
*/
QSet<DiagramTextItem *> Diagram::selectedTexts() const {
	QSet<DiagramTextItem *> selected_texts;
	foreach(QGraphicsItem *item, selectedItems()) {
		if (ConductorTextItem *cti = qgraphicsitem_cast<ConductorTextItem *>(item)) {
			selected_texts << cti;
		} else if (ElementTextItem *eti = qgraphicsitem_cast<ElementTextItem *>(item)) {
			selected_texts << eti;
		} else if (IndependentTextItem *iti = qgraphicsitem_cast<IndependentTextItem *>(item)) {
			selected_texts << iti;
		}
	}
	return(selected_texts);
}

/**
 * @brief Diagram::selectedConductorTexts
 * @return the list of conductor texts selected
 */
QSet<ConductorTextItem *> Diagram::selectedConductorTexts() const {
	QSet<ConductorTextItem *> selected_texts;
	foreach(QGraphicsItem *item, selectedItems()) {
		if (ConductorTextItem *cti = qgraphicsitem_cast<ConductorTextItem *>(item)) {
			selected_texts << cti;
		}
	}
	return(selected_texts);
}

/// @return true si le presse-papier semble contenir un schema
bool Diagram::clipboardMayContainDiagram() {
	QString clipboard_text = QApplication::clipboard() -> text().trimmed();
	bool may_be_diagram = clipboard_text.startsWith("<diagram") && clipboard_text.endsWith("</diagram>");
	return(may_be_diagram);
}

/**
	@return le projet auquel ce schema appartient ou 0 s'il s'agit d'un schema
	independant.
*/
QETProject *Diagram::project() const {
	return(project_);
}

/**
	@param project le nouveau projet auquel ce schema appartient ou 0 s'il
	s'agit d'un schema independant. Indiquer 0 pour rendre ce schema independant.
*/
void Diagram::setProject(QETProject *project) {
	project_ = project;
}

/**
	@return the folio number of this diagram within its parent project, or -1
	if it is has no parent project
*/
int Diagram::folioIndex() const {
	if (!project_) return(-1);
	return(project_ -> folioIndex(this));
}

/**
	@param fallback_to_project When a diagram does not have a declared version,
	this method will use the one declared by its parent project only if
	fallback_to_project is true.
	@return the declared QElectroTech version of this diagram
*/
qreal Diagram::declaredQElectroTechVersion(bool fallback_to_project) const {
	if (diagram_qet_version_ != -1) {
		return diagram_qet_version_;
	}
	if (fallback_to_project && project_) {
		return(project_ -> declaredQElectroTechVersion());
	}
	return(-1);
}

/**
	@return true si le schema est en lecture seule
*/
bool Diagram::isReadOnly() const {
	return(read_only_);
}

/**
	@param read_only true pour passer le schema en lecture seule, false sinon
*/
void Diagram::setReadOnly(bool read_only) {
	if (read_only_ != read_only) {
		read_only_ = read_only;
		emit(readOnlyChanged(read_only_));
	}
}

/**
	@return Le contenu du schema. Les conducteurs sont tous places dans
	conductorsToMove.
*/
DiagramContent Diagram::content() const {
	DiagramContent dc;
	foreach(QGraphicsItem *qgi, items()) {
		if (Element *e = qgraphicsitem_cast<Element *>(qgi)) {
			dc.elements << e;
		} else if (IndependentTextItem *iti = qgraphicsitem_cast<IndependentTextItem *>(qgi)) {
			dc.textFields << iti;
		} else if (Conductor *c = qgraphicsitem_cast<Conductor *>(qgi)) {
			dc.conductorsToMove << c;
		}
	}
	return(dc);
}

/**
	@return le contenu selectionne du schema.
*/
DiagramContent Diagram::selectedContent() {
	DiagramContent dc;
	
	// recupere les elements deplaces
	foreach (QGraphicsItem *item, selectedItems()) {
		if (Element *elmt = qgraphicsitem_cast<Element *>(item)) {
			dc.elements << elmt;
		} else if (IndependentTextItem *iti = qgraphicsitem_cast<IndependentTextItem *>(item)) {
			dc.textFields << iti;
		} else if (Conductor *c = qgraphicsitem_cast<Conductor *>(item)) {
			// recupere les conducteurs selectionnes isoles (= non deplacables mais supprimables)
			if (
				!c -> terminal1 -> parentItem() -> isSelected() &&\
				!c -> terminal2 -> parentItem() -> isSelected()
			) {
				dc.otherConductors << c;
			}
		} else if (DiagramImageItem *dii = qgraphicsitem_cast<DiagramImageItem *>(item)) {
			dc.images << dii;
		}
	}
	
	// pour chaque element deplace, determine les conducteurs qui seront modifies
	foreach(Element *elmt, dc.elements) {
		foreach(Terminal *terminal, elmt -> terminals()) {
			foreach(Conductor *conductor, terminal -> conductors()) {
				Terminal *other_terminal;
				if (conductor -> terminal1 == terminal) {
					other_terminal = conductor -> terminal2;
				} else {
					other_terminal = conductor -> terminal1;
				}
				// si les deux elements du conducteur sont deplaces
				if (dc.elements.contains(other_terminal -> parentElement())) {
					dc.conductorsToMove << conductor;
				} else {
					dc.conductorsToUpdate << conductor;
				}
			}
		}
	}
	
	return(dc);
}

/**
	@return true s'il est possible de tourner les elements selectionnes.
	Concretement, cette methode retourne true s'il y a des elements selectionnes
	et qu'au moins l'un d'entre eux peut etre pivote.
*/
bool Diagram::canRotateSelection() const {
	foreach(QGraphicsItem * qgi, selectedItems()) {
		if (qgraphicsitem_cast<IndependentTextItem *>(qgi)) {
			return(true);
		} else if (qgraphicsitem_cast<ElementTextItem *>(qgi)) {
			return(true);
		} else if (qgraphicsitem_cast<ConductorTextItem *>(qgi)) {
			return(true);
		} else if (Element *e = qgraphicsitem_cast<Element *>(qgi)) {
			// l'element est-il pivotable ?
			if (e -> orientation().current() != e -> orientation().next()) {
				return(true);
			}
		} else if (qgraphicsitem_cast<DiagramImageItem *>(qgi)) {
			return (true);
		}
	}
	return(false);
}
