#pragma once

#include <array>

#include <QVariantMap>
#include <QString>
#include <QDir>
#include <QFile>
#include <QByteArray>

double degree2rad(double val);
double lati_to_meter(double diff);
double longti_to_meter(double diff, double lati);
double meter_to_lati(double m);
double meter_to_longti(double m, double lati);

bool create_dir(const QString& path);

bool epsg_convert(int insrs, double& x, double& y);
bool wkt_convert(const QString& inwkt, double& x, double& y);

QString get_parent_name(const QString& file_path);
QString get_file_name(const QString& file_path);

bool write_file(const QString& out_file, const QByteArray& bytes);
