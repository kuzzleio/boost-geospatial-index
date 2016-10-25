#include "shape.hpp"

namespace bg = boost::geometry;

Shape::Shape(std::string id, std::shared_ptr<box> b) {
  _type = ShapeType::bbox;
  _id = id;
  _box = b;
  _envelope = b;
}

bool Shape::covered(point const& p) {
  switch(_type) {
    case ShapeType::bbox:
      /*
       the index handles bounding boxes, and already retrieves covering
       boxes, so there is no need for further calculation
       */
      return true;
  }

  return false;
}

const char *Shape::getId() {
  return _id.c_str();
}

box Shape::getEnvelope() {
  return *_envelope;
}
