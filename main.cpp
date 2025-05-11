#include "map.h"
#include "node.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>

#include <iostream>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<int> dis(0, 255);

SDL_Window *window;
SDL_Renderer *renderer;

TTF_Font *mainFont;

bool running = 1;

int mouseX, mouseY;
int worldX, worldY;

float dx = 0, dy = 0;
float mouseDownX, mouseDownY;
float nodeDownX, nodeDownY;
bool rightDown = false;
bool leftDown = false;
bool typing = false;
bool settingParent = false;
bool typingFilename = false;

SDL_Surface *filenameSurface;

bool ctrlDown = false;

std::string home = std::getenv("HOME");

std::string filename = "map";

int width, height;

bool onColorSlider = 0;

SDL_Color clipboardColor = {};

int zoomInt = 100;
float zoom = 1;

void mouseDown(SDL_Event event) {
  if (event.button.button == 3) {
    mouseDownX = worldX;
    mouseDownY = worldY;
    rightDown = 1;
  }

  if (event.button.button == 1) {
    if (Map::curMap->currentNode) {
      if (mouseY > 20 && mouseY < 275 && mouseX > width - 140) {
        onColorSlider = 1;

        return;
      } else {
        onColorSlider = 0;
      }
    }
  }

  if (event.button.button == 1) {
    if (filenameSurface)
      if (mouseX < filenameSurface->w && mouseY < filenameSurface->h) {
        typingFilename = 1;
        SDL_StartTextInput();
        return;
      }

    leftDown = 0;
    for (const auto &node : Map::curMap->nodes) {
      float dx = node->getX() - worldX;
      float dy = node->getY() - worldY;
      int d = static_cast<int>(std::sqrt(dx * dx + dy * dy));
      if (d < node->getRadius()) {
        if (settingParent) {
          if (!Map::curMap->currentNode) {
            settingParent = 0;
            return;
          }
          Map::curMap->currentNode->toggleParent(node);
          settingParent = 0;
        } else {
          Map::curMap->currentNode = node;
          leftDown = 1;
          mouseDownX = worldX;
          mouseDownY = worldY;

          nodeDownX = node->getX();
          nodeDownY = node->getY();
        }
        break;
      }
    }
    if (!leftDown) {
      Map::curMap->currentNode = nullptr;
      typing = 0;
    }
  }
}

void mouseUp(SDL_Event event) {
  if (event.button.button == 3) {
    rightDown = 0;
    dx += worldX - mouseDownX;
    dy += worldY - mouseDownY;
  }

  if (event.button.button == 1) {
    leftDown = false;
    onColorSlider = 0;
  }
}

void mouseScroll(SDL_Event event) {

  float prex = mouseX / zoom;
  float prey = mouseY / zoom;

  if (event.wheel.preciseY > 0)
    zoomInt += 5;
  if (event.wheel.preciseY < 0)
    zoomInt -= 5;

  if (zoomInt > 100) zoomInt = 100;
  if (zoomInt < 10) zoomInt = 10;

  zoom = zoomInt / 100.0;

  float pstx = mouseX / zoom;
  float psty = mouseY / zoom;

  dx -= prex - pstx;
  dy -= prey - psty;
}

void keyDown(SDL_Event event) {
  SDL_Keycode key = event.key.keysym.sym;
  if (key == SDLK_LCTRL)
    ctrlDown = 1;

  if (key == SDLK_n && ctrlDown) {
    std::shared_ptr<Node> node =
        Node::create(1920 / 2 - dx, 1080 / 2 - dy, mainFont);
    Map::curMap->parentNodes.push_back(node);
  }

  if (key == SDLK_BACKSPACE && ctrlDown && Map::curMap->currentNode) {
    Map::curMap->currentNode->setText(renderer, "");
    typing = 1;
    SDL_StartTextInput();
  }

  if (key == SDLK_RETURN && Map::curMap->currentNode) {
    typing = !typing;

    if (typing)
      SDL_StartTextInput();
    else
      SDL_StopTextInput();
  }

  if (key == SDLK_RETURN && !typing) {
    typingFilename = 0;
    SDL_StopTextInput();
  }

  if (key == SDLK_BACKSPACE) {
    if (typingFilename)
      if (filename.length() > 0)
        filename.pop_back();
    if (typing && Map::curMap->currentNode)
      Map::curMap->currentNode->popChar(renderer);
  }

  if (key == SDLK_DELETE && Map::curMap->currentNode) {
    Map::curMap->currentNode->destruct();
    Map::curMap->currentNode = nullptr;
  }

  if (key == SDLK_s && ctrlDown && Map::curMap->currentNode) {
    settingParent = 1;
  }

  if (key == SDLK_c && ctrlDown && Map::curMap->currentNode) {
    Map::curMap->currentNode->clear();
  }

  if (key == SDLK_o && ctrlDown) {
    Map::curMap->loadMap(home + "/.mind/" + filename + ".mind", mainFont, &dx,
                         &dy);
    zoomInt = 100;
    zoom = 1;
  }

  if (key == SDLK_w && ctrlDown) {
    Map::curMap->saveMap(home + "/.mind/" + filename + ".mind");
  }

  if (key == SDLK_a && ctrlDown) {
    if (Map::curMap->currentNode) {
      clipboardColor = Map::curMap->currentNode->getBgColor();
    }
  }

  if (key == SDLK_r && ctrlDown) {
    if (Map::curMap->currentNode) {
      Map::curMap->currentNode->setBgColor(dis(gen), dis(gen), dis(gen));
    }
  }

  if (key == SDLK_z && ctrlDown) {
    if (Map::curMap->currentNode) {
      Map::curMap->currentNode->setBgColor(clipboardColor.r, clipboardColor.g,
                                           clipboardColor.b);
    }
  }

  if (key == SDLK_ESCAPE) {
    settingParent = 0;
    typing = 0;
    typingFilename = 0;
    SDL_StopTextInput();
  }
}

void keyUp(SDL_Event event) {
  SDL_Keycode key = event.key.keysym.sym;
  if (key == SDLK_LCTRL)
    ctrlDown = 0;
}

void typed(char text[32]) {
  if (typingFilename) {
    filename += text;
  } else if (typing)
    if (Map::curMap->currentNode)
      Map::curMap->currentNode->appendText(renderer, text);
}

int main() {
  std::cout << home << '\n';
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  window = SDL_CreateWindow("Mapifier", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 1920, 1080,
                            SDL_WINDOW_SHOWN);

  if (!window) {
    std::cout << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
    return 0;
  }

  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer) {
    std::cout << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
    return 0;
  }

  mainFont = TTF_OpenFont((home + "/.mind/res/mainFont.ttf").c_str(), 18);

  if (!mainFont) {
    std::cout << "TTF_OpenFont failed: " << TTF_GetError() << '\n';
    return 0;
  }

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

  Map::curMap = new Map();

  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_MOUSEBUTTONDOWN:
        mouseDown(event);
        break;
      case SDL_MOUSEBUTTONUP:
        mouseUp(event);
        break;
      case SDL_MOUSEWHEEL:
        mouseScroll(event);
        break;
      case SDL_KEYDOWN:
        keyDown(event);
        break;
      case SDL_KEYUP:
        keyUp(event);
        break;
      case SDL_TEXTINPUT:
        typed(event.text.text);
        break;
      }
    }

    if (onColorSlider && mouseY >= 20 && mouseY <= 275) {
      if (mouseX > width - 120 && mouseX < width - 100)
        Map::curMap->currentNode->setBgColor(275 - mouseY, -1, -1);
      if (mouseX > width - 80 && mouseX < width - 60)
        Map::curMap->currentNode->setBgColor(-1, 275 - mouseY, -1);
      if (mouseX > width - 40 && mouseX < width - 20)
        Map::curMap->currentNode->setBgColor(-1, -1, 275 - mouseY);
    }

    SDL_GetMouseState(&mouseX, &mouseY);
    worldX = static_cast<int>(mouseX / zoom - dx);
    worldY = static_cast<int>(mouseY / zoom - dy);

    if (leftDown && Map::curMap->currentNode) {
      if (ctrlDown) {
        Map::curMap->currentNode->setXRec(nodeDownX + worldX - mouseDownX);
        Map::curMap->currentNode->setYRec(nodeDownY + worldY - mouseDownY);
      } else {
        Map::curMap->currentNode->setX(nodeDownX + worldX - mouseDownX);
        Map::curMap->currentNode->setY(nodeDownY + worldY - mouseDownY);
      }
    }

    SDL_GetWindowSize(window, &width, &height);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    Map::curMap->dx = dx + (rightDown ? worldX - mouseDownX : 0);
    Map::curMap->dy = dy + (rightDown ? worldY - mouseDownY : 0);

    SDL_RenderSetScale(renderer, 1, 1);

    for (const auto &node : Map::curMap->nodes)
      if (node)
        node->renderLines(renderer, zoom);

    SDL_RenderSetScale(renderer, zoom, zoom);

    for (const auto &node : Map::curMap->nodes)
      if (node)
        node->render(renderer);

    SDL_RenderSetScale(renderer, 1, 1);

    if (filenameSurface)
      SDL_FreeSurface(filenameSurface);
    filenameSurface = TTF_RenderText_Blended(
        mainFont, (" File: " + filename + ".mind").c_str(),
        SDL_Color{0, 0, 0, 255});
    SDL_Texture *filenameTexture =
        SDL_CreateTextureFromSurface(renderer, filenameSurface);
    SDL_Rect rect = {0, 0, filenameSurface->w + 10, filenameSurface->h};
    SDL_SetRenderDrawColor(renderer, 150, 200, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);
    rect = {0, 0, filenameSurface->w, filenameSurface->h};
    SDL_RenderCopy(renderer, filenameTexture, nullptr, &rect);
    SDL_DestroyTexture(filenameTexture);

    if (Map::curMap->currentNode) {
      SDL_Rect rect = {width - 140, 0, 140, 335};
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderFillRect(renderer, &rect);
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderDrawRect(renderer, &rect);

      for (int i = 0; i < 255; i++) {
        SDL_Rect rect = {width - 120, 275 - i, 20, 1};

        int r = Map::curMap->currentNode->getBgColor().r;
        int g = Map::curMap->currentNode->getBgColor().g;
        int b = Map::curMap->currentNode->getBgColor().b;
        
        SDL_SetRenderDrawColor(renderer, i, g, b, 255);
        SDL_RenderFillRect(renderer, &rect);

        rect.x = width - 80;
        SDL_SetRenderDrawColor(renderer, r, i, b, 255);
        SDL_RenderFillRect(renderer, &rect);

        rect.x = width - 40;
        SDL_SetRenderDrawColor(renderer, r, g, i, 255);
        SDL_RenderFillRect(renderer, &rect);
      }

      SDL_Color col = Map::curMap->currentNode->getBgColor();

      SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);

      rect = {width - 120, 275 - col.r - 2, 20, 5};
      SDL_RenderDrawRect(renderer, &rect);
      rect = {width - 80, 275 - col.g - 2, 20, 5};
      SDL_RenderDrawRect(renderer, &rect);
      rect = {width - 40, 275 - col.b - 2, 20, 5};
      SDL_RenderDrawRect(renderer, &rect);

      rect = {width - 120, 295, 100, 20};
      SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
      SDL_RenderFillRect(renderer, &rect);
    }

    SDL_Surface* zoomSurf = TTF_RenderText_Blended(mainFont, (std::to_string(static_cast<int>(zoom * 100)) + std::string("%")).c_str(), SDL_Color{0, 0, 0, 255});
    SDL_Texture* zoomText = SDL_CreateTextureFromSurface(renderer, zoomSurf);
    rect = {0, height - zoomSurf->h, zoomSurf->w, zoomSurf->h};
    SDL_RenderCopy(renderer, zoomText, nullptr, &rect);

    SDL_FreeSurface(zoomSurf);
    SDL_DestroyTexture(zoomText);

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}
