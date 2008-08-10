#include "recentfiles.h"
#include "qetapp.h"

RecentFiles::RecentFiles(const QString &identifier, int size, QObject *parent) :
	QObject(parent),
	identifier_(identifier.isEmpty() ? "unnamed" : identifier),
	size_(size > 0 ? size : 10),
	menu_(0)
{
	mapper_ = new QSignalMapper(this);
	connect(mapper_, SIGNAL(mapped(const QString &)), this, SLOT(handleMenuRequest(const QString &)));
	
	extractFilesFromSettings();
	buildMenu();
}

/**
	Destructeur
	@todo determiner s'il faut detruire ou non le menu
*/
RecentFiles::~RecentFiles() {
	delete menu_;
}

/**
	@return le nombre de fichiers a retenir
*/
int RecentFiles::size() const {
	return(size_);
}

/**
	@return un menu listant les derniers fichiers ouverts
*/
QMenu *RecentFiles::menu() const {
	return(menu_);
}

/**
	Oublie les fichiers recents
*/
void RecentFiles::clear() {
	list_.clear();
	buildMenu();
}

/**
	Sauvegarde les fichiers r�cents dans la configuration
*/
void RecentFiles::save() {
	saveFilesToSettings();
}

/**
	Gere les actions sur le menu
*/
void RecentFiles::handleMenuRequest(const QString &filepath) {
	emit(fileOpeningRequested(filepath));
}

/**
	Gere le fait qu'un fichier ait ete ouvert
	@param filepath Chemin du fichier ouvert
*/
void RecentFiles::fileWasOpened(const QString &filepath) {
	insertFile(filepath);
	buildMenu();
}

/**
	lit la liste des fichiers recents dans la configuration
*/
void RecentFiles::extractFilesFromSettings() {
	// oublie la liste des fichiers recents
	list_.clear();
	
	// recupere les derniers fichiers ouverts dans la configuration
	for (int i = size_ ; i >= 1  ; -- i) {
		QString key(identifier_ + "-recentfiles/file" + QString::number(i));
		QString value(QETApp::settings().value(key, QString()).toString());
		insertFile(value);
	}
}

/**
	Insere un fichier dans la liste des fichiers recents
*/
void RecentFiles::insertFile(const QString &filepath) {
	// evite d'inserer un chemin de fichier vide ou en double
	if (filepath.isEmpty()) return;
	list_.removeAll(filepath);
	
	// insere le chemin de fichier
	list_.push_front(filepath);
	
	// s'assure que l'on ne retient pas plus de fichiers que necessaire
	while (list_.count() > size_) list_.removeLast();
}

/**
	ecrit la liste des fichiers recents dans la configuration
*/
void RecentFiles::saveFilesToSettings() {
	for (int i = 0 ; i < size_ && i < list_.count() ; ++ i) {
		QString key(identifier_ + "-recentfiles/file" + QString::number(i + 1));
		QETApp::settings().setValue(key, list_[i]);
	}
}

/**
	Construit le menu
*/
void RecentFiles::buildMenu() {
	// reinitialise le menu
	if (!menu_) {
		menu_ = new QMenu(tr("&R\351cemment ouvert(s)"));
		menu_ -> setIcon(QIcon(":/ico/open.png"));
	} else {
		menu_ -> clear();
	}
	
	// remplit le menu
	foreach (QString filepath, list_) {
		// creee une nouvelle action pour le fichier
		QAction *action = new QAction(filepath, 0);
		menu_ -> addAction(action);
		
		// lie l'action et le mapper
		mapper_ -> setMapping(action, filepath);
		connect(action, SIGNAL(triggered()), mapper_, SLOT(map()));
	}
}