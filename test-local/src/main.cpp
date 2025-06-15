#include <iostream>
#include <string>
#include <map>
#include <value.h>

void panic_here(const char *s){
    std::cerr << "Panic: " << s << std::endl;
    std::exit(1);
}


int main() {
    Value v;
    v["x"]="xxxxxx";
    v["y"]["z"]=(float)1234.5;
    v["y"]["a"]=Value::ArrayType{1, 2, 3, 4, 5};
    v["y"]["b"]=Value::ObjectType{{"key1", "value1"}, {"key2", "value2"}};
    v["y"]["c"]=Value::ObjectType{{"key3", "value3"}, {"key4", "value4"}};
    v["z"]["x1"]=true;
    std::string s;
    s = v.toJson(true);
    printf("JSON: %s\n", s.c_str());

    return 0;
}
