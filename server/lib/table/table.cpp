#include "table.h"
#include "def.h"
#include "events.h"

#include <server/lib/gameplay/state/defs.h>

#include <array>
#include <unordered_map>
#include <unordered_set>

using namespace NActors;

namespace NTichu::NServer::NTable {
namespace {

using namespace NTichu::NGameplay::NState;

struct TMember {
    TUserId User{INVALID_USER_ID};
};

class TMembership {
public:
    size_t Count() const noexcept {
        size_t count = 0;
        for (const auto& member: Membership_) {
            if (member.User != INVALID_USER_ID) {
                ++count;
            }
        }

        return count;
    }

    bool Complete() const noexcept {
        return Count() == (size_t) EPosition::NUM;
    }

    EPosition UserPosition(TUserId user) const noexcept {
        for (size_t i = 0; i < (size_t) EPosition::NUM; ++i) {
            if (Membership_[i].User == user) {
                return (EPosition) i;
            }
        }

        return EPosition::INVALID;
    }

    TErrorOr<EPosition, std::string> Join(TUserId user) {
        if (Complete()) {
            return std::string("table is full");
        }

        if (ValidatePosition(UserPosition(user))) {
            return UserPosition(user);
        }

        for (size_t i = 0; i < (size_t) EPosition::NUM; ++i) {
            if (Membership_[i].User == INVALID_USER_ID) {
                Membership_[i].User = user;

                return (EPosition) i;
            }
        }

        assert(false && "internal error");
    }

    bool Leave(TUserId user) {
        EPosition pos = UserPosition(user);
        if (!ValidatePosition(pos)) {
            return false;
        }

        Membership_[(size_t) pos].User = INVALID_USER_ID;
        return true;
    }

private:
    std::array<TMember, (size_t) EPosition::NUM> Membership_;
};

class TTableManager {
public:
    TTablePlace JoinAny(TUserId user) {
        TTableMap::iterator it;

        if (Pending_.empty()) {
            ++Tag_;

            TValue value;
            value.Options.Name = "default";

            std::tie(it, std::ignore) = Pending_.emplace(std::make_pair(Tag_, std::move(value)));
        } else {
            it = Pending_.begin();
        }

        return Join(it, user);
    }

    TErrorOr<TTablePlace, std::string> Join(TTableId table, TUserId user) {
        auto it = Pending_.find(table);

        if (it == Pending_.end()) {
            if (Complete_.find(table) != Complete_.end()) {
                return std::string("table is full");
            } else {
                return std::string("table not found");
            }
        }

        return Join(it, user);
    }

    TErrorOr<void, std::string> Leave(TTableId table, TUserId user) {
        auto it = Pending_.find(table);
        if (it != Pending_.end()) {
            it->second.Membership.Leave(user);
            if (it->second.Membership.Count() == 0) {
                Pending_.erase(it);
            }

            return {};
        }

        it = Complete_.find(table);
        if (it != Complete_.end()) {
            it->second.Membership.Leave(user);
            Pending_.emplace(std::make_pair(it->first, std::move(it->second)));
            Complete_.erase(it);

            return {};
        }

        return std::string("table not found");
    }

    TTablePlace Create(TUserId user, TTableOptions options) {
        ++Tag_;

        TValue value;
        value.Options = std::move(options);
        auto [it, _] = Pending_.emplace(std::make_pair(Tag_, std::move(value)));

        return Join(it, user);
    }

    std::vector<TTableListItem> List() const {
        std::vector<TTableListItem> response;
        response.reserve(Pending_.size());

        for (const auto& [key, value]: Pending_) {
            response.emplace_back(TTableListItem{
                .Table = key,
                .Options = value.Options,
                .Joined = value.Membership.Count()
            });
        }

        return response;
    }

private:
    struct TValue {
        TMembership Membership;
        TTableOptions Options;
    };

    using TTableMap = std::unordered_map<TTableId, TValue>;

    ui64 Tag_{0};
    TTableMap Pending_;
    TTableMap Complete_;

private:
    TTablePlace Join(TTableMap::iterator it, TUserId user) {
        auto& value = it->second;
        auto key = it->first;

        auto r = value.Membership.Join(user);

        assert(r.Succeed() && "internal error");

        TTablePlace response{
            .Table = key,
            .Position = r.Value()
        };

        if (value.Membership.Complete()) {
            Complete_.emplace(std::make_pair(key, std::move(value)));
            Pending_.erase(it);
        }

        return response;
    }
};

class TTableManagerActor: public IActor {
public:
    TTableManagerActor(TActorConfig& config)
        : IActor(config)
    {
    }

private:
    void OnJoinAny(TEvJoinAnyRequest::TPtr ev) {
        auto place = Manager_.JoinAny(ev->User);
        ev->Answer(ev->Sender(), MakeEvent<TEvJoinResponse>(place));
        NotifyTables();
    }

    void OnJoin(TEvJoinRequest::TPtr ev) {
        auto response = Manager_.Join(ev->Table, ev->User);
        ev->Answer(ev->Sender(), MakeEvent<TEvJoinResponse>(std::move(response)));
        NotifyTables();
    }

    void OnLeave(TEvLeaveRequest::TPtr ev) {
        auto response = Manager_.Leave(ev->Table, ev->User);
        if (response.Fail()) {
            ev->Answer(ev->Sender(), MakeEvent<TEvLeaveResponse>(response.Error()));
        } else {
            ev->Answer(ev->Sender(), MakeEvent<TEvLeaveResponse>());
        }

        NotifyTables();
    }

    void OnCreate(TEvCreateRequest::TPtr ev) {
        auto response = Manager_.Create(ev->User, ev->Options);
        ev->Answer(ev->Sender(), MakeEvent<TEvCreateResponse>(response));

        NotifyTables();
    }

    void OnSubscribeTables(TEvSubscribeTables::TPtr ev) {
        Watchers_.emplace(ev->Sender());
        Send(ev->Sender(), MakeEvent<TEvTables>(Manager_.List()));
    }

    void OnUnSubscribeTables(TEvUnSubscribeTables::TPtr ev) {
        Watchers_.erase(ev->Sender());
    }

    void NotifyTables() {
        auto tables = Manager_.List();
        for (const auto& actor: Watchers_) {
            Send(actor, MakeEvent<TEvTables>(tables));
        }
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TEvJoinAnyRequest, OnJoinAny);
        CREATE_HANDLER(TEvJoinRequest, OnJoin);
        CREATE_HANDLER(TEvCreateRequest, OnCreate);
        CREATE_HANDLER(TEvLeaveRequest, OnLeave);
        CREATE_HANDLER(TEvSubscribeTables, OnSubscribeTables);
        CREATE_HANDLER(TEvUnSubscribeTables, OnUnSubscribeTables);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    TTableManager Manager_;
    std::unordered_set<TActorId> Watchers_;
};

}

NActors::TActorId CreateTableManager() {
    return TActorSystem::Instance().Spawn<TTableManagerActor>();
}

} // namespace NTichu::NServer