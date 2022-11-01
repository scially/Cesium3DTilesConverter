#pragma once

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <BaseObject.h>
#include <BoundingVolumeRegion.h>

namespace scially {
    /// <summary>
    /// The boundingVolume.box property is an array of 12 numbers 
    /// that define an oriented bounding box in a right-handed 3-axis (x, y, z) Cartesian coordinate system 
    /// where the z-axis is up. 
    /// The first three elements define the x, y, and z values for the center of the box. 
    /// The next three elements (with indices 3, 4, and 5) define the x-axis direction and half-length. 
    /// The next three elements (indices 6, 7, and 8) define the y-axis direction and half-length. 
    /// The last three elements (indices 9, 10, and 11) define the z-axis direction and half-length.
    /// </summary>
    class BoundingVolumeBox : public BaseObject {
    public:
        explicit BoundingVolumeBox(BoundingVolumeRegion r): region(r){}
        BoundingVolumeBox() = default;

        double centerX() {
            return (region.east + region.west) / 2;
        }
        double centerY() {
            return (region.north + region.south) / 2;
        }
        double centerZ() {
            return (region.minHeight + region.maxHeight) / 2;
        }

        double directionX0 () {
            return 0;
        }
        double directionX1 () {
            return 0;
        }
        double halfXLength () {
            return (region.east - region.west) / 2;
        }

        double directionY0() {
            return 0;
        }

        double directionY1() {
            return 0;
        }

        double halfYLength () {
            return (region.north - region.south) / 2;
        }

        double directionZ0 () {
            return 0;
        }

        double directionZ1() {
            return 0;
        }

        double halfZLength () {
            return (region.maxHeight - region.minHeight) / 2;
        }

        virtual QJsonValue write() override;
        virtual void read(const QJsonValue& object) override;
        virtual QString typeName() override {
            return "box";
        }

        void setMax(const osg::Vec3d& max);
        void setMin(const osg::Vec3d& min);
        void setMax(const osg::Vec3f& max);
        void setMin(const osg::Vec3f& min);

        void mergeMax(const osg::Vec3d& max);
        void mergeMin(const osg::Vec3d& min);
        void mergeMax(const osg::Vec3f& max);
        void mergeMin(const osg::Vec3f& min);

        BoundingVolumeBox merge(BoundingVolumeBox bounding) const;

        double geometricError() const;
        osg::Vec3d getMax() const;
        osg::Vec3d getMin() const;

        virtual ~BoundingVolumeBox() {}
    private:
        BoundingVolumeRegion region;
    };
}
