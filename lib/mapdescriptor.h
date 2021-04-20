#ifndef MAPDESCRIPTOR_H
#define MAPDESCRIPTOR_H

#include <QDir>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>
#include "yaml-cpp/yaml.h"
#include "fortunestreetdata.h"

enum RuleSet : quint32 {
    Standard = 0,
    Easy = 1
};

struct OriginPoint {
    float x = 0, y = 0;

    bool operator==(const OriginPoint &other) const;
    explicit operator QString() const;
};

enum Character : quint32 {
    CharacterMario      = 0x00,
    Luigi      = 0x01,
    Peach      = 0x02,
    Yoshi      = 0x03,
    Bowser     = 0x04,
    Toad       = 0x05,
    DonkeyKong = 0x06,
    Wario      = 0x07,
    Waluigi    = 0x08,
    Daisy      = 0x09,
    Birdo      = 0x0A,
    DiddyKong  = 0x0B,
    BowserJr   = 0x0C,
    Slime      = 0x0D,
    Princessa  = 0x0E,
    Kiryl      = 0x0F,
    Yangus     = 0x10,
    Angelo     = 0x11,
    Platypunk  = 0x12,
    Bianca     = 0x13,
    Alena      = 0x14,
    Carver     = 0x15,
    Jessica    = 0x16,
    Dragonlord = 0x17,
    Stella     = 0x18,
    Patty      = 0x19
};
QString tourCharacterToString(Character character);
Character stringToTourCharacter(const QString &str);

enum BgmId: quint32 {
    BGM_MAP_CIRCUIT      =  0,
    BGM_MAP_PEACH        =  1,
    BGM_MAP_KOOPA        =  2,
    BGM_MAP_GHOSTSHIP    =  3,
    BGM_MAP_MAJINZOU     =  4,
    BGM_MAP_SINOKAZAN    =  5,
    BGM_MAP_SLABACCA     =  6,
    BGM_MAP_KANDATA      =  7,
    BGM_MAP_KANDATA_old  =  8,
    BGM_MAP_ALEFGARD     =  9,
    BGM_MAP_ALEFGARD_old = 10,
    BGM_MAP_YOSHI        = 11,
    BGM_MAP_STADIUM      = 12,
    BGM_MAP_DOLPIC       = 13,
    BGM_MAP_SMB          = 14,
    BGM_MAP_STARSHIP     = 15,
    BGM_MAP_EGG          = 16,
    BGM_MAP_TRODAIN      = 17,
    BGM_MAP_TRODAIN_old  = 18,
    BGM_MAP_DHAMA        = 19,
    BGM_MAP_DHAMA_old    = 20,
    BGM_MAP_ANGEL        = 21,
    BGM_MENU             = 22,
    BGM_GOALPROP         = 23,
    BGM_WINNER           = 24,
    BGM_CHANCECARD       = 25,
    BGM_STOCK            = 26,
    BGM_AUCTION          = 27,
    BGM_CASINO_SLOT      = 28,
    BGM_CASINO_BLOCK     = 29,
    BGM_CASINO_RACE      = 32,
    BGM_TITLE            = 33,
    BGM_SAVELOAD         = 35,
    BGM_SAVELOAD_old     = 36,
    BGM_WIFI             = 37,
    BGM_ENDING           = 39
};
QString bgmIdToString(BgmId bgmId);
BgmId stringToBgmId(const QString &str);

struct MapDescriptor {
    qint8 mapSet = -1;
    qint8 zone = -1;
    qint8 order = -1;
    bool isPracticeBoard = false;
    quint32 unlockId = 0;
    RuleSet ruleSet = Standard;
    quint32 initialCash = 0;
    quint32 targetAmount = 0;
    quint32 baseSalary = 0;
    quint32 salaryIncrement = 0;
    quint32 maxDiceRoll = 0;
    bool ventureCards[128] = {false};
    QString frbFiles[4];
    QVector<OriginPoint> switchRotationOrigins;
    BoardTheme theme = Mario;
    QString background;
    BgmId bgmId = BGM_MAP_CIRCUIT;
    QString mapIcon;
    LoopingMode loopingMode = None;
    float loopingModeRadius = 0;
    float loopingModeHorizontalPadding = 0;
    float loopingModeVerticalSquareCount = 0;
    quint32 tourBankruptcyLimit = 1;
    quint32 tourInitialCash = 0;
    Character tourCharacters[3] = {CharacterMario, Luigi, Peach};
    quint32 tourClearRank = 2;
    quint32 nameMsgId = 0;
    quint32 descMsgId = 0;
    QMap<QString, QString> names;
    QMap<QString, QString> descs;
    QString internalName;
    QString mapDescriptorFilePath;

    QSet<SquareType> readFrbFileInfo(const QDir &paramDir);

    QString toMd() const;
    bool operator==(const MapDescriptor &other) const;

    // return value: whether this was successful
    bool fromMd(const YAML::Node &yaml);

    MapDescriptor &setFromImport(const MapDescriptor &other);

    friend QDebug &operator<<(QDebug &debugStream, const MapDescriptor &obj);
};

// VALIDATION FUNCTIONS
void getPracticeBoards(const QVector<MapDescriptor> &descriptors, short &easyPracticeBoard, short &standardPracticeBoard, QStringList &errorMsgs);
QMap<int, int> getMapSets(const QVector<MapDescriptor> &descriptors, QStringList &errorMsgs);
QMap<int, int> getMapZones(const QVector<MapDescriptor> &descriptors, int mapSet, QStringList &errorMsgs);
QMap<int, int> getMapOrderings(const QVector<MapDescriptor> &descriptors, int mapSet, int zone, QStringList &errorMsgs);

#endif // MAPDESCRIPTOR_H
