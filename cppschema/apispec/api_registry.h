#pragma once

#include <cassert>
#include <map>
#include <string>
#include <functional>

namespace cppschema {

template <typename API>
class ApiRegistry {
public:
    // A type-erased wrapper that handles the conversion internally
    using RawDispatcher = std::function<void*(const void*)>;
    using InstanceDeleter = std::function<void(void*)>;

    static ApiRegistry& Get() {
        static ApiRegistry instance;
        return instance;
    }

    // Deletes current backend method dispatchers. Used for cleanup and re-registration.
    void Clear() {
        dispatchers_.clear();
        if (backend_instance_ && deleter_) {
            deleter_(backend_instance_);
        }
        backend_instance_ = nullptr;
        deleter_ = nullptr;
    }

    // Registers the backend instance and its lifecycle management
    void SetBackend(void* instance, InstanceDeleter deleter) {
        assert(instance != nullptr && instance != backend_instance_ &&
            "Backend instance must be non-null and different from current");
        Clear(); // Ensure previous state is wiped
        backend_instance_ = instance;
        deleter_ = std::move(deleter);
    }

    void RegisterHandler(const std::string& name, RawDispatcher func) {
        dispatchers_[name] = std::move(func);
    }

    // This is called by the Dispatcher
    template <typename Req, typename Res>
    Res Call(const std::string& name, const Req& req) {
        assert(backend_instance_ != nullptr && "Backend not set");
        auto it = dispatchers_.find(name);
        assert(it != dispatchers_.end() && "Method not implemented");
        void* rawRes = it->second(static_cast<const void*>(&req));
        // Cast back to the expected type and clean up the heap-allocated response
        Res* typedRes = static_cast<Res*>(rawRes);
        Res finalRes = std::move(*typedRes);
        delete typedRes; 
        return finalRes;
    }

    ~ApiRegistry() {
        Clear();
    }

private:
    // No public instance creation; only the singleton instance is allowed
    ApiRegistry() = default;

    // Deleted copy/move constructors and assignment operators to enforce singleton pattern
    ApiRegistry(const ApiRegistry&) = delete;
    ApiRegistry& operator=(const ApiRegistry&) = delete;
    ApiRegistry(ApiRegistry&&) = delete;
    ApiRegistry& operator=(ApiRegistry&&) = delete;

    // Internal storage for method dispatchers.
    std::map<std::string, RawDispatcher> dispatchers_;

    // Backend instance and its deleter for lifecycle management
    void* backend_instance_ = nullptr;
    InstanceDeleter deleter_ = nullptr;
};

/**
 * RAII class to register a backend and ensure it is cleared 
 * when the scope is exited.
 */
template <typename API, typename Impl>
class ScopedRegister {
public:
    // Takes ownership of the instance and the pointer struct
    ScopedRegister(Impl* instance, const typename API::template ImplPtrs<Impl>& ptrs) {
        RegisterBackend<API, Impl>(instance, ptrs);
    }

    // Explicitly disallow copying and moving.
    ScopedRegister(const ScopedRegister&) = delete;
    ScopedRegister& operator=(const ScopedRegister&) = delete;
    ScopedRegister(ScopedRegister&&) = delete;
    ScopedRegister& operator=(ScopedRegister&&) = delete;

    ~ScopedRegister() {
        // Automatically wipes the registry and deletes the instance
        ApiRegistry<API>::Get().Clear();
    }
};

}  // namespace cppschema
