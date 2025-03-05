

#include <msg_info.h>
#include <optional>
#include <serdes.h>
#include <string>
#include <vector>

/*

Res InfoTopic::serialize(Serializer &ser) {
  RET_ERR(ser.map_begin(), "Failed to encode map");
  int idx = 0;
  RET_ERR(ser.serialize(idx++, name), "Failed to encode name");
  RET_ERR(ser.serialize(idx++, desc), "Failed to encode desc");
  RET_ERR(ser.map_end(), "Failed to encode map");
  return Res::Ok();
}

Res InfoTopic::deserialize(Deserializer &des) {
  des.iterate_map([&](Deserializer &des, uint32_t key) {
    switch (key) {
    case TOPIC_NAME_KEY: {
      RET_ERR(des.deserialize(name), "Failed to decode string");
      break;
    }
    case TOPIC_DESC_KEY: {
      RET_ERR(des.deserialize(desc), "Failed to decode string");
      break;
    }
    default:
      // todo skip item
      return Res::Ok(); // ignore unknown keys
    }
    return Res::Ok();
  });
  return Res::Ok();
}



InfoProp::InfoProp(uint32_t id, std::string name, std::string desc,
                   PropType type, PropMode mode) {
  this->id = id;
  this->name = name;
  this->desc = desc;
  this->type = type;
  this->mode = mode;
}

Res InfoProp::serialize(Serializer &ser) {
  RET_ERR(ser.map_begin(), "Failed to encode map");
  RET_ERR(ser.serialize(PROP_ID_KEY, id), "Failed to encode id");
  RET_ERR(ser.serialize(PROP_NAME_KEY, name), "Failed to encode name");
  RET_ERR(ser.serialize(PROP_DESC_KEY, desc), "Failed to encode desc");
  RET_ERR(ser.serialize(PROP_TYPE_KEY, (std::optional<uint32_t>)type),
          "Failed to encode type");
  RET_ERR(ser.serialize(PROP_MODE_KEY, (std::optional<uint32_t>)mode),
          "Failed to encode mode");
  RET_ERR(ser.map_end(), "Failed to encode map");
  return Res::Ok();
}

Res InfoProp::deserialize(Deserializer &des) {
  des.iterate_map([&](Deserializer &des, uint32_t key) {
    switch (key) {
    case PROP_ID_KEY: {
      RET_ERR(des.deserialize(id), "Failed to decode uint32_t");
      break;
    }
    case PROP_NAME_KEY: {
      RET_ERR(des.deserialize(name), "Failed to decode string");
      break;
    }
    case PROP_DESC_KEY: {
      RET_ERR(des.deserialize(desc), "Failed to decode string");
      break;
    }
    case PROP_TYPE_KEY: {
      uint32_t tp;
      RET_ERR(des.deserialize(tp), "Failed to decode PropType");
      type = (PropType)tp;
      break;
    }
    case PROP_MODE_KEY: {
      uint32_t md;
      RET_ERR(des.deserialize(md), "Failed to decode PropMode");
      mode = (PropMode)md;
      break;
    }

    default:
      // todo skip item
      return Res::Ok(); // ignore unknown keys
    }
    return Res::Ok();
  });
  return Res::Ok();
}
*/