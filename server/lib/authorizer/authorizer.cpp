#include "authorizer.h"

#include <unordered_map>

using namespace NActors;

namespace NTichu::NServer {

class TAuthorizer: public IActor {
public:
    explicit TAuthorizer(TActorConfig& config, TWeakActor userManager)
        : IActor(config)
        , UserManager_(std::move(userManager))
    {
        auto now = std::chrono::system_clock::now();
        Tag = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    }

private:
    TAuthToken GenerateToken() {
        ++Initializer;
        return TAuthToken{Initializer, Tag};
    }

    void OnProxyReq(TAuthEv::TProxyReq::TPtr event) {
        auto ptr = AuthTable_.find(event->Token);

        if (ptr == AuthTable_.end()) {
            Send(event->Sender(), MakeEvent<TAuthEv::TProxyError>("invalid auth token"));
            return;
        }

        TActorSystem::Send(
                event->Sender(),
                UserManager_,
                MakeEvent<TUserEv::TProxy>(ptr->second, std::move(event->Event))
            );
    }

    void OnRegisterReq(TAuthEv::TRegisterReq::TPtr ev) {
        auto token = GenerateToken();
        auto userId = TUserId {
            .Identity = token.Identity,
            .Tag = token.Tag
        };

        AuthTable_.emplace(std::make_pair(token, userId));

        Send(ev->Sender(), MakeEvent<TAuthEv::TRegisterResp>(token));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TAuthEv::TProxyReq, OnProxyReq);
        CREATE_HANDLER(TAuthEv::TRegisterReq, OnRegisterReq);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TWeakActor UserManager_;
    std::unordered_map<TAuthToken, TUserId> AuthTable_;

    ui64 Initializer{0};
    ui64 Tag{0};
};

NActors::TWeakActor CreateAuthorizer(NActors::TActorSystem sys, NActors::TWeakActor userManager) {
    return sys.Spawn<TAuthorizer>(std::move(std::move(userManager)));
}

} // namespace NTichu::NServer
