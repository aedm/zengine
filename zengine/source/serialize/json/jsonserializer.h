#pragma once

#include <include/dom/document.h>
#include <string>
#include <rapidjson/include/rapidjson/document.h>

using namespace std;

class JSONSerializer {
public:
  JSONSerializer(Node* root);
  string GetJSON();

private:
  rapidjson::Document mJsonDocument;

  rapidjson::Value* Serialize(Node* node);

};