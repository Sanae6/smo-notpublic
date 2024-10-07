#pragma once

#include <math/seadBoundBox.h>
#include <math/seadMatrix.h>
#include <math/seadQuat.h>
#include <math/seadVector.h>

namespace al {
  void verticalizeVec(sead::Vector3f*, const sead::Vector3f&, const sead::Vector3f&);
  void parallelizeVec(sead::Vector3f*, const sead::Vector3f&, const sead::Vector3f&);
  template <typename T>
  inline T clamp(T value, T min, T max) {
    if (value < min)
      return min;
    if (value > max)
      return max;
    return value;
  }
  inline f32 lerpValue(f32 x, f32 y, f32 time) {
    if (time < 0.0f)
      time = 0.0f;
    else if (time > 1.0f)
      time = 1.0f;
    return (x * (1.0f - time)) + (time * y);
  }
  bool isNear(f32 value, f32 target, f32 tolerance);
  bool isNear(const sead::Vector2f& value, const sead::Vector2f& target, f32 tolerance);
  bool isNear(const sead::Vector3f& value, const sead::Vector3f& target, f32 tolerance);
  bool isNear(const sead::Matrix34f& value, const sead::Matrix34f& target, f32 tolerance);
  bool isNearZero(f32 value, f32 tolerance);
  bool isNearZero(const sead::Vector2f& value, f32 tolerance);
  bool isNearZero(const sead::Vector3f& value, f32 tolerance);
  bool isNearZero(const sead::Matrix34f& value, f32 tolerance);
  bool isNearZeroOrGreater(f32 value, f32 tolerance);
  bool isNearZeroOrLess(f32 value, f32 tolerance);

  f32 modf(f32 a, f32 b);

  void calcQuatSide(sead::Vector3f* out, const sead::Quatf& quat);
  void calcQuatUp(sead::Vector3f* out, const sead::Quatf& quat);
  void calcQuatGravity(sead::Vector3f* out, const sead::Quatf& quat);
  void calcQuatFront(sead::Vector3f* out, const sead::Quatf& quat);

  void makeMtxRotateTrans(sead::Matrix34f* out, const sead::Vector3f& rotate, const sead::Vector3f& trans);
  void makeMtxFrontUpPos(sead::Matrix34f* out, const sead::Vector3f& front, const sead::Vector3f& up,
                         const sead::Vector3f& pos);
  void makeMtxUpFrontPos(sead::Matrix34f* out, const sead::Vector3f& up, const sead::Vector3f& front,
                         const sead::Vector3f& pos);

} // namespace al
