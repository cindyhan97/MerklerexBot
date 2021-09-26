#pragma once

#include <vector>
#include "OrderBookEntry.h"
#include "OrderBook.h"
#include "Wallet.h"
#include "logger.h"


class MerkelMain
{
public:
    MerkelMain();
    /** Call this to start the sim */
    void init();
private:
    void printMenu();
    void printHelp();
    void printMarketStats();
    void enterAsk();
    void enterBid();
    void printWallet();
    void gotoNextTimeframe();
    int getUserOption();
    void processUserOption(int userOption);
    double linearprediction(OrderBookType type, std::string product, std::string time);
    void MarketAnalyze();
    void OrderGenerator();
    void removeAskOrder();
    void removeBidOrder();
    

    std::string currentTime;
    std::vector<std::string> suggestionsIn;
    std::vector<OrderBookEntry> ordersofUser;


    OrderBook orderBook{"20200601.csv"};
    Wallet wallet;
    logger recordWallet{"R4A.txt"};
    logger recordOrder{"R4B.txt"};
    logger recordSuccessOrder{"R4C.txt"};
    

};
