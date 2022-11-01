#pragma once

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QException>
#include <ogr_spatialref.h>

namespace scially {
	class OGRException : QException {
	public:
		OGRException(const QString& err) : error(err) {}
		OGRException(OGRErr err) {
			switch (err) {
			case OGRERR_NOT_ENOUGH_DATA:
				error =  "Not enough data";
				break;
			case OGRERR_NOT_ENOUGH_MEMORY:
				error =  "Not enough memory";
				break;
			case OGRERR_UNSUPPORTED_GEOMETRY_TYPE:
				error = "Unsupported geometry type";
				break;
			case OGRERR_UNSUPPORTED_OPERATION:
				error = "Unsupported operation";
				break;
			case OGRERR_CORRUPT_DATA:
				error = "Corrupt data";
				break;
			case OGRERR_FAILURE:
				error = "Failure";
				break;
			case OGRERR_UNSUPPORTED_SRS:
				error = "Unsupported srs";
				break;
			case OGRERR_INVALID_HANDLE:
				error = "Invalid handle";
				break;
			case OGRERR_NON_EXISTING_FEATURE:
				error = "Non existing feataure";
				break;
			default:
				error = "Unkonwn error";
			}
		}
		
        virtual const char* what() const noexcept override {
			return error.toStdString().data();
		}
	private:
		QString error;
	};
}
