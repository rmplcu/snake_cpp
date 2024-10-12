#include "snake.hpp"
#include <vector>
#include <iostream>
#include <random>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>

snake::Snake::Snake()
{
  this->blocks = {{}}; // 0,0,UP
}

snake::Snake::Snake(int x, int y, Direction d)
{
  this->blocks = {{{x, y}, d}};
}

void snake::Snake::add()
{
  Point new_d = snake::direction_to_point[snake::opposite_direction[blocks[blocks.size() - 1].direction]];
  SnakeBlock new_block = {
      {
          blocks[blocks.size() - 1].position.x + 1 * new_d.x,
          blocks[blocks.size() - 1].position.y + 1 * new_d.y,
      },
      blocks[blocks.size() - 1].direction};

  blocks.push_back(new_block);
}


void snake::Snake::update()
{
  Point new_d = snake::direction_to_point[this->blocks[0].direction];

  SnakeBlock new_bl = {
      {this->blocks[0].position.x + 1 * new_d.x,
       this->blocks[0].position.y + 1 * new_d.y},
      this->blocks[0].direction};

  this->blocks.pop_back();
  this->blocks.insert(this->blocks.begin(), new_bl);
}

void snake::Snake::turn(const Direction d)
{
  if (d != snake::opposite_direction[this->blocks[0].direction])
  {
    this->blocks[0].direction = d;
  }
}

void snake::Snake::print()
{
  std::cout << "[";
  for (unsigned int i = 0; i < blocks.size(); ++i)
  {
    std::cout << "(" << blocks[i].position.x << "," << blocks[i].position.y << "," << blocks[i].direction << ")";
    if (i < blocks.size() - 1)
    {
      std::cout << ", ";
    }
  }
  std::cout << "]" << '\n';
}

snake::SnakeBlock snake::Snake::head()
{
  return this->blocks[0];
}

snake::SnakeBlock snake::Snake::tail()
{
    return this->blocks[this->blocks.size() - 1];
}

snake::Game::Game()
{
    srand(time(NULL));

    this->snake = Snake(GRID_SIZE / 2, GRID_SIZE / 2, Direction::UP);
    this->map_set(GRID_SIZE / 2, GRID_SIZE / 2, &this->snake_color);
}

void snake::Game::spawn_cube(SDL_Renderer *renderer, SDL_Color color, int x, int y) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect box;
    box.x = x * BLOCK_SIZE;
    box.y = y * BLOCK_SIZE;
    box.h = box.w = BLOCK_SIZE;

    SDL_RenderFillRect(renderer, &box);
}

void snake::Game::clear_cube(SDL_Renderer* renderer, int x, int y) {
    this->spawn_cube(renderer, { 0, 0, 0, 0 }, x, y);
}

void snake::Game::map_set(int x, int y, SDL_Color *c) {
    std::string k = std::to_string(x) + "," + std::to_string(y);
    this->cache[k] = c;
}

SDL_Color* snake::Game::map_get(int x, int y) {
    std::string k = std::to_string(x) + "," + std::to_string(y);

    return cache.find(k) == cache.end() ? &this->empty_color : cache[k];
}

void snake::Game::map_remove(int x, int y) {
    std::string k = std::to_string(x) + "," + std::to_string(y);
    this->cache.erase(k);
}

void snake::Game::update(SDL_Renderer *renderer)
{

  SnakeBlock sb = this->snake.tail();
  this->clear_cube(renderer, sb.position.x, sb.position.y); //render update (snake moves)
  this->map_remove(sb.position.x, sb.position.y); //cache update (snake remove)

  this->snake.update(); //snake update

  sb = this->snake.head();

  //Check if game is over
  if (sb.position.x < 0 || sb.position.x >= GRID_SIZE || sb.position.y < 0 || sb.position.y >= GRID_SIZE || this->map_get(sb.position.x, sb.position.y) == &this->snake_color)
  {
    this->end();
  }

  //Eat fruit
  if (this->map_get(sb.position.x, sb.position.y) == &(this->fruit_color))
  {
    
    this->map_remove(sb.position.x, sb.position.y); //update cache (fruit remove)
    
    //Snake
    this->snake.add();
    
    //New fruit
    Point p = this->spawn_fruit();
    this->spawn_cube(renderer, this->fruit_color, p.x, p.y); //render update (fruit)
    this->map_set(p.x, p.y, &this->fruit_color);//update cache (fruit add)
  }
  this->spawn_cube(renderer, this->snake_color, sb.position.x, sb.position.y); //render update (snake moves)
  this->map_set(sb.position.x, sb.position.y, &this->snake_color); //cache update (snake add)
}

snake::Point snake::Game::spawn_fruit() {
    int r;
    std::string k;
    for (int i = 0; i < 5; ++i) {
        r = rand() % (GRID_SIZE * GRID_SIZE);
        k = std::to_string(r / GRID_SIZE) + "," + std::to_string(r % GRID_SIZE);
        
        if (this->cache.find(k) == cache.end()) return { r / GRID_SIZE, r% GRID_SIZE };
    }

    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            k = std::to_string(i) + "," + std::to_string(j);
            if (this->cache.find(k) == cache.end()) return { r / GRID_SIZE, r % GRID_SIZE };
        }
    }

    this->end();

    return {};
}

void snake::Game::run(SDL_Window * window, SDL_Renderer *renderer)
{
    //Hack to get window to stay up
    SDL_Event e;
    
    while (!this->over) {
        SDL_Keycode kc;
        //Handle inputs
        while (SDL_PollEvent(&e)) { 
            
            if (e.type == SDL_QUIT) over = true;
            if (e.type == SDL_KEYDOWN) {
                kc = e.key.keysym.sym;
            }

        }
        if (snake::cmd_to_dir.find(kc) != snake::cmd_to_dir.end()) this->snake.turn(snake::cmd_to_dir[kc]);
        this->update(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / FPS);
    }

    TTF_Init();
    TTF_Font *Sans = TTF_OpenFont("/Users/Utente/OneDrive/Desktop/Progetti/snake_cpp/AbhayaLibre-Regular.ttf", 44);
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "GAME OVER!", { 0xFF, 0xFF, 0xFF });
    SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

    SDL_Rect Message_rect;
    Message_rect.x = 0;
    Message_rect.y = 0;
    Message_rect.w = SCREEN_SIZE;
    Message_rect.h = SCREEN_SIZE;

    SDL_RenderCopy(renderer, Message, NULL, &Message_rect);
    SDL_RenderPresent(renderer);
    
    SDL_Delay(1000);

    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(Message);

}

void snake::Game::start() {
    SDL_Window* window = NULL;

    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    }
    else
    {
        //Create window
        window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_SIZE, SCREEN_SIZE, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        }
        else
        {
            
            //Fill the surface black

            //Update the surface
            //SDL_UpdateWindowSurface(window);

            //Create renderer
            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            
            if (!renderer) {
                printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
                return;
            }

            SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
            SDL_RenderClear(renderer);

            this->spawn_cube(renderer, this->snake_color, this->snake.head().position.x, this->snake.head().position.y); //render snake
            Point p = this->spawn_fruit(); //spawn fruit
            this->map_set(p.x, p.y, &this->fruit_color); //update cache
            this->spawn_cube(renderer, this->fruit_color, p.x, p.y); //render fruit

            SDL_RenderPresent(renderer);

            this->run(window, renderer);
        }
    }

    //Destroy window
    SDL_DestroyWindow(window);

    //Quit SDL subsystems
    SDL_Quit();

}

void snake::Game::end()
{
  this->over = true;
}