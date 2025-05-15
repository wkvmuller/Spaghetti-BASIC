#ifndef RENUMBER_H
#define RENUMBER_H

#include "program_structure.h"
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

// Renumber BASIC program lines and update line references
// newStart: starting line number for renumbering
// delta: increment between lines
// oldStart: only renumber lines >= oldStart
void handleRENUMBER(int newStart, int delta, int oldStart);

#endif // RENUMBER_H
