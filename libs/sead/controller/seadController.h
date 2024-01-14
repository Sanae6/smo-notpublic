#pragma once

#include "container/seadOffsetList.h"
#include "controller/seadControllerBase.h"

namespace sead {
    class ControllerMgr;

    class ControllerAddon;

    namespace ControllerDefine {
        enum AddonId : int {
        };
        enum ControllerId : int {
            cController_FullKey = 0,
            cController_Handheld = 1,
            cController_JoyDual = 2,
            cController_JoyLeft = 3,
            cController_JoyRight = 4,
            cController_Unknown = 5,
            _15 = 15,
            _16 = 16
        };
        enum DeviceId : int {
        };

    }  // namespace ControllerDefine

    enum class PadIdx
    {
        A = 0,
        B = 1,
        C = 2,
        X = 3,
        Y = 4,
        Z = 5,
        RightStick = 6,  // Also Right-Stick Click
        LeftStick = 7,  // Also Left-Stick Click
        Home = 8,
        Minus = 9,
        Plus = 10,
        Start = 11,
        Select = 12,
        ZL = C,
        ZR = Z,
        L = 13,
        R = 14,
        Touch = 15,
        Up = 16,
        Down = 17,
        Left = 18,
        Right = 19,
        LeftStickUp = 20,
        LeftStickDown = 21,
        LeftStickLeft = 22,
        LeftStickRight = 23,
        RightStickUp = 24,
        RightStickDown = 25,
        RightStickLeft = 26,
        RightStickRight = 27,
        Max = 28
    };

    class Controller : public ControllerBase {
    SEAD_RTTI_OVERRIDE(Controller, ControllerBase)

    public:
        Controller(ControllerMgr *);

        virtual ~Controller();

        virtual void calc();

        virtual bool isConnected();

        ControllerAddon *getAddonByOrder(ControllerDefine::AddonId, int);

        ControllerAddon *getAddon(ControllerDefine::AddonId);

    protected:
        virtual void calcImpl_() = 0;

        virtual bool isIdle_();

        virtual void setIdle_();

    private:
        int mControllerId;
        ControllerMgr *mMgr;
        OffsetList<ControllerAddon> mAddonList;
        OffsetList<void *> _160;  // unknown type
    };

}  // namespace sead
