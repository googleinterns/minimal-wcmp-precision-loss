//
// Created by wwt on 6/14/20.
//

#ifndef WCMP_MAIN_H
#define WCMP_MAIN_H


#include <vector>
#include <string>
#include <iostream>
#include <ostream>
#include <cstdio>
#include <sstream>
#include <algorithm>

#define TABLE_SIZE 16000
#define MAX_GROUP_SIZE 256

enum SwitchType {
	s1,
	s2,
	s3
};

std::string ntoa(double n);
std::string itoa(int n);

#endif //WCMP_MAIN_H
