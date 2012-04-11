/*      KB.H
 *
 * Keyboard scan codes
 *
 * PUBLIC DOMAIN by Petteri Kangaslampi 1994
*/

/****************************************************************************\
*       Keyboard hardware scan codes: (make codes)
*       ------------------------------------------
* Note! I have not even proofread these. They _should_ be correct, but if
* something seems strange, don't trust them.
\****************************************************************************/

enum kbScanCodes
{
    kbBackspace = 0x0E,
    kbCapsLock = 0x3A,
    kbEnter = 0x1C,
    kbEsc = 0x01,
    kbAlt = 0x38,
    kbCtrl = 0x1D,
    kbLeftShift = 0x2A,
    kbNumLock = 0x45,
    kbRightShift = 0x36,
    kbScrollLock = 0x46,
    kbSpace = 0x39,
    kbSysReq = 0x54,
    kbTab = 0x0F,
    kbF1 = 0x3B,
    kbF2 = 0x3C,
    kbF3 = 0x3D,
    kbF4 = 0x3E,
    kbF5 = 0x3F,
    kbF6 = 0x40,
    kbF7 = 0x41,
    kbF8 = 0x42,
    kbF9 = 0x43,
    kbF10 = 0x44,
    kbF11 = 0x57,
    kbF12 = 0x58,
    kbIns = 0x52,
    kbEnd = 0x4F,
    kbDownArrow = 0x50,
    kbPageDown = 0x51,
    kbLeftArrow = 0x4B,
    kbKeypad5 = 0x4C,
    kbRightArrow = 0x4D,
    kbHome = 0x47,
    kbUpArrow = 0x48,
    kbPageUp = 0x49,
    kbDel = 0x53,
    kbKeypadAsterix = 0x37,
    kbKeypadMinus = 0x4A,
    kbKeypadPlus = 0x4E,
    kbA = 0x1E,
    kbB = 0x30,
    kbC = 0x2E,
    kbD = 0x20,
    kbE = 0x12,
    kbF = 0x21,
    kbG = 0x22,
    kbH = 0x23,
    kbI = 0x17,
    kbJ = 0x24,
    kbK = 0x25,
    kbL = 0x26,
    kbM = 0x32,
    kbN = 0x31,
    kbO = 0x18,
    kbP = 0x19,
    kbQ = 0x10,
    kbR = 0x13,
    kbS = 0x1F,
    kbT = 0x14,
    kbU = 0x16,
    kbV = 0x2F,
    kbW = 0x11,
    kbX = 0x2D,
    kbY = 0x15,
    kbZ = 0x2C,
    kb1 = 0x02,
    kb2 = 0x03,
    kb3 = 0x04,
    kb4 = 0x05,
    kb5 = 0x06,
    kb6 = 0x07,
    kb7 = 0x08,
    kb8 = 0x09,
    kb9 = 0x0A,
    kb0 = 0x0B,
    kbMinus = 0x0C,
    kbEquals = 0x0D,
    kbLeftBracket = 0x1A,
    kbRightBracket = 0x1B,
    kbSemicolon = 0x27,
    kbApostrophe = 0x28,
    kbGrave = 0x29,
    kbBackslash = 0x2B,
    kbComma = 0x33,
    kbPeriod = 0x34,
    kbSlash = 0x35
};
/*
#ifndef VK_A
#define VK_A 65
#define VK_B 66
#define VK_C 67
#define VK_D 68
#define VK_E 69
#define VK_F 70
#define VK_G 71
#define VK_H 72
#define VK_I 73
#define VK_J 74
#define VK_K 75
#define VK_L 76
#define VK_M 77
#define VK_N 78
#define VK_O 79
#define VK_P 80
#define VK_Q 81
#define VK_R 82
#define VK_S 83
#define VK_T 84
#define VK_U 85
#define VK_V 86
#define VK_W 87
#define VK_X 88
#define VK_Y 89
#define VK_Z 90
#endif*/

