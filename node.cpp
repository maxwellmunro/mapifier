#include "node.h"
#include "map.h"
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <cstdio>
#include <iostream>

BOOST_CLASS_EXPORT_IMPLEMENT(Node);

std::shared_ptr<Node> Node::create(float x, float y, TTF_Font *font) {
  auto node = std::shared_ptr<Node>(new Node(x, y, font));
  Map::curMap->nodes.push_back(node);
  return node;
}

Node::Node(float x, float y, TTF_Font *font) : x(x), y(y), font(font) {
  this->bgColor = {37, 232, 250, 255};
  this->textColor = {0, 0, 0, 255};

  this->text = "Insert Text";
}

void Node::destruct() {
  try {
    if (auto self = shared_from_this()) {

      std::vector<std::shared_ptr<Node>> parentsCopy = this->parents;
      for (const auto &parent : parentsCopy) {
        if(parent) {
          parent->removeNode(self);
        }
      }

      std::vector<std::shared_ptr<Node>> childrenCopy = this->children;
        for (const auto &child : childrenCopy) {
          if(child) {
             child->removeParent(self);
          }
        }

      std::vector<int> parentNodeIndicesToErase;
      for (int i = 0; i < Map::curMap->parentNodes.size(); i++) {
        if (Map::curMap->parentNodes[i] == self) {
          parentNodeIndicesToErase.push_back(i);
        }
      }
      for (int i = parentNodeIndicesToErase.size() - 1; i >= 0; --i) {
        Map::curMap->parentNodes.erase(Map::curMap->parentNodes.begin() + parentNodeIndicesToErase[i]);
      }

      std::vector<int> nodeIndicesToErase;
      for (int i = 0; i < Map::curMap->nodes.size(); i++) {
        if (Map::curMap->nodes[i] == self) {
          nodeIndicesToErase.push_back(i);
        }
      }
      for (int i = nodeIndicesToErase.size() - 1; i >= 0; --i) {
        Map::curMap->nodes.erase(Map::curMap->nodes.begin() + nodeIndicesToErase[i]);
      }
    } else {
      std::cout << "Error: shared_from_this() failed\n";
    }
  } catch (const std::bad_weak_ptr &e) {
    std::cout << "shared_from_this failed: " << e.what() << '\n';
  }
}

SDL_Color Node::getBgColor() const { return this->bgColor; }
SDL_Color Node::getTxtColor() const { return this->textColor; }

void Node::setBgColor(int r, int g, int b) {
  if (r >= 0 && r <= 255)
    this->bgColor.r = r;
  if (g >= 0 && g <= 255)
    this->bgColor.g = g;
  if (b >= 0 && b <= 255)
    this->bgColor.b = b;
}

float Node::getX() const { return this->x; }
float Node::getY() const { return this->y; }

void Node::setX(float x) { this->x = x; }
void Node::setY(float y) { this->y = y; }

void Node::setXRec(float x) {
  for (const auto& node : this->children) {
    node->setXRec(node->getX() - this->x + x);
  }

  this->x = x;
}

void Node::setYRec(float y) {
  for (const auto& node : this->children) {
    node->setYRec(node->getY() - this->y + y);
  }

  this->y = y;
}

float Node::getRadius() const { return this->radius; }

void Node::addNode(std::shared_ptr<Node> node) {

  this->children.push_back(node);
}

void Node::removeNode(std::shared_ptr<Node> node) {
  for (int i = 0; i < children.size(); i++)
    if (children[i] == node) {
      children.erase(children.begin() + i);
    }
}

void Node::toggleParent(std::shared_ptr<Node> node) {
  if (node == shared_from_this())
    return;

  bool exists = 0;

  for (int i = 0; i < this->parents.size(); i++) {
    if (this->parents[i] == node) {
      this->parents[i]->removeNode(shared_from_this());
      this->parents.erase(this->parents.begin() + i);
      exists = 1;
    }
  }

  if (!exists) {
    this->parents.push_back(node);
    node->addNode(shared_from_this());
  }
}

void Node::addParent(std::shared_ptr<Node> node) {
  if (node == shared_from_this())
    return;

  for (const auto &parent : this->parents)
    if (parent == node)
      return;

  this->parents.push_back(node);
  node->addNode(shared_from_this());
}

void Node::removeParent(std::shared_ptr<Node> node) {
  for (int i = 0; i < this->parents.size(); i++) {
    if (this->parents[i] == node) {
      this->parents.erase(this->parents.begin() + i);
      node->removeNode(shared_from_this());
    }
  }
}

void Node::clear() {
  for (const auto &node : this->parents)
    node->removeNode(shared_from_this());

  for (const auto &node : this->children)
    node->removeParent(shared_from_this());

  this->parents.clear();
  this->children.clear();
}

SDL_Surface *Node::renderMultilineSurface(const char *text, TTF_Font *font,
                                          SDL_Color color) {
  char *textCopy = strdup(text);
  char *line = strtok(textCopy, "\n");

  int totalHeight = 0;
  int maxWidth = 0;
  char *lines[128];
  int numLines = 0;

  while (line && numLines < 128) {
    lines[numLines++] = strdup(line);

    int w, h;
    TTF_SizeUTF8(font, line, &w, &h);
    if (w > maxWidth)
      maxWidth = w;
    totalHeight += h;

    line = strtok(NULL, "\n");
  }

  SDL_Surface *final =
      SDL_CreateRGBSurface(0, maxWidth, totalHeight, 32, 0x00FF0000, 0x0000FF00,
                           0x000000FF, 0xFF000000);

  int y = 0;

  if (!final) {
    fprintf(stderr, "Failed to create final surface: %s\n", SDL_GetError());
    goto cleanup;
  }

  for (int i = 0; i < numLines; ++i) {
    SDL_Surface *lineSurf = TTF_RenderUTF8_Blended(font, lines[i], color);
    if (!lineSurf) {
      fprintf(stderr, "Render failed: %s\n", TTF_GetError());
      continue;
    }

    SDL_Rect dest = {this->centeredText ? (final->w - lineSurf->w) / 2 : 0, y,
                     lineSurf->w, lineSurf->h};
    SDL_BlitSurface(lineSurf, NULL, final, &dest);
    y += lineSurf->h;

    if (lineSurf) SDL_FreeSurface(lineSurf);
  }

cleanup:
  for (int i = 0; i < numLines; ++i)
    free(lines[i]);
  free(textCopy);

  return final;
}

void Node::updateTextTexture(SDL_Renderer *renderer) {
  if (this->textSurface) SDL_FreeSurface(this->textSurface);
  this->textSurface =
      TTF_RenderText_Solid(this->font, this->text.c_str(), this->textColor);

  if (!this->textSurface) {
    std::cout << "TTF_RenderText_Solid failed: " << TTF_GetError() << '\n';
    return;
  }

  int area = this->textSurface->w * this->textSurface->h;
  int len = static_cast<int>(sqrt(area));

  std::vector<std::string> words = {};

  std::string curWord = "";
  int width = 0;
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];

    if (c == ' ' && width > len) {
      width = 0;
      words.push_back(curWord);
      curWord = "";
      continue;
    }

    curWord += c;
    int tempWidth, tempHeight;
    char chr[2] = {c, '\0'};
    TTF_SizeText(font, chr, &tempWidth, &tempHeight);
    width += tempWidth;

    if (i + 1 == text.length())
      words.push_back(curWord);
  }

  std::string tempText = "";

  for (int i = 0; i < words.size(); i++) {
    std::string word = words[i];
    tempText += word;
    if (i + 1 != words.size())
      tempText += '\n';
  }

  if (this->textSurface) SDL_FreeSurface(this->textSurface);
  this->textSurface =
      renderMultilineSurface(tempText.c_str(), this->font, this->textColor);

  this->radius = static_cast<float>(
                     std::sqrt(this->textSurface->w * this->textSurface->w +
                               this->textSurface->h * this->textSurface->h)) /
                     2 +
                 10;

  if (!this->textSurface) {
    std::cout << "TTF_RenderText_Solid failed: " << TTF_GetError() << '\n';
    return;
  }

  if (this->textTexture) SDL_DestroyTexture(this->textTexture);
  this->textTexture = SDL_CreateTextureFromSurface(renderer, this->textSurface);

  if (!this->textTexture) {
    std::cout << "SDL_CreateTextureFromSurface failed: " << SDL_GetError()
              << '\n';
    return;
  }
}

void Node::setText(SDL_Renderer *renderer, std::string text) {
  this->text = text;
  updateTextTexture(renderer);
}

void Node::appendText(SDL_Renderer *renderer, char text[32]) {
  this->text += text;
  updateTextTexture(renderer);
}

void Node::popChar(SDL_Renderer *renderer) {
  if (this->text.empty())
    return;

  this->text.pop_back();
  updateTextTexture(renderer);
}

void Node::tick(float dt) {}

void Node::render(SDL_Renderer *renderer) {
  if (this->radius < 0) this->radius = 50;
  if (this->radius > 500) this->radius = 500;

  if (Map::curMap->currentNode == shared_from_this()) {
    filledCircleRGBA(renderer, this->x + Map::curMap->dx,
                     this->y + Map::curMap->dy, this->radius + 10, 0, 255, 0,
                     100);
  }

  filledCircleRGBA(renderer, this->x + Map::curMap->dx,
                   this->y + Map::curMap->dy, this->radius, this->bgColor.r,
                   this->bgColor.g, this->bgColor.b, 255);

  aacircleRGBA(renderer, this->x + Map::curMap->dx, this->y + Map::curMap->dy,
               this->radius, 0, 0, 0, 255);

  if (this->text.length() > 0) {
    if (this->textSurface == nullptr || updateText) {
      this->updateTextTexture(renderer);
      this->updateText = 0;
    } else {
      SDL_Rect rect = {static_cast<int>(this->x + Map::curMap->dx -
                                        this->textSurface->w / 2.0),
                       static_cast<int>(this->y + Map::curMap->dy -
                                        this->textSurface->h / 2.0),
                       this->textSurface->w, this->textSurface->h};
      SDL_RenderCopy(renderer, this->textTexture, nullptr, &rect);
    }
  }
}

void rotate(float &x, float &y, float angle) {
  float theta = static_cast<float>(std::atan(y / x) + (x < 0 ? M_PI : 0));
  theta += angle;

  float d = static_cast<float>(sqrt(x * x + y * y));

  x = d * cos(theta);
  y = d * sin(theta);
}

void Node::renderLines(SDL_Renderer *renderer, float zoom) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  for (const auto &node : this->children) {
    SDL_RenderDrawLine(
        renderer, (this->x + Map::curMap->dx) * zoom, (this->y + Map::curMap->dy) * zoom,
        (node->getX() + Map::curMap->dx) * zoom, (node->getY() + Map::curMap->dy) * zoom);

    float dx = node->getX() - this->x;
    float dy = node->getY() - this->y;

    float d = sqrt(dx * dx + dy * dy);

    float angle = static_cast<float>(std::atan(dy / dx) + (dx < 0 ? M_PI : 0));
    float cx = d - node->getRadius();
    float cy = 0;

    float x1 = d - node->getRadius() - 20;
    float x2 = x1;

    float y1 = -10;
    float y2 = 10;

    rotate(x1, y1, angle);
    rotate(x2, y2, angle);
    rotate(cx, cy, angle);

    x1 += this->x;
    x2 += this->x;

    y1 += this->y;
    y2 += this->y;

    cx += this->x;
    cy += this->y;

    SDL_RenderDrawLine(renderer, (cx + Map::curMap->dx) * zoom, (cy + Map::curMap->dy ) * zoom,
                       (x1 + Map::curMap->dx) * zoom, (y1 + Map::curMap->dy) * zoom);
    SDL_RenderDrawLine(renderer, (cx + Map::curMap->dx) * zoom, (cy + Map::curMap->dy) * zoom,
                       (x2 + Map::curMap->dx) * zoom, (y2 + Map::curMap->dy) * zoom);
  }
}

void Node::setFont(TTF_Font* font) {
  this->font = font;
}
