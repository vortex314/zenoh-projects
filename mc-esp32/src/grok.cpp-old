#include <cstdint>
#include <functional>
#include <queue>



template <typename M>
class ActorRef {
    virtual void tell(const M& m)=0;
};

// Platform-agnostic configuration
#ifdef NO_OS
    #define QUEUE_SIZE 64
    using QueueIndex = uint8_t;
#else
    #include <memory>
    using QueueIndex = size_t;
#endif

// Base message type with custom type ID
class Message {
public:
    using TypeID = uint32_t;
    Message() = default;
    Message(const Message& ) = default;
    virtual ~Message() = default;
    virtual TypeID getTypeID() const = 0;
};

// Type ID generator (compile-time or runtime)
namespace TypeRegistry {
    inline uint32_t nextTypeID = 0;
    template<typename T>
    uint32_t getTypeID() {
        static uint32_t id = nextTypeID++;
        return id;
    }
}

// Actor reference template
template<typename MsgType>
class ActorRef {
public:
    using Handler = std::function<void(const MsgType&)>;
    
    explicit ActorRef(Handler handler) : handler_(handler) {}
    
    void send(const MsgType& msg) const {
        if (handler_) {
            handler_(msg);
        }
    }

private:
    Handler handler_;
};

// Actor base class
class Actor {
public:
    Actor() = default;
    virtual ~Actor() = default;
    virtual void processMessages() = 0;

protected:
#ifdef NO_OS
    struct MessageEntry {
        void (*handler)(void*, const Message*);
        void* context;
        const Message* message;
    };
    MessageEntry messageQueue_[QUEUE_SIZE];
    QueueIndex queueHead_ = 0;
    QueueIndex queueTail_ = 0;
#else
    std::queue<std::pair<std::function<void()>, std::unique_ptr<const Message>>> messageQueue_;
#endif
};

// Concrete actor with multiple message types
template<typename... MsgTypes>
class TypedActor : public Actor {
public:
    // Create reference for specific message type
    template<typename MsgType>
    ActorRef<MsgType> getRef() {
        static_assert((std::is_same_v<MsgType, MsgTypes> || ...),
                     "Reference type must match one of the actor's message types");
        return ActorRef<MsgType>([this](const MsgType& msg) {
            enqueueMessage(msg);
        });
    }

    // Define handlers for each message type
    template<typename MsgType>
    void setHandler(std::function<void(const MsgType&)> handler) {
        static_assert((std::is_same_v<MsgType, MsgTypes> || ...),
                     "Handler type must match one of the actor's message types");
        handlers_[TypeRegistry::getTypeID<MsgType>()] = [handler](const Message& msg) {
            handler(static_cast<const MsgType&>(msg));
        };
    }

    void processMessages() override {
#ifdef NO_OS
        while (queueHead_ != queueTail_) {
            auto& entry = messageQueue_[queueHead_];
            entry.handler(entry.context, entry.message);
            delete entry.message;
            queueHead_ = (queueHead_ + 1) % QUEUE_SIZE;
        }
#else
        while (!messageQueue_.empty()) {
            auto& [handler, msg] = messageQueue_.front();
            handler();
            messageQueue_.pop();
        }
#endif
    }

private:
    template<typename MsgType>
    void enqueueMessage(const MsgType& msg) {
        static_assert((std::is_base_of_v<Message, MsgType> && (std::is_same_v<MsgType, MsgTypes> || ...)),
                     "Message type must be derived from Message and match actor's types");
        
        auto typeID = TypeRegistry::getTypeID<MsgType>();
        auto it = handlers_.find(typeID);
        if (it == handlers_.end()) {
            return; // No handler for this type
        }

#ifdef NO_OS
        if ((queueTail_ + 1) % QUEUE_SIZE != queueHead_) {
            auto* msgCopy = new MsgType(msg);
            messageQueue_[queueTail_] = {
                [](void* ctx, const Message* m) {
                    auto* self = static_cast<TypedActor*>(ctx);
                    auto id = m->getTypeID();
                    auto it = self->handlers_.find(id);
                    if (it != self->handlers_.end()) {
                        it->second(*m);
                    }
                },
                this,
                msgCopy
            };
            queueTail_ = (queueTail_ + 1) % QUEUE_SIZE;
        }
#else
        auto msgCopy = std::make_unique<MsgType>(msg);
        messageQueue_.emplace([h = it->second, m = std::move(msgCopy)]() {
            h(*m);
        }, std::move(msgCopy));
#endif
    }

    std::unordered_map<Message::TypeID, std::function<void(const Message&)>> handlers_;
};

// Example message types
struct PositionMsg : public Message {
    float x, y;
    PositionMsg() = default;
    PositionMsg(float x, float y) :x(x),y(y){};
    PositionMsg(const PositionMsg& other) = default;
    Message::TypeID getTypeID() const override {
        return TypeRegistry::getTypeID<PositionMsg>();
    }
    ~PositionMsg() = default;
};

struct VelocityMsg : Message {
    float vx, vy;
    VelocityMsg() = default;
    VelocityMsg(const VelocityMsg& ) = default;
    Message::TypeID getTypeID() const override {
        return TypeRegistry::getTypeID<VelocityMsg>();
    }
};

// Example usage
int main() {
    TypedActor<PositionMsg, VelocityMsg> actor;

    // Set handlers
    actor.setHandler<PositionMsg>([](const PositionMsg& msg) {
        // Handle position update
    });
    actor.setHandler<VelocityMsg>([](const VelocityMsg& msg) {
        // Handle velocity update
    });

    // Get reference for sending position messages
 //   auto posRef = actor.getRef<PositionMsg>();
    
    // Create another actor
    TypedActor<PositionMsg> sender;
 //  sender.setHandler<PositionMsg>([&posRef](const PositionMsg& msg) {
 //       posRef.send(msg); // Forward message
 //   });
    PositionMsg p = PositionMsg(1.0,2.0);
    PositionMsg q =p;
    VelocityMsg v = VelocityMsg();
    VelocityMsg v1 = VelocityMsg(v);
    // Send message
 //   posRef.send(PositionMsg(1.0f,2.0f));

    // Process messages
    actor.processMessages();
    sender.processMessages();

    return 0;
}