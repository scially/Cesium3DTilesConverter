#pragma once

#include <BaseObject.h>
#include <BoundingVolumeBox.h>
#include <BoundingVolumeRegion.h>
#include <BoundingVolumeSphere.h>

#include <QSharedPointer>
#include <QString>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include <optional>

namespace scially {

    class BoundingVolume : public BaseObject {
    public:
        BoundingVolume() = default;
        explicit BoundingVolume(const BoundingVolumeBox &box): box(box){}
        explicit BoundingVolume(const BoundingVolumeRegion &region): region(region){}
        explicit BoundingVolume(const BoundingVolumeSphere &sphere): sphere(sphere){}

        virtual QString typeName() override {
            return "boundingVolume";
        }

        virtual QJsonValue write() override{
            if(box.has_value()){
                return box->write();
            }else if(region.has_value()){
                return region->write();
            }else if(sphere.has_value()){
                return sphere->write();
            }else{
                // TODO: BoundingVolume type must be box,region,sphere
                qCritical() << "BoundingVolume not set";
                return QJsonValue();
            }
        }

        bool hasValue() const {
            return box.has_value() || region.has_value() || sphere.has_value();
        }

        inline BoundingVolume& operator =(const BoundingVolumeBox &box) {
            this->box = box;
            return *this;
        }
        inline BoundingVolume& operator =(const BoundingVolumeRegion &region){
            this->region = region;
            return *this;
        }
        inline BoundingVolume& operator =(const BoundingVolumeSphere &sphere){
            this->sphere = sphere;
            return *this;
        }

        virtual void read(const QJsonValue& object) override{
            if (object.type() != QJsonValue::Object) {
                qCritical() << "value is not BoundingVolume\n";
                return;
            } 

            QJsonObject bounding = object.toObject();
            if(bounding.keys().size() != 1){
                qCritical() << "BoundingVolume type must specideifed \n";
                return;
            }
            QString boundingType = bounding.keys().at(0);
            if(boundingType == "box"){
                BoundingVolumeBox boxResult;
                boxResult.read(bounding[boundingType]);
                box = boxResult;
            }else if(boundingType == "region"){
                BoundingVolumeRegion regionResult;
                regionResult.read(bounding[boundingType]);
                region = regionResult;
            }else if(boundingType == "sphere"){
                BoundingVolumeSphere sphereResult;
                sphereResult.read(bounding[boundingType]);
                sphere = sphereResult;
            }else{
                qCritical() << "BoundingVolume type must be box,region,sphere\n";
            }

        }

        virtual ~BoundingVolume() {}

        std::optional<BoundingVolumeBox> box;
        std::optional<BoundingVolumeRegion> region;
        std::optional<BoundingVolumeSphere> sphere;
    };
}

