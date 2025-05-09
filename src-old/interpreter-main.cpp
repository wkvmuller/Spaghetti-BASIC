
void runInterpreter(const std::map<int, std::string> &programSource) {
  for (std::map<int, std::string>::const_iterator it = programSource.begin();
       it != programSource.end(); ++it) {
    std::istringstream iss(it->second);
    std::string keyword;
    iss >> keyword;
    for (size_t i = 0; i < keyword.length(); ++i)
      keyword[i] = toupper(keyword[i]);

    StatementType stmt = identifyStatement(keyword);
    switch (stmt) {
    case ST_PRINTFILEUSING: 
      executePRINTFILEUSING(it->second);
    case ST_LET:
      executeLET(it->second);
      break;
    case ST_PRINT:
      executePRINT(it->second);
      break;
    case ST_INPUT:
      executeINPUT(it->second);
      break;
    case ST_GOTO:
      executeGOTO(it->second);
      break;
    case ST_IF:
      executeIF(it->second);
      break;
    case ST_FOR:
      executeFOR(it->second);
      break;
    case ST_NEXT:
      executeNEXT(it->second);
      break;
    case ST_READ:
      executeREAD(it->second);
      break;
    case ST_DATA:
      executeDATA(it->second);
      break;
    case ST_RESTORE:
      executeRESTORE(it->second);
      break;
    case ST_END:
      executeEND(it->second);
      break;
    case ST_DEF:
      executeDEF(it->second);
      break;
    case ST_DIM:
      executeDIM(it->second);
      break;
    case ST_REM:
      executeREM(it->second);
      break;
    case ST_STOP:
      executeSTOP(it->second);
      break;
    case ST_GOSUB:
      executeGOSUB(it->second);
      break;
    case ST_RETURN:
      executeRETURN(it->second);
      break;
    case ST_ON:
      executeON(it->second);
      break;
    case ST_MAT:
      executeMAT(it->second);
      break;
    case ST_FORMAT:
      executeFORMAT(it->second);
      break;
    case ST_BEEP:
      executeBEEP(it->second);
      break;
    case ST_OPEN:
      executeOPEN(it->second);
      break;
    case ST_CLOSE:
      executeCLOSE(it->second);
      break;
    case ST_PRINTFILE:
      executePRINTFILE(it->second);
      break;
    case ST_INPUTFILE:
      executeINPUTFILE(it->second);
      break;
    case ST_WHILE:
      executeWHILE(it->second);
      break;
    case ST_WEND:
      executeWEND(it->second);
      break;
    case ST_REPEAT:
      executeREPEAT(it->second);
      break;
    case ST_UNTIL:
      executeUNTIL(it->second);
      break;
    case ST_SEED:
      executeSEED(it->second);
      break;
    case ST_MATREAD: executeMATREAD(it->second); break;

    default:
      return "Unhandled statement: " << it->second << std::endl;
    }
  }
}

