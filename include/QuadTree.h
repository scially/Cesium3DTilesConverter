#pragma once

#include <ogrsf_frmts.h>
#include <functional>
#include <QVector>
#include <QString>

namespace gzpi {

    class QuadTree {
    public:
        QuadTree() = default;
        QuadTree(double minX, double maxX, double minY, double maxY){
            envelope.MinX = minX;
            envelope.MaxX = maxX;
            envelope.MinY = minY;
            envelope.MaxY = maxY;
        }
        QuadTree(const OGREnvelope &e): envelope(e) {}
        bool add(long long id, const OGREnvelope& box);
        void setNo(int r, int c, int z){
            row  = r;
            col  = c;
            zoom = z;
        }
        void setEnvelope(const OGREnvelope &e){
            envelope.MinX = e.MinX;
            envelope.MaxX = e.MaxX;
            envelope.MinY = e.MinY;
            envelope.MaxY = e.MaxY;
        }

        void traverse(std::function<void(QuadTree*)> f){
            f(this);
            for(auto iter = nodes.begin(); iter != nodes.end(); iter++){
                (*iter)->traverse(f);
            }
        }

        int geomsSize() const { return geoms.size(); }
        int getGeomFID(int i) const { return geoms.at(i); }

        virtual ~QuadTree() {
            for(auto iter = nodes.begin(); iter != nodes.end(); iter++){
                if(*iter != nullptr)
                    delete *iter;
            }
        }

    private:
        void split();

        OGREnvelope envelope;
        QVector<QuadTree*> nodes;
        long long id;
        int row = 0;
        int col = 0;
        int zoom = 0;
        QVector<int> geoms;
    };
}
