/*
	Copyright 2006-2008 Xavier Guerrin
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
#ifndef QET_APP_H
#define QET_APP_H
#include <QApplication>
#include <QTranslator>
#include <QtGui>
class QETDiagramEditor;
class QETElementEditor;
/**
	Cette classe represente l'application QElectroTech.
	
*/
class QETApp : public QApplication {
	Q_OBJECT
	// constructeurs, destructeur
	public:
	QETApp(int &, char **);
	virtual ~QETApp();
	
	private:
	QETApp(const QETApp &);
	
	// methodes
	public:
	void setLanguage(const QString &);
	static void printHelp();
	static void printVersion();
	static void printLicense();
	
	static QString commonElementsDir();
	static QString customElementsDir();
	static QString configDir();
	static QSettings &settings();
	static QString languagesPath();
	static QString realPath(const QString &);
	static QString symbolicPath(const QString &);
#ifdef QET_ALLOW_OVERRIDE_CED_OPTION
	public:
	static void overrideCommonElementsDir(const QString &);
	private:
	static QString common_elements_dir; ///< Dossier contenant la collection d'elements commune
#endif
#ifdef QET_ALLOW_OVERRIDE_CD_OPTION
	public:
	static void overrideConfigDir(const QString &);
	private:
	static QString config_dir; ///< Dossier contenant la configuration et la collection d'elements de l'utilisateur
#endif
	public:
	static QString diagramTextsFont();
	
	protected:
	bool event(QEvent *);
	
	// attributs
	private:
	QTranslator qtTranslator;
	QTranslator qetTranslator;
	QSystemTrayIcon *qsti;
	QMenu *menu_systray;
	QAction *quitter_qet;
	QAction *reduce_appli;
	QAction *restore_appli;
	QAction *reduce_diagrams;
	QAction *restore_diagrams;
	QAction *reduce_elements;
	QAction *restore_elements;
	QAction *new_diagram;
	QAction *new_element;
	QHash<QMainWindow *, QByteArray> window_geometries;
	QHash<QMainWindow *, QByteArray> window_states;
	bool every_editor_reduced;
	bool every_diagram_reduced;
	bool every_diagram_visible;
	bool every_element_reduced;
	bool every_element_visible;
	QSignalMapper signal_map;
	QSettings *qet_settings;
	static QString diagram_texts_font;
	
	public slots:
	void systray(QSystemTrayIcon::ActivationReason);
	void reduceEveryEditor();
	void restoreEveryEditor();
	void reduceDiagramEditors();
	void restoreDiagramEditors();
	void reduceElementEditors();
	void restoreElementEditors();
	void newDiagramEditor();
	void newElementEditor();
	bool closeEveryEditor();
	void setMainWindowVisible(QMainWindow *, bool);
	void invertMainWindowVisibility(QWidget *);
	void quitQET();
	void checkRemainingWindows();
	
	// methodes privees
	private slots:
	void cleanup();
	
	private:
	QList<QETDiagramEditor *> diagramEditors() const;
	QList<QETElementEditor *> elementEditors() const;
	QList<QWidget *> floatingToolbarsAndDocksForMainWindow(QMainWindow *) const;
	void buildSystemTrayMenu();
	void fetchWindowStats(const QList<QETDiagramEditor *> &diagrams, const QList<QETElementEditor *> &elements);
};

/**
	Cette classe represente le style de QElectroTech
	Il s'agit de modifications simples appliquees aux styles communs de Qt
*/
class QETStyle : public QPlastiqueStyle {
	Q_OBJECT
	public:
	QETStyle();
	virtual ~QETStyle();
	virtual int styleHint(StyleHint hint, const QStyleOption * = 0, const QWidget * = 0, QStyleHintReturn * = 0) const;
	
	protected slots:
	virtual QIcon standardIconImplementation(StandardPixmap, const QStyleOption * = 0, const QWidget * = 0) const;
};
#endif
