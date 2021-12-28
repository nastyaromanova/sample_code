#pragma once

#include <unordered_map>
#include <string>

#include "object.h"

class Object;

std::unordered_map<std::string, std::shared_ptr<Object>> GetBuiltInFunctions();