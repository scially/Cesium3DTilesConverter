#pragma once

#include "BoundingVolumeReadWriter.h"
#include "ContentReadWriter.h"

#include <Cesium3DTiles/RootTile.h>

#include <QJsonObject>

namespace scially {

    class RootTileReadWriter{
    public:
        RootTile readFromJson(const QJsonValue& value) const {
            RootTile tile;
            tile.boundingVolume = mBoundingVolumeReadWriter.readFromJson(value["boundingVolume"]);
            tile.geometricError = value["geometricError"].toDouble();
            
            for (const auto& v : value["transform"].toArray()) {
                tile.transform.append(v.toDouble());
            }

            tile.content = mContentReadWriter.readFromJson(value["content"]);
            tile.refine = value["refine"].toString();
           
            for (const auto& v : value["children"].toArray()) {
                RootTile r = readFromJson(v);
                tile.children.append(r);
            }
            return tile;
        }

        QJsonValue writeToJson(const RootTile& tile) const {
            QJsonObject object;

            object["boundingVolume"] = mBoundingVolumeReadWriter.writeToJson(tile.boundingVolume);
            object["geometricError"] = tile.geometricError;
            object["refine"] = tile.refine;

            if (tile.transform != tile.TRANSFORM) {
                QJsonArray transformJsonValue;
                for (double v : tile.transform) {
                    transformJsonValue.append(v);
                }
                object["transform"] = transformJsonValue;
            }
            
            if (tile.content.has_value()) {
                object["content"] = mContentReadWriter.writeToJson(tile.content.value());
            }

            QJsonArray childrenJsonValue;
            for (const auto& child : tile.children) {
                QJsonValue childJsonValue = writeToJson(child);
                childrenJsonValue.append(childJsonValue);
            }
            if(!childrenJsonValue.isEmpty())
                object["children"] = childrenJsonValue;
            
            return object;
        }
    private:
        ContentReadWriter mContentReadWriter;
        BoundingVolumeReadWriter mBoundingVolumeReadWriter;
    };
}
