#include "vanilladatabase.h"

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

static const QString VENTURE_CARD_DESCS[128] = {
    "Adventurous turning point! You can choose which way to move on your next go, (player's name).",
    "Venture on! Roll the die again and move forward.",
    "Venture through space! Zoom over to any non-venture, non-suit square you like!",
    "Moneymaking venture! Roll the die and get 40 times the number shown in gold coins from the player in 1st place!",
    "Venture through space! Zoom over to any shop or vacant plot!",
    "Venture through space! Zoom over to any venture or suit square!",
    "Special bonus! Your shops all grow by 7%!",
    "Venture on! Everyone's shop prices increase by 30%! Now roll the die and move again.",
    "Venture on! Everyone's shops close for the day! Now roll the die and move again.",
    "Venture on! Everyone's shop prices cut in half! Now roll the die and move again.",
    "Moneymaking venture! Roll the die and get 11 times the number shown in gold coins from all other players!",
    "Capital venture! You can invest capital in any of your shops.",
    "Misadventure! The values of all your shops drop by 13%!",
    "Misadventure! You give everyone 30G each!",
    "Moneymaking venture! Roll the die and get 50 times the number shown in gold coins from the bank!",
    "Random venture! Shops expand in three districts picked at random!",
    "Special bonus! You receive half of your salary!",
    "Misadventure! The bank is forcibly buying you out! You're compelled to sell a shop for only twice its value.",
    "Price hike venture! Your shop prices go up by 30% until your next turn.",
    "Revaluation venture! You can expand any one of your shops by 20%.",
    "Random venture! You receive 20 stocks in a district picked at random!",
    "Cashback venture! You can sell a shop back to the bank for twice its shop value.",
    "Revaluation venture! You can expand any one of your shops by 50%.",
    "Misadventure! The bank is forcibly buying you out! You're compelled to sell a shop for 200G more than its value.",
    "Misadventure! Your shop prices halve until your next turn!",
    "Lucky venture! You get a big commission until your next turn!",
    "Special bonus! You receive 27 times the number of shops you own in gold coins from the bank!",
    "Cameo adventure! A goodybag appears!",
    "Freebie! Take a Heart!",
    "Venture on! All shops charge a 100G flat rate! Now roll the die and move again.",
    "Random venture! Shops expand by 10% in a district picked at random!",
    "Random venture! Shops expand by 20% in a district picked at random!",
    "Cashback venture! You can sell a shop back to the bank for three times its shop value.",
    "Dicey adventure! Roll 1/3/5 and your shops close for the day. Roll 2/4/6 and everyone else's shops close.",
    "Stock venture! You can sell stocks you own at 35% above the market value.",
    "Capital venture! You can pay 100G for the chance to invest in your shops.",
    "Random venture! Shops expand by 30% in a district picked at random!",
    "Stock venture! You can buy stocks in a district of your choice at 10% above the market value.",
    "Suit venture! Buy a Suit Yourself card for 100G.",
    "Misadventure! You give away 10% of your ready cash to the player in last place!",
    "Misadventure! Stock prices fall by 10% in a district picked at random!",
    "Misadventure! Stock prices fall by 20% in a district picked at random!",
    "Misadventure! You pay an assets tax of two gold coins per unit of stock that you own!",
    "Misadventure! Roll the die and pay 44 times the number in gold coins to the player in last place!",
    "Dicey adventure! Roll 1/3/5 to warp to a take-a-break square. Roll 2/4/6 to warp to the arcade.",
    "Misadventure! You drop your wallet and lose 10% of your ready cash!",
    "Dicey adventure! Roll 2-6 to get all the suits. Roll 1 and lose all your suits.",
    "Misadventure! All shops in a district picked at random fall in value by 10%!",
    "Misadventure! All shops in a district picked at random fall in value by 20%!",
    "Venture on! Move forward the same number of squares again.",
    "Venture on! Move forward 1 square more.",
    "Venture on! Move forward another 2 squares.",
    "Venture through space! Zoom over to the bank!",
    "Venture through space! Pay 100G to zoom straight to the bank!",
    "Venture on! Roll the die again and move forward (with an invitation to browse thrown in!).",
    "Venture on! Roll the die again and move forward (with a half-price special offer thrown in!).",
    "Venture through space! Zoom to any square you like.",
    "Venture through space! Pay 100G to zoom to any non-venture, non-suit square you like!",
    "Stock venture! You can buy stocks in a district of your choice at 10% below the market value.",
    "Random venture! Stock prices increase by 10% in a district picked at random!",
    "Special bonus! You receive a 10% dividend on your stocks!",
    "Special bonus! You receive a 20% dividend on your stocks!",
    "Random venture! Stock prices increase by 20% in a district picked at random!",
    "Random venture! Stock prices increase by 30% in a district picked at random!",
    "Forced buyout! You can buy a vacant plot or shop for five times its value, whether someone else owns it or not.",
    "Special bonus! You receive 10 of the most valuable stocks!",
    "Stock venture! You can buy stocks in a district of your choice.",
    "Special arcade adventure! You're invited to play Memory Block!",
    "Stock venture! You can sell stocks you own at 20% above the market value.",
    "Special bonus! You get a sudden promotion and receive a salary! (You lose any suits you have.)",
    "Capital venture! You can invest up to 200G of the bank's money in your shops.",
    "Dicey adventure! Roll 1/3/5 to take 20 times the number of your shops in gold coins. Roll 2/4/6 to pay the same.",
    "Property venture! You can buy any unowned shop or vacant plot.",
    "Misadventure! You are forced to auction one of your shops (with a starting price of twice the shop's value).",
    "Property venture! You can buy any unowned shop or vacant plot for twice its value.",
    "Special arcade adventure! You're invited to play Round the Blocks!",
    "Freebie! Take five of each district's stocks.",
    "Property venture! You can buy any unowned shop or vacant plot for 200G more than its value.",
    "Forced buyout! You can buy a vacant plot or shop for three times its value, whether someone else owns it or not.",
    "Freebie! Take a Spade!",
    "Misadventure! All other players can only move forward 1 on their next turn.",
    "Freebie! Take a Club!",
    "Dicey adventure! Roll 1/3/5 and warp to a random location. Roll 2/4/6 and everyone else warps.",
    "Moneymaking venture! The winning player must pay you 10% of their ready cash!",
    "Moneymaking venture! Roll the die and get 85 times the number shown in gold coins from the bank!",
    "Moneymaking venture! Take 100G from all other players!",
    "Venture on! Roll the special all-7s-and-8s die and move forward again.",
    "Misadventure! All other players swap places!",
    "Freebie! All players take a Suit Yourself card!",
    "Price hike venture! All shop prices go up by 30% until your next turn.",
    "Cameo adventure! A healslime appears!",
    "Cameo adventure! Lakitu appears!",
    "Dicey adventure! Roll 1/3/5 and your shops expand by 10%. Roll 2/4/6 and everyone else's shops expand by 5%.",
    "Freebie! Take a Diamond!",
    "Misadventure! You throw an impromptu party. All other players come to your location!",
    "Misadventure! All players scramble to another player's location!",
    "Stock rise venture! Increase stock value by 20% in a district of your choice.",
    "Forced buyout! You can buy a vacant plot or shop for four times its value, whether someone else owns it or not.",
    "Freebie! What's inside...?",
    "Freebie! Take a Suit Yourself card!",
    "Special bonus! Your shops all grow by 21%!",
    "Moneymaking venture! Roll the die and get 33 times the number shown in gold coins from all other players!",
    "Misadventure! The values of all your shops drop by 25%!",
    "Misadventure! You give everyone 80G each!",
    "Moneymaking venture! Roll the die and get the number shown x your level x 40G from the bank!",
    "Freebie! Roll the die and get half the number shown of Suit Yourself cards! (Decimals will be rounded down.)",
    "Revaluation venture! You can expand any one of your shops by 30%.",
    "Cashback venture! You can sell a shop back to the bank for four times its shop value.",
    "Revaluation venture! You can expand any one of your shops by 75%.",
    "Special bonus! You receive 77 times the number of shops you own in gold coins from the bank!",
    "Cashback venture! You can sell a shop back to the bank for 500G more than its shop value.",
    "Special bonus! You receive 100 times the number of shops you own in gold coins!",
    "Moneymaking venture! Roll the die and get the number shown x your level x 20G from the bank!",
    "Moneymaking venture! Take your level times 40G from all other players!",
    "Misadventure! All other players can only move forward 7 on their next turn.",
    "Moneymaking venture! Roll the die and get 60 times the number shown in gold coins from the player in 1st place!",
    "Adventurous turning point! Everyone gets to choose which way to move on their next go.",
    "Lucky venture! You get a really big commission until your next turn!",
    "Misadventure! You give 20% of your ready cash to the player in last place!",
    "Misadventure! You drop your wallet and lose 20% of your ready cash!",
    "Capital venture! You can invest up to 400G of the bank's money in your shops.",
    "Moneymaking venture! The winning player must pay you 20% of their ready cash!",
    "Dicey adventure! Roll 1/3/5 and your shops expand by 20%. Roll 2/4/6 and everyone else's shops expand by 5%.",
    "Suit venture! Buy a Suit Yourself card for 50G.",
    "Dicey adventure! Roll 1/3/5 to warp to a boon square. Roll 2/4/6 to warp to the arcade.",
    "Revaluation venture! Roll the die and expand your shops by 2% for each number.",
    "Special arcade adventure! You're invited to play Round the Blocks and Memory Block!",
    "Special bonus! You receive 55 times the number of shops you own in gold coins from the bank!"
};

const QString &getVentureCardDesc(int ventureCard) {
    return VENTURE_CARD_DESCS[ventureCard];
}

}
