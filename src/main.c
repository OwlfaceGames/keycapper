
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define MAX_KEYS 32
#define FADE_DURATION 2000  // Time in milliseconds for a key to fade out

typedef struct {
    char text[32];
    int x, y;
    Uint32 pressTime;
    bool active;
} KeyDisplay;

KeyDisplay keyDisplays[MAX_KEYS];
int activeKeyCount = 0;

void addKeyDisplay(const char* keyName) {
    // Find an inactive slot or the oldest active one if all are active
    int index = 0;
    Uint32 oldestTime = SDL_GetTicks();
    
    for (int i = 0; i < MAX_KEYS; i++) {
        if (!keyDisplays[i].active) {
            index = i;
            break;
        }
        if (keyDisplays[i].pressTime < oldestTime) {
            oldestTime = keyDisplays[i].pressTime;
            index = i;
        }
    }
    
    // Set up the new key display
    strncpy(keyDisplays[index].text, keyName, 31);
    keyDisplays[index].text[31] = '\0';
    
    // Position in a grid-like pattern
    keyDisplays[index].x = 50 + (index % 8) * 150;
    keyDisplays[index].y = 50 + (index / 8) * 100;
    
    keyDisplays[index].pressTime = SDL_GetTicks();
    keyDisplays[index].active = true;
    
    if (activeKeyCount < MAX_KEYS) {
        activeKeyCount++;
    }
}

const char* getKeyName(SDL_Keycode key) {
    static char buffer[32];
    
    // Handle special keys
    switch (key) {
        case SDLK_RETURN: return "RETURN";
        case SDLK_ESCAPE: return "ESC";
        case SDLK_BACKSPACE: return "BACKSPACE";
        case SDLK_SPACE: return "SPACE";
        case SDLK_TAB: return "TAB";
        case SDLK_LSHIFT: return "LSHIFT";
        case SDLK_RSHIFT: return "RSHIFT";
        case SDLK_LCTRL: return "LCTRL";
        case SDLK_RCTRL: return "RCTRL";
        case SDLK_LALT: return "LALT";
        case SDLK_RALT: return "RALT";
        case SDLK_UP: return "UP";
        case SDLK_DOWN: return "DOWN";
        case SDLK_LEFT: return "LEFT";
        case SDLK_RIGHT: return "RIGHT";
        default:
            // For regular keys, just return the character
            if (key >= 32 && key <= 126) {
                sprintf(buffer, "%c", (char)key);
            } else {
                sprintf(buffer, "KEY_%d", key);
            }
            return buffer;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("KeyCapper", 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         SDL_WINDOWPOS_UNDEFINED, 
                                         WINDOW_WIDTH, 
                                         WINDOW_HEIGHT, 
                                         SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Load font
    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 24);
    if (!font) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        printf("Trying system font location...\n");
        
        // Try common system font locations
        font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
        if (!font) {
            font = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", 24);
        }
        if (!font) {
            font = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 24);
        }
        
        if (!font) {
            printf("Could not find any usable font. Please provide a font file.\n");
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            return 1;
        }
    }
    
    // Define colors
    SDL_Color textColor = {255, 255, 255, 255};  // White
    SDL_Color bgColor = {0, 0, 0, 255};          // Black background for text
    SDL_Color chromaKeyColor = {0, 255, 0, 255}; // Chroma key green
    
    bool quit = false;
    SDL_Event e;
    
    // Initialize key displays
    for (int i = 0; i < MAX_KEYS; i++) {
        keyDisplays[i].active = false;
    }
    
    Uint32 lastUpdateTime = SDL_GetTicks();
    
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                // Add key to display
                addKeyDisplay(getKeyName(e.key.keysym.sym));
            }
        }
        
        // Clear screen with chroma key green
        SDL_SetRenderDrawColor(renderer, chromaKeyColor.r, chromaKeyColor.g, chromaKeyColor.b, chromaKeyColor.a);
        SDL_RenderClear(renderer);
        
        Uint32 currentTime = SDL_GetTicks();
        
        // Update and render active keys
        for (int i = 0; i < MAX_KEYS; i++) {
            if (keyDisplays[i].active) {
                Uint32 elapsedTime = currentTime - keyDisplays[i].pressTime;
                
                if (elapsedTime > FADE_DURATION) {
                    keyDisplays[i].active = false;
                    activeKeyCount--;
                    continue;
                }
                
                // Calculate alpha based on elapsed time
                Uint8 alpha = 255 - (Uint8)((elapsedTime * 255) / FADE_DURATION);
                
                // Create a surface for the text
                SDL_Surface* textSurface = TTF_RenderText_Shaded(font, keyDisplays[i].text, textColor, bgColor);
                if (textSurface) {
                    // Create texture from surface
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    
                    // Set the alpha for fading
                    SDL_SetTextureAlphaMod(textTexture, alpha);
                    
                    // Render the text
                    SDL_Rect renderRect = {
                        keyDisplays[i].x,
                        keyDisplays[i].y,
                        textSurface->w,
                        textSurface->h
                    };
                    
                    SDL_RenderCopy(renderer, textTexture, NULL, &renderRect);
                    
                    // Clean up
                    SDL_FreeSurface(textSurface);
                    SDL_DestroyTexture(textTexture);
                }
            }
        }
        
        // Update the screen
        SDL_RenderPresent(renderer);
        
        // Cap to ~60 FPS
        SDL_Delay(16);
    }
    
    // Clean up
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}

