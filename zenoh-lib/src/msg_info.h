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

  Res serialize(Serializer &ser) ;

   Res deserialize(Deserializer &des) ;
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
  InfoProp (uint32_t id, std::string name, std::string desc, PropType type, PropMode mode);

  Res serialize(Serializer &ser) ;

  Res deserialize(Deserializer &des) ;

};

#endif