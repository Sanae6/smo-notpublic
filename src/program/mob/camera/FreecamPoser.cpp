#include "FreecamPoser.hpp"

#include <al/Library/Camera/CameraPoserFunction.hpp>
#include <al/Library/Controller/JoyPadUtil.h>
#include <al/Library/Math/MathLengthUtil.hpp>
#include <al/Library/Math/MathUtil.h>
#include <al/Library/Math/MathVectorUtil.h>
#include <game/Player/PlayerActorBase.h>
#include <logger/Logger.hpp>
#include <mob/MonsterState.hpp>

PoserFreecam::PoserFreecam(const char* poserName) : CameraPoser(poserName) { this->initOrthoProjectionParam(); }

void PoserFreecam::init(void) {
  alCameraPoserFunction::initSnapShotCameraCtrlZoomRollMove(this);
  alCameraPoserFunction::initCameraVerticalAbsorber(this);
  alCameraPoserFunction::initCameraAngleCtrl(this);
}

void PoserFreecam::loadParam(const al::ByamlIter&) {}

void PoserFreecam::start(al::CameraStartInfo const&) {
  Logger::log("Starting FRaceFreecam");

  sead::Vector3f faceDir;
  sead::Vector3f targetFront;

  if (alCameraPoserFunction::isSceneCameraFirstCalc(this)) {
    alCameraPoserFunction::calcTargetTrans(&mTargetTrans, this);
    targetFront = sead::Vector3f(0, 0, 0);
    alCameraPoserFunction::calcTargetFront(&targetFront, this);

    faceDir = mTargetTrans - targetFront;

  } else {
    sead::LookAtCamera* curLookCam = alCameraPoserFunction::getLookAtCamera(this);
    sead::Vector3f curPos = curLookCam->getPos();
    sead::Vector3f curAt = curLookCam->getAt();
    targetFront = sead::Vector3f(curPos.x - curAt.x, 0.0, curPos.z - curAt.z);
    al::tryNormalizeOrDirZ(&targetFront);
    faceDir = targetFront + mTargetTrans;
  }

  mPosition = faceDir;
  mTargetFovy = mDefaultFovy = mFovyDegree;

  PlayerActorBase* player = mob::MonsterState::instance()->player;
  if (player) {
    mPosition = player->mPoseKeeper->mTrans - (faceDir * 500);
    mPosition.y += 300.f;
    mTargetTrans = player->mPoseKeeper->mTrans;
  }
}

void PoserFreecam::update() {
  if (al::isPadHoldY(-1))
    mTargetFovy += 1.0f / 3.0f;
  else if (al::isPadHoldX(-1))
    mTargetFovy -= 1.0f / 3.0f;

  mTargetFovy = al::clamp(mTargetFovy, 15.0f, mDefaultFovy);
  mFovyDegree = al::lerpValue(mFovyDegree, mTargetFovy, 0.15f);

  // Update movement speed data
  calcSpeed();

  // calculates the targets direction through only the X and Z axis
  sead::Vector3f targetDir = sead::Vector3f(mPosition.x - mTargetTrans.x, 0.0f, mPosition.z - mTargetTrans.z);
  al::tryNormalizeOrDirZ(&targetDir);

  sead::Vector3f rotatedVec;
  sead::Vector3f rightVec;

  calcRotVec(targetDir, &rotatedVec, &rightVec);
  movementNoCenter(&rotatedVec, &rightVec);
}

void PoserFreecam::movement() { al::CameraPoser::movement(); }

void PoserFreecam::calcSpeed() {
  if (!al::isPadHoldZR(-1) || mIsPrecise) {
    if (al::isPadTriggerZL(-1)) {
      mSpeed = mPerciseSpeed;
      mIsPrecise = true;
    }

    if (al::isPadReleaseZL(-1)) {
      mSpeed = mDefaultSpeed;
      mIsPrecise = false;
    }
  }

  if (!al::isPadHoldZL(-1) || mSpeed.x > mDefaultSpeed.x) {
    if (al::isPadHoldZR(-1)) {
      mSpeed.x += 1.f;
      mSpeed.y += 0.5f;

      mSpeed.x = al::clamp(mSpeed.x, mDefaultSpeed.x, mMaxSpeed.x);
      mSpeed.y = al::clamp(mSpeed.y, mDefaultSpeed.y, mMaxSpeed.y);
    }

    if (al::isPadReleaseZR(-1)) {
      mSpeed = mDefaultSpeed;
    }
  }
}

void PoserFreecam::calcRotVec(sead::Vector3f targetDir, sead::Vector3f* rotatedVec, sead::Vector3f* rightVec) {
  sead::Vector2f playerRInput(0, 0);
  alCameraPoserFunction::calcCameraRotateStick(&playerRInput, this);

  playerRInput.x = al::lerpValue(mLastRInput.x, playerRInput.x, 0.15);
  playerRInput.y = al::lerpValue(mLastRInput.y, playerRInput.y, 0.15);
  mLastRInput = playerRInput;

  if (mIsPrecise)
    playerRInput /= 12.f;

  // rotates target direction by the cameras X input
  if (mIsPrecise)
    al::rotateVectorDegreeY(
        &targetDir, (playerRInput.x > 0.0f ? playerRInput.x : -playerRInput.x) < 0.01f ? 0.0f : playerRInput.x * -2.0f);
  else
    al::rotateVectorDegreeY(
        &targetDir, (playerRInput.x > 0.0f ? playerRInput.x : -playerRInput.x) < 0.01f ? 0.0f : playerRInput.x * -2.0f);

  mAngle += playerRInput.y * -2.0f;
  mAngle = al::clamp(mAngle, -89.0f, 89.0f);

  *rotatedVec = targetDir;

  // calculates cross product of target direction and cameras current up direction
  sead::Vector3f crossVec;
  crossVec.setCross(targetDir, mCameraUp);
  // rotates target direction by the cross product and vertical angle
  al::rotateVectorDegree(rotatedVec, *rotatedVec, crossVec, mAngle);

  // Calculate the camera's right
  *rightVec = *rotatedVec;
  al::rotateVectorDegreeY(rightVec, 90.f);
}

void PoserFreecam::movementNoCenter(sead::Vector3f* rotatedVec, sead::Vector3f* rightVec) {
  // Move target trans based on input and rotation
  auto& playerLInput = al::getLeftStick(-1);
  float tempTransY = mTargetTrans.y; // Temp store the old y position of the camera to restore at the end

  mTargetTrans += (*rightVec * playerLInput.x) * mSpeed.x;    // Move camera right-left based on stick
  mTargetTrans += (*rotatedVec * -playerLInput.y) * mSpeed.x; // Move camera forward-backward based on stick

  mTargetTrans.y = tempTransY; // Restore vertical position of target

  // Calculate vertical position target
  mTargetTrans.y += al::isPadHoldR(-1) * mSpeed.y;
  mTargetTrans.y -= al::isPadHoldL(-1) * mSpeed.y;

  mPosition = mTargetTrans + *rotatedVec;
}