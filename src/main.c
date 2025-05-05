
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define MAX_KEYS 64        // Increased to handle more keys
#define FADE_DURATION 2000 // Time in milliseconds for keys to fade out
#define KEY_GAP 4          // 4 pixel gap between keys
#define FONT_SIZE 36       // Larger text size
#define LEFT_MARGIN 50     // Left margin for alignment
#define RIGHT_MARGIN 50    // Right margin for alignment
#define MAX_WIDTH (WINDOW_WIDTH - LEFT_MARGIN - RIGHT_MARGIN)

typedef struct {
  char text[32];
  int width;
  int height;
  bool active;
} KeyDisplay;

KeyDisplay keyDisplays[MAX_KEYS];
int activeKeyCount = 0;
Uint32 lastKeyPressTime = 0;
int currentLineWidth = 0; // Track the current line width
TTF_Font *font = NULL;    // Global font variable

void addKeyDisplay(const char *keyName, int width, int height) {
  // Find an inactive slot or reuse the oldest one if all are active
  int index = 0;
  for (int i = 0; i < MAX_KEYS; i++) {
    if (!keyDisplays[i].active) {
      index = i;
      break;
    }
  }

  // If we found an inactive slot, increment activeKeyCount
  if (!keyDisplays[index].active) {
    activeKeyCount++;
  }

  // Set up the new key display
  strncpy(keyDisplays[index].text, keyName, 31);
  keyDisplays[index].text[31] = '\0';
  keyDisplays[index].active = true;
  keyDisplays[index].width = width;
  keyDisplays[index].height = height;

  // Update the last key press time for all keys to fade together
  lastKeyPressTime = SDL_GetTicks();
}

const char *getMacKeyName(SDL_Keycode key) {
  static char buffer[32];

  // Handle Mac-specific keys with text for arrow keys
  switch (key) {
  case SDLK_RETURN:
    return "Return";
  case SDLK_ESCAPE:
    return "Esc";
  case SDLK_BACKSPACE:
    return "Delete";
  case SDLK_SPACE:
    return "Space";
  case SDLK_TAB:
    return "Tab";
  case SDLK_LSHIFT:
    return "Shift";
  case SDLK_RSHIFT:
    return "Shift";
  case SDLK_LCTRL:
    return "Control";
  case SDLK_RCTRL:
    return "Control";
  case SDLK_LALT:
    return "Opt";
  case SDLK_RALT:
    return "Opt";
  case SDLK_LGUI:
    return "Cmd";
  case SDLK_RGUI:
    return "Cmd";
  case SDLK_UP:
    return "Up";
  case SDLK_DOWN:
    return "Down";
  case SDLK_LEFT:
    return "Left";
  case SDLK_RIGHT:
    return "Right";
  case SDLK_CAPSLOCK:
    return "Caps Lock";
  case SDLK_PAGEUP:
    return "Page Up";
  case SDLK_PAGEDOWN:
    return "Page Down";
  case SDLK_HOME:
    return "Home";
  case SDLK_END:
    return "End";
  case SDLK_DELETE:
    return "Del";
  case SDLK_F1:
    return "F1";
  case SDLK_F2:
    return "F2";
  case SDLK_F3:
    return "F3";
  case SDLK_F4:
    return "F4";
  case SDLK_F5:
    return "F5";
  case SDLK_F6:
    return "F6";
  case SDLK_F7:
    return "F7";
  case SDLK_F8:
    return "F8";
  case SDLK_F9:
    return "F9";
  case SDLK_F10:
    return "F10";
  case SDLK_F11:
    return "F11";
  case SDLK_F12:
    return "F12";
  default:
    // For regular keys, just return the character
    if (key >= 32 && key <= 126) {
      sprintf(buffer, "%c", (char)key);
    } else {
      sprintf(buffer, "Key_%d", key);
    }
    return buffer;
  }
}

// Pre-measure text dimensions
void measureText(TTF_Font *font, const char *text, int *width, int *height) {
  TTF_SizeText(font, text, width, height);
}

// Process a key press
void processKeyPress(SDL_Keycode key) {
  const char *keyName = getMacKeyName(key);

  // Pre-measure the key width and height before adding it
  int keyWidth, keyHeight;
  measureText(font, keyName, &keyWidth, &keyHeight);

  // Check if we need to wrap to the beginning
  if (currentLineWidth + keyWidth + (currentLineWidth > 0 ? KEY_GAP : 0) >
      MAX_WIDTH) {
    // Reset all keys to start a new line
    for (int i = 0; i < MAX_KEYS; i++) {
      keyDisplays[i].active = false;
    }
    activeKeyCount = 0;
    currentLineWidth = 0;
  }

  // Add key to display
  addKeyDisplay(keyName, keyWidth, keyHeight);

  // Update current line width (add key width + gap)
  if (currentLineWidth > 0) {
    currentLineWidth += KEY_GAP;
  }
  currentLineWidth += keyWidth;
}

// Global event filter to capture key events even when window loses focus
int eventFilter(void* userdata, SDL_Event* event) {
  if (event->type == SDL_KEYDOWN) {
    // Process key press directly from the event filter
    processKeyPress(event->key.keysym.sym);
  }
  
  // Return 1 to process the event by SDL
  return 1;
}

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  if (TTF_Init() < 0) {
    printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
    SDL_Quit();
    return 1;
  }

  // Create window without always-on-top flag
  SDL_Window *window = SDL_CreateWindow(
      "KeyCapper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  // Load font - try Mac system fonts first
  font = TTF_OpenFont("/System/Library/Fonts/SFNSDisplay.ttf", FONT_SIZE);
  if (!font) {
    font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", FONT_SIZE);
  }
  if (!font) {
    font = TTF_OpenFont("/Library/Fonts/Arial.ttf", FONT_SIZE);
  }
  if (!font) {
    printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
    printf("Please provide a font file.\n");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 1;
  }

  // Define colors
  SDL_Color textColor = {255, 255, 255, 255};  // White
  SDL_Color bgColor = {0, 0, 0, 255};          // Black background for text
  SDL_Color chromaKeyColor = {0, 255, 0, 255}; // Chroma key green

  // Initialize key displays
  for (int i = 0; i < MAX_KEYS; i++) {
    keyDisplays[i].active = false;
    keyDisplays[i].width = 0;
    keyDisplays[i].height = 0;
  }

  // Enable global keyboard tracking
  SDL_StartTextInput();

  // For Mac, we need to enable background events
  SDL_SetHint(SDL_HINT_MAC_BACKGROUND_APP, "1");
  
  // Set up the event filter to capture keys even when window loses focus
  SDL_SetEventFilter(eventFilter, NULL);

  // Main loop
  bool quit = false;
  SDL_Event e;

  while (!quit) {
    // Process events
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
      // Key events are handled by the event filter
    }

    // Clear screen with chroma key green
    SDL_SetRenderDrawColor(renderer, chromaKeyColor.r, chromaKeyColor.g,
                           chromaKeyColor.b, chromaKeyColor.a);
    SDL_RenderClear(renderer);

    Uint32 currentTime = SDL_GetTicks();

    // Only proceed if we have active keys
    if (activeKeyCount > 0) {
      // Calculate alpha based on elapsed time since last key press
      Uint32 elapsedTime = currentTime - lastKeyPressTime;

      if (elapsedTime > FADE_DURATION) {
        // Reset all keys if fade time has passed
        for (int i = 0; i < MAX_KEYS; i++) {
          keyDisplays[i].active = false;
        }
        activeKeyCount = 0;
        currentLineWidth = 0;
      } else {
        // Calculate alpha for fading (both text and background)
        Uint8 alpha = 255 - (Uint8)((elapsedTime * 255) / FADE_DURATION);

        // Calculate Y position to center vertically
        int maxHeight = 0;
        for (int i = 0; i < MAX_KEYS; i++) {
          if (keyDisplays[i].active && keyDisplays[i].height > maxHeight) {
            maxHeight = keyDisplays[i].height;
          }
        }

        int y = WINDOW_HEIGHT / 2 - maxHeight / 2;

        // First, determine the total width of all keys to create a universal background
        int totalWidth = 0;
        int activeKeys = 0;
        
        for (int i = 0; i < MAX_KEYS; i++) {
          if (keyDisplays[i].active) {
            if (activeKeys > 0) {
              totalWidth += KEY_GAP;
            }
            totalWidth += keyDisplays[i].width;
            activeKeys++;
          }
        }
        
        // Draw a single background for all keys
        if (activeKeys > 0) {
          SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
          SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, alpha);
          SDL_Rect bgRect = {
              LEFT_MARGIN - 4,     // Small padding left
              y - 4,               // Small padding top
              totalWidth + 8,      // Total width plus padding
              maxHeight + 8        // Height plus padding
          };
          SDL_RenderFillRect(renderer, &bgRect);
        }

        // Render all keys
        int currentX = LEFT_MARGIN;

        for (int i = 0; i < MAX_KEYS; i++) {
          if (keyDisplays[i].active) {
            // Create a surface for the text
            SDL_Surface *textSurface =
                TTF_RenderText_Blended(font, keyDisplays[i].text, textColor);
            if (textSurface) {
              // Create texture from surface
              SDL_Texture *textTexture =
                  SDL_CreateTextureFromSurface(renderer, textSurface);

              // Set the alpha for fading text
              SDL_SetTextureAlphaMod(textTexture, alpha);

              // Render the text
              SDL_Rect renderRect = {currentX, y, textSurface->w,
                                     textSurface->h};

              SDL_RenderCopy(renderer, textTexture, NULL, &renderRect);

              // Update X position for next key
              currentX += textSurface->w + KEY_GAP;

              // Clean up
              SDL_FreeSurface(textSurface);
              SDL_DestroyTexture(textTexture);
            }
          }
        }
      }
    }

    // Update the screen
    SDL_RenderPresent(renderer);

    // Small delay to reduce CPU usage
    SDL_Delay(16);
  }

  // Clean up
  SDL_StopTextInput();
  TTF_CloseFont(font);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();

  return 0;
}

