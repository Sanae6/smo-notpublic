#pragma once

#include "al/camera/ICameraInput.h"

class MouseCameraInput : public al::ICameraInput
{
public:
    MouseCameraInput(const al::ICameraInput* input);

    void calcInputStick(sead::Vector2f*) const override;
    bool isTriggerReset() const override;
    bool isHoldZoom() const override;
    bool tryCalcSnapShotMoveStick(sead::Vector2f*) const override;
    bool isHoldSnapShotZoomIn() const override;
    bool isHoldSnapShotZoomOut() const override;
    bool isHoldSnapShotRollLeft() const override;
    bool isHoldSnapShotRollRight() const override;
    void calcGyroPose(sead::Vector3f*, sead::Vector3f*, sead::Vector3f*) const override;

protected:
    const al::ICameraInput* passthroughInput;
};