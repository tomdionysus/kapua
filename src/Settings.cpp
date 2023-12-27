//
// Kapua Config class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "Settings.hpp"

namespace Kapua {

bool Settings::writeSettings(uint64_t id, const std::vector<Item>& items) {
  FILE* file = fopen(filename_.c_str(), "w");
  if (!file) {
    std::cerr << "Failed to open file for writing: " << filename_ << std::endl;
    return false;
  }

  yaml_emitter_t emitter;
  yaml_emitter_initialize(&emitter);

  if (!yaml_emitter_open_file(&emitter, file)) {
    std::cerr << "Failed to open file for writing: " << filename_ << std::endl;
    fclose(file);
    return false;
  }

  yaml_event_t event;

  // Start the YAML document
  yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
  if (!yaml_emitter_emit(&emitter, &event)) {
    std::cerr << "Failed to start YAML stream" << std::endl;
    fclose(file);
    return false;
  }

  // Write settings
  writeSettingsSection(&emitter, id, items);

  // End the YAML document
  yaml_stream_end_event_initialize(&event);
  if (!yaml_emitter_emit(&emitter, &event)) {
    std::cerr << "Failed to end YAML stream" << std::endl;
    fclose(file);
    return false;
  }

  // Clean up
  yaml_emitter_delete(&emitter);
  fclose(file);
  return true;
}

void Settings::writeSettingsSection(yaml_emitter_t* emitter, uint64_t id, const std::vector<Item>& items) {
  yaml_event_t event;

  // Start the settings mapping
  yaml_mapping_start_event_initialize(&event, NULL, NULL, 1, YAML_BLOCK_MAPPING_STYLE);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to start settings mapping" << std::endl;
    return;
  }

  // Write the ID
  writeUint64Field(emitter, "ID", id);

  // Write the Items sequence
  yaml_scalar_style_t scalar_style = YAML_PLAIN_SCALAR_STYLE;
  yaml_sequence_start_event_initialize(&event, NULL, NULL, 1, scalar_style);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to start Items sequence" << std::endl;
    return;
  }

  for (const auto& item : items) {
    // Write each item
    yaml_mapping_start_event_initialize(&event, NULL, NULL, 1, scalar_style);
    if (!yaml_emitter_emit(emitter, &event)) {
      std::cerr << "Failed to start item mapping" << std::endl;
      return;
    }

    writeUint64Field(emitter, "ID", item.id);

    // Write the sockaddr_in address
    // Assuming you have a function to convert sockaddr_in to a string
    std::string addressStr = sockaddr_inToString(item.address);
    writeStringField(emitter, "Address", addressStr);

    // End the item mapping
    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(emitter, &event)) {
      std::cerr << "Failed to end item mapping" << std::endl;
      return;
    }
  }

  // End the Items sequence
  yaml_sequence_end_event_initialize(&event);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to end Items sequence" << std::endl;
    return;
  }

  // End the settings mapping
  yaml_mapping_end_event_initialize(&event);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to end settings mapping" << std::endl;
    return;
  }
}

void Settings::writeUint64Field(yaml_emitter_t* emitter, const char* fieldName, uint64_t value) {
  yaml_event_t event;

  // Write field name
  yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t*)fieldName, -1, 1, 1, YAML_PLAIN_SCALAR_STYLE);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to write field name: " << fieldName << std::endl;
    return;
  }

  // Write field value
  yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t*)std::to_string(value).c_str(), -1, 1, 1, YAML_PLAIN_SCALAR_STYLE);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to write field value for " << fieldName << std::endl;
  }
}

void Settings::writeStringField(yaml_emitter_t* emitter, const char* fieldName, const std::string& value) {
  yaml_event_t event;

  // Write field name
  yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t*)fieldName, -1, 1, 1, YAML_PLAIN_SCALAR_STYLE);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to write field name: " << fieldName << std::endl;
    return;
  }

  // Write field value
  yaml_scalar_event_initialize(&event, NULL, NULL, (yaml_char_t*)value.c_str(), -1, 1, 1, YAML_PLAIN_SCALAR_STYLE);
  if (!yaml_emitter_emit(emitter, &event)) {
    std::cerr << "Failed to write field value for " << fieldName << std::endl;
  }
}

// Assuming you have a function to convert sockaddr_in to a string
std::string Settings::sockaddr_inToString(sockaddr_in address) {
  char buffer[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(address.sin_addr), buffer, INET_ADDRSTRLEN);
  return std::string(buffer);
}

bool Settings::readSettings(uint64_t& id, std::vector<Item>& items) {
  FILE* file = fopen(filename_.c_str(), "r");
  if (!file) {
    std::cerr << "Failed to open file for reading: " << filename_ << std::endl;
    return false;
  }

  yaml_parser_t parser;
  if (!yaml_parser_initialize(&parser)) {
    std::cerr << "Failed to initialize YAML parser" << std::endl;
    fclose(file);
    return false;
  }

  yaml_parser_set_input_file(&parser, file);

  if (!parseSettings(&parser, id, items)) {
    std::cerr << "Failed to parse settings from YAML" << std::endl;
    fclose(file);
    yaml_parser_delete(&parser);
    return false;
  }

  fclose(file);
  yaml_parser_delete(&parser);
  return true;
}

private:
std::string filename_;

bool Settings::parseSettings(yaml_parser_t* parser, uint64_t& id, std::vector<Item>& items) {
  yaml_event_t event;
  yaml_event_type_t eventType;

  while (true) {
    if (!yaml_parser_parse(parser, &event)) {
      std::cerr << "YAML parsing error" << std::endl;
      return false;
    }

    eventType = event.type;

    if (eventType == YAML_STREAM_END_EVENT) {
      break;
    }

    if (eventType == YAML_SCALAR_EVENT) {
      const char* fieldName = (const char*)event.data.scalar.value;

      if (std::string(fieldName) == "ID") {
        if (!parseUint64Field(parser, id)) {
          return false;
        }
      } else if (std::string(fieldName) == "Items") {
        if (!parseItems(parser, items)) {
          return false;
        }
      }
    }

    yaml_event_delete(&event);
  }

  return true;
}

bool Settings::parseUint64Field(yaml_parser_t* parser, uint64_t& value) {
  yaml_event_t event;
  if (!yaml_parser_parse(parser, &event) || event.type != YAML_SCALAR_EVENT) {
    std::cerr << "Failed to parse uint64_t field" << std::endl;
    return false;
  }

  value = std::strtoull((const char*)event.data.scalar.value, NULL, 10);
  yaml_event_delete(&event);

  return true;
}

bool Settings::parseItems(yaml_parser_t* parser, std::vector<Item>& items) {
  yaml_event_t event;
  yaml_event_type_t eventType;

  while (true) {
    if (!yaml_parser_parse(parser, &event)) {
      std::cerr << "YAML parsing error" << std::endl;
      return false;
    }

    eventType = event.type;

    if (eventType == YAML_SEQUENCE_END_EVENT) {
      break;
    }

    if (eventType == YAML_MAPPING_START_EVENT) {
      Item item;
      if (!parseItem(parser, item)) {
        return false;
      }
      items.push_back(item);
    }

    yaml_event_delete(&event);
  }

  return true;
}

bool Settings::parseItem(yaml_parser_t* parser, Item& item) {
  yaml_event_t event;
  yaml_event_type_t eventType;

  while (true) {
    if (!yaml_parser_parse(parser, &event)) {
      std::cerr << "YAML parsing error" << std::endl;
      return false;
    }

    eventType = event.type;

    if (eventType == YAML_MAPPING_END_EVENT) {
      break;
    }

    if (eventType == YAML_SCALAR_EVENT) {
      const char* fieldName = (const char*)event.data.scalar.value;

      if (std::string(fieldName) == "ID") {
        if (!parseUint64Field(parser, item.id)) {
          return false;
        }
      } else if (std::string(fieldName) == "Address") {
        if (!parseAddress(parser, item.address)) {
          return false;
        }
      }
    }

    yaml_event_delete(&event);
  }

  return true;
}

bool Settings::parseAddress(yaml_parser_t* parser, sockaddr_in& address) {
  yaml_event_t event;
  if (!yaml_parser_parse(parser, &event) || event.type != YAML_SCALAR_EVENT) {
    std::cerr << "Failed to parse address field" << std::endl;
    return false;
  }

  const char* addressStr = (const char*)event.data.scalar.value;
  inet_pton(AF_INET, addressStr, &(address.sin_addr));
  address.sin_family = AF_INET;  // Assuming IPv4

  yaml_event_delete(&event);

  return true;
}
}  // namespace Kapua