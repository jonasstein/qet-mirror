#ifndef CONDUCER_PROFILE_H
#define CONDUCER_PROFILE_H
#include <QList>
#include "qet.h"
class Conducer;
class ConducerSegmentProfile;
/**
	Cette classe contient le profil (= les caracteristiques essentielles) d'un
	conducteur.
*/
class ConducerProfile {
	public:
	// constructeurs, destructeur
	ConducerProfile();
	ConducerProfile(Conducer *conducer);
	ConducerProfile(const ConducerProfile &);
	ConducerProfile &operator=(const ConducerProfile &);
	virtual ~ConducerProfile();
	
	// attributs
	public:
	QList<ConducerSegmentProfile *> segments;
	QET::Orientation beginOrientation;
	QET::Orientation endOrientation;
	
	// methodes
	public:
	bool isNull() const;
	void setNull();
	qreal width() const;
	qreal height() const;
	uint nbSegments(QET::ConducerSegmentType) const;
	QList<ConducerSegmentProfile *> horizontalSegments();
	QList<ConducerSegmentProfile *> verticalSegments();
	void fromConducer(Conducer *);
};
QDebug &operator<<(QDebug d, ConducerProfile &);
#endif