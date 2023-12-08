#include "slippievents.h"

PreFrameData::PreFrameData(const QByteArray &data) {

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::ByteOrder::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);

    stream >> frameNumber >> playerIndex >> isFollower >> randomSeed >> actionStateId
        >> posX >> posY >> facingDirection >> joyStickX >> joyStickY >> cstickX >> cstickY >> triggerValue
        >> processedButtonsData >> physicalButtonsData
        >> physicalLTrigger >> physicalRTrigger
        >> ucfX >> percent >> ucfY;

    isEmpty = false;
}

PostFrameData::PostFrameData(const QByteArray &data) {

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::ByteOrder::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::FloatingPointPrecision::SinglePrecision);

    stream >> frameNumber >> playerIndex >> isFollower >> charId >> actionStateId
        >> posX >> posY >> facingDirection >> percent >> shieldSize
        >> lastHitAttackId >> comboCount >> lastHitBy >> stocks >> actionStateFrameCounter;

    // read directly into bit fields
    stream.readRawData(&bitFlagStart, 5);

    stream >> actionStateData >> airborne >> lastGroundId
        >> jumpsRemaining >> lCancelStatus >> hurtboxCollisionState
        >> xSpeedSelfAir >> ySpeedSelf >> xSpeedAttack >> ySpeedAttack >> xSpeedSelfGround
        >> hitlagFrameRemaining >> animationIndex;

    isEmpty = false;
}
