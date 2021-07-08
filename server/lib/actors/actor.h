#pragma once

#include <caf/all.hpp>

#include <server/lib/types/types.h>

#include <memory>
#include <chrono>
#include <vector>
#include <type_traits>
#include <functional>

#define CREATE_STATE_FUNC(Name) \
    void Name(TEventPtr ev)

#define CREATE_HANDLER(TEvent, Handler) \
    do { \
        auto casted = std::dynamic_pointer_cast<TEvent>(ev); \
        if (casted) { \
            Handler(std::move(casted)); \
            return; \
        } \
    } while (false)

#define AS_STATE_FUNC(Name) \
    [this](TEventPtr ev) { Name(std::move(ev)); }

#define NEW_EVENT(TType) \
    struct TType: public NActors::TEvent< TType >

namespace NActors {

using TDuration = std::chrono::duration<ui64>;

class TActorSystem;
class IActor;

class TActorId {
public:
    TActorId() = default;

private:
    friend class IActor;
    friend class TActorSystem;

    template <class TActorType>
    TActorId(TActorType actor)
        : Actor_(caf::actor_cast<caf::weak_actor_ptr>(actor))
    {
    }

    caf::weak_actor_ptr Actor_;
};

class TEventBase;
using TEventPtr = std::shared_ptr<TEventBase>;

class TEventBase {
    friend class IActor;
    friend class TActorSystem;

    template <class TEvent, class... TArgs>
    friend TEventPtr MakeEvent(TArgs... args);

public:
    virtual ~TEventBase() = default;

    TActorId Sender() const;

private:
    TActorId Sender_;
};

template <class T>
class TEvent: public TEventBase {
public:
    using TPtr = std::shared_ptr<T>;
};

template <class TEvent, class... TArgs>
TEventPtr MakeEvent(TArgs... args) {
    static_assert(std::is_base_of_v<TEventBase, TEvent>);
    return std::make_shared<TEvent>(std::forward<TArgs>(args)...);
}

using TActorConfig = caf::actor_config;

class IActor: public caf::event_based_actor {
protected:
    using TBase = caf::event_based_actor;
    using THandler = std::function<void(TEventPtr)>;

    explicit IActor(TActorConfig& config);

    void Send(TActorId to, TEventPtr ev);
    void Schedule(TActorId to, TEventPtr ev, TDuration delay);

    virtual THandler Bootstrap() = 0;

    void Become(THandler handler);

    void Quit();

    TActorId Self() const;

    TActorSystem System();

private:
    caf::behavior make_behavior() override;

    caf::actor* Self_;
};

class TActorSystem {
public:
    explicit TActorSystem(caf::actor_system& sys);

    template <class TActor, class... TArgs>
    TActorId Spawn(TArgs... args);

    static void Send(TActorId from, TActorId to, TEventPtr ev);

    static TActorSystem Instance(caf::actor_system* sys = nullptr) {
        static TActorSystem system(*sys);
        return system;
    }

private:
    caf::actor_system& Sys_;
};

template <class TActor, class... TArgs>
TActorId TActorSystem::Spawn(TArgs... args) {
    return Sys_.spawn<TActor>(std::forward<TArgs>(args)...);
}

} // namespace NActors

CAF_BEGIN_TYPE_ID_BLOCK(BASE, caf::first_custom_type_id)

    CAF_ADD_TYPE_ID(BASE, (NActors::TEventPtr))

CAF_END_TYPE_ID_BLOCK(BASE)

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(NActors::TEventPtr)

#define ACTORS_MAIN() \
    void caf_main(caf::actor_system& sys) { \
        TActorSystem::Instance(&sys); \
        ActorsMain(); \
    } \
    CAF_MAIN(caf::id_block::BASE)
