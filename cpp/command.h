#ifndef TTCP_COMMAND_H
#define TTCP_COMMAND_H

#include "common.h"

class CommandLine :nocopyable
{
private:
   StringArg host;
   StringArg port;
   int bsNumber;
   int bsLength; 
   bool isClinet;
};


#endif /*TTCP_COMMAND_H */