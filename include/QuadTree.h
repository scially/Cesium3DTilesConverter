#pragma once

#include <ogrsf_frmts.h>
#include <functional>

#include <QVector>
#include <QString>
#include <QSharedPointer>

namespace scially {

    class QuadTree {
    public:
        QuadTree() = default;
        QuadTree(double minX, double maxX, double minY, double maxY);
        QuadTree(OGREnvelope e): envelope_(e) {}

        bool add(int id, OGREnvelope box) noexcept;
        void setNo(int r, int c, int l) noexcept {
            row_  = r;
            col_  = c;
            level_ = l;
        }
        void setEnvelope(OGREnvelope e) noexcept {          
            envelope_.MinX = e.MinX;
            envelope_.MaxX = e.MaxX;
            envelope_.MinY = e.MinY;
            envelope_.MaxY = e.MaxY;    
        }

        void traverse(std::function<void(QuadTree*)> f){
            if(!geoms_.isEmpty())
                f(this);
            
            for(auto iter = nodes_.begin(); iter != nodes_.end(); iter++){
                (*iter)->traverse(f);
            }
        }

        int geomSize() const noexcept{ return geoms_.size(); }
        int geomFID(int i) const noexcept { return geoms_.at(i); }
        int row() const noexcept { return row_; }
        int col() const noexcept { return col_; }
        int level() const noexcept { return level_; }
       
    private:
        void split() noexcept;

        OGREnvelope envelope_;
        QVector<QSharedPointer<QuadTree>> nodes_;
        int row_ = 0;
        int col_ = 0;
        int level_ = 0;
        QVector<int> geoms_;
    };
}