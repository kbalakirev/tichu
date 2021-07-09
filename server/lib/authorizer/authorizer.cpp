#include "authorizer.h"

#include <unordered_map>
#include <sstream>

using namespace NActors;

namespace NTichu::NServer {

using namespace NAuthorizerEvent;

class TAuthorizer: public IActor {
public:
    explicit TAuthorizer(TActorConfig& config)
        : IActor(config)
    {
    }

private:
    void OnRegisterRequest(TRegisterRequest::TPtr ev) {
        ++Tag_;

        std::stringstream ss;
        ss << Tag_;

        auto [_, found] = AuthTable_.emplace(std::make_pair(ss.str(), Tag_));
        assert(!found);

        ev->Answer(Self(), MakeEvent<TRegisterResponse>(ss.str()));
    }

    void OnAuthRequest(TAuthRequest::TPtr ev) {
        auto it = AuthTable_.find(ev->Token);

        if (it == AuthTable_.end()) {
            ev->Answer(Self(), MakeEvent<TAuthResponse>(INVALID_USER_ID));
            return;
        }

        const TUserId& userId = it->second;

        ev->Answer(Self(), MakeEvent<TAuthResponse>(userId));
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TRegisterRequest, OnRegisterRequest);
        CREATE_HANDLER(TAuthRequest, OnAuthRequest);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    using TTable = std::unordered_map<TAuthToken , TUserId>;
    TTable AuthTable_;

    ui64 Tag_{0};
};

NActors::TActorId CreateAuthorizer() {
    return TActorSystem::Instance().Spawn<TAuthorizer>();
}

} // namespace NTichu::NServer
