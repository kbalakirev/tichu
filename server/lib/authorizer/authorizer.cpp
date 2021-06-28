#include "authorizer.h"

#include <unordered_map>

using namespace NActors;

namespace NTichu::NServer {

using namespace NEvAuthorizer;

class TAuthorizer: public IActor {
public:
    explicit TAuthorizer(TActorConfig& config)
        : IActor(config)
    {
        auto now = std::chrono::system_clock::now();
        Tag = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    }

private:
    TAuthToken GenerateToken() {
        ++Initializer;

        TAuthToken token;
        token.resize(sizeof(ui64) + sizeof(ui64));
        std::memcpy(token.data(), &Tag, sizeof(ui64));
        std::memcpy(token.data() + sizeof(ui64), &Initializer, sizeof(ui64));

        return token;
    }

    void OnProxyReq(TProxyReq::TPtr ev) {
        auto ptr = AuthTable_.find(ev->Token);

        if (ptr == AuthTable_.end()) {
            Send(ev->Sender(), MakeEvent<TProxyError>("invalid auth token"));
            return;
        }

        TActorSystem::Send(ev->Sender(), ev->To, ev->Event);
    }

    void OnRegisterReq(TRegisterReq::TPtr ev) {
        auto token = GenerateToken();
        TUserId userId = Initializer;

        AuthTable_.emplace(std::make_pair(token, userId));

        Send(ev->Sender(), MakeEvent<TRegisterResp>(token));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TProxyReq, OnProxyReq);
        CREATE_HANDLER(TRegisterReq, OnRegisterReq);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    using TTable = std::unordered_map<TAuthToken , TUserId>;
    TTable AuthTable_;

    ui64 Initializer{0};
    ui64 Tag{0};
};

NActors::TActorId CreateAuthorizer() {
    return TActorSystem::Instance().Spawn<TAuthorizer>();
}

} // namespace NTichu::NServer
