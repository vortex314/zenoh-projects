#include <cstdint>
#include <array>
#include <cassert>

// CRTP base class for messages
template <typename Derived>
struct Message {
    Derived& asDerived() { return static_cast<Derived&>(*this); }
    const Derived& asDerived() const { return static_cast<const Derived&>(*this); }
};

// Specific message types
struct StartMessage : Message<StartMessage> {
    uint32_t priority;
    StartMessage(uint32_t p) : priority(p) {}
};

struct StopMessage : Message<StopMessage> {
    bool force;
    StopMessage(bool f) : force(f) {}
};

struct DataMessage : Message<DataMessage> {
    int32_t value;
    DataMessage(int32_t v) : value(v) {}
};

// CRTP base class for actors
template <typename Derived>
class Actor {
public:
    template <typename MessageType>
    void process(const MessageType& msg) {
        static_cast<Derived*>(this)->handle(msg.asDerived());
    }
};

// Concrete actor implementation
class MyActor : public Actor<MyActor> {
public:
    void handle(const StartMessage& msg) {
        // Example: Start a peripheral (e.g., enable a motor)
        // Use msg.priority
    }

    void handle(const StopMessage& msg) {
        // Example: Stop a peripheral
        // Use msg.force
    }

    void handle(const DataMessage& msg) {
        // Example: Process sensor data
        // Use msg.value
    }
};


template <typename ActorType>
// Generic message queue
class MessageQueue {
    // Abstract message handler (type-erased dispatcher)
    class MessageHandler {
    public:
        virtual void dispatch(ActorType& actor) const = 0;
        virtual ~MessageHandler() = default;
    };

    // Concrete handler for specific message types
    template <typename T>
    class ConcreteMessageHandler : public MessageHandler {
        T msg;
    public:
        ConcreteMessageHandler(const T& m) : msg(m) {}
        void dispatch(ActorType& actor) const override {
            actor.process(msg);
        }
    };

    // Fixed-size queue
    static constexpr size_t QUEUE_SIZE = 10;
    std::array<MessageHandler*, QUEUE_SIZE> queue = {};
    size_t head = 0, tail = 0;

public:
    MessageQueue() = default;
    ~MessageQueue() {
        // Clean up dynamically allocated handlers
        for (size_t i = tail; i < head; ++i) {
            delete queue[i];
        }
    }

    // Push a message into the queue
    template <typename T>
    bool push(const T& msg) {
        if (head >= QUEUE_SIZE) {
            return false; // Queue full
        }
        queue[head++] = new ConcreteMessageHandler<T>(msg); // Allocate handler
        return true;
    }

    // Dispatch all messages to the actor
    void dispatch(MyActor& actor) {
        while (tail < head) {
            queue[tail]->dispatch(actor);
            delete queue[tail];
            queue[tail++] = nullptr;
        }
        head = tail = 0; // Reset queue
    }
};

// Example usage
void runActorWithGenericQueue() {
    MyActor actor;
    MessageQueue<MyActor> queue;

    // Push different message types
    queue.push(StartMessage(1));
    queue.push(DataMessage(42));
    queue.push(StopMessage(true));

    // Dispatch all messages
    queue.dispatch(actor); // Calls MyActor::handle for each message
}