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

struct compareSymbols {
    bool operator()(pair<string, int> a, pair<string, int> b) {
        return a.second == b.second ? a.first < b.first : a.second < b.second;
    }
};

int main(int argc, char **argv) {
    string inputLine, word;
    string orderID;
    map<string, set<string> > executions;
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
            return 0;
        } else if (message[0] == "SHOW") {
            // TODO
            if (message[1] == "HIGHEST") {
            
            // NOT WORKING?
            } else if (message[1] == "BBO") {
                auto &buyPrices = buyOrders.get<1>();
                auto high = buyPrices.rbegin();

                printf("Highest bid price:\n %f %s\n", high->price, high->orderID.c_str());

                auto &sellPrices = sellOrders.get<1>();
                auto low = sellPrices.begin();

                printf("Lowest ask price:\n %f \n%s\n", low->price, low->orderID.c_str());

                continue;
            } else if (message[1] == "TOTAL") {
                if (message[2] == "SHARES") {
                    printf("Total shares executed:\n %d\n", totalSharesExecuted);
                } else if (message[2] == "EXECS") {
                    printf("Total number of executions:\n %d\n", numExecutions);
                }
                continue;
            } else if (message[1] == "LEVEL") {
                int n = stoi(message[3]);

                if (message[2] == "B") {
                    auto &buyPrices = buyOrders.get<1>();
                    auto frnt = buyPrices.begin();
                    for (int i = 0; i < n; i++) { frnt++; }

                    printf("BUY LEVEL %d: %d\n", n, frnt->price);
                } else if (message[2] == "S") {
                    auto &sellPrices = sellOrders.get<1>();
                    auto back = sellPrices.end();
                    for (int i = 0; i <= n; i++) { back--; }

                    printf("SELL LEVEL %d: %d\n", n, back->price);
                } else {
                    continue;
                }
            }


            return 0;
        } else if (message[1] == "A") {
            // ignore message due to incorrect number of fields or incorrect field values
            // TODO : FINISH FIELD VALUE CHECKS
            if (message.size() != 7) {
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
            if (message.size() != 5) {
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
            } else {
                continue;
            }
        } else if(message[1] == "C") {
            // ignore message due to incorrect number of fields or incorrect field values
            // TODO : FINISH FIELD VALUE CHECKS
            if (message.size() != 4) {
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
    return 0;
}