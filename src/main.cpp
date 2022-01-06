#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QDomDocument>
#include <QJsonDocument>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QTime>

#include "osgb23dtiles.h"
#include "utils.h"

#define MAX_1V1 100

void convert_osgb(
	const QString& input, const QString& output,
	int maxLevel,
	int threadCount,
	std::optional<double> centerX,
	std::optional<double> centerY,
	std::optional<double> regionOffset);

void convert_shapefile(const QString& input, const QString& output, const QString& height);
void convert_gltf(const QString& input, const QString& output);
void convert_b3dm(const QString& input, const QString& output);

int main(int argc, char** argv) {
	// 1.…Ë÷√GDAL_DATA
	// 2.…Ë÷√PROJ_LIB
	CPLSetConfigOption("GDAL_DATA", "D:\\vcpkg\\installed\\x86-windows\\share\\gdal");
	QCoreApplication app(argc, argv);
	app.setApplicationName("Make 3dtile program");
	app.setApplicationVersion("1.0.0");
	app.setOrganizationName("hwang <imhwang@126.com>");

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();

	QCommandLineOption inputOption("input", "Set the input file (Required)", "input");
	QCommandLineOption outputOption("output", "Set the output file (Required)", "output");
	QCommandLineOption formatOption("format", "osgb, shp, gltf (Required)", "format");
	QCommandLineOption xOption("x", "Set the offset x (Option) ");
	QCommandLineOption yOption("y", "Set the offset y (Option) ");
	QCommandLineOption zOption("z", "Set the offset z (Option) ");
	QCommandLineOption maxLevelOption("maxlvl", "Set the max of process level (Option)", "100", "100");
	QCommandLineOption heightOption("height", "Set the shapefile height field (Requried for shp)", "height");
	QCommandLineOption threadOption("thread", "Set the thread count (Default 4)", "4", "4");

	parser.addOption(inputOption);
	parser.addOption(outputOption);
	parser.addOption(formatOption);
	parser.addOption(xOption);
	parser.addOption(yOption);
	parser.addOption(zOption);
	parser.addOption(maxLevelOption);
	parser.addOption(heightOption);
	parser.addOption(threadOption);

	parser.parse(app.arguments());

	if (app.arguments().size() == 1)
		parser.showHelp();

	if (!parser.isSet("input") || !parser.isSet("output") || !parser.isSet("format")) 
		parser.showHelp(-1);
	
	QString input = parser.value("input");
	QString output = parser.value("output");
	QString format = parser.value("format");
	int maxLevel = parser.value("maxlvl").toInt();
	int threadCount = parser.value("thread").toInt() < 4 ? 4 : parser.value("thread").toInt();

	QFile inputFile(input);
	if (!inputFile.exists()) {
		qFatal("%s does not exists.", input);
		return -1;
	}
	QTime beginTime = QTime::currentTime();
	if (format == "osgb") {
		std::optional<double> centerX;
		std::optional<double> centerY;
		std::optional<double> regionOffset;

		if (parser.isSet("x")) 
			centerX = parser.value("x").toDouble();
		if (parser.isSet("y"))
			centerY = parser.value("y").toDouble();
		if (parser.isSet("z"))
			regionOffset = parser.value("z").toDouble();
		convert_osgb(input, output, maxLevel, threadCount, centerX, centerY, regionOffset);
	}
		
	else if (format == "shape") {
		if (parser.isSet("height"))
			parser.showHelp();
		QString height = parser.value("height");
		convert_shapefile(input, output, height);
	}
		
	else if (format == "gltf")
		convert_gltf(input, output);
	else if (format == "b3dm")
		convert_b3dm(input, output);
	else{
		qFatal("not support now.", input);
		return -1;
	}
	QTime endTime = QTime::currentTime();
	qInfo() << "start process: " << beginTime.secsTo(endTime) << "s";
	return 0;
}


void convert_osgb(
	const QString& input, const QString& output,
	int maxLevel,
	int threadCount,
	std::optional<double> centerX,
	std::optional<double> centerY,
	std::optional<double> regionOffset) {

	QFile metadataXmlFile(input + "/metadata.xml");
	if (!metadataXmlFile.exists()) {
		qWarning() << input + "/metadata.xml" << "is missing";
		return;
	}

	if (!metadataXmlFile.open(QFile::ReadOnly)) {
		qWarning() << "open" << input + "/metadata.xml" << " failed";
		return;
	}

	QDomDocument metadataXmlReader;
	if (!metadataXmlReader.setContent(&metadataXmlFile)) {
		qWarning() << "parse" << input + "/metadata.xml" << " failed";
		return;
	}
	
	QDomNodeList tmpNodes = metadataXmlReader.elementsByTagName("SRS");
	assert(tmpNodes.size() == 1);
	QDomNode srsNode = tmpNodes.at(0);
	tmpNodes = metadataXmlReader.elementsByTagName("SRSOrigin");
	assert(tmpNodes.size() == 1);
	QDomNode originNode = tmpNodes.at(0);

	QStringList srsNodeSplit = srsNode.toElement().text().split(":");

	double center_x = 0;
	double center_y = 0;
	double trans_region = 0;

	if (srsNodeSplit[0] == "ENU") {
		QStringList coors = srsNodeSplit[1].split(",");
		center_x = coors[1].toDouble();
		center_y = coors[0].toDouble();
		qInfo("ENU: x->%f, y->%f", center_x, center_y);
	}
	else if (srsNodeSplit[0] == "EPSG") {
		QStringList coors = originNode.toElement().text().split(",");
		center_x = coors[0].toDouble();
		center_y = coors[1].toDouble();
		
		epsg_convert(srsNodeSplit[1].toInt(), center_x, center_y);
		qInfo("EPSG: x->%f, y->%f", center_x, center_y);
	}
	else {
		// WKT
		wkt_convert(srsNode.toElement().text(), center_x, center_y);
		qInfo("WKT: x->%f, y->%2f", center_x, center_y);
	}

	if (centerX.has_value() && centerY.has_value() && regionOffset.has_value()) {
		center_x = centerX.value();
		center_y = centerY.value();
		trans_region = regionOffset.value();
	}

	osgb_batch_convert(input, output, maxLevel, threadCount, center_x, center_y, trans_region);
}

void convert_shapefile(const QString& input, const QString& output, const QString& height) {

}
void convert_gltf(const QString& input, const QString& output) {

}
void convert_b3dm(const QString& input, const QString& output){

}