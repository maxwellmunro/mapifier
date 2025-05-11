#ifndef NODE_H
#define NODE_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>

class Node : public std::enable_shared_from_this<Node> {
public:
  static std::shared_ptr<Node> create(float x, float y, TTF_Font *font);

  void destruct();

  SDL_Color getBgColor() const;
  SDL_Color getTxtColor() const;

  void setBgColor(int r, int g, int b);

  float getX() const;
  float getY() const;

  void setX(float x);
  void setY(float y);

  void setXRec(float x);
  void setYRec(float y);

  float getRadius() const;

  void addNode(std::shared_ptr<Node> node);
  void removeNode(std::shared_ptr<Node> node);
  void clear();

  void toggleParent(std::shared_ptr<Node> node);
  void addParent(std::shared_ptr<Node> node);
  void removeParent(std::shared_ptr<Node> node);

  void setText(SDL_Renderer *renderer, std::string text);
  void appendText(SDL_Renderer *renderer, char text[32]);
  void popChar(SDL_Renderer *renderer);

  void tick(float dt);
  void render(SDL_Renderer *renderer);
  void renderLines(SDL_Renderer *renderer, float zoom);

  void setFont(TTF_Font* font);

private:
  Node(float x, float y, TTF_Font *font);
  void updateTextTexture(SDL_Renderer *renderer);
  SDL_Surface *renderMultilineSurface(const char *text, TTF_Font *font,
                                      SDL_Color color);

  std::vector<std::shared_ptr<Node>> parents;
  std::vector<std::shared_ptr<Node>> children;

  bool updateText = 0;

  float x;
  float y;

  float radius = 0;

  std::string text;

  SDL_Color textColor;
  SDL_Color bgColor;

  SDL_Surface *textSurface = nullptr;
  SDL_Texture *textTexture = nullptr;
  TTF_Font *font;

  bool centeredText = 1;

  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &boost::serialization::make_nvp("x", x);
    ar &boost::serialization::make_nvp("y", y);
    ar &boost::serialization::make_nvp("radius", radius);
    ar &boost::serialization::make_nvp("text", text);
    ar &boost::serialization::make_nvp("textColor_r", textColor.r);
    ar &boost::serialization::make_nvp("textColor_g", textColor.g);
    ar &boost::serialization::make_nvp("textColor_b", textColor.b);
    ar &boost::serialization::make_nvp("textColor_a", textColor.a);
    ar &boost::serialization::make_nvp("bgColor_r", bgColor.r);
    ar &boost::serialization::make_nvp("bgColor_g", bgColor.g);
    ar &boost::serialization::make_nvp("bgColor_b", bgColor.b);
    ar &boost::serialization::make_nvp("bgColor_a", bgColor.a);
    ar &boost::serialization::make_nvp("centeredText", centeredText);
    ar &boost::serialization::make_nvp("parents", parents);
    ar &boost::serialization::make_nvp("children", children);
  }

  Node() : x(0), y(0), font(nullptr) {}
};

#endif
