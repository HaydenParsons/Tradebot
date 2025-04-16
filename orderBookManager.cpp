#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <cctype>
#include <algorithm>
#include <set>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>

using namespace std;
using namespace boost::multi_index;

struct order {
    string orderID;
    int shares;
    string symbol;
    float price;
};

typedef multi_index_container<
    order,
    indexed_by<
        // fast lookup by orderID using hashed index
        hashed_unique< member<order, string, &order::orderID> >,
        // price-sorted ordered index
        ordered_non_unique< member<order, float, &order::price> >
    >
> orderBook;

void printOrdersByPrice(const boost::multi_index::nth_index<orderBook, 1>::type& ordersByPrice, string symbol) {
    for (auto it = ordersByPrice.begin(); it != ordersByPrice.end(); ) {
        float price = it->price;
        int numOrders = 0;
        int totalShares = 0;

        // sum shares and count number of orders at current price
        auto range = ordersByPrice.equal_range(price);
        for (auto orderIt = range.first; orderIt != range.second; orderIt++) {
            if (orderIt->symbol == symbol) {
                numOrders++;
                totalShares += orderIt->shares;
            }
        }

        // print out needed info and move on to the next price
        if (numOrders > 0) { printf("%f %d %d\n", price, totalShares, numOrders); }
        it = range.second; 
    }
}

// prints out the symbol with the highest remaining shares across all price levels and the state of that book
// automatically called at the end once all messages have been input, but can also be called with the input "SHOW,HIGHEST"
void showHighestRemaining(orderBook &buyOrders, orderBook &sellOrders) {
    string highestSharesSymbol;
    float price = 0;
    int highestSharesSum = 0, numOrders = 0;
    map<string, int> remainingShares;

    auto &buyByPrice = buyOrders.get<1>();
    auto &sellByPrice = sellOrders.get<1>();

    highestSharesSymbol = buyByPrice.begin()->symbol;

    for (auto &buyOrder : buyByPrice) {
        remainingShares[buyOrder.symbol] += buyOrder.shares;
        if (highestSharesSum < remainingShares[buyOrder.symbol] || (highestSharesSum == remainingShares[buyOrder.symbol] && highestSharesSymbol > buyOrder.symbol)) {
            highestSharesSum = remainingShares[buyOrder.symbol];
            highestSharesSymbol = buyOrder.symbol;
        }
    }
    for (auto &sellOrder : sellByPrice) {
        remainingShares[sellOrder.symbol] += sellOrder.shares;
        if (highestSharesSum < remainingShares[sellOrder.symbol] || (highestSharesSum == remainingShares[sellOrder.symbol] && highestSharesSymbol > sellOrder.symbol)) {
            highestSharesSum = remainingShares[sellOrder.symbol];
            highestSharesSymbol = sellOrder.symbol;
        }
    }

    printf("%s\nSELL:\n", highestSharesSymbol.c_str());
    printOrdersByPrice(sellByPrice, highestSharesSymbol);
    printf("BUY:\n");
    printOrdersByPrice(buyByPrice, highestSharesSymbol);
}

// helper function to check value of integer float fields
bool isAllDigits(string& str) {
    return !str.empty() && all_of(str.begin(), str.end(), ::isdigit);
}


int main(int argc, char **argv) {
    string inputLine, word;
    orderBook buyOrders;
    orderBook sellOrders;

    int totalSharesExecuted = 0, numExecutions = 0;

    auto &buyByID = buyOrders.get<0>();
    auto &sellByID = sellOrders.get<0>();

    while (getline(cin, inputLine)) {
        vector<string> message;
        stringstream ss(inputLine);

        while (getline(ss, word, ',')) {
            message.push_back(word);
        }

        if (message[0] == "QUIT") {
            showHighestRemaining(buyOrders, sellOrders);
            return 0;
        } else if (message[0] == "SHOW") {
            if (message[1] == "HIGHEST") {
                showHighestRemaining(buyOrders, sellOrders);
            } else if (message[1] == "BBO") {
                auto &buyPrices = buyOrders.get<1>();
                auto high = buyPrices.rbegin();

                if (high == buyPrices.rend()) {
                    printf("No buy orders remaining\n");
                } else {   
                    printf("Highest bid price:\n%f %s\n", high->price, high->orderID.c_str());
                }

                auto &sellPrices = sellOrders.get<1>();
                auto low = sellPrices.begin();
                
                if (low == sellPrices.end()) {
                    printf("No sell orders remaining\n");
                } else {   
                    printf("Lowest ask price:\n%f %s\n", low->price, low->orderID.c_str());
                }
            } else if (message[1] == "TOTAL") {
                if (message[2] == "SHARES") {
                    printf("Total shares executed:\n%d\n", totalSharesExecuted);
                } else if (message[2] == "EXECS") {
                    printf("Total number of executions:\n%d\n", numExecutions);
                }
            } else if (message[1] == "LEVEL") {
                int n = stoi(message[3]);

                if (message[2] == "B") {
                    auto &buyPrices = buyOrders.get<1>();
                    if (n > buyPrices.size()) {
                        printf("Only %d buy price levels remaining\n", buyPrices.size());
                    } else {
                        auto buyPrice = buyPrices.rbegin();
                        for (int i = 0; i < n - 1; i++) { buyPrice++; }
                        
                        printf("BUY level %d:\n%f\n", n, buyPrice->price);
                    }
                } else if (message[2] == "S") {
                    auto &sellPrices = sellOrders.get<1>();
                    if (n > sellPrices.size()) {
                        printf("Only %d sell price levels remaining\n", sellPrices.size());
                    } else {
                        auto sellPrice = sellPrices.begin();
                        for (int i = 0; i < n - 1; i++) { sellPrice++; }
    
                        printf("SELL level %d:\n%f\n", n, sellPrice->price);
                    }

                }
            }
        } else if (message[1] == "A") {
            // ignore message due to incorrect number of fields or incorrect field values
            if (message.size() != 7 
            || !isAllDigits(message[0])         // Timestamp
            || message[1].size() != 1           // Message Type
            || message[2].size() > 10           // Order ID
            || message[3].size() != 1           // Side
            || !isAllDigits(message[4])         // Shares
            || message[5].size() > 8            // Symbol
            || !isAllDigits(message[6])) {      // Price
                continue;
            }

            if (message[3] == "B") {
                buyOrders.insert(order{
                    message[2],                     // Order ID     (string)
                    stoi(message[4]),               // Shares       (int)
                    message[5],                     // Symbol       (string)
                    stof(message[6]) / 10000.0f     // Price        (float)
                });
            } else if (message[3] == "S") {
                sellOrders.insert(order{
                    message[2],                     // Order ID     (string)
                    stoi(message[4]),               // Shares       (int)
                    message[5],                     // Symbol       (string)
                    stof(message[6]) / 10000.0f     // Price        (float)
                });
            }

        // if Execute Order, update book as needed
        } else if(message[1] == "E") {
            // ignore message due to incorrect number of fields or incorrect field values
            // TODO : FINISH FIELD VALUE CHECKS
            if (message.size() != 5 
            || !isAllDigits(message[0])         // Timestamp
            || message[1].size() != 1           // Message Type
            || message[2].size() > 10           // Order ID
            || !isAllDigits(message[3])) {      // Shares
                continue;
            }

            auto bit = buyByID.find(message[2]);
            auto sit = sellByID.find(message[2]);
            int ms = stoi(message[3]);

            if (bit != buyByID.end() && bit != nullptr) {
                numExecutions++;
                if (bit->shares > ms) {
                    totalSharesExecuted += ms;
                    buyByID.modify(bit, [&ms](order &o) { o.shares -= ms; });
                } else {
                    totalSharesExecuted += bit->shares;
                    buyByID.erase(bit);
                }
            } else if (sit != sellByID.end()) {
                numExecutions++;
                if (sit->shares > ms) {
                    totalSharesExecuted += ms;
                    sellByID.modify(sit, [&ms](order &o) { o.shares -= ms; });
                } else {
                    totalSharesExecuted += sit->shares;
                    sellByID.erase(sit);
                }
            }
        } else if(message[1] == "C") {
            // ignore message due to incorrect number of fields or incorrect field values
            // TODO : FINISH FIELD VALUE CHECKS
            if (message.size() != 4 
            || !isAllDigits(message[0])         // Timestamp
            || message[1].size() != 1           // Message Type
            || message[2].size() > 10           // Order ID
            || !isAllDigits(message[3])) {      // Shares
                continue;
            }

            auto bit = buyByID.find(message[2]);
            auto sit = sellByID.find(message[2]);
            int ms = stoi(message[3]);

            if (bit != buyByID.end()) {
                if (bit->shares > ms) {
                    buyByID.modify(bit, [&ms](order &o) { o.shares -= ms; });
                } else {
                    buyByID.erase(bit);
                }
            } else if (sit != sellByID.end()) {
                if (sit->shares > ms) {
                    sellByID.modify(sit, [&ms](order &o) { o.shares -= ms; });
                } else {
                    sellByID.erase(sit);
                }
            }
        }
    }
    showHighestRemaining(buyOrders, sellOrders);
    return 0;
}