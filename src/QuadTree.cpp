#include <QuadTree.h>
#include <Utils.h>

namespace scially {

    QuadTree::QuadTree(double minX, double maxX, double minY, double maxY) {
        envelope_.MinX = minX;
        envelope_.MaxX = maxX;
        envelope_.MinY = minY;
        envelope_.MaxY = maxY;
    }

    bool QuadTree::add(int id, OGREnvelope box) noexcept {
        if (!envelope_.Intersects(box)) {
            return false;
        }
        if (envelope_.MaxX - envelope_.MinX <= METERIC) {
            geoms_.append(id);
            return true;
        }

        if (envelope_.Intersects(box)) {
            if (nodes_.isEmpty()) {
                split();
            }
            for (int i = 0; i < 4; i++) {
                //when box is added to a node, stop the loop
                if(nodes_[i]->add(id, box)){
                    return true;
                }
            }
        }
        return false;
    }

    void QuadTree::split() noexcept {
        double cX = (envelope_.MinX + envelope_.MaxX) / 2.0;
        double cY = (envelope_.MinY + envelope_.MaxY) / 2.0;
        nodes_.resize(4);
        for (int i = 0; i < 4; i++) {
            OGREnvelope box;
            switch (i) {
                case 0:
                    box.MinX = envelope_.MinX;
                    box.MaxX = cX;
                    box.MinY = envelope_.MinY;
                    box.MaxY = cY;

                    nodes_[i].reset(new QuadTree(box));
                    nodes_[i]->setNo(row_ * 2, col_ * 2, level_ + 1);
                    break;
                 case 1:
                    box.MinX = cX;
                    box.MaxX = envelope_.MaxX;
                    box.MinY = envelope_.MinY;
                    box.MaxY = cY;

                    nodes_[i].reset(new QuadTree(box));
                    nodes_[i]->setNo(row_ * 2 + 1, col_ * 2, level_ + 1);
                    break;
                  case 2:
                    box.MinX = cX;
                    box.MaxX = envelope_.MaxX;
                    box.MinY = cY;
                    box.MaxY = envelope_.MaxY;

                    nodes_[i].reset(new QuadTree(box));
                    nodes_[i]->setNo(row_ * 2 + 1, col_ * 2 + 1, level_ + 1);
                    break;
                  case 3:
                    box.MinX = envelope_.MinX;
                    box.MaxX = cX;
                    box.MinY = cY;
                    box.MaxY = envelope_.MaxY;

                    nodes_[i].reset(new QuadTree(box));
                    nodes_[i]->setNo(row_ * 2, col_ * 2 + 1, level_ + 1);
                    break;
              }
        }
    }
}
