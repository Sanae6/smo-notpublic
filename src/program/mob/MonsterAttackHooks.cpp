#include "MonsterAttackHooks.hpp"
#include <rs/util/SensorUtil.h>

bool mob::sendMsgEnemyAttack(al::HitSensor* l, al::HitSensor* r) {
  return rs::sendMsgHosuiAttack(l, r) || rs::sendMsgEnemyAttackStrong(l, r) || rs::sendMsgHackAttack(l, r);
}