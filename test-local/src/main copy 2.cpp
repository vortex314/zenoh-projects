#include <iostream>
#include <string>
#include <map>

template <typename ValueType>
class MyContainer {
public:
    // Proxy class to distinguish between read and write operations
    class Proxy {
    public:
        Proxy(MyContainer& container, const std::string& key)
            : _value(container), key_(key) {}

        // Conversion operator for reading
        operator ValueType() const {
            return _value.get(key_);
        }

        // Assignment operator for writing
        ValueType& operator=(const ValueType& value) {
            return _value.set(key_, value);
        }

    private:
        MyContainer& _value;
        std::string key_;
    };

    // Overloaded operator[] that returns a proxy
    Proxy operator[](const std::string& key) {
        return Proxy(*this, key);
    }

    // Get value (const version, doesn't create if doesn't exist)
    const ValueType& get(const std::string& key) const {
        auto it = data_.find(key);
        if (it == data_.end()) {
            throw std::out_of_range("Key not found");
        }
        return it->second;
    }

    // Set value (creates if doesn't exist)
    ValueType& set(const std::string& key, const ValueType& value) {
        return data_[key] = value;
    }

private:
    std::map<std::string, ValueType> data_;
};

int main() {
    MyContainer<int> container;

    // Writing - creates the element
    container["one"] = 1;

    // Reading - doesn't create
    try {
        std::cout << "one: " << container["one"] << std::endl;
        std::cout << "two: " << container["two"] << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}
