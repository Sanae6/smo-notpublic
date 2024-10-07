#pragma once
#include <al/Library/Camera/CameraPoser.hpp>

typedef int PoserFreecamCubeMapMode_;

class PoserFreecam : public al::CameraPoser {
  public:
  PoserFreecam(char const*);
  virtual void loadParam(al::ByamlIter const&) override;
  virtual void start(al::CameraStartInfo const&) override;
  virtual void init() override;
  void reset();
  virtual void movement() override;
  virtual void update() override;

  void calcSpeed();
  void calcFovy();

  void calcRotVec(sead::Vector3f targetDir, sead::Vector3f* rotatedVec, sead::Vector3f* rightVec);
  void movementNoCenter(sead::Vector3f* rotatedVec, sead::Vector3f* rightVec);

  float mAngle = 20.f;
  float mDefaultFovy;
  float mTargetFovy;

  bool mIsPrecise = false;
  const sead::Vector2f mDefaultSpeed = {50.f, 20.f};
  const sead::Vector2f mPerciseSpeed = {3.f, 1.5f};
  const sead::Vector2f mMaxSpeed = {1200.f, 400.f};

  sead::Vector2f mSpeed = mDefaultSpeed;
  sead::Vector2f mLastRInput = sead::Vector2f::zero;
};