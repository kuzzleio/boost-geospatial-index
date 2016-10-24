#include "shape.hpp"

namespace bg = boost::geometry;

Shape::Shape(std::shared_ptr<std::string> id, std::shared_ptr<box> b) {
  _type = ShapeType::bbox;
  _id = id;
  _box = b;
}

bool Shape::covered(point const& p) {
  switch(_type) {
    case ShapeType::bbox:
      return bg::covered_by(p, *_box);
  }

  return false;
}

const char *Shape::getId() {
  return _id->c_str();
}
