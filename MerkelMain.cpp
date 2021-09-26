#include "MerkelMain.h"
#include <iostream>
#include <vector>
#include "OrderBookEntry.h"
#include "CSVReader.h"
#include "OrderBook.h"
#include "Wallet.h"
#include <algorithm>
#include "logger.h"

MerkelMain::MerkelMain()
{

}

void MerkelMain::init()
{
    int input;
    currentTime = orderBook.getEarliestTime();
    wallet.insertCurrency("BTC", 100);
    wallet.insertCurrency("USDT", 10000);
    wallet.insertCurrency("DOGE", 100000);
    for (int i = 0; i < 20; i++) {//the bot needs 20 times data for the linear prediction
        gotoNextTimeframe();
    }


    while (true)
    {
        printMenu();
        input = getUserOption();
        processUserOption(input);
    }
}


void MerkelMain::printMenu()
{
    // 1 print help
    std::cout << "1: Print help " << std::endl;
    // 2 print exchange stats
    std::cout << "2: Print exchange stats" << std::endl;
    // 3 make an offer
    std::cout << "3: Make an offer " << std::endl;
    // 4 make a bid 
    std::cout << "4: Make a bid " << std::endl;
    // 5 print wallet
    std::cout << "5: Print wallet " << std::endl;
    // 7 analyze the market
    std::cout << "7: Market analyze " << std::endl;
    // 8 continue   
    std::cout << "8: OrderInGenerate " << std::endl;
    // 6 continue   
    std::cout << "6: Continue " << std::endl;

    std::cout << "============== " << std::endl;

    std::cout << "Current time is: " << currentTime << std::endl;
}

void MerkelMain::printHelp()
{
    std::cout << "Help - your aim is to make money. Analyse the market and make bids and offers. " << std::endl;
}

void MerkelMain::printMarketStats()
{
    for (std::string const& p : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << p << std::endl;
        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask,
            p, currentTime);
        std::cout << "Asks seen: " << entries.size() << std::endl;
        std::cout << "Max ask: " << OrderBook::getHighPrice(entries) << std::endl;
        std::cout << "Min ask: " << OrderBook::getLowPrice(entries) << std::endl;



    }
    // std::cout << "OrderBook contains :  " << orders.size() << " entries" << std::endl;
    // unsigned int bids = 0;
    // unsigned int asks = 0;
    // for (OrderBookEntry& e : orders)
    // {
    //     if (e.orderType == OrderBookType::ask)
    //     {
    //         asks ++;
    //     }
    //     if (e.orderType == OrderBookType::bid)
    //     {
    //         bids ++;
    //     }  
    // }    
    // std::cout << "OrderBook asks:  " << asks << " bids:" << bids << std::endl;

}

void MerkelMain::enterAsk()
{
    std::cout << "Make an ask - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input;
    std::getline(std::cin, input);

    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    if (tokens.size() != 3)
    {
        std::cout << "MerkelMain::enterAsk Bad input! " << input << std::endl;
    }
    else {
        try {
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::ask
            );
            obe.username = "simuser";
            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
            }
            else {
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << " MerkelMain::enterAsk Bad input " << std::endl;
        }
    }
}

void MerkelMain::enterBid()
{
    std::cout << "Make an bid - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input;
    std::getline(std::cin, input);

    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    if (tokens.size() != 3)
    {
        std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;
    }
    else {
        try {
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2],
                currentTime,
                tokens[0],
                OrderBookType::bid
            );
            obe.username = "simuser";

            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
            }
            else {
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << " MerkelMain::enterBid Bad input " << std::endl;
        }
    }
}


void MerkelMain::printWallet()
{
    std::cout << wallet.toString() << std::endl;
}


void MerkelMain::gotoNextTimeframe()
{
    recordWallet.outputWallet(currentTime, wallet.toString());//print out the wallet of this timestamp
    recordOrder.outputEachOrder(currentTime, ordersofUser);
    std::cout << "Going to next time frame. " << std::endl;
    suggestionsIn.clear();//clean the suggestion of last time saved
    for (std::string p : orderBook.getKnownProducts())
    {
        double orderSuccessCount = 0;//transaction success times of current products
        double avgprice = 0;
        std::cout << "matching " << p << std::endl;
        int sizes = orderBook.getOrders(OrderBookType::ask, p, currentTime).size() + orderBook.getOrders(OrderBookType::bid, p, currentTime).size();
        std::vector<OrderBookEntry> sales = orderBook.matchAsksToBids(p, currentTime);
        std::cout << "Sales: " << sales.size() << std::endl;    
        for (OrderBookEntry& sale : sales)
        {
            avgprice += sale.price;
            std::cout << "Sale price: " << sale.price << " amount " << sale.amount << std::endl; 
            //output the success transaction record to to the txt File
            recordSuccessOrder.outputFlow("Success sale: " + sale.product + " , " + std::to_string(sale.price) + " , "
                + std::to_string(sale.amount) + " , " + sale.timestamp);
            orderSuccessCount ++ ;
            if (sale.username == "simuser")
            {
                // update the wallet
                wallet.processSale(sale);
            }
        }
        if (orderSuccessCount != 0) {//if current product has success transaction.
            recordSuccessOrder.outputFlow("Product: " + p + 
                + " Average transaction Price: " + std::to_string(avgprice / orderSuccessCount) +
                " Success rate of current product match: " + std::to_string(orderSuccessCount / sizes * 100 )+"%");

        }
        
    }
        currentTime = orderBook.getNextTime(currentTime);  
}

void MerkelMain::MarketAnalyze() {
    suggestionsIn.clear();//everytime start to analyze, the programme should clean the vector of suggestions for users.
    for (std::string p : orderBook.getKnownProducts()) {
        //double bidprediction = linearprediction(OrderBookType::bid, p, currentTime);
        std::cout << "analyzing: " << p << std::endl;
        std::vector <OrderBookEntry> asks = orderBook.getOrders(OrderBookType::ask, p, currentTime);
        double avgAsk = orderBook.getAvgPrice(asks);
        std::vector <OrderBookEntry> bids = orderBook.getOrders(OrderBookType::bid, p, currentTime);
        double avgBid = orderBook.getAvgPrice(bids);
        std::cout << "the prediction result of " << p << " average bid is " << linearprediction(OrderBookType::bid, p, currentTime) << std::endl;
        double minbidprediction = linearprediction(OrderBookType::ask, p, currentTime);
        std::cout << "the prediction result of " << p << " average ask is " << linearprediction(OrderBookType::ask, p, currentTime) << std::endl;
        if (avgAsk < linearprediction(OrderBookType::ask, p, currentTime)) {//if the ask price goes up, its good to buy
            suggestionsIn.push_back(p);
        }
    }
    for (std::string ps : suggestionsIn) {
        std::cout << "the product u should buy in : " << ps << std::endl;
    }
}

void MerkelMain::OrderGenerator() {
    std::vector<OrderBookEntry> availableAsk;
    for (std::string p : suggestionsIn) {//loop for the suggestionIn and create all the orders which can profit
        double askAmount = 0;
        std::vector <OrderBookEntry> asks = orderBook.getOrders(OrderBookType::ask, p, currentTime);
        double avgAsk = orderBook.getAvgPrice(asks);
        for (auto iter = asks.begin(); iter != asks.end(); iter++) {//create the bid order, price below the avg ask and amount equals to those ask orders
            if ((*iter).price < avgAsk) askAmount += (*iter).amount;
        }
        OrderBookEntry OrderIn = OrderBookEntry(avgAsk, askAmount, currentTime, p, OrderBookType::bid, "simuser");
        if (wallet.canFulfillOrder(OrderIn)) {
            availableAsk.push_back(OrderIn);
            ordersofUser.push_back(OrderIn);//collect the history prices for future creating ask order.
        }

    }
    for (auto iter = availableAsk.begin(); iter != availableAsk.end(); iter++) {
        orderBook.insertOrder(*iter);
        std::cout << "Insert the bid: " << (*iter).product << " price: " << (*iter).price << " amount: "<< (*iter).amount << std::endl;
    }

    std::vector<OrderBookEntry> currentAsk;//the ask order of user in current timestamp
    for (std::string p : orderBook.getKnownProducts()) {
        double InAmount = 0;
        double historyInPrice = 0;
        int sizeOfInorder = 0;
        std::string currency = CSVReader::tokenise(p, '/')[0];//the first half part of the product is the currency need to be sufficient in the wallet.
        for (auto iter = ordersofUser.begin(); iter != ordersofUser.end(); iter++) {
            if ((*iter).product == p && (*iter).orderType == OrderBookType::bid) {
                historyInPrice += (*iter).price;
                sizeOfInorder++;
            }
        }
        historyInPrice /= sizeOfInorder;
        if (isnan(historyInPrice)) continue;//if there is no history record of this products, the varible will be nan. 
        historyInPrice *= 1.0004;
        if (wallet.returnCurrencyAmount(currency) == 0) continue;
        OrderBookEntry OrderOut = OrderBookEntry(historyInPrice, wallet.returnCurrencyAmount(currency)/3, currentTime, p, OrderBookType::ask, "simuser");//sell the prices as 
        if (wallet.canFulfillOrder(OrderOut)) {//the history bid's price * 1.00004, make sure that user can earn 0.04%.
            currentAsk.push_back(OrderOut);
            ordersofUser.push_back(OrderOut);
        }
    }

    for (auto iter = currentAsk.begin(); iter != currentAsk.end(); iter++) {
    orderBook.insertOrder(*iter);
    std::cout << "Insert the ask: " << (*iter).product << " price: " << (*iter).price << " Amount: " <<(*iter).amount << std::endl;
    }
    
}

void MerkelMain::removeAskOrder() {//remove the ask order from user
    if (currentTime != orderBook.getLatestTime()) {
    for (std::string p : orderBook.getKnownProducts()) {//check every product
        double historyInPrice = 0;
        int sizeOfCurrentProducts = 0;
        for (auto iter = ordersofUser.begin(); iter != ordersofUser.end(); iter++) {
            if ((*iter).product == p && (*iter).orderType == OrderBookType::bid) {
                historyInPrice += (*iter).price;
                sizeOfCurrentProducts++;
            }
        }
        historyInPrice /= sizeOfCurrentProducts;
        if (isnan(historyInPrice)) continue;//if there is no history record of this products, the varible will be nan.
        std::vector <OrderBookEntry> bids = orderBook.getOrders(OrderBookType::bid, p, currentTime);
        if (orderBook.getHighPrice(bids) < historyInPrice) {//in the current time, the ask order is impossible to be matched so user should withdrawl.
            for (auto iter = ordersofUser.begin(); iter != ordersofUser.end(); iter++) {
                if ((*iter).product == p && (*iter).timestamp == currentTime && (*iter).orderType == OrderBookType::ask) {
                    OrderBookEntry OrderErase = OrderBookEntry((*iter).price, (*iter).amount, currentTime, p, OrderBookType::bid, "simuser");
                    //insert the reversed order which u wanna remove, so that the wrong ask order can be withdrawl.
                    orderBook.insertOrder(OrderErase);
                    std::cout << "Order erase: " << OrderErase.product << "Order Type: Ask " << " Amount: " << OrderErase.amount << " Price: " << OrderErase.price << std::endl;
                }
            }
        }
    }
    }
}

void MerkelMain::removeBidOrder() {//remove the bid order from user
    if (currentTime != orderBook.getLatestTime()) {
        for (std::string p : suggestionsIn) {//if current ask price is higher than user's in price, 
            double historyPrice = 0;// then current user's bid order should be withdrawl.
            int sizeOfCurrentProducts = 0;
            double indexofBid = 0;
            for (auto iter = ordersofUser.begin(); iter != ordersofUser.end(); iter++) {
                if ((*iter).product == p && (*iter).orderType == OrderBookType::bid) {
                    historyPrice += (*iter).price;
                    sizeOfCurrentProducts++;
                }
            }
            historyPrice /= sizeOfCurrentProducts;
            if (isnan(historyPrice)) continue;//if there is no history record of this products, the varible will be nan.
            std::vector <OrderBookEntry> asks = orderBook.getOrders(OrderBookType::ask, p, currentTime);
            if (historyPrice < orderBook.getLowPrice(asks)) {
                for (auto iter = ordersofUser.begin(); iter != ordersofUser.end(); iter++) {
                    if ((*iter).product == p && (*iter).timestamp == currentTime && (*iter).orderType == OrderBookType::bid) {
                        OrderBookEntry OrderErase = OrderBookEntry((*iter).price, (*iter).amount, currentTime, p, OrderBookType::ask, "simuser");
                        std::cout << "Order erase: " << OrderErase.product << "Order Type: Bid" << " Amount: " << OrderErase.amount << "Price: " << OrderErase.price << std::endl;
                    }
                }
            }
        }
    }
}


double MerkelMain::linearprediction(OrderBookType type, std::string product, std::string time) {//return the value according to the order type
    std::vector <OrderBookEntry> entriesForAnalyze;
    if (time == orderBook.getEarliestTime()) {//make sure that the data is enough.
        std::cout << "have not enough data, cannot predict!" << std::endl;
        return 0;
    }
    std::string starttime = orderBook.getEarliestTime();
    double sumX = 0;
    double sumY = 0;
    double avgX = 0;
    double avgY = 0;
    double xx = 0;
    double xy = 0;
    double m = 0;
    double c = 0;
    double predictResult;
    std::vector<double> prices;
    
    if (type == OrderBookType::bid) {
    //calculate the prediction function of min bid from start until current time.
        while (starttime < time && starttime < orderBook.getLatestTime()) {
            std::vector<OrderBookEntry> entry = orderBook.getOrders(OrderBookType::bid, product, starttime);
            for (int i = 0; i < entry.size(); i++) {
                if (entry[i].username == "dataset") {
                    entriesForAnalyze.push_back(entry[i]);
                }
            }
            double currentPrice = orderBook.getAvgPrice(entriesForAnalyze);//take the average bid price
            prices.push_back(currentPrice);
            starttime = orderBook.getNextTime(starttime);
        }
        for (int i = 0; i < prices.size(); i++) {
            sumY += prices[i];// the sum of Y value (price)
            sumX += i + 1;//the sum of X value
            xx += (i + 1) * (i + 1);
            xy += (i + 1) * prices[i];
        }
        avgX = sumX / prices.size();
        avgY = sumY / prices.size();
        m = (xy - sumX * sumY / prices.size())/(xx - sumX * sumY / prices.size());//simplized the linear regression formular
        c = avgY - avgX * m;
        //std::cout << "The prediction bid formular is " << m << "x + " << c << std::endl;
        predictResult = m * (prices.size() + 1) + c;
    }

    if (type == OrderBookType::ask) {
        //calculate the prediction function of max ask from start until current time.
        while (starttime < time && starttime < orderBook.getLatestTime()) {
            std::vector<OrderBookEntry> entry = orderBook.getOrders(OrderBookType::ask, product, starttime);
            for (int i = 0; i < entry.size(); i++) {
                if (entry[i].username == "dataset") {
                    entriesForAnalyze.push_back(entry[i]);
                }
            }
            double currentPrice = orderBook.getAvgPrice(entriesForAnalyze);
            prices.push_back(currentPrice);
            starttime = orderBook.getNextTime(starttime);
        }

        for (int i = 0; i < prices.size(); i++) {
            sumY += prices[i];// the sum of Y value (price)
            sumX += i + 1;//the sum of X value
            xx += (i + 1) * (i + 1);
            xy += (i + 1) * prices[i];
        }
        avgX = sumX / prices.size();
        avgY = sumY / prices.size();
        m = (xy - sumX * sumY / prices.size()) / (xx - sumX * sumY / prices.size());//simplized the linear regression formular
        c = avgY - avgX * m;
        //std::cout << "The prediction ask formular is " << m << "x + " << c << std::endl;
        predictResult = m * (prices.size()) + c;
    }
    
    return predictResult;
}



int MerkelMain::getUserOption()
{
    int userOption = 0;
    std::string line;
    std::cout << "Type in 1-6" << std::endl;
    std::getline(std::cin, line);
    try {
        userOption = std::stoi(line);
    }
    catch (const std::exception& e)
    {
        // 
    }
    std::cout << "You chose: " << userOption << std::endl;
    return userOption;
}

void MerkelMain::processUserOption(int userOption)
{
    if (userOption == 0) // bad input
    {
        std::cout << "Invalid choice. Choose 1-6" << std::endl;
    }
    if (userOption == 1)
    {
        printHelp();
    }
    if (userOption == 2)
    {
        printMarketStats();
    }
    if (userOption == 3)
    {
        enterAsk();
    }
    if (userOption == 4)
    {
        enterBid();
    }
    if (userOption == 5)
    {
        printWallet();
    }
    if (userOption == 6)
    {
        printWallet();
        MarketAnalyze();
        OrderGenerator();
        removeAskOrder();
        removeBidOrder();
        gotoNextTimeframe();
    }
    if (userOption == 7)
    {
        MarketAnalyze();
    }
    if (userOption == 8)
    {
        OrderGenerator();
    }
}