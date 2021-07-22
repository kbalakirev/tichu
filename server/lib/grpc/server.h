#pragma once

#include <grpcpp/grpcpp.h>
#include <google/protobuf/message.h>

#include <server/lib/actors/actor.h>

#include <memory>
#include <atomic>
#include <functional>
#include <cassert>
#include <type_traits>

namespace NGrpc {

using TGrpcMessage = google::protobuf::Message;

NEW_EVENT(TEvLostConnection) {
};

enum class ERpcMode {
    STREAMING_READ_WRITE,
    STREAMING_WRITE,
    STREAMING_READ,
    NO_STREAMING
};

template <typename T>
struct TSignature {
    static_assert(!std::is_same_v<T, T>, "not an async handler");
};

template <class Service, class Request, class Response>
struct TSignature<
    void (Service::*const)(grpc::ServerContext*, Request*, grpc::ServerAsyncResponseWriter<Response>*, grpc::CompletionQueue*, grpc::ServerCompletionQueue*, void*)
>
{
    using TRequest = Request;
    using TResponse = Response;
    using TResponder = grpc::ServerAsyncResponseWriter<Response>;

    static constexpr const ERpcMode RpcMode = ERpcMode::NO_STREAMING;
};

template <class Service, class Request, class Response>
struct TSignature<
    void (Service::*const)(grpc::ServerContext*, Request*, grpc::ServerAsyncWriter<Response>*, grpc::CompletionQueue*, grpc::ServerCompletionQueue*, void*)
>
{
    using TRequest = Request;
    using TResponse = Response;
    using TResponder = grpc::ServerAsyncWriter<Response>;

    static constexpr const ERpcMode RpcMode = ERpcMode::STREAMING_WRITE;
};

#define REGISTER_RPC_HANDLER(Service, Rpc) \
struct T##Rpc##Handler { \
    using TService = Service::AsyncService; \
    static constexpr const auto Handle = &TService::Request##Rpc;   \
    static constexpr const auto RpcMode = ::NGrpc::TSignature<decltype(Handle)>::RpcMode;   \
    using TRequest = ::NGrpc::TSignature<decltype(Handle)>::TRequest; \
    using TResponse = ::NGrpc::TSignature<decltype(Handle)>::TResponse; \
    using TResponder = ::NGrpc::TSignature<decltype(Handle)>::TResponder; \
}

template <ERpcMode mode, class TRequest, class TResponse>
struct TGrpcReaderWriter {
};

template <class TRequest, class TResponse>
struct TGrpcReaderWriter<ERpcMode::STREAMING_WRITE, TRequest, TResponse> {
public:
    using TBase = grpc::ServerAsyncWriter<TResponse>;

    explicit TGrpcReaderWriter(grpc::ServerContext* ctx)
        : Base_(ctx)
    {
    }

    TBase* Get() {
        return &Base_;
    }

    void Write(const TResponse& message, void* tag) {
        Base_.Write(message, tag);
    }

    void Finish(const TResponse& message, grpc::Status status, void* tag) {
        Base_.WriteAndFinish(message, grpc::WriteOptions(), status, tag);
    }
     
private:
    TBase Base_;
};

template <class TRequest, class TResponse>
struct TGrpcReaderWriter<ERpcMode::NO_STREAMING, TRequest, TResponse> {
public:
    using TBase = grpc::ServerAsyncResponseWriter<TResponse>;

    TGrpcReaderWriter(grpc::ServerContext* ctx)
        : Base_(ctx)
    {
    }

    TBase* Get() {
        return &Base_;
    }

    void Write(const TResponse& message, void* tag) {
        assert(false && "undefined method for NO_STREAMING rpc mode");
    }

    void Finish(const TResponse& message, grpc::Status status, void* tag) {
        Base_.Finish(message, status, tag);
    }
     
private:
    TBase Base_;
};

class TCallData;

class IGrpcContext {
public:
    friend class TCallData;

    explicit IGrpcContext(TCallData* tag);

    virtual ~IGrpcContext() = default;

    void Write(const TGrpcMessage& message);
    void Finish(const TGrpcMessage& message, grpc::Status status);

    virtual TGrpcMessage& GetRequest() = 0;

    void Destroy();

private:
    virtual void WriteImpl(const TGrpcMessage& message) = 0;
    virtual void FinishImpl(const TGrpcMessage& message, grpc::Status status) = 0;

    virtual void Listen() = 0;

protected:
    TCallData* Tag_;
};

template <class TRpc>
class TRpcContext: public IGrpcContext {
public:
    using TBase = IGrpcContext;

    TRpcContext(TCallData* tag, TRpc::TService* service, grpc::ServerCompletionQueue* cq)
        : TBase(tag)
        , Writer_(&Ctx_)
        , Service_(service)
        , CQ_(cq)
    {
    }

    TRpc::TRequest& GetRequest() override {
        return Request_;
    }

private:

    void WriteImpl(const TGrpcMessage& message) override {
        const typename TRpc::TResponse* response = dynamic_cast<const typename TRpc::TResponse*>(&message);
        assert(response && "message type mismatch");
        Writer_.Write(*response, Tag_);
    }

    void FinishImpl(const TGrpcMessage& message, grpc::Status status) {
        const typename TRpc::TResponse* response = dynamic_cast<const typename TRpc::TResponse*>(&message);
        assert(response && "message type mismatch");
        Writer_.Finish(*response, status, Tag_);
    }

    void Listen() override {
        (Service_->*TRpc::Handle)(
            &Ctx_,
            &Request_,
            Writer_.Get(),
            CQ_,
            CQ_,
            Tag_
        );
    }

private:
    grpc::ServerContext Ctx_;
    TGrpcReaderWriter<TRpc::RpcMode, typename TRpc::TRequest, typename TRpc::TResponse> Writer_;

    TRpc::TService* Service_;
    grpc::ServerCompletionQueue* CQ_;

    TRpc::TRequest Request_;
};

using IGrpcContextPtr = std::unique_ptr<IGrpcContext>;

using TRpcActorFactory = std::function<NActors::TActorId(IGrpcContext*)>;
using TGrpcContextFactory = std::function<IGrpcContextPtr(TCallData*)>;

template <class TRpc>
TGrpcContextFactory CreateGrpcContextFactory(typename TRpc::TService* service, grpc::ServerCompletionQueue* cq) {
    return [service, cq](TCallData* tag) {
        return std::make_unique<TRpcContext<TRpc>>(tag, service, cq);
    };
}

class TCallData {
public:
    TCallData(TGrpcContextFactory ctxFactory, TRpcActorFactory actorFactory);

    void Proceed(bool ok);

    void OnWrite();
    void OnFinish();

private:
    enum EStatus {
        CREATE,
        REQUEST,
        WRITING,
        PROCESSING,
        FINISH
    };

private:
    EStatus GetStatus() const;

    void ChangeStatus(EStatus status);

private:
    IGrpcContextPtr Ctx_;
    std::atomic<EStatus> Status_;

    TGrpcContextFactory CtxFactory_;
    TRpcActorFactory ActorFactory_;

    NActors::TActorId Actor_;
};

struct TGrpcServerOptions {
    std::string Address;
};

template <class TService>
class TGrpcServer {
public:
    TGrpcServer(TGrpcServerOptions options)
        : Options_(std::move(options))
    {
    }

    ~TGrpcServer() {
        Server_->Shutdown();
        CQ_->Shutdown();
    }

    template <class TRpc>
    void RegisterRpc(TRpcActorFactory factory) {
        new TCallData(
            CreateGrpcContextFactory<TRpc>(&Service_, CQ_.get()),
            std::move(factory)
        );
    }

    void Initialize() {
        grpc::ServerBuilder builder;

        builder.AddListeningPort(Options_.Address, grpc::InsecureServerCredentials());

        builder.RegisterService(&Service_);

        CQ_ = builder.AddCompletionQueue();

        Server_ = builder.BuildAndStart();
    }

    void Run() {
        void* tag;
        bool ok;

        while (true) {
            CQ_->Next(&tag, &ok);
            static_cast<TCallData*>(tag)->Proceed(ok);
        }
    }

private:
    TGrpcServerOptions Options_;

    std::unique_ptr<grpc::ServerCompletionQueue> CQ_;
    TService::AsyncService Service_;
    std::unique_ptr<grpc::Server> Server_;
};

} // namespace NGrpc
