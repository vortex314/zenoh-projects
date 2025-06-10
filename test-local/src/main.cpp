#include <stdio.h>
#include <value.h>
#include <option.h>
#include <result.h>
int main()
{
    printf(" Hello \n");
    Value v;
    v["int"] = 10;
    v["string"] = "Hello world string";
    v["publish"]["rpm_target"] = 2300;
    if ( v["publish"].is<Value::ObjectType>() ){
        printf(" is object type\n");
    }
    auto publish = v["publish"];
    v["publish"]["rpm_measured"] = 2023.5;
    v["publish"]["rpm_measured"].inspect<double>([](auto f ){ printf(" inspect %f \n",f);});
    double rpm_measured;
    v["publish"]["rpm_measured"].set(rpm_measured);
    rpm_measured = v["publish"]["rpm_measured"].as<double>();
    
    printf(" setting rpm_measured %f \n",rpm_measured);

    auto v1 = v;
    auto v2 = v["publish"];
    auto v3 = v.clone();

    printf(" v[int] = %lld \n", v["int"].as<Value::IntType>());
    printf(" v[string] = %s \n", v["string"].as<Value::StringType>().c_str());
    v1["publish"]["rpm_target"] = 3000;
    v1["int"] = 13;
    printf(" toJson v %s \n", v.toJson(true).c_str());
    printf(" toJson v1 %s \n", v1.toJson(true).c_str());
    printf(" toJson v2 %s \n", v2.toJson(true).c_str());
    printf(" toJson v3 %s \n", v3.toJson(true).c_str());

}

void panic_here(const char *s)
{

    printf("%s\n", s);
    fflush(stdout);
    exit(-1);
}