#include <Tileset.h>
#include <Utils.h>

#include <QDataStream>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

namespace scially {
    QJsonValue AssetProperties::write() {
        QJsonObject object;
        for (auto iter = assets.constKeyValueBegin(); iter != assets.constKeyValueEnd(); iter++) {
            object[iter->first] = iter->second;
        }
        return object;
    }

    void AssetProperties::read(const QJsonValue& object) {
        assets.clear();
        if (!object.isObject())
            return;

        for (const auto& key : object.toObject().keys()) {
            assets[key] = object[key].toString();
        }
    }

    QJsonValue BaseTile::write() {
        QJsonObject object;
        
        object[asset.typeName()] = asset.write();
        object["geometricError"] = geometricError;
        object[root.typeName()] = root.write();
        return object;
    }
    
    void BaseTile::read(const QJsonValue& object) {
        if (!object.isObject())
            return;
    
        asset.read(object["asset"]);
        geometricError = object["geometricError"].toDouble();
        root.read(object["root"]);

    }

    QJsonValue BoundingVolumeBox::write() {
        QJsonObject object;

        QJsonArray array;
        array.append(centerX());      array.append(centerY());      array.append(centerZ());
        array.append(halfXLength());  array.append(directionX0());  array.append(directionX1());
        array.append(directionY0());  array.append(halfYLength());  array.append(directionY1());
        array.append(directionZ0());  array.append(directionZ1());  array.append(halfZLength());

        object["box"] = array;
        return object;
    }

    void BoundingVolumeBox::read(const QJsonValue& object) {
        if(!object.isArray()){
            qCritical() << "parse BoundingVolumeBox: " << object << "failed\n";
            return;
        }

        double centerX = object[0].toDouble(),        centerY = object[1].toDouble(),        centerZ = object[2].toDouble();
        double directionX0 = object[4].toDouble(),    directionX1 = object[5].toDouble(),    halfXLength = object[3].toDouble();
        double directionY0 = object[6].toDouble(),    directionY1 = object[8].toDouble(),    halfYLength = object[7].toDouble();
        double directionZ0 = object[9].toDouble(),    directionZ1 = object[10].toDouble(),   halfZLength = object[11].toDouble();

        region.east = centerX + halfXLength;
        region.west = centerX - halfXLength;
        region.north = centerY + halfYLength;
        region.south = centerY - halfYLength;
        region.maxHeight = centerZ + halfZLength;
        region.minHeight = centerZ - halfZLength;
    }

    void BoundingVolumeBox::setMax(const osg::Vec3d& max){
        region.setMax(max);
    }
    void BoundingVolumeBox::setMin(const osg::Vec3d& min){
        region.setMax(min);
    }
    void BoundingVolumeBox::setMax(const osg::Vec3f& max){
        region.setMax(max);
    }
    void BoundingVolumeBox::setMin(const osg::Vec3f& min){
        region.setMin(min);
    }
    void BoundingVolumeBox::mergeMax(const osg::Vec3d& max) {
        region.mergeMax(max);
    }

    void BoundingVolumeBox::mergeMin(const osg::Vec3d & min) {
        region.mergeMin(min);
    }

    void BoundingVolumeBox::mergeMax(const osg::Vec3f& max) {
        region.mergeMax(max);
    }

    void BoundingVolumeBox::mergeMin(const osg::Vec3f& min) {
       region.mergeMin(min);
    }

    osg::Vec3d BoundingVolumeBox::getMax() const {
        return region.getMax();
    }
    osg::Vec3d BoundingVolumeBox::getMin() const {
        return region.getMax();
    }
    double BoundingVolumeBox::geometricError() const {
        return region.geometricError();
    }

    BoundingVolumeBox BoundingVolumeBox::merge(BoundingVolumeBox bounding) const {
      BoundingVolumeBox box(region.merge(bounding.region));
      return box;
   }

   QJsonValue BoundingVolumeRegion::write() {
        QJsonObject object;

        QJsonArray array;
        array.append(west);
        array.append(south);
        array.append(east);
        array.append(north);
        array.append(minHeight);
        array.append(maxHeight);

        object["region"] = array;
        return object;
    }

    void BoundingVolumeRegion::read(const QJsonValue& object) {
        if(!object.isArray()){
            qCritical() << "parse BoundingVolumeRegion: " << object << " failed\n";
        }

        west = object[0].toDouble();
        south = object[1].toDouble();
        east = object[2].toDouble();
        north = object[3].toDouble();
        minHeight = object[4].toDouble();
        maxHeight = object[5].toDouble();
    }


    osg::Vec3d BoundingVolumeRegion::getMax() const {
        return osg::Vec3d(west, south, minHeight);
    }
    osg::Vec3d BoundingVolumeRegion::getMin() const {
        return osg::Vec3d(east, north, maxHeight);
    }

    void BoundingVolumeRegion::setMax(const osg::Vec3d& max) {
        east = max[0];
        north = max[1];
        maxHeight = max[2];
    }

    void BoundingVolumeRegion::setMin(const osg::Vec3d& min) {
        west = min[0];
        south = min[1];
        minHeight = min[2];
    }

    void BoundingVolumeRegion::setMax(const osg::Vec3f& max) {
        east = max[0];
        north = max[1];
        maxHeight = max[2];
    }

    void BoundingVolumeRegion::setMin(const osg::Vec3f& min) {
        west = min[0];
        south = min[1];
        minHeight = min[2];
    }

    void BoundingVolumeRegion::mergeMax(const osg::Vec3d& max) {
        osg::Vec3d mergeValue = max < getMax() ? getMax() : max;
        setMax(mergeValue);
    }

    void BoundingVolumeRegion::mergeMin(const osg::Vec3d & min) {
        osg::Vec3d mergeValue = min < getMin() ? min : getMin();
        setMin(mergeValue);
    }

    void BoundingVolumeRegion::mergeMax(const osg::Vec3f& max) {
        osg::Vec3d _max(max[0], max[1], max[2]);
        mergeMax(_max);
    }

    void BoundingVolumeRegion::mergeMin(const osg::Vec3f& min) {
        osg::Vec3d _min(min[0], min[1], min[2]);
        mergeMin(_min);
    }

    BoundingVolumeRegion BoundingVolumeRegion::toRadin(double lon, double lat) const {
        double lonr = osg::DegreesToRadians(lon);
        double latr = osg::DegreesToRadians(lat);

        double west = lonr + meterToLon(this->west, latr);
        double south = latr + meterToLat(this->south);

        double east = lonr + meterToLon(this->east, latr);
        double north = latr + meterToLat(this->north);

        BoundingVolumeRegion region;
        region.west = west;
        region.south = south;
        region.east = east;
        region.north = north;
        region.minHeight = minHeight;
        region.maxHeight = maxHeight;
        return region;
    }

    BoundingVolumeRegion BoundingVolumeRegion::fromCenterXY(double centerX, double centerY, double xDiff, double yDiff, double minHeight, double maxHeight) {
        BoundingVolumeRegion region;
        double centerXr = osg::DegreesToRadians(centerX);
        double centerYr = osg::DegreesToRadians(centerY);
        double xDiffr = osg::DegreesToRadians(xDiff) * 1.05;
        double yDiffr = osg::DegreesToRadians(yDiff) * 1.05;
        region.west = centerXr - xDiffr / 2;
        region.east = centerXr + xDiffr / 2;  
        region.north = centerYr + yDiffr / 2;
        region.south = centerYr - yDiffr / 2;
        region.minHeight = minHeight;
        region.maxHeight = maxHeight;
        return region;  
    }

    QJsonValue BoundingVolumeSphere::write() {
        QJsonObject object;

        QJsonArray array;
        array.append(centerX);
        array.append(centerY);
        array.append(centerZ);
        array.append(radius);

        object["sphere"] = array;
        return object;
    }

    void BoundingVolumeSphere::read(const QJsonValue& object) {
        if(!object.isArray()){
            qCritical() << "parse BoundingVolumeSphere: " << object << "failed\n";
            return;
        }

        centerX = object[0].toDouble();
        centerY = object[1].toDouble();
        centerZ = object[2].toDouble();
        radius = object[3].toDouble();
    }

    QJsonValue ContentTile::write() {
        QJsonObject object;
        if (boundingVolume.has_value() && boundingVolume->hasValue())
            object["boundingVolume"] = boundingVolume->write();
        object["uri"] = uri;
        return object;
    }

    void ContentTile::read(const QJsonValue& object) {
       if(!object.isObject())
            return;
       uri = object["uri"].toString();

       QJsonValue boundingVolumeJson = object["boundingVolume"];
       if(boundingVolumeJson.isObject()){
           BoundingVolume boundingResult;
           boundingResult.read(boundingVolumeJson);
           boundingVolume = boundingResult;
       }

    }

    void Refine::read(const QJsonValue& object) {
       if(object.isString()){
            type = object.toString();
       }
    }

    QJsonValue Refine::write() {
        return type;
    }

    RootTile::RootTile() {
        children.clear();
    }

    QJsonValue RootTile::write() {
        QJsonObject object;
        object["boundingVolume"] = boundingVolume.write();
        object["geometricError"] = geometricError;
        object["refine"] = refine.write();
        object[transform.typeName()] = transform.write();

        if (content.has_value()) {
            object["content"] = content->write();
        }

        QJsonArray childrenValue;
        for (auto& child : children) {
            childrenValue.append(child.write());
        }
        object["children"] = childrenValue;
        return object;
    }

    void RootTile::read(const QJsonValue& object) {
        if (object.isNull())
            return;

        boundingVolume.read(object["boundingVolume"]);
        geometricError = object["geometricError"].toDouble();
        transform.read(object["transform"]);

        content.emplace();
        content->read(object["content"]);

        refine.read(object["refine"]);

        QJsonArray values = object["children"].toArray();
        for (const auto& value : values) {
            RootTile r;
            r.read(value);
            children.append(r);
        }
    }

    QJsonValue TileMatrix::write() {
        QJsonArray array;
        array.append(_mat[0][0]);  array.append(_mat[1][0]);  array.append(_mat[2][0]);  array.append(_mat[3][0]);
        array.append(_mat[0][1]);  array.append(_mat[1][1]);  array.append(_mat[2][1]);  array.append(_mat[3][1]);
        array.append(_mat[0][2]);  array.append(_mat[1][2]);  array.append(_mat[2][2]);  array.append(_mat[3][2]);
        array.append(_mat[0][3]);  array.append(_mat[1][3]);  array.append(_mat[2][3]);  array.append(_mat[3][3]);

        return array;
    }

    void TileMatrix::read(const QJsonValue& object) {
        if(!object.isArray())
            return;

        _mat[0][0] = object[0].toDouble();  _mat[1][0] = object[1].toDouble();  _mat[2][0] = object[2].toDouble();  _mat[3][0]= object[3].toDouble();
        _mat[0][1] = object[4].toDouble();  _mat[1][1] = object[5].toDouble();  _mat[2][1] = object[6].toDouble();  _mat[3][1]= object[7].toDouble();
        _mat[0][2] = object[8].toDouble();  _mat[1][2] = object[9].toDouble();  _mat[2][2] = object[10].toDouble(); _mat[3][2] = object[11].toDouble();
        _mat[0][3] = object[12].toDouble(); _mat[1][3] = object[13].toDouble(); _mat[2][3] = object[14].toDouble(); _mat[3][3] = object[15].toDouble();
    }

    TileMatrix TileMatrix::fromXYZ(double lon, double lat, double minHeight) {
        double lonr = osg::DegreesToRadians(lon);
        double latr = osg::DegreesToRadians(lat);

        constexpr double ellipsodA = 40680631590769;
        constexpr double ellipsodB = 40680631590769;
        constexpr double ellipsodC = 40408299984661.4;

        double xn = std::cos(lonr) * std::cos(latr);
        double yn = std::sin(lonr) * std::cos(latr);
        double zn = std::sin(latr);

        double x0 = ellipsodA * xn;
        double y0 = ellipsodB * yn;
        double z0 = ellipsodC * zn;
        double gamma = std::sqrt(xn * x0 + yn * y0 + zn * z0);
        double px = x0 / gamma;
        double py = y0 / gamma;
        double pz = z0 / gamma;

        double dx = xn * minHeight;
        double dy = yn * minHeight;
        double dz = zn * minHeight;

        osg::Vec3d eastVec(-y0, x0, 0);
        osg::Vec3d northVec(
            (y0 * eastVec[2] - eastVec[1] * z0),
            (z0 * eastVec[0] - eastVec[2] * x0),
            (x0 * eastVec[1] - eastVec[0] * y0));

        osg::Vec3d eastUnit =  eastVec  / eastVec.length();
        osg::Vec3d northUnit = northVec / northVec.length();
        
        return TileMatrix(
            eastUnit[0], northUnit[0], xn, px + dx,
            eastUnit[1], northUnit[1], yn, py + dy,
            eastUnit[2], northUnit[2], zn, pz + dz,
            0,           0,            0,  1);
    }


    QByteArray Batched3DModel::write(bool withHeight) const {
		QByteArray buffer;
        QDataStream dataStream(&buffer, QIODevice::WriteOnly);
        dataStream.setByteOrder(QDataStream::LittleEndian);

        // feature json and feature binary
        QString featureJsonStr = QString(R"({"BATCH_LENGTH":%1})").arg(batchLength);

        while (featureJsonStr.size() % 4 != 0) {
            featureJsonStr.append(' ');
        }

        // batch json and batch binary
        QJsonObject batchJson;
        batchJson["batchId"] = arrayToQJsonArray(batchID.begin(), batchID.end());
        batchJson["name"] = arrayToQJsonArray(names.begin(), names.end());

        if (withHeight) {
            batchJson["height"] = arrayToQJsonArray(heights.begin(), heights.end());
        }

        QString batchJsonStr = jsonDump(batchJson);
        while (batchJsonStr.size() % 4 != 0) {
            batchJsonStr.append(' ');
        }


        // feature binary and batch binary not implement
        int totalSize = 28 /*header size*/ + featureJsonStr.size() + batchJsonStr.size() + glbBuffer.size();

        dataStream.writeRawData("b3dm", 4);
        dataStream << 1;
        dataStream << totalSize;
        dataStream << featureJsonStr.size();
        dataStream << 0; // feature binary
        dataStream << batchJsonStr.size();
        dataStream << 0;  // batch binary

        dataStream.writeRawData(featureJsonStr.toStdString().data(), featureJsonStr.size());
        dataStream.writeRawData(batchJsonStr.toStdString().data(), batchJsonStr.size());
        dataStream.writeRawData(glbBuffer.data(), glbBuffer.size());

		return buffer;
	}

}
