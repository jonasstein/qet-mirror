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
#ifndef QET_ARGUMENTS_H
#define QET_ARGUMENTS_H
#include <QtCore>
/**
	Cette classe represente un ensemble d'arguments que l'application peut
	recevoir en parametres. Initialisee a partir d'une liste de chaine de
	caracteres, chaque instance de cette classe permet d'acceder aux differents
	types de fichiers et options passes en parametres.
*/
class QETArguments : public QObject {
	Q_OBJECT
	
	// constructeurs, destructeur
	public:
	QETArguments(QObject * = 0);
	QETArguments(const QList<QString> &, QObject * = 0);
	QETArguments(const QETArguments &);
	QETArguments &operator=(const QETArguments &);
	virtual ~QETArguments();
	
	// methodes
	public:
	virtual void setArguments(const QList<QString> &);
	virtual QList<QString> arguments() const;
	virtual QList<QString> files() const;
	virtual QList<QString> projectFiles() const;
	virtual QList<QString> elementFiles() const;
#ifdef QET_ALLOW_OVERRIDE_CED_OPTION
	virtual bool commonElementsDirSpecified() const;
	virtual QString commonElementsDir() const;
#endif
#ifdef QET_ALLOW_OVERRIDE_CD_OPTION
	virtual bool configDirSpecified() const;
	virtual QString configDir() const;
#endif
	virtual bool printHelpRequested() const;
	virtual bool printLicenseRequested() const;
	virtual bool printVersionRequested() const;
	virtual QList<QString> options() const;
	virtual QList<QString> unknownOptions() const;
	
	private:
	void clear();
	void parseArguments(const QList<QString> &);
	void handleFileArgument(const QString &);
	void handleOptionArgument(const QString &);
	
	// attributs
	private:
	QList<QString> project_files_;
	QList<QString> element_files_;
	QList<QString> options_;
	QList<QString> unknown_options_;
#ifdef QET_ALLOW_OVERRIDE_CED_OPTION
	QString common_elements_dir_;
#endif
#ifdef QET_ALLOW_OVERRIDE_CD_OPTION
	QString config_dir_;
#endif
	bool print_help_;
	bool print_license_;
	bool print_version_;
};
#endif