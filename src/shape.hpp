#ifndef _BOOSTSPATIALINDEX_SHAPE
#define _BOOSTSPATIALINDEX_SHAPE
#include <memory>
#include <string>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>

enum ShapeType {bbox, circle, annulus, poly};

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree>> point;
typedef boost::geometry::model::box<point> box;
typedef boost::geometry::model::polygon<point, false, false> polygon;

class Shape {
  protected:
    std::string id;
    ShapeType type;
    box envelope; // the minimum bounding rectangle embedding the provided shape
    box envelopeFromCircle(point const& p, double radius);
    double getDistance(point const& origin, point const& p);

    /*
     * shape-specific storage objects
     * (boxes do not need those as the envelope already describe these)
     */
    // - circle and annulus
    point center;
    double outerRadius;
    //std::pair<double, double>

    // annulus
    double innerRadius;

    // polygon
    polygon polygonShape;

  public:
    // Registers a box shape
    Shape(std::string id_, box const& b);

    // Registers a circle shape. Radius must be in meters
    Shape(std::string id_, point const& p, double radius);

    // Registers an annulus shape. Radiuses must be in meters
    Shape(std::string id_, point const& p, double outerRadius_, double innerRadius_);

    // Registers a polygon shape
    Shape(std::string id_, polygon const& p);

    // getters
    const char *getId();
    box getEnvelope();

    // others
    bool covered(point const& p);
};

#endif
