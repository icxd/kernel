//
// Created by icxd on 11/14/24.
//

#pragma once

#include "LibCore/Defines.hpp"
#include "OwnPtr.hpp"
#include "Types.hpp"
#include <LibCpp/cstddef.hpp>

namespace Core {

  template <typename>
  class Function;

  template <typename Ret, typename... Args>
  class Function<Ret(Args...)> {
  public:
    Function() = default;
    Function(std::nullptr_t) {}

    template <
        typename CallableType,
        class = typename EnableIf<
            !(IsPointer<CallableType>::value &&
              IsFunction<typename RemovePointer<CallableType>::Type>::value) &&
            IsRvalueReference<CallableType &&>::value>::Type>
    Function(CallableType &&callable)
        : m_callable_wrapper(
              make<CallableWrapper<CallableType>>(move(callable))) {}

    template <typename FunctionType,
              class = typename EnableIf<IsPointer<FunctionType>::value &&
                                        IsFunction<typename RemovePointer<
                                            FunctionType>::Type>::value>::Type>
    Function(FunctionType callable)
        : m_callable_wrapper(
              make<CallableWrapper<FunctionType>>(move(callable))) {}

    Ret operator()(Args... args) const {
      ASSERT(m_callable_wrapper);
      return m_callable_wrapper->call(forward<Args>(args)...);
    }

    explicit operator bool() const { return !!m_callable_wrapper; }

    template <
        typename CallableType,
        class = typename EnableIf<
            !(IsPointer<CallableType>::value &&
              IsFunction<typename RemovePointer<CallableType>::Type>::value) &&
            IsRvalueReference<CallableType &&>::value>::Type>
    Function &operator=(CallableType &&callable) {
      m_callable_wrapper = make<CallableWrapper<CallableType>>(move(callable));
      return *this;
    }

    template <typename FunctionType,
              class = typename EnableIf<IsPointer<FunctionType>::value &&
                                        IsFunction<typename RemovePointer<
                                            FunctionType>::Type>::value>::Type>
    Function &operator=(FunctionType callable) {
      m_callable_wrapper = make<CallableWrapper<FunctionType>>(move(callable));
      return *this;
    }

    Function &operator=(std::nullptr_t) {
      m_callable_wrapper = nullptr;
      return *this;
    }

  private:
    class CallableWrapperBase {
    public:
      virtual ~CallableWrapperBase() {}
      virtual Ret call(Args...) const = 0;
    };

    template <typename CallableType>
    class CallableWrapper : public CallableWrapperBase {
    public:
      explicit CallableWrapper(CallableType &&callable)
          : m_callable(move(callable)) {}

      CallableWrapper(const CallableWrapper &) = delete;
      CallableWrapper &operator=(const CallableWrapper &) = delete;

    private:
      CallableType m_callable;
    };

    OwnPtr<CallableWrapperBase> m_callable_wrapper;
  };

} // namespace Core
