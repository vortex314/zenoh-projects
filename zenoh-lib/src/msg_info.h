#ifndef MSG_INFO_H
#define MSG_INFO_H

#include <optional>
#include <string>
#include <vector>
#include <serdes.h>

// using namespace std;
#define TOPIC_NAME_KEY 0
#define TOPIC_DESC_KEY 1

class InfoTopic :public Serializable{
public:
  std::optional<std::string> name;
  std::optional<std::string> desc;
  InfoTopic(){};

  Res serialize(Serializer &ser) {
    RET_ERR(ser.map_begin(), "Failed to encode map");
    int idx=0;
    RET_ERR(ser.serialize(idx++, name), "Failed to encode name");
    RET_ERR(ser.serialize(idx++, desc), "Failed to encode desc");
    RET_ERR(ser.map_end(), "Failed to encode map");
    return Res::Ok();
  }

   Res deserialize(Deserializer &des) {
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
};

typedef enum PropMode {
  PROP_READ = 0,
  PROP_WRITE = 1,
  PROP_READ_WRITE = 2,
} PropMode;
typedef enum PropType {
  PROP_UINT = 0,
  PROP_SINT = 1,
  PROP_STR = 2,
  BYTES = 3,
  PROP_FLOAT = 4,
  PROP_OBJECT = 5,
  PROP_ARRAY = 6,
} PropType;

#define PROP_ID_KEY 0
#define PROP_NAME_KEY 1
#define PROP_DESC_KEY 2
#define PROP_TYPE_KEY 3
#define PROP_MODE_KEY 4

class InfoProp : public Serializable{
public:
  std::optional<uint32_t> id;
  std::optional<std::string> name;
  std::optional<std::string> desc;
  std::optional<PropType> type;
  std::optional<PropMode> mode;

  InfoProp(){};
  InfoProp (uint32_t id, std::string name, std::string desc, PropType type, PropMode mode){
    this->id = id;
    this->name = name;
    this->desc = desc;
    this->type = type;
    this->mode = mode;
  }

  Res serialize(Serializer &ser) {
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

  Res deserialize(Deserializer &des) {
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
};


class Msg {
public:
  std::optional<uint32_t> dst;         // 0
  std::optional<uint32_t> src;         // 1
  std::optional<InfoProp> info_prop;   // 6
  std::optional<InfoTopic> info_topic; // 7
  std::optional<Bytes> publish;        // 4

  Msg(){};

  Res serialize(Serializer &ser);
  static Res deserialize(Deserializer &des, Msg &msg);
};

#define DST_KEY 0
#define SRC_KEY 1
#define INFO_PROP_KEY 6
#define INFO_TOPIC_KEY 7
#define PUBLISH_KEY 4

Res Msg::serialize(Serializer &ser) {
  RET_ERR(ser.map_begin(), "Failed to encode map");
  RET_ERR(ser.serialize(DST_KEY, dst), "Failed to encode dst");
  RET_ERR(ser.serialize(SRC_KEY, src), "Failed to encode src");
  RET_ERR(ser.map_end(), "Failed to encode map");
  return Res::Ok();
}

Res Msg::deserialize(Deserializer &des, Msg &msg) { return Res::Ok(); }

#endif