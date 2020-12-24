#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "tands.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>

using namespace std;

void print_summary(map<string, int> client_trans, double time_start, double time_end);

#define TRUE   1
#define FALSE  0