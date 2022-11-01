#pragma once

#include <ogrsf_frmts.h>
#include <functional>
#include <QVector>
#include <QString>

namespace scially {

    class QuadTree {
    public:
        QuadTree() = default;
        QuadTree(double minX, double maxX, double minY, double maxY);
        QuadTree(const OGREnvelope &e): envelope(e) {}

        bool add(int id, const OGREnvelope& box);
        
        void setNo(int r, int c, int l){
            row  = r;
            col  = c;
            level = l;
        }
        void setEnvelope(const OGREnvelope &e){
            envelope.MinX = e.MinX;
            envelope.MaxX = e.MaxX;
            envelope.MinY = e.MinY;
            envelope.MaxY = e.MaxY;
        }

        void traverse(std::function<void(QuadTree*)> f){
            if(!geoms.isEmpty())
                f(this);
            
            for(auto iter = nodes.begin(); iter != nodes.end(); iter++){
                (*iter)->traverse(f);
            }
        }

        int geomsSize() const { return geoms.size(); }
        int getGeomFID(int i) const { return geoms.at(i); }
        int getRow() const { return row; }
        int getCol() const { return col; }
        int getLevel() const { return level; }
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
        int row = 0;
        int col = 0;
        int level = 0;
        QVector<int> geoms;
    };
}
