/**
* @file hid.h
* @brief Functions that help process gamepad inputs.
*/

#pragma once

#include <gfx/seadColor.h>
#include <nx/types.h>
#include <nn/util.h>

namespace nn {
    namespace hid {
        struct NpadHandheldState;
        struct NpadStyleTag;

        struct ControllerSupportArg {
           u8 mMinPlayerCount;
           u8 mMaxPlayerCount;
           u8 mTakeOverConnection;
           bool mLeftJustify;
           bool mPermitJoyconDual;
           bool mSingleMode;
           bool mUseColors;
           sead::Color4u8 mColors[4];
           u8 mUsingControllerNames;
           char mControllerNames[4][0x81];
        };

        struct ControllerSupportResultInfo {
           int mPlayerCount;
           int mSelectedId;
        };

        void InitializeNpad();
        void SetSupportedNpadIdType(u32 const*, u64);
        void SetSupportedNpadStyleSet(nn::util::BitFlagSet<32, nn::hid::NpadStyleTag>);
        void GetNpadStyleSet(u32 const&);
        void GetNpadStates(nn::hid::NpadHandheldState*, s32, u32 const&);
        int ShowControllerSupport(nn::hid::ControllerSupportResultInfo*,
                                        ControllerSupportArg const&);

        enum class MouseButton {
            Left,
            Right,
            Middle,
            Forward,
            Back
        };

        enum class MouseAttribute {
            Transferable,
            IsConnected
        };

        struct MouseState {
            u64 samplingNumber;
            s32 x;
            s32 y;
            s32 deltaX;
            s32 deltaY;
            s32 wheelDeltaX;
            s32 wheelDeltaY;
            nn::util::BitFlagSet<32, MouseButton> buttons;
            nn::util::BitFlagSet<32, MouseAttribute> attributes;
        };

        enum class KeyboardModifier {
            Control,
            Shift,
            LeftAlt,
            RightAlt,
            Gui,
            CapsLock,
            ScrollLock,
            NumLock,
            Katakana,
            Hiragana
        };

        enum class KeyboardKey {
            A = 4,
            B = 5,
            C = 6,
            D = 7,
            E = 8,
            F = 9,
            G = 10,
            H = 11,
            I = 12,
            J = 13,
            K = 14,
            L = 15,
            M = 16,
            N = 17,
            O = 18,
            P = 19,
            Q = 20,
            R = 21,
            S = 22,
            T = 23,
            U = 24,
            V = 25,
            W = 26,
            X = 27,
            Y = 28,
            Z = 29,
            D1 = 30,
            D2 = 31,
            D3 = 32,
            D4 = 33,
            D5 = 34,
            D6 = 35,
            D7 = 36,
            D8 = 37,
            D9 = 38,
            D0 = 39,
            Return = 40,
            Escape = 41,
            Backspace = 42,
            Tab = 43,
            Space = 44,
            Minus = 45,
            Plus = 46,
            OpenBracket = 47,
            CloseBracket = 48,
            Pipe = 49,
            Tilde = 50,
            Semicolon = 51,
            Quote = 52,
            Backquote = 53,
            Comma = 54,
            Period = 55,
            Slash = 56,
            CapsLock = 57,
            F1 = 58,
            F2 = 59,
            F3 = 60,
            F4 = 61,
            F5 = 62,
            F6 = 63,
            F7 = 64,
            F8 = 65,
            F9 = 66,
            F10 = 67,
            F11 = 68,
            F12 = 69,
            PrintScreen = 70,
            ScrollLock = 71,
            Pause = 72,
            Insert = 73,
            Home = 74,
            PageUp = 75,
            Delete = 76,
            End = 77,
            PageDown = 78,
            RightArrow = 79,
            LeftArrow = 80,
            DownArrow = 81,
            UpArrow = 82,
            NumLock = 83,
            NumPadDivide = 84,
            NumPadMultiply = 85,
            NumPadSubtract = 86,
            NumPadAdd = 87,
            NumPadEnter = 88,
            NumPad1 = 89,
            NumPad2 = 90,
            NumPad3 = 91,
            NumPad4 = 92,
            NumPad5 = 93,
            NumPad6 = 94,
            NumPad7 = 95,
            NumPad8 = 96,
            NumPad9 = 97,
            NumPad0 = 98,
            NumPadDot = 99,
            Backslash = 100,
            Application = 101,
            Power = 102,
            NumPadEquals = 103,
            F13 = 104,
            F14 = 105,
            F15 = 106,
            F16 = 107,
            F17 = 108,
            F18 = 109,
            F19 = 110,
            F20 = 111,
            F21 = 112,
            F22 = 113,
            F23 = 114,
            F24 = 115,
            NumPadComma = 133,
            Ro = 135,
            KatakanaHiragana = 136,
            Yen = 137,
            Henkan = 138,
            Muhenkan = 139,
            NumPadCommaPc98 = 140,
            HangulEnglish = 144,
            Hanja = 145,
            Katakana = 146,
            Hiragana = 147,
            ZenkakuHankaku = 148,
            LeftControl = 224,
            LeftShift = 225,
            LeftAlt = 226,
            LeftGui = 227,
            RightControl = 228,
            RightShift = 229,
            RightAlt = 230,
            RightGui = 231,
        };

        struct KeyboardState {
            u64 samplingNumber;
            nn::util::BitFlagSet<32, KeyboardModifier> modifiers;
            nn::util::BitFlagSet<256, KeyboardKey> keys;

            inline bool isKeyDown(KeyboardKey key) const {
                return keys.isBitSet(key);
            }
        };

        void InitializeMouse();
        void GetMouseState(nn::hid::MouseState*);
        void InitializeKeyboard();
        void GetKeyboardState(nn::hid::KeyboardState*);
    }  // namespace hid
}  // namespace nn