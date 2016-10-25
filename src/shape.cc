#include "shape.hpp"
#include <cmath>
#include <boost/math/constants/constants.hpp>

namespace bg = boost::geometry;

const double pi = boost::math::constants::pi<double>();
const int earthRadius = 6378137;

// Registers a bounding box
Shape::Shape(std::string id_, box const& b) {
  type = ShapeType::bbox;
  id = id_;
  envelope = b;
}

// Registers a circle
Shape::Shape(std::string id_, point const& p, double radius) {
  type = ShapeType::circle;
  id = id_;
  envelope = envelopeFromCircle(p, radius);
  center = p;
  outerRadius = radius;
}

// Registers an annulus
Shape::Shape(std::string id_, point const& p, double outerRadius_, double innerRadius_) {
  type = ShapeType::annulus;
  id = id_;
  envelope = envelopeFromCircle(p, outerRadius_);
  center = p;
  outerRadius = outerRadius_;
  innerRadius = innerRadius_;
}

// Registers a Polygon
Shape::Shape(std::string id_, polygon const& p) {
  type = ShapeType::poly;
  id = id_;
  envelope = bg::return_envelope<box>(p);
  polygonShape = p;
}

// Rough & fast approximation, should be enough to get a circle's MBR
box Shape::envelopeFromCircle(point const& p, double radius) {
  double
    lat = bg::get<1>(p),
    lon = bg::get<0>(p),
    dlat = radius / earthRadius,
    dlon = radius / (earthRadius*cos(pi*lat/180));

  dlat *= 180 / pi;
  dlon *= 180 / pi;

  return box(point(lon - dlon, lat - dlat), point(lon + dlon, lat + dlat));
}

bool Shape::covered(point const& p) {
  double distance;

  switch(type) {
    case ShapeType::bbox:
      /*
       the r* tree index handles bounding boxes so for boxes shapes,
       the approximation is the result
       */
      return true;
    case ShapeType::circle:
      return bg::distance(center, p) <= outerRadius;
    case ShapeType::annulus:
      distance = bg::distance(center, p);
      return distance <= outerRadius && distance >= innerRadius;
    case ShapeType::poly:
      return bg::covered_by(p, polygonShape);
  }

  return false;
}

const char *Shape::getId() {
  return id.c_str();
}

box Shape::getEnvelope() {
  return envelope;
}
