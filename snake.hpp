#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include <map>
#include <string>
#include <SDL.h>

#define GRID_SIZE 64
#define SCREEN_SIZE 512
#define BLOCK_SIZE SCREEN_SIZE / GRID_SIZE
#define FPS 20

namespace snake
{
  enum Direction
  {
    UP,
    LEFT,
    RIGHT,
    DOWN
  };

  struct Point
  {
    int x;
    int y;
  };

  static Direction opposite_direction[4] = {DOWN, RIGHT, LEFT, UP};
  static Point direction_to_point[4] = {{0, -1}, {-1, 0}, {1, 0}, {0, 1}};
  static std::map<SDL_Keycode, Direction> cmd_to_dir = {{SDLK_w, Direction::UP}, {SDLK_s, Direction::DOWN}, {SDLK_a, Direction::LEFT}, {SDLK_d, Direction::RIGHT}};

  struct SnakeBlock
  {
    Point position;
    Direction direction;
  };

  class Snake
  {
  private:
    std::vector<SnakeBlock> blocks;

  public:
    Snake();
    Snake(int x, int y, Direction d);
    void add();
    void print();
    void update();
    SnakeBlock tail();
    SnakeBlock head();
    void turn(const Direction d);
  };

  class Game
  {
  private:
    snake::Snake snake;
    std::map<std::string, SDL_Color*> cache;
    SDL_Color snake_color = { 0x00, 0xFF, 0x00, 0x00 };
    SDL_Color fruit_color = { 0xFF, 0x00, 0x00, 0x00};
    SDL_Color empty_color = { 0x00, 0x00, 0x00, 0x00 };
    bool over = false;

    void update(SDL_Renderer *renderer);
    //Spawns a box of @color il position @x @y
    void spawn_cube(SDL_Renderer* renderer, SDL_Color color, int x, int y);
    Point spawn_fruit();

    //Cache operations
    void map_set(int x, int y, SDL_Color *c);
    SDL_Color *map_get(int x, int y);
    void map_remove(int x, int y);
    
    void clear_cube(SDL_Renderer *renderer, int x, int y);
    void end();
    void run(SDL_Window *window, SDL_Renderer *renderer);

  public:
    Game();
    void start();
  };

}
#endif