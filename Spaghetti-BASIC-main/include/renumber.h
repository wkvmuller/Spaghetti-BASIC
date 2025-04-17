#ifndef RENUMBER_H
#define RENUMBER_H

#include <string>
#include <vector>

void renumberSource(std::vector<std::string>& lines, int newStart = 10, int delta = 10, int oldStart = 0);

#endif // RENUMBER_H
