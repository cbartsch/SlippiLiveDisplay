#ifndef SLIPPIEVENTS_H
#define SLIPPIEVENTS_H

#include <QObject>

// from: https://github.com/project-slippi/slippi-wiki/blob/master/SPEC.md#pre-frame-update
struct PreFrameData {
    PreFrameData() = default;
    PreFrameData(const PreFrameData &other) = default;
    PreFrameData &operator=(const PreFrameData &other) = default;

    PreFrameData(const QByteArray &data);

    bool isEmpty = true;

    qint32 frameNumber;
    quint8 playerIndex;
    bool isFollower;
    quint32 randomSeed;
    quint16 actionStateId;
    float posX, posY, facingDirection,
        joyStickX, joyStickY, cstickX, cstickY, triggerValue;

    struct Buttons {
        bool dpadLeft : 1, dpadRight : 1, dpadDown: 1, dpadUp: 1;
        bool z : 1, r : 1, l : 1;
        bool unused : 1;
        bool a : 1, b : 1, x : 1, y : 1;
        bool start : 1;
        quint8 unused2 : 3;
    };

    struct ProcessedButtons {
        Buttons physicalButtons;
        bool joyStickUp : 1, joyStickDown : 1, joyStickLeft: 1, joyStickRight: 1;
        bool cStickUp : 1, cStickDown : 1, cStickLeft: 1, cStickRight: 1;
        quint8 unused : 7;
        bool anyTrigger : 1;
    };

    union { ProcessedButtons processedButtons; quint32 processedButtonsData; };
    union { Buttons physicalButtons;           quint16 physicalButtonsData;  };

    float physicalLTrigger, physicalRTrigger;
    qint8 ucfX;
    float percent;
    qint8 ucfY;
};

// from: https://github.com/project-slippi/slippi-wiki/blob/master/SPEC.md#post-frame-update
struct PostFrameData {
    PostFrameData() = default;
    PostFrameData(const PostFrameData &other) = default;
    PostFrameData &operator=(const PostFrameData &other) = default;

    PostFrameData(const QByteArray &data);

    bool isEmpty = true;

    qint32 frameNumber;
    quint8 playerIndex;
    bool isFollower;
    quint8 charId;
    quint16 actionStateId;
    float posX, posY, facingDirection, percent, shieldSize;
    quint8 lastHitAttackId, comboCount, lastHitBy, stocks;
    float actionStateFrameCounter;

    union {
        char bitFlagStart;
        struct {
            quint8 : 4; bool isReflectActive : 1;  quint8 : 3;
            quint8 : 2; bool hasIntangibility : 1; bool isFastFalling : 1;    quint8 : 1; bool isInHitlag: 1;          quint8 : 2;
            quint8 : 7; bool isShieldActive : 1;
            quint8 : 2; bool isInHitStun : 1;      bool isTouchingShield : 1; quint8 : 2; bool isPowershieldActive : 1; quint8 : 2;
            quint8 : 3; bool isFollowerBit : 1;    bool isSleeping: 1;        quint8 : 1; bool isDead: 1;               bool isOffscreen : 1;
        };
    };

    float actionStateData; // eg. hitstun remaining
    bool airborne;
    quint16 lastGroundId;
    quint8 jumpsRemaining, lCancelStatus, hurtboxCollisionState;
    float xSpeedSelfAir, ySpeedSelf, xSpeedAttack, ySpeedAttack, xSpeedSelfGround, hitlagFrameRemaining;
    quint32 animationIndex;
};

#endif // SLIPPIEVENTS_H
