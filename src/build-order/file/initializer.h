#ifndef FILE_INITIALIZER_H
#define FILE_INITIALIZER_H

#include <build-order/file/read-file.h>

void initializeResources(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeTasks(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeEvents(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeMeanTime(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeSecondaryAttributes();

void initializeOverallMaximum(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeMaximumPerResource(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeEquivalence(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeTaskCosts(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeTaskPrerequisites(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeTaskConsume(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeTaskBorrow(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeTaskProduce(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeEventBonus(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeEventTime(std::fstream &file, std::string &buffer, std::stringstream &sstream);
void initializeEventTrigger(std::fstream &file, std::string &buffer, std::stringstream &sstream);

#endif
