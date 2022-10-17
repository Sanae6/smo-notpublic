#include "MouseCameraInput.hpp"
#include "al/util.hpp"
#include "logger/Logger.hpp"
#include "nn/hid.h"
namespace al {
    struct SimpleCameraInput : public ICameraInput {
        void* internal;
        SimpleCameraInput(int);
    };
}

MouseCameraInput::MouseCameraInput(const al::ICameraInput* input) {
    passthroughInput = input == nullptr ? new al::SimpleCameraInput(-1) : input;
}

void MouseCameraInput::calcInputStick(sead::Vector2f* out) const {
    nn::hid::MouseState state{};
    nn::hid::GetMouseState(&state);
    *out = sead::Vector2f((float) state.deltaX / 4.0f, (float) -state.deltaY / 4.0f);
}

bool MouseCameraInput::isTriggerReset() const { return false; }

bool MouseCameraInput::isHoldZoom() const { return passthroughInput->isHoldZoom(); }

bool MouseCameraInput::tryCalcSnapShotMoveStick(sead::Vector2f* out) const {
    this->calcInputStick(out);
    return !al::isNearZero(out->x, out->y);
}

bool MouseCameraInput::isHoldSnapShotZoomIn() const { return passthroughInput->isHoldSnapShotZoomIn(); }

bool MouseCameraInput::isHoldSnapShotZoomOut() const { return passthroughInput->isHoldSnapShotZoomOut(); }

bool MouseCameraInput::isHoldSnapShotRollLeft() const { return passthroughInput->isHoldSnapShotRollLeft(); }

bool MouseCameraInput::isHoldSnapShotRollRight() const { return passthroughInput->isHoldSnapShotRollRight(); }

void MouseCameraInput::calcGyroPose(sead::Vector3f* one, sead::Vector3f* two, sead::Vector3f* three) const {
    passthroughInput->calcGyroPose(one, two, three);
}