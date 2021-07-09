#include "table.h"

#include <server/lib/flags/flags.h>

#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <array>
#include <memory>
#include <cassert>

using namespace NActors;

namespace NTichu::NServer {
namespace {

using namespace NTableEvent;
using namespace NTichu::NGameplay::NState;

using TTableMember = NActors::TActorId;

class TTableMembership {
public:
    EPosition Join(TTableMember member) {
        if (Complete()) {
            return EPosition::INVALID;
        }

        for (size_t i = 0; i < (size_t) EPosition::NUM; ++i) {
            if (!Members_[i].has_value()) {
                return (EPosition) i;
            }
        }

        return EPosition::INVALID;
    }

    bool Leave(EPosition position) {
        if (!ValidatePosition(position)) {
            return false;
        }

        if (!Members_[(ui32) position].has_value()) {
            return false;
        }

        Members_[(ui32) position].reset();
        return true;
    }

    bool Complete() const {
        return Count() != (size_t) EPosition::NUM;
    }

    size_t Count() const {
        size_t count = 0;
        for (const auto& member: Members_) {
            count += member.has_value();
        }

        return count;
    }

private:
    using TMembers = std::array<std::optional<TTableMember>, (ui32) EPosition::NUM>;

    TMembers Members_;
};

class TTableManager: public IActor {
public:
    explicit TTableManager(TActorConfig& config)
        : IActor(config)
    {
    }

private:
    void NotifyOne(TActorId actor) {
        std::vector<TTableListItem> tables;
        tables.reserve(Pending_.size());

        for (const auto& [key, value]: Pending_) {
            TTableListItem item{
                .TableId = key,
                .Options = value.Options,
                .Count = value.Membership.Count()
            };

            tables.emplace_back(std::move(item));
        }

        Send(actor, MakeEvent<TSubscribeEvent>(std::move(tables)));
    }

    void NotifyAll() {
        for (auto actor: Subscribers_) {
            NotifyOne(actor);
        }
    }

    void OnJoinAny(TJoinAnyRequest::TPtr ev) {
        TTable table;

        if (!Pending_.empty()) {
            auto& value = Pending_.begin()->second;
            auto key = Pending_.begin()->first;

            table.TableId = key;
            table.Position = value.Membership.Join(ev->Sender());

            if (value.Membership.Complete()) {
                Complete_.emplace(std::make_pair(key, std::move(value)));
                Pending_.erase(Pending_.begin());
            }

        } else {
            ++Tag_;

            TTableOptions options {
                .Name = "default"
            };

            TValue value {
                .Options = options
            };

            table.TableId = Tag_;
            table.Position = value.Membership.Join(ev->Sender());
        }

        ev->Answer(Self(), MakeEvent<TJoinTableResponse>(table));

        NotifyAll();
    }

    void OnCreateTable(TCreateTableRequest::TPtr ev) {
        ++Tag_;

        TValue value {
            .Options = ev->Options
        };

        TTable table {
            .TableId = Tag_,
            .Position = value.Membership.Join(ev->Sender())
        };

        Pending_.emplace(std::make_pair(Tag_, std::move(value)));

        ev->Answer(Self(), MakeEvent<TCreateTableResponse>(table));

        NotifyAll();
    }

    void OnJoinTable(TJoinTableRequest::TPtr ev) {
        TTableId taId = ev->Table;

        auto it = Pending_.find(taId);

        if (it == Pending_.end()) {
            TFindTableError error;

            if (Complete_.find(taId) != Complete_.end()) {
                error = TFindTableError{TFindTableError::TABLE_IS_FULL, "table is full"};
            } else {
                error = TFindTableError{TFindTableError::TABLE_NOT_EXISTS, "table doesn't exist"};
            }

            ev->Answer(Self(), MakeEvent<TJoinTableResponse>(std::move(error)));

            return;
        }

        auto& value = it->second;
        auto key = it->first;

        TTable table {
            .TableId = key,
            .Position = value.Membership.Join(ev->Sender())
        };

        if (value.Membership.Complete()) {
            Complete_.emplace(std::make_pair(key, std::move(value)));
            Pending_.erase(it);
        }

        ev->Answer(Self(), MakeEvent<TJoinTableResponse>(table));

        NotifyAll();
    }

    void OnSubscribe(TSubscribeRequest::TPtr ev) {
        Subscribers_.emplace(ev->Sender());
    }

    void OnUnSubscribe(TUnSubscribeRequest::TPtr ev) {
        Subscribers_.erase(ev->Sender());
    }

    void OnLeaveTable(TLeaveTableRequest::TPtr ev) {
        auto table = ev->Table;

        bool complete = false;

        auto it = Pending_.find(table.TableId);
        if (it == Pending_.end()) {
            it = Complete_.find(table.TableId);

            if (it == Complete_.end()) {
                auto error = TFindTableError{TFindTableError::TABLE_NOT_EXISTS, "user is not member of the table"};
                ev->Answer(Self(), MakeEvent<TLeaveTableResponse>(std::move(error)));
                return;
            }
        }

        bool suc = it->second.Membership.Leave(table.Position);

        if (suc) {
            if (complete) {
                TValue& value = it->second;
                Pending_.emplace(std::make_pair(table.TableId, std::move(value)));
                Complete_.erase(it);
            }

            ev->Answer(Self(), MakeEvent<TLeaveTableResponse>(table));
            NotifyAll();
        } else {
            auto error = TFindTableError{TFindTableError::UNKNOWN, "user is not member of the table"};
            ev->Answer(Self(), MakeEvent<TLeaveTableResponse>(std::move(error)));
            return;
        }
    }

    CREATE_STATE_FUNC(StateWork) {
        CREATE_HANDLER(TJoinAnyRequest, OnJoinAny);
        CREATE_HANDLER(TCreateTableRequest, OnCreateTable);
        CREATE_HANDLER(TJoinTableRequest, OnJoinTable);
        CREATE_HANDLER(TSubscribeRequest, OnSubscribe);
        CREATE_HANDLER(TUnSubscribeRequest, OnUnSubscribe);
        CREATE_HANDLER(TLeaveTableRequest, OnLeaveTable);
    }

    THandler Bootstrap() override {
        return AS_STATE_FUNC(StateWork);
    }

private:
    struct TValue {
        TTableOptions Options;
        TTableMembership Membership;
    };

    using TTables = std::unordered_map<TTableId, TValue>;

    TTables Pending_;
    TTables Complete_;

    ui64 Tag_{0};

    std::unordered_set<TActorId> Subscribers_;
};

}

NActors::TActorId CreateTableManager() {
    return TActorSystem::Instance().Spawn<TTableManager>();
}

} // namespace NTichu::NServer