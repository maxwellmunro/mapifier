#ifndef MAP_H
#define MAP_H

#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <memory>

#include "node.h"
#include <vector>

class Map {
public:

  std::vector<std::shared_ptr<Node>> parentNodes;
  std::vector<std::shared_ptr<Node>> nodes;

  std::shared_ptr<Node> currentNode = nullptr;

  float dx;
  float dy;
  
  inline static Map* curMap = nullptr;

  void saveMap(const std::string &filename);
  void loadMap(const std::string &filename, TTF_Font* font, float *dx, float *dy);

private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar &boost::serialization::make_nvp("parentNodes", parentNodes);
    ar &boost::serialization::make_nvp("nodes", nodes);
    ar &boost::serialization::make_nvp("currentNode", currentNode);
    ar &boost::serialization::make_nvp("dx", dx);
    ar &boost::serialization::make_nvp("dy", dy);
  }
  
};

#endif
