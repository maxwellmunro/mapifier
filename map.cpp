#include "map.h"
#include <exception>
#include <fstream>

void Map::saveMap(const std::string &filename) {
  std::ofstream ofs(filename, std::ios::binary);
  boost::archive::binary_oarchive oa(ofs);
  oa << *this;
  ofs.close();
}

void Map::loadMap(const std::string &filename, TTF_Font *font, float *dx,
                  float *dy) {
  try {

    this->nodes.clear();
    this->parentNodes.clear();

    std::ifstream ifs(filename, std::ios::binary);
    boost::archive::binary_iarchive ia(ifs);
    ia >> *this;
    ifs.close();

    for (const auto &node : this->nodes) {
      node->setFont(font);
    }

    (*dx) = this->dx;
    (*dy) = this->dy;
  } catch (std::exception e) {
  }
}
