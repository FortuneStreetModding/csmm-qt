#ifndef MUSIC_H
#define MUSIC_H

#include <QString>
#include <QVector>
#include <QMap>

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
static const QMap<QString, BgmId> stringToBgmIds = {
    {"BGM_MAP_CIRCUIT", BGM_MAP_CIRCUIT},
    {"BGM_MAP_PEACH", BGM_MAP_PEACH},
    {"BGM_MAP_KOOPA", BGM_MAP_KOOPA},
    {"BGM_MAP_GHOSTSHIP", BGM_MAP_GHOSTSHIP},
    {"BGM_MAP_MAJINZOU", BGM_MAP_MAJINZOU},
    {"BGM_MAP_SINOKAZAN", BGM_MAP_SINOKAZAN},
    {"BGM_MAP_SLABACCA", BGM_MAP_SLABACCA},
    {"BGM_MAP_KANDATA", BGM_MAP_KANDATA},
    {"BGM_MAP_KANDATA_old", BGM_MAP_KANDATA_old},
    {"BGM_MAP_ALEFGARD", BGM_MAP_ALEFGARD},
    {"BGM_MAP_ALEFGARD_old", BGM_MAP_ALEFGARD_old},
    {"BGM_MAP_YOSHI", BGM_MAP_YOSHI},
    {"BGM_MAP_STADIUM", BGM_MAP_STADIUM},
    {"BGM_MAP_DOLPIC", BGM_MAP_DOLPIC},
    {"BGM_MAP_SMB", BGM_MAP_SMB},
    {"BGM_MAP_STARSHIP", BGM_MAP_STARSHIP},
    {"BGM_MAP_EGG", BGM_MAP_EGG},
    {"BGM_MAP_TRODAIN", BGM_MAP_TRODAIN},
    {"BGM_MAP_TRODAIN_old", BGM_MAP_TRODAIN_old},
    {"BGM_MAP_DHAMA", BGM_MAP_DHAMA},
    {"BGM_MAP_DHAMA_old", BGM_MAP_DHAMA_old},
    {"BGM_MAP_ANGEL", BGM_MAP_ANGEL},
    {"BGM_MENU", BGM_MENU},
    {"BGM_GOALPROP", BGM_GOALPROP},
    {"BGM_WINNER", BGM_WINNER},
    {"BGM_CHANCECARD", BGM_CHANCECARD},
    {"BGM_STOCK", BGM_STOCK},
    {"BGM_AUCTION", BGM_AUCTION},
    {"BGM_CASINO_SLOT", BGM_CASINO_SLOT},
    {"BGM_CASINO_BLOCK", BGM_CASINO_BLOCK},
    {"BGM_CASINO_RACE", BGM_CASINO_RACE},
    {"BGM_TITLE", BGM_TITLE},
    {"BGM_SAVELOAD", BGM_SAVELOAD},
    {"BGM_SAVELOAD_old", BGM_SAVELOAD_old},
    {"BGM_WIFI", BGM_WIFI},
    {"BGM_ENDING", BGM_ENDING}
};

namespace Bgm {
    static QString bgmIdToString(BgmId bgmId) {
        return stringToBgmIds.key(bgmId);
    }
    static BgmId stringToBgmId(const QString &str) {
        return stringToBgmIds.value(str);
    }
}

enum MusicType: quint32 {
    map                    = 0,   // BGM
    stock                  = 26,  // BGM
    ventureCards           = 25,  // BGM
    auction                = 27,  // BGM
    targetMet              = 23,  // BGM
    win                    = 24,  // BGM
    guestAppear            = 105, // ME
    guestLeave             = 106, // ME
    badVentureCard         = 104, // ME
    takeAbreak             = 100, // ME
    promotionMii           = 95,  // ME (played by Mii characters)
    promotionMario         = 96,  // ME (played by Mario characters)
    promotionDragonQuest   = 97,  // ME (played by DragonQuest characters)
    forcedBuyout           = 99,  // ME
    domination             = 98,  // ME
    bankruptcy             = 101, // ME
    // arcade
    roundTheBlocks         = 28,  // BGM
    roundTheBlocksWin      = 108, // ME
    roundTheBlocks777      = 109, // ME
    roundTheBlocksWinMario = 110, // ME (prize win theme but always the Mario theme variant? -> still a mystery where this is used by the game)
    memoryBlock            = 29,  // BGM
    dartOfGold             = 30,  // BGM
    slurpodromeSelect      = 31,  // BGM
    slurpodromeStart       = 102, // ME
    slurpodromeRace        = 32,  // BGM
    slurpodromeWin         = 112  // ME
};

static const QMap<QString, MusicType> stringToMusicTypes = {
    {"map"                 , map},
    {"stock"               , stock},
    {"ventureCards"        , ventureCards},
    {"auction"             , auction},
    {"targetMet"           , targetMet},
    {"win"                 , win},
    {"guestAppear"         , guestAppear},
    {"guestLeave"          , guestLeave},
    {"badVentureCard"      , badVentureCard},
    {"takeAbreak"          , takeAbreak},
    {"promotionMii"        , promotionMii},
    {"promotionMario"      , promotionMario},
    {"promotionDragonQuest", promotionDragonQuest},
    {"forcedBuyout"        , forcedBuyout},
    {"domination"          , domination},
    {"bankruptcy"          , bankruptcy},
    // arcade
    {"roundTheBlocks"      , roundTheBlocks},
    {"roundTheBlocksWin"   , roundTheBlocksWin},
    {"roundTheBlocks777"   , roundTheBlocks777},
    {"memoryBlock"         , memoryBlock},
    {"dartOfGold"          , dartOfGold},
    {"slurpodromeSelect"   , slurpodromeSelect},
    {"slurpodromeStart"    , slurpodromeStart},
    {"slurpodromeRace"     , slurpodromeRace},
    {"slurpodromeWin"      , slurpodromeWin}
};

namespace Music {
    static QString musicTypeToString(MusicType musicType) {
        return stringToMusicTypes.key(musicType);
    }
    static MusicType stringToMusicType(const QString &str) {
        return stringToMusicTypes.value(str);
    }
    static bool isMusicType(const QString &str) {
        return stringToMusicTypes.contains(str);
    }
    static bool musicTypeIsBgm(MusicType musicType) {
        switch(musicType) {
        case map:
        case stock:
        case ventureCards:
        case auction:
        case targetMet:
        case win:
        case roundTheBlocks:
        case memoryBlock:
        case dartOfGold:
        case slurpodromeSelect:
        case slurpodromeRace:
            return true;
        default:
            return false;
        }
    }

}
struct MusicEntry {
    QString brstmBaseFilename;
    quint8 volume = 50;
    qint32 brsarIndex = -1;
    quint32 brstmFileSize = -1;

    bool operator==(const MusicEntry &other) const {
        return brstmBaseFilename == other.brstmBaseFilename &&
               volume            == other.volume;
    }
};


#endif // MUSIC_H
