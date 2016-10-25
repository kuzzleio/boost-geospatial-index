#ifndef _BOOSTSPATIALINDEX_SHAPE
#define _BOOSTSPATIALINDEX_SHAPE
#include <memory>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>

enum ShapeType {bbox, ring, annulus, polygon};

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree>> point;
typedef boost::geometry::model::box<point> box;

class Shape {
  protected:
    std::string _id;
    ShapeType _type;
    std::shared_ptr<box> _box;
    std::shared_ptr<box> _envelope;

  public:
    Shape(std::string id, std::shared_ptr<box> b);
    bool covered(point const& p);
    const char *getId();
    box getEnvelope();
};

#endif
