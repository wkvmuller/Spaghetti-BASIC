#ifndef FILEIO_H
#define FILEIO_H

#include "program_structure.h"

// Loads a BASIC program from program.filename
void load(PROGRAM_STRUCTURE &program);

// Saves a BASIC program to program.filename
void save(PROGRAM_STRUCTURE &program);

#endif // FILEIO_H
