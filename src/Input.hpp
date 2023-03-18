#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <unordered_set>
#include <vector>

enum KeyCode
{
    KeyUnkown = 0,

    KeyReturn = SDLK_RETURN,
    KeyEscape = SDLK_ESCAPE,
    KeyBackspace = SDLK_BACKSPACE,
    KeyTab = SDLK_TAB,
    KeySpace = SDLK_SPACE,
    KeyExclamation = SDLK_EXCLAIM,
    KeyDoubleQuote = SDLK_QUOTEDBL,
    KeyHash = SDLK_HASH,
    KeyPercent = SDLK_PERCENT,
    KeyDollar = SDLK_DOLLAR,
    KeyAmpersand = SDLK_AMPERSAND,
    KeySingleQuote = SDLK_QUOTE,
    KeyLeftParenthesis = SDLK_LEFTPAREN,
    KeyRightParenthesis = SDLK_RIGHTPAREN,
    KeyAsterisk = SDLK_ASTERISK,
    KeyPlus = SDLK_PLUS,
    KeyComma = SDLK_COMMA,
    KeyMinus = SDLK_MINUS,
    KeyPeriod = SDLK_PERIOD,
    KeySlash = SDLK_SLASH,
    Key0 = SDLK_0,
    Key1 = SDLK_1,
    Key2 = SDLK_2,
    Key3 = SDLK_3,
    Key4 = SDLK_4,
    Key5 = SDLK_5,
    Key6 = SDLK_6,
    Key7 = SDLK_7,
    Key8 = SDLK_8,
    Key9 = SDLK_9,
    KeyColon = SDLK_COLON,
    KeySemicolon = SDLK_SEMICOLON,
    KeyLess = SDLK_LESS,
    KeyEquals = SDLK_EQUALS,
    KeyGreater = SDLK_GREATER,
    KeyQuestion = SDLK_QUESTION,
    KeyAt = SDLK_AT,

    KeyLeftBracket = SDLK_LEFTBRACKET,
    KeyBackslash = SDLK_BACKSLASH,
    KeyRightBracket = SDLK_RIGHTBRACKET,
    KeyCaret = SDLK_CARET,
    KeyUnderscore = SDLK_UNDERSCORE,
    KeyBackquote = SDLK_BACKQUOTE,
    KeyA = SDLK_a,
    KeyB = SDLK_b,
    KeyC = SDLK_c,
    KeyD = SDLK_d,
    KeyE = SDLK_e,
    KeyF = SDLK_f,
    KeyG = SDLK_g,
    KeyH = SDLK_h,
    KeyI = SDLK_i,
    KeyJ = SDLK_j,
    KeyK = SDLK_k,
    KeyL = SDLK_l,
    KeyM = SDLK_m,
    KeyN = SDLK_n,
    KeyO = SDLK_o,
    KeyP = SDLK_p,
    KeyQ = SDLK_q,
    KeyR = SDLK_r,
    KeyS = SDLK_s,
    KeyT = SDLK_t,
    KeyU = SDLK_u,
    KeyV = SDLK_v,
    KeyW = SDLK_w,
    KeyX = SDLK_x,
    KeyY = SDLK_y,
    KeyZ = SDLK_z,

    KeyCapsLock = SDLK_CAPSLOCK,

    KeyF1 = SDLK_F1,
    KeyF2 = SDLK_F2,
    KeyF3 = SDLK_F3,
    KeyF4 = SDLK_F4,
    KeyF5 = SDLK_F5,
    KeyF6 = SDLK_F6,
    KeyF7 = SDLK_F7,
    KeyF8 = SDLK_F8,
    KeyF9 = SDLK_F9,
    KeyF10 = SDLK_F10,
    KeyF11 = SDLK_F11,
    KeyF12 = SDLK_F12,

    KeyPrintScreen = SDLK_PRINTSCREEN,
    KeyScrollLock = SDLK_SCROLLLOCK,
    KeyPause = SDLK_PAUSE,
    KeyInsert = SDLK_INSERT,
    KeyHome = SDLK_HOME,
    KeyPageUp = SDLK_PAGEUP,
    KeyDelete = SDLK_DELETE,
    KeyEnd = SDLK_END,
    KeyPageDown = SDLK_PAGEDOWN,
    KeyRight = SDLK_RIGHT,
    KeyLeft = SDLK_LEFT,
    KeyDown = SDLK_DOWN,
    KeyUp = SDLK_UP,

    KeyNumLockClear = SDLK_NUMLOCKCLEAR,
    KeyDivide = SDLK_KP_DIVIDE,
    KeyMultiply = SDLK_KP_MULTIPLY,
    KeyKeypadMinus = SDLK_KP_MINUS,
    KeyKeypadPlus = SDLK_KP_PLUS,
    KeyEnter = SDLK_KP_ENTER,
    KeyKeypad1 = SDLK_KP_1,
    KeyKeypad2 = SDLK_KP_2,
    KeyKeypad3 = SDLK_KP_3,
    KeyKeypad4 = SDLK_KP_4,
    KeyKeypad5 = SDLK_KP_5,
    KeyKeypad6 = SDLK_KP_6,
    KeyKeypad7 = SDLK_KP_7,
    KeyKeypad8 = SDLK_KP_8,
    KeyKeypad9 = SDLK_KP_9,
    KeyKeypad0 = SDLK_KP_0,
    KeyKeypadPeriod = SDLK_KP_PERIOD,

    KeyApplication = SDLK_APPLICATION,
    KeyPower = SDLK_POWER,
    KeyKeypadEquals = SDLK_KP_EQUALS,
    KeyF13 = SDLK_F13,
    KeyF14 = SDLK_F14,
    KeyF15 = SDLK_F15,
    KeyF16 = SDLK_F16,
    KeyF17 = SDLK_F17,
    KeyF18 = SDLK_F18,
    KeyF19 = SDLK_F19,
    KeyF20 = SDLK_F20,
    KeyF21 = SDLK_F21,
    KeyF22 = SDLK_F22,
    KeyF23 = SDLK_F23,
    KeyF24 = SDLK_F24,
    KeyExecute = SDLK_EXECUTE,
    KeyHelp = SDLK_HELP,
    KeyMenu = SDLK_MENU,
    KeySelect = SDLK_SELECT,
    KeyStop = SDLK_STOP,
    KeyAgain = SDLK_AGAIN,
    KeyUndo = SDLK_UNDO,
    KeyCut = SDLK_CUT,
    KeyCopy = SDLK_COPY,
    KeyPaste = SDLK_PASTE,
    KeyFind = SDLK_FIND,
    KeyMute = SDLK_MUTE,
    KeyVolumeUp = SDLK_VOLUMEUP,
    KeyVolumeDown = SDLK_VOLUMEDOWN,
    KeyKeypadComma = SDLK_KP_COMMA,
    KeyKeypadEqualsAs400 = SDLK_KP_EQUALSAS400,

    KeyAltErase = SDLK_ALTERASE,
    KeySysReq = SDLK_SYSREQ,
    KeyCancel = SDLK_CANCEL,
    KeyClear = SDLK_CLEAR,
    KeyPrior = SDLK_PRIOR,
    KeyReturn2 = SDLK_RETURN2,
    KeySeperator = SDLK_SEPARATOR,
    KeyOut = SDLK_OUT,
    KeyOper = SDLK_OPER,
    KeyClearAgain = SDLK_CLEARAGAIN,
    KeyCrSel = SDLK_CRSEL,
    KeyExSel = SDLK_EXSEL,

    KeyKeypad00 = SDLK_KP_00,
    KeyKeypad000 = SDLK_KP_000,
    KeyThousandsSeperator = SDLK_THOUSANDSSEPARATOR,
    KeyDecimalSeperator = SDLK_DECIMALSEPARATOR,
    KeyCurrencyUnit = SDLK_CURRENCYUNIT,
    KeyCurrencySubUnit = SDLK_CURRENCYSUBUNIT,
    KeyKeypadLeftParenthesis = SDLK_KP_LEFTPAREN,
    KeyKeypadRightParenthesis = SDLK_KP_RIGHTPAREN,
    KeyKeypadLeftBrace = SDLK_KP_LEFTBRACE,
    KeyKeypadRightBrace = SDLK_KP_RIGHTBRACE,
    KeyKeypadTab = SDLK_KP_TAB,
    KeyKeypadBackspace = SDLK_KP_BACKSPACE,
    KeyKeypadA = SDLK_KP_A,
    KeyKeypadB = SDLK_KP_B,
    KeyKeypadC = SDLK_KP_C,
    KeyKeypadD = SDLK_KP_D,
    KeyKeypadE = SDLK_KP_E,
    KeyKeypadF = SDLK_KP_F,
    KeyKeypadXor = SDLK_KP_XOR,
    KeyKeypadPower = SDLK_KP_POWER,
    KeyKeypadPercent = SDLK_KP_PERCENT,
    KeyKeypadLess = SDLK_KP_LESS,
    KeyKeypadGreater = SDLK_KP_GREATER,
    KeyKeypadAmpersand = SDLK_KP_AMPERSAND,
    KeyKeypadDoubleAmpersand = SDLK_KP_DBLAMPERSAND,
    KeyKeypadVerticalBar = SDLK_KP_VERTICALBAR,
    KeyKeypadDoubleVerticalBar = SDLK_KP_DBLVERTICALBAR,
    KeyKeypadColon = SDLK_KP_COLON,
};

enum MouseButton {
    MouseButtonLeft = SDL_BUTTON_LEFT,
    MouseButtonMiddle = SDL_BUTTON_MIDDLE,
    MouseButtonRight = SDL_BUTTON_RIGHT,
    MouseButtonX1 = SDL_BUTTON_X1,
    MouseButtonX2 = SDL_BUTTON_X2,
};

class Input
{
public:
    void UpdateStateKeyDown(KeyCode keyCode);
    void UpdateStateKeyUp(KeyCode keyCode);
    void Update();

    bool IsKeyHeld(KeyCode keyCode);
    bool WasKeyPressed(KeyCode keyCode);
    bool WasKeyReleased(KeyCode keyCode);

    const std::vector<KeyCode> &GetPressedKeys();

    int32_t GetMouseX();
    int32_t GetMouseY();

    void UpdateStateMouseDown(MouseButton mouseButton);
    void UpdateStateMouseUp(MouseButton mouseButton);

    bool IsMouseButtonHeld(MouseButton mouseButton);
    bool WasMouseButtonPressed(MouseButton mouseButton);
    bool WasMouseButtonReleased(MouseButton mouseButton);

  private:
    std::unordered_set<KeyCode> heldKeys;
    std::unordered_set<KeyCode> pressedKeys;
    std::unordered_set<KeyCode> releasedKeys;
    std::vector<KeyCode> allPressedKeys;

    std::unordered_set<MouseButton> heldMouseButtons;
    std::unordered_set<MouseButton> pressedMouseButtons;
    std::unordered_set<MouseButton> releasedMouseButtons;
};