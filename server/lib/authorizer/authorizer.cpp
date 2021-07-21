#include "authorizer.h"
#include "events.h"
#include "def.h"

#include <unordered_map>
#include <sstream>

using namespace NActors;

namespace NTichu::NServer::NAuthorizer {
namespace {

class TAuthorizer: public IActor {
public:
    explicit TAuthorizer(TActorConfig& config)
        : IActor(config)
    {
    }

private:
    void OnRegisterRequest(TEvRegisterRequest::TPtr ev) {
        ++Tag_;

        std::stringstream ss;
        ss << Tag_;

        auto [_, notFound] = AuthTable_.emplace(std::make_pair(ss.str(), Tag_));
        assert(notFound);

        ev->Answer(Self(), MakeEvent<TEvRegisterResponse>(ss.str()));
    }

    void OnAuthRequest(TEvAuthRequest::TPtr ev) {
        auto it = AuthTable_.find(ev->Token);

        if (it == AuthTable_.end()) {
            ev->Answer(Self(), MakeEvent<TEvAuthResponse>(INVALID_USER_ID));
            return;
        }

        const TUserId& userId = it->second;

        ev->Answer(Self(), MakeEvent<TEvAuthResponse>(userId));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TEvRegisterRequest, OnRegisterRequest);
        CREATE_HANDLER(TEvAuthRequest, OnAuthRequest);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    using TTable = std::unordered_map<TAuthToken , TUserId>;
    TTable AuthTable_;

    ui64 Tag_{0};
};

}

NActors::TActorId CreateAuthorizer() {
    return TActorSystem::Instance().Spawn<TAuthorizer>();
}

} // namespace NTichu::NServer::NAuthorizer
