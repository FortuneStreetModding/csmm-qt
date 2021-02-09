#include "vanilladatabase.h"
#include "mapdescriptor.h"

namespace VanillaDatabase {

struct MapSetZone {
    qint8 mapSet;
    qint8 zone;
    qint8 order;
    qint8 mapId;
};

static const MapSetZone MAP_SET_ZONE_TABLE[] = {
    {1, 0, 0, 3},
    {1, 0, 1, 7},
    {1, 0, 2, 1},
    {1, 0, 3, 0},
    {1, 0, 4, 4},
    {1, 0, 5, 2},

    {1, 1, 0, 9},
    {1, 1, 1, 17},
    {1, 1, 2, 10},
    {1, 1, 3, 13},
    {1, 1, 4, 12},
    {1, 1, 5, 14},

    {1, 2, 0, 15},
    {1, 2, 1, 5},
    {1, 2, 2, 6},
    {1, 2, 3, 8},
    {1, 2, 4, 11},
    {1, 2, 5, 16},

    {0, 0, 0, 24},
    {0, 0, 1, 28},
    {0, 0, 2, 22},
    {0, 0, 3, 21},
    {0, 0, 4, 25},
    {0, 0, 5, 23},

    {0, 1, 0, 30},
    {0, 1, 1, 38},
    {0, 1, 2, 31},
    {0, 1, 3, 34},
    {0, 1, 4, 33},
    {0, 1, 5, 35},

    {0, 2, 0, 36},
    {0, 2, 1, 26},
    {0, 2, 2, 27},
    {0, 2, 3, 29},
    {0, 2, 4, 32},
    {0, 2, 5, 37},
};

qint8 getVanillaMapSet(int mapId) {
    for (auto &entry: MAP_SET_ZONE_TABLE) {
        if (entry.mapId == mapId) return entry.mapSet;
    }
    return -1;
}
qint8 getVanillaZone(int mapId) {
    for (auto &entry: MAP_SET_ZONE_TABLE) {
        if (entry.mapId == mapId) return entry.zone;
    }
    return -1;
}
qint8 getVanillaOrder(int mapId) {
    for (auto &entry: MAP_SET_ZONE_TABLE) {
        if (entry.mapId == mapId) return entry.order;
    }
    return -1;
}

struct VentureCard {
    bool defaultEasyMode;
    bool defaultStandardMode;
    QString description;
};

static const VentureCard VENTURE_CARD_TABLE[128] = {
    { true  , true  , "Adventurous turning point! You can choose which way to move on your next go, (player's name)."                    },
    { true  , true  , "Venture on! Roll the die again and move forward."                                                                 },
    { true  , true  , "Venture through space! Zoom over to any non-venture, non-suit square you like!"                                   },
    { true  , false , "Moneymaking venture! Roll the die and get 40 times the number shown in gold coins from the player in 1st place!"  },
    { true  , true  , "Venture through space! Zoom over to any shop or vacant plot!"                                                     },
    { true  , true  , "Venture through space! Zoom over to any venture or suit square!"                                                  },
    { true  , true  , "Special bonus! Your shops all grow by 7%!"                                                                        },
    { true  , true  , "Venture on! Everyone's shop prices increase by 30%! Now roll the die and move again."                             },
    { true  , true  , "Venture on! Everyone's shops close for the day! Now roll the die and move again."                                 },
    { true  , true  , "Venture on! Everyone's shop prices cut in half! Now roll the die and move again."                                 },
    { true  , true  , "Moneymaking venture! Roll the die and get 11 times the number shown in gold coins from all other players!"        },
    { true  , true  , "Capital venture! You can invest capital in any of your shops."                                                    },
    { false , true  , "Misadventure! The values of all your shops drop by 13%!"                                                          },
    { true  , false , "Misadventure! You give everyone 30G each!"                                                                        },
    { true  , true  , "Moneymaking venture! Roll the die and get 50 times the number shown in gold coins from the bank!"                 },
    { false , false , "Random venture! Shops expand in three districts picked at random!"                                                },
    { true  , true  , "Special bonus! You receive half of your salary!"                                                                  },
    { true  , true  , "Misadventure! The bank is forcibly buying you out! You're compelled to sell a shop for only twice its value."     },
    { true  , true  , "Price hike venture! Your shop prices go up by 30% until your next turn."                                          },
    { true  , true  , "Revaluation venture! You can expand any one of your shops by 20%."                                                },
    { false , true  , "Random venture! You receive 20 stocks in a district picked at random!"                                            },
    { true  , true  , "Cashback venture! You can sell a shop back to the bank for twice its shop value."                                 },
    { true  , true  , "Revaluation venture! You can expand any one of your shops by 50%."                                                },
    { true  , false , "Misadventure! The bank is forcibly buying you out! You're compelled to sell a shop for 200G more than its value." },
    { true  , true  , "Misadventure! Your shop prices halve until your next turn!"                                                       },
    { true  , true  , "Lucky venture! You get a big commission until your next turn!"                                                    },
    { true  , true  , "Special bonus! You receive 27 times the number of shops you own in gold coins from the bank!"                     },
    { false , false , "Cameo adventure! A goodybag appears!"                                                                             },
    { true  , true  , "Freebie! Take a Heart!"                                                                                           },
    { true  , true  , "Venture on! All shops charge a 100G flat rate! Now roll the die and move again."                                  },
    { false , true  , "Random venture! Shops expand by 10% in a district picked at random!"                                              },
    { false , true  , "Random venture! Shops expand by 20% in a district picked at random!"                                              },
    { true  , true  , "Cashback venture! You can sell a shop back to the bank for three times its shop value."                           },
    { false , true  , "Dicey adventure! Roll 1/3/5 and your shops close for the day. Roll 2/4/6 and everyone else's shops close."        },
    { false , false , "Stock venture! You can sell stocks you own at 35% above the market value."                                        },
    { false , true  , "Capital venture! You can pay 100G for the chance to invest in your shops."                                        },
    { false , false , "Random venture! Shops expand by 30% in a district picked at random!"                                              },
    { false , true  , "Stock venture! You can buy stocks in a district of your choice at 10% above the market value."                    },
    { true  , true  , "Suit venture! Buy a Suit Yourself card for 100G."                                                                 },
    { true  , true  , "Misadventure! You give away 10% of your ready cash to the player in last place!"                                  },
    { false , true  , "Misadventure! Stock prices fall by 10% in a district picked at random!"                                           },
    { false , true  , "Misadventure! Stock prices fall by 20% in a district picked at random!"                                           },
    { false , true  , "Misadventure! You pay an assets tax of two gold coins per unit of stock that you own!"                            },
    { true  , false , "Misadventure! Roll the die and pay 44 times the number in gold coins to the player in last place!"                },
    { false , false , "Dicey adventure! Roll 1/3/5 to warp to a take-a-break square. Roll 2/4/6 to warp to the arcade."                  },
    { false , true  , "Misadventure! You drop your wallet and lose 10% of your ready cash!"                                              },
    { false , false , "Dicey adventure! Roll 2-6 to get all the suits. Roll 1 and lose all your suits."                                  },
    { false , true  , "Misadventure! All shops in a district picked at random fall in value by 10%!"                                     },
    { false , false , "Misadventure! All shops in a district picked at random fall in value by 20%!"                                     },
    { true  , true  , "Venture on! Move forward the same number of squares again."                                                       },
    { true  , false , "Venture on! Move forward 1 square more."                                                                          },
    { false , false , "Venture on! Move forward another 2 squares."                                                                      },
    { false , true  , "Venture through space! Zoom over to the bank!"                                                                    },
    { false , true  , "Venture through space! Pay 100G to zoom straight to the bank!"                                                    },
    { true  , true  , "Venture on! Roll the die again and move forward (with an invitation to browse thrown in!)."                       },
    { true  , true  , "Venture on! Roll the die again and move forward (with a half-price special offer thrown in!)."                    },
    { true  , true  , "Venture through space! Zoom to any square you like."                                                              },
    { true  , false , "Venture through space! Pay 100G to zoom to any non-venture, non-suit square you like!"                            },
    { false , true  , "Stock venture! You can buy stocks in a district of your choice at 10% below the market value."                    },
    { false , true  , "Random venture! Stock prices increase by 10% in a district picked at random!"                                     },
    { false , true  , "Special bonus! You receive a 10% dividend on your stocks!"                                                        },
    { false , false , "Special bonus! You receive a 20% dividend on your stocks!"                                                        },
    { false , true  , "Random venture! Stock prices increase by 20% in a district picked at random!"                                     },
    { false , false , "Random venture! Stock prices increase by 30% in a district picked at random!"                                     },
    { true  , false , "Forced buyout! You can buy a vacant plot or shop for five times its value, whether someone else owns it or not."  },
    { false , false , "Special bonus! You receive 10 of the most valuable stocks!"                                                       },
    { false , true  , "Stock venture! You can buy stocks in a district of your choice."                                                  },
    { false , false , "Special arcade adventure! You're invited to play Memory Block!"                                                   },
    { false , false , "Stock venture! You can sell stocks you own at 20% above the market value."                                        },
    { false , true  , "Special bonus! You get a sudden promotion and receive a salary! (You lose any suits you have.)"                   },
    { true  , true  , "Capital venture! You can invest up to 200G of the bank's money in your shops."                                    },
    { true  , true  , "Dicey adventure! Roll 1/3/5 to take 20 times the number of your shops in gold coins. Roll 2/4/6 to pay the same." },
    { true  , true  , "Property venture! You can buy any unowned shop or vacant plot."                                                   },
    { true  , false , "Misadventure! You are forced to auction one of your shops (with a starting price of twice the shop's value)."     },
    { true  , true  , "Property venture! You can buy any unowned shop or vacant plot for twice its value."                               },
    { true  , false , "Special arcade adventure! You're invited to play Round the Blocks!"                                               },
    { false , true  , "Freebie! Take five of each district's stocks."                                                                    },
    { true  , true  , "Property venture! You can buy any unowned shop or vacant plot for 200G more than its value."                      },
    { true  , false , "Forced buyout! You can buy a vacant plot or shop for three times its value, whether someone else owns it or not." },
    { true  , true  , "Freebie! Take a Spade!"                                                                                           },
    { true  , true  , "Misadventure! All other players can only move forward 1 on their next turn."                                      },
    { true  , true  , "Freebie! Take a Club!"                                                                                            },
    { false , false , "Dicey adventure! Roll 1/3/5 and warp to a random location. Roll 2/4/6 and everyone else warps."                   },
    { true  , true  , "Moneymaking venture! The winning player must pay you 10% of their ready cash!"                                    },
    { false , false , "Moneymaking venture! Roll the die and get 85 times the number shown in gold coins from the bank!"                 },
    { true  , true  , "Moneymaking venture! Take 100G from all other players!"                                                           },
    { true  , false , "Venture on! Roll the special all-7s-and-8s die and move forward again."                                           },
    { false , false , "Misadventure! All other players swap places!"                                                                     },
    { false , false , "Freebie! All players take a Suit Yourself card!"                                                                  },
    { false , false , "Price hike venture! All shop prices go up by 30% until your next turn."                                           },
    { false , false , "Cameo adventure! A healslime appears!"                                                                            },
    { false , false , "Cameo adventure! Lakitu appears!"                                                                                 },
    { false , false , "Dicey adventure! Roll 1/3/5 and your shops expand by 10%. Roll 2/4/6 and everyone else's shops expand by 5%."     },
    { true  , true  , "Freebie! Take a Diamond!"                                                                                         },
    { false , false , "Misadventure! You throw an impromptu party. All other players come to your location!"                             },
    { false , false , "Misadventure! All players scramble to another player's location!"                                                 },
    { false , false , "Stock rise venture! Increase stock value by 20% in a district of your choice."                                    },
    { true  , true  , "Forced buyout! You can buy a vacant plot or shop for four times its value, whether someone else owns it or not."  },
    { false , false , "Freebie! What's inside...?"                                                                                       },
    { true  , true  , "Freebie! Take a Suit Yourself card!"                                                                              },
    { false , false , "Special bonus! Your shops all grow by 21%!"                                                                       },
    { true  , false , "Moneymaking venture! Roll the die and get 33 times the number shown in gold coins from all other players!"        },
    { false , false , "Misadventure! The values of all your shops drop by 25%!"                                                          },
    { false , false , "Misadventure! You give everyone 80G each!"                                                                        },
    { true  , false , "Moneymaking venture! Roll the die and get the number shown x your level x 40G from the bank!"                     },
    { true  , false , "Freebie! Roll the die and get half the number shown of Suit Yourself cards! (Decimals will be rounded down.)"     },
    { true  , false , "Revaluation venture! You can expand any one of your shops by 30%."                                                },
    { true  , false , "Cashback venture! You can sell a shop back to the bank for four times its shop value."                            },
    { false , false , "Revaluation venture! You can expand any one of your shops by 75%."                                                },
    { false , false , "Special bonus! You receive 77 times the number of shops you own in gold coins from the bank!"                     },
    { false , false , "Cashback venture! You can sell a shop back to the bank for 500G more than its shop value."                        },
    { false , false , "Special bonus! You receive 100 times the number of shops you own in gold coins!"                                  },
    { true  , false , "Moneymaking venture! Roll the die and get the number shown x your level x 20G from the bank!"                     },
    { true  , false , "Moneymaking venture! Take your level times 40G from all other players!"                                           },
    { true  , false , "Misadventure! All other players can only move forward 7 on their next turn."                                      },
    { true  , false , "Moneymaking venture! Roll the die and get 60 times the number shown in gold coins from the player in 1st place!"  },
    { true  , false , "Adventurous turning point! Everyone gets to choose which way to move on their next go."                           },
    { false , false , "Lucky venture! You get a really big commission until your next turn!"                                             },
    { false , false , "Misadventure! You give 20% of your ready cash to the player in last place!"                                       },
    { false , false , "Misadventure! You drop your wallet and lose 20% of your ready cash!"                                              },
    { false , false , "Capital venture! You can invest up to 400G of the bank's money in your shops."                                    },
    { false , false , "Moneymaking venture! The winning player must pay you 20% of their ready cash!"                                    },
    { false , false , "Dicey adventure! Roll 1/3/5 and your shops expand by 20%. Roll 2/4/6 and everyone else's shops expand by 5%."     },
    { false , false , "Suit venture! Buy a Suit Yourself card for 50G."                                                                  },
    { false , false , "Dicey adventure! Roll 1/3/5 to warp to a boon square. Roll 2/4/6 to warp to the arcade."                          },
    { false , false , "Revaluation venture! Roll the die and expand your shops by 2% for each number."                                   },
    { false , false , "Special arcade adventure! You're invited to play Round the Blocks and Memory Block!"                              },
    { false , false , "Special bonus! You receive 55 times the number of shops you own in gold coins from the bank!"                     }
};

const QString &getVentureCardDesc(int ventureCard) {
    return VENTURE_CARD_TABLE[ventureCard].description;
}

bool isDefaultVentureCards(const bool ventureCards[], RuleSet ruleSet) {
    bool match = true;
    for(int i=0; i<128; i++) {
        if(ruleSet == Standard) {
            if(VENTURE_CARD_TABLE[i].defaultStandardMode != ventureCards[i])
                match = false;
        } else {
            if(VENTURE_CARD_TABLE[i].defaultEasyMode != ventureCards[i])
                match = false;
        }
    }
    return match;
}

void setDefaultVentureCards(RuleSet ruleSet, bool outVentureCards[]) {
    for(int i=0; i<128; i++) {
        if(ruleSet == Standard) {
            outVentureCards[i] = VENTURE_CARD_TABLE[i].defaultStandardMode;
        } else {
            outVentureCards[i] = VENTURE_CARD_TABLE[i].defaultEasyMode;
        }
    }
}

struct Background {
    QString background;
    QString description;
    BgmId bgmId;
    QString mapIcon;
    QString mapTpl;
};

static const Background BACKGROUND_TABLE[] = {
    { "bg101"  , "Trodain Castle"    , BGM_MAP_TRODAIN   , "p_bg_101" , "ui_menu007_bg101.tpl" },
    { "bg109"  , "The Observatory"   , BGM_MAP_ANGEL     , "p_bg_109" , "ui_menu007_bg109.tpl" },
    { "bg102"  , "Ghost Ship"        , BGM_MAP_GHOSTSHIP , "p_bg_102" , "ui_menu007_bg102.tpl" },
    { "bg105"  , "Slimenia"          , BGM_MAP_SLABACCA  , "p_bg_105" , "ui_menu007_bg105.tpl" },
    { "bg104"  , "Mt. Magmageddon"   , BGM_MAP_SINOKAZAN , "p_bg_104" , "ui_menu007_bg104.tpl" },
    { "bg106"  , "Robbin' Hood Ruins", BGM_MAP_KANDATA   , "p_bg_106" , "ui_menu007_bg106.tpl" },
    { "bg004"  , "Mario Stadium"     , BGM_MAP_STADIUM   , "p_bg_004" , "ui_menu007_bg004.tpl" },
    { "bg008"  , "Starship Mario"    , BGM_MAP_STARSHIP  , "p_bg_008" , "ui_menu007_bg008.tpl" },
    { "bg002"  , "Mario Circuit"     , BGM_MAP_CIRCUIT   , "p_bg_002" , "ui_menu007_bg002.tpl" },
    { "bg001"  , "Yoshi's Island"    , BGM_MAP_YOSHI     , "p_bg_001" , "ui_menu007_bg001.tpl" },
    { "bg005"  , "Delfino Plaza"     , BGM_MAP_DOLPIC    , "p_bg_005" , "ui_menu007_bg005.tpl" },
    { "bg003"  , "Peach's Castle"    , BGM_MAP_PEACH     , "p_bg_003" , "ui_menu007_bg003.tpl" },
    { "bg107"  , "Alefgard"          , BGM_MAP_ALEFGARD  , "p_bg_107" , "ui_menu007_bg107.tpl" },
    { "bg006"  , "Super Mario Bros"  , BGM_MAP_SMB       , "p_bg_006" , "ui_menu007_bg006.tpl" },
    { "bg007"  , "Bowser's Castle"   , BGM_MAP_KOOPA     , "p_bg_007" , "ui_menu007_bg007.tpl" },
    { "bg009"  , "Good Egg Galaxy"   , BGM_MAP_EGG       , "p_bg_009" , "ui_menu007_bg009.tpl" },
    { "bg103"  , "The Colossus"      , BGM_MAP_MAJINZOU  , "p_bg_103" , "ui_menu007_bg103.tpl" },
    { "bg103_e", "The Colossus Easy" , BGM_MAP_MAJINZOU  , "p_bg_103" , "ui_menu007_bg103.tpl" },
    { "bg108"  , "Alltrades Abbey"   , BGM_MAP_DHAMA     , "p_bg_108" , "ui_menu007_bg108.tpl" },
    { "bg901"  , "Practice Board"    , BGM_MENU          , "p_bg_901" , "" }
};

QString getVanillaTpl(const QString &mapIcon) {
    for (auto &entry: BACKGROUND_TABLE) {
        if (entry.mapIcon == mapIcon) return entry.mapTpl;
    }
    return "";
}

bool hasVanillaTpl(const QString &mapIcon) {
    for (auto &entry: BACKGROUND_TABLE) {
        if (entry.mapIcon == mapIcon) return true;
    }
    return false;
}

BgmId getDefaultBgmId(const QString &background) {
    for (auto &entry: BACKGROUND_TABLE) {
        if (entry.background == background) return entry.bgmId;
    }
    return BGM_TITLE;
}

bool hasDefaultBgmId(const QString &background) {
    for (auto &entry: BACKGROUND_TABLE) {
        if (entry.background == background) return true;
    }
    return false;
}

QString getDefaultMapIcon(const QString &background) {
    for (auto &entry: BACKGROUND_TABLE) {
        if (entry.background == background) return entry.mapIcon;
    }
    return "";
}

bool hasDefaultMapIcon(const QString &background) {
    for (auto &entry: BACKGROUND_TABLE) {
        if (entry.background == background) return true;
    }
    return false;
}

}
