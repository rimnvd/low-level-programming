#ifndef LLP3_INTERACTIVE_MODE_H
#define LLP3_INTERACTIVE_MODE_H

#pragma once

#include "../commands/commands/commands.h"
#include "msg.pb.h"

void handle_query(FILE* f, Query_tree tree, char** response_);

#endif //LLP3_INTERACTIVE_MODE_H
