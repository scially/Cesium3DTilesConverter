#pragma once

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QException>
#include <ogr_spatialref.h>

namespace gzpi {
	class OGRException : QException {
	public:
		OGRException(OGRErr err) : error(err) {}
		
		virtual const char* what() const override {
			switch (error) {
			case OGRERR_NOT_ENOUGH_DATA:
				return "Not enough data";
				break;
			case OGRERR_NOT_ENOUGH_MEMORY:
				return "Not enough memory";
				break;
			case OGRERR_UNSUPPORTED_GEOMETRY_TYPE:
				return "Unsupported geometry type";
				break;
			case OGRERR_UNSUPPORTED_OPERATION:
				return "Unsupported operation";
				break;
			case OGRERR_CORRUPT_DATA:
				return "Corrupt data";
				break;
			case OGRERR_FAILURE:
				return "Failure";
				break;
			case OGRERR_UNSUPPORTED_SRS:
				return "Unsupported srs";
				break;
			case OGRERR_INVALID_HANDLE:
				return "Invalid handle";
				break;
			case OGRERR_NON_EXISTING_FEATURE:
				return "Non existing feataure";
				break;
			default:
				return "Unkonwn error";
			}	
		}
	private:
		OGRErr error;
	};
}
