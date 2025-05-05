
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Include macOS-specific headers for global event monitoring
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define MAX_KEYS 64        // Increased to handle more keys
#define FADE_DURATION 2000 // Time in milliseconds for keys to fade out
#define KEY_GAP 4          // 4 pixel gap between keys
#define FONT_SIZE 36       // Larger text size
#define BUTTON_FONT_SIZE 18 // Smaller font size for button
#define LEFT_MARGIN 50     // Left margin for alignment
#define RIGHT_MARGIN 50    // Right margin for alignment
#define MAX_WIDTH (WINDOW_WIDTH - LEFT_MARGIN - RIGHT_MARGIN)
#define BUTTON_WIDTH 120   // Width of the toggle button
#define BUTTON_HEIGHT 40   // Height of the toggle button

typedef struct {
  char text[32];
  int width;
  int height;
  bool active;
} KeyDisplay;

typedef struct {
  SDL_Rect rect;
  char text[32];
  bool hovered;
  bool pressed;
} Button;

KeyDisplay keyDisplays[MAX_KEYS];
int activeKeyCount = 0;
Uint32 lastKeyPressTime = 0;
int currentLineWidth = 0; // Track the current line width
TTF_Font *font = NULL;    // Global font variable
TTF_Font *buttonFont = NULL; // Font for button text
bool shouldQuit = false;  // Global flag for quitting
bool rightAligned = false; // Flag for right-to-left alignment
Button toggleButton;      // Toggle button for alignment

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

const char *getMacKeyName(int keyCode) {
  static char buffer[32];

  // Map macOS key codes to readable names
  switch (keyCode) {
    case 36: return "Return";
    case 53: return "Esc";
    case 51: return "Bksp";  // Changed from "Delete" to "Bksp" for backspace
    case 49: return "Space";
    case 48: return "Tab";
    case 56: return "Shift";  // Left Shift
    case 60: return "Shift";  // Right Shift
    case 59: return "Ctrl";   // Left Control (abbreviated)
    case 62: return "Ctrl";   // Right Control (abbreviated)
    case 58: return "Opt";    // Left Option
    case 61: return "Opt";    // Right Option
    case 55: return "Cmd";    // Left Command
    case 54: return "Cmd";    // Right Command
    case 126: return "Up";
    case 125: return "Down";
    case 123: return "Left";
    case 124: return "Right";
    case 57: return "Caps";   // Abbreviated Caps Lock
    case 116: return "PgUp";  // Abbreviated Page Up
    case 121: return "PgDn";  // Abbreviated Page Down
    case 115: return "Home";
    case 119: return "End";
    case 117: return "Del";   // Forward delete key
    case 122: return "F1";
    case 120: return "F2";
    case 99: return "F3";
    case 118: return "F4";
    case 96: return "F5";
    case 97: return "F6";
    case 98: return "F7";
    case 100: return "F8";
    case 101: return "F9";
    case 109: return "F10";
    case 103: return "F11";
    case 111: return "F12";
    default:
      // For regular keys, try to get the character
      if (keyCode >= 0 && keyCode <= 127) {
        // Map common keys - this is not complete
        const char* keyMap[] = {
          "a", "s", "d", "f", "h", "g", "z", "x", "c", "v", "§", "b", "q", "w", "e", "r",
          "y", "t", "1", "2", "3", "4", "6", "5", "=", "9", "7", "-", "8", "0", "]", "o",
          "u", "[", "i", "p", "Return", "l", "j", "'", "k", ";", "\\", ",", "/", "n", "m", ".",
          "Tab", "Space", "`", "Bksp", "", "Esc", "", "Cmd", "Shift", "Caps", "Opt", "Ctrl", "", "", "", "",
          "", ".", "", "*", "", "+", "", "", "", "", "", "/", "Return", "", "-", "", "", "", "", "", "",
          "Up", "Down", "Right", "Left", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
        };
        
        if (keyCode < 128 && keyMap[keyCode][0] != '\0') {
          return keyMap[keyCode];
        }
      }
      
      sprintf(buffer, "Key_%d", keyCode);
      return buffer;
  }
}

// Pre-measure text dimensions
void measureText(TTF_Font *font, const char *text, int *width, int *height) {
  TTF_SizeText(font, text, width, height);
}

// Initialize the toggle button
void initToggleButton() {
  toggleButton.rect.x = WINDOW_WIDTH - BUTTON_WIDTH - 20; // 20px from right edge
  toggleButton.rect.y = WINDOW_HEIGHT - BUTTON_HEIGHT - 20; // 20px from bottom edge
  toggleButton.rect.w = BUTTON_WIDTH;
  toggleButton.rect.h = BUTTON_HEIGHT;
  strcpy(toggleButton.text, "Toggle Align");
  toggleButton.hovered = false;
  toggleButton.pressed = false;
}

// Check if a point is inside the button
bool isPointInButton(int x, int y, Button *button) {
  return (x >= button->rect.x && x < button->rect.x + button->rect.w &&
          y >= button->rect.y && y < button->rect.y + button->rect.h);
}

// Draw the toggle button
void drawButton(SDL_Renderer *renderer, TTF_Font *font, Button *button) {
  // Draw button background
  SDL_Color bgColor = {100, 100, 100, 255}; // Gray background
  if (button->hovered) {
    bgColor.r = 120;
    bgColor.g = 120;
    bgColor.b = 120;
  }
  if (button->pressed) {
    bgColor.r = 80;
    bgColor.g = 80;
    bgColor.b = 80;
  }
  
  SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
  SDL_RenderFillRect(renderer, &button->rect);
  
  // Draw button border
  SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
  SDL_RenderDrawRect(renderer, &button->rect);
  
  // Draw button text
  SDL_Color textColor = {255, 255, 255, 255}; // White text
  SDL_Surface *textSurface = TTF_RenderText_Blended(font, button->text, textColor);
  if (textSurface) {
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    
    // Center text in button
    SDL_Rect textRect = {
      button->rect.x + (button->rect.w - textSurface->w) / 2,
      button->rect.y + (button->rect.h - textSurface->h) / 2,
      textSurface->w,
      textSurface->h
    };
    
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
  }
}

// Process a key press
void processKeyPress(const char* keyName) {
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

#ifdef __APPLE__
// macOS global key event callback
CGEventRef keyboardCaptureCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    // Handle both key down and flag changed events (for modifier keys)
    if (type != kCGEventKeyDown && type != kCGEventFlagsChanged) {
        return event;
    }
    
    // Get the key code and modifiers
    CGKeyCode keyCode;
    CGEventFlags flags = CGEventGetFlags(event);
    static CGEventFlags lastFlags = 0;
    
    if (type == kCGEventKeyDown) {
        // For regular key presses
        keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
        
        // Get the key name
        const char* keyName = getMacKeyName(keyCode);
        
        // Process the key press in the SDL thread
        SDL_Event sdlEvent;
        sdlEvent.type = SDL_USEREVENT;
        sdlEvent.user.code = 1;  // Custom code for key press
        sdlEvent.user.data1 = (void*)strdup(keyName);  // Pass the key name
        SDL_PushEvent(&sdlEvent);
    } 
    else if (type == kCGEventFlagsChanged) {
        // For modifier key events
        // Check which modifier key changed
        
        // Command key
        if ((flags & kCGEventFlagMaskCommand) != (lastFlags & kCGEventFlagMaskCommand)) {
            if (flags & kCGEventFlagMaskCommand) {
                // Command key pressed
                SDL_Event sdlEvent;
                sdlEvent.type = SDL_USEREVENT;
                sdlEvent.user.code = 1;
                sdlEvent.user.data1 = (void*)strdup("Cmd");
                SDL_PushEvent(&sdlEvent);
            }
        }
        
        // Option/Alt key
        if ((flags & kCGEventFlagMaskAlternate) != (lastFlags & kCGEventFlagMaskAlternate)) {
            if (flags & kCGEventFlagMaskAlternate) {
                // Option key pressed
                SDL_Event sdlEvent;
                sdlEvent.type = SDL_USEREVENT;
                sdlEvent.user.code = 1;
                sdlEvent.user.data1 = (void*)strdup("Opt");
                SDL_PushEvent(&sdlEvent);
            }
        }
        
        // Control key
        if ((flags & kCGEventFlagMaskControl) != (lastFlags & kCGEventFlagMaskControl)) {
            if (flags & kCGEventFlagMaskControl) {
                // Control key pressed
                SDL_Event sdlEvent;
                sdlEvent.type = SDL_USEREVENT;
                sdlEvent.user.code = 1;
                sdlEvent.user.data1 = (void*)strdup("Ctrl");
                SDL_PushEvent(&sdlEvent);
            }
        }
        
        // Shift key
        if ((flags & kCGEventFlagMaskShift) != (lastFlags & kCGEventFlagMaskShift)) {
            if (flags & kCGEventFlagMaskShift) {
                // Shift key pressed
                SDL_Event sdlEvent;
                sdlEvent.type = SDL_USEREVENT;
                sdlEvent.user.code = 1;
                sdlEvent.user.data1 = (void*)strdup("Shift");
                SDL_PushEvent(&sdlEvent);
            }
        }
        
        // Update last flags
        lastFlags = flags;
    }
    
    // Allow the event to pass through to the application
    return event;
}

// Set up macOS global event monitoring
void setupGlobalKeyCapture() {
    // Create an event tap to monitor key down and flags changed events
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap,  // Capture events for all apps in the current session
        kCGHeadInsertEventTap,  // Insert at the head of the event queue
        kCGEventTapOptionDefault,  // Default options
        eventMask,  // Capture key down and flags changed events
        keyboardCaptureCallback,  // Callback function
        NULL  // User data
    );
    
    if (!eventTap) {
        printf("Failed to create event tap. Make sure your app has accessibility permissions.\n");
        return;
    }
    
    // Create a run loop source from the event tap
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    
    // Add the source to the current run loop
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    
    // Enable the event tap
    CGEventTapEnable(eventTap, true);
    
    // Start the event tap in a separate thread
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        CFRunLoopRun();
    });
    
    printf("Global key capture initialized.\n");
}
#endif

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

  // Create window
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

  // Load main font for keys
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
  
  // Load smaller font for button
  buttonFont = TTF_OpenFont("/System/Library/Fonts/SFNSDisplay.ttf", BUTTON_FONT_SIZE);
  if (!buttonFont) {
    buttonFont = TTF_OpenFont("/System/Library/Fonts/Helvetica.ttc", BUTTON_FONT_SIZE);
  }
  if (!buttonFont) {
    buttonFont = TTF_OpenFont("/Library/Fonts/Arial.ttf", BUTTON_FONT_SIZE);
  }
  if (!buttonFont) {
    // Fall back to main font if button font can't be loaded
    buttonFont = font;
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

  // Initialize toggle button
  initToggleButton();

  // For Mac, set up global key capture
  #ifdef __APPLE__
  setupGlobalKeyCapture();
  #endif

  // Main loop
  bool quit = false;
  SDL_Event e;

  while (!quit) {
    // Process events
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      } else if (e.type == SDL_USEREVENT && e.user.code == 1) {
        // Handle key press from global event monitor
        const char* keyName = (const char*)e.user.data1;
        processKeyPress(keyName);
        free(e.user.data1);  // Free the allocated string
      } else if (e.type == SDL_MOUSEMOTION) {
        // Check if mouse is hovering over the button
        int mouseX = e.motion.x;
        int mouseY = e.motion.y;
        toggleButton.hovered = isPointInButton(mouseX, mouseY, &toggleButton);
      } else if (e.type == SDL_MOUSEBUTTONDOWN) {
        // Check if button is clicked
        int mouseX = e.button.x;
        int mouseY = e.button.y;
        if (isPointInButton(mouseX, mouseY, &toggleButton)) {
          toggleButton.pressed = true;
        }
      } else if (e.type == SDL_MOUSEBUTTONUP) {
        // Check if button is released
        int mouseX = e.button.x;
        int mouseY = e.button.y;
        if (toggleButton.pressed && isPointInButton(mouseX, mouseY, &toggleButton)) {
          // Toggle alignment
          rightAligned = !rightAligned;
          
          // Clear all keys when toggling to start fresh
          for (int i = 0; i < MAX_KEYS; i++) {
            keyDisplays[i].active = false;
          }
          activeKeyCount = 0;
          currentLineWidth = 0;
        }
        toggleButton.pressed = false;
      }
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
          
          SDL_Rect bgRect;
          if (rightAligned) {
            // Right-aligned background
            bgRect.x = WINDOW_WIDTH - RIGHT_MARGIN - totalWidth - 4;
            bgRect.y = y - 4;
            bgRect.w = totalWidth + 8;
            bgRect.h = maxHeight + 8;
          } else {
            // Left-aligned background
            bgRect.x = LEFT_MARGIN - 4;
            bgRect.y = y - 4;
            bgRect.w = totalWidth + 8;
            bgRect.h = maxHeight + 8;
          }
          
          SDL_RenderFillRect(renderer, &bgRect);
        }

        // Render all keys based on alignment
        if (rightAligned) {
          // Right-to-left rendering
          int currentX = WINDOW_WIDTH - RIGHT_MARGIN;
          
          // Render keys in reverse order for right-to-left
          for (int i = MAX_KEYS - 1; i >= 0; i--) {
            if (keyDisplays[i].active) {
              // Create a surface for the text
              SDL_Surface *textSurface =
                  TTF_RenderText_Blended(font, keyDisplays[i].text, textColor);
              if (textSurface) {
                // Position text right-aligned
                currentX -= textSurface->w;
                
                // Create texture from surface
                SDL_Texture *textTexture =
                    SDL_CreateTextureFromSurface(renderer, textSurface);

                // Set the alpha for fading text
                SDL_SetTextureAlphaMod(textTexture, alpha);

                // Render the text
                SDL_Rect renderRect = {currentX, y, textSurface->w,
                                       textSurface->h};

                SDL_RenderCopy(renderer, textTexture, NULL, &renderRect);

                // Update X position for next key (move left)
                currentX -= KEY_GAP;

                // Clean up
                SDL_FreeSurface(textSurface);
                SDL_DestroyTexture(textTexture);
              }
            }
          }
        } else {
          // Left-to-right rendering (original behavior)
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
    }

    // Draw the toggle button with the smaller font
    drawButton(renderer, buttonFont, &toggleButton);

    // Update the screen
    SDL_RenderPresent(renderer);

    // Small delay to reduce CPU usage
    SDL_Delay(16);
  }

  // Clean up
  TTF_CloseFont(font);
  if (buttonFont != font) {
    TTF_CloseFont(buttonFont);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_Quit();
  SDL_Quit();

  return 0;
}

