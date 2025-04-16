# Tradebot

## Requirements
- [Boost](https://archives.boost.io/release/1.88.0/source/)

## Compiling and Running
- Compile: g++ -I"path/to/your/local/boost/folder" orderBookManager.cpp -o  orderBookManager
    - This is just an example of how I used g++, but obviously you can use whatever c++ compiler you prefer 
    - An example of a path would be "C:\boost_1_88_0\boost_1_88_0". This is just the contents extracted from the zip file downloaded from the link provided in requirements above.
- Running: The executable can be used to manually input messages through stdin, but I would recommend adding commands to the bottom of your csv file and piping the contents into it (as seen in testInputs.csv)
    - ./orderBookManager 
    - cat yourFileName.csv | orderBookManager

## Usages
Outside of the standard message types outlined in the prompt, there are these messages to call the query functions and to quit the program if manually entering messages.
- QUIT : quits the program
- SHOW,HIGHEST : outputs the symbol with the highest remaining shares and the state of the book for that symbol.
- SHOW,BBO : outputs the best bid and best offer prices
- SHOW,TOTAL,SHARES : outputs the total shares executed
- SHOW,TOTAL,EXECS :  outputs the number of executions
- SHOW,LEVEL, B/S,N : outputs the Nth level of the given side
    - Choose either B or S to show the corresponding side
    - N is a 1-indexed integer (i.e. to show the first level of buy orders, the command is SHOW,LEVEL,B,1)

## Referenecs
These are just boost docs and examples that I found very helpful when learning how to use this library
- https://www.boost.org/doc/libs/1_88_0/libs/multi_index/doc/examples.html
- https://www.boost.org/doc/libs/1_88_0/libs/multi_index/example/complex_structs.cpp
- https://www.boost.org/doc/libs/1_88_0/libs/multi_index/example/basic.cpp