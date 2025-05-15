#ifndef RENUMBER_H
#define RENUMBER_H

#include <map>
#include <string>

void renumberSource(std::map<int, std::string> &program, int newStart = 10,
                    int delta = 10, int oldStart = 0);

#endif // RENUMBER_H
