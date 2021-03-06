#ifndef DIAGRAMCONTEXTWIDGET_H
#define DIAGRAMCONTEXTWIDGET_H
#include <QWidget>
#include "diagramcontext.h"
class QLabel;
class QTableWidget;
/**
	This class provides a table which enables end users to edit the key/value
	pairs of a DiagamContext.
*/
class DiagramContextWidget : public QWidget {
	Q_OBJECT
	// Constructor, destructor
	public:
	DiagramContextWidget(QWidget *parent = 0);
	virtual ~DiagramContextWidget();
	private:
	DiagramContextWidget(const DiagramContextWidget &);
	
	// methods
	public:
	bool isReadOnly();
	DiagramContext context() const;
	void setContext(const DiagramContext &);
	int nameLessRowsCount() const;
	void setSorting(bool enable, int column, Qt::SortOrder order );
	
	signals:
	
	public slots:
	void setReadOnly(bool);
	void clear();
	int highlightNonAcceptableKeys();
	void refreshFormatLabel();
	
	private slots:
	void checkTableRows();
	
	private:
	void initWidgets();
	void initLayout();
	
	// attributes
	private:
	QLabel *format_label; ///< label used to detail keys format
	QTableWidget *table_; ///< table used to enter key/value pairs
};

#endif
