#pragma once



class PlayerInput {
    public:
        bool isMove(void) const;
        bool isMoveDeepDown(void) const;
        bool isMoveDeepDownNoSnap(void) const;
        bool isNoInput(void) const;
        void calcMoveInput(sead::Vector3f* outVec, const sead::Vector3f& gravity) const;
};
