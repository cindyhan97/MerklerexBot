#include <iostream>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <string>
#include "OrderBookEntry.h"
#pragma once

class logger
{
public:
	logger(std::string filepath);//create the file
	void outputFlow(std::string s);
	void outputWallet(std::string timestamp, std::string s);
	void outputEachOrder(std::string timestamp, std::vector<OrderBookEntry> orders);
	void outputSuccessOrder(std::string timestamp, std::vector<OrderBookEntry> orders);
	
	
private:
	//object of output the text to file.
	std::ofstream outFile;
	std::string filepath;
};

