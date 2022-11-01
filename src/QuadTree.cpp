#include <QuadTree.h>
#include <Utils.h>

namespace scially {

    QuadTree::QuadTree(double minX, double maxX, double minY, double maxY) {
        envelope.MinX = minX;
        envelope.MaxX = maxX;
        envelope.MinY = minY;
        envelope.MaxY = maxY;
    }

    bool QuadTree::add(int id, const OGREnvelope& box) {
          if (!envelope.Intersects(box)) {
              return false;
          }
          if (envelope.MaxX - envelope.MinX <= METERIC) {
              geoms.append(id);
              return true;
          }

          if (envelope.Intersects(box)) {
              if (nodes.isEmpty()) {
                  split();
              }
              for (int i = 0; i < 4; i++) {
                  //when box is added to a node, stop the loop
                  if(nodes[i]->add(id, box)){
                      return true;
                  }
              }
          }
          return false;
    }

    void QuadTree::split() {
        double cX = (envelope.MinX + envelope.MaxX) / 2.0;
        double cY = (envelope.MinY + envelope.MaxY) / 2.0;
        nodes.resize(4);
        for (int i = 0; i < 4; i++) {
            OGREnvelope box;
            switch (i) {
                case 0:
                    box.MinX = envelope.MinX;
                    box.MaxX = cX;
                    box.MinY = envelope.MinY;
                    box.MaxY = cY;

                    nodes[i] = new QuadTree(box);
                    nodes[i]->setNo(row * 2, col * 2, level + 1);
                    break;
                 case 1:
                    box.MinX = cX;
                    box.MaxX = envelope.MaxX;
                    box.MinY = envelope.MinY;
                    box.MaxY = cY;

                    nodes[i] = new QuadTree(box);
                    nodes[i]->setNo(row * 2 + 1, col * 2, level + 1);
                    break;
                  case 2:
                    box.MinX = cX;
                    box.MaxX = envelope.MaxX;
                    box.MinY = cY;
                    box.MaxY = envelope.MaxY;

                    nodes[i] = new QuadTree(box);
                    nodes[i]->setNo(row * 2 + 1, col * 2 + 1, level + 1);
                    break;
                  case 3:
                    box.MinX = envelope.MinX;
                    box.MaxX = cX;
                    box.MinY = cY;
                    box.MaxY = envelope.MaxY;

                    nodes[i] = new QuadTree(box);
                    nodes[i]->setNo(row * 2, col * 2 + 1, level + 1);
                    break;
              }
        }
    }
}
