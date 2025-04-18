//@HEADER
// ************************************************************************
//
//                        Kokkos v. 4.0
//       Copyright (2022) National Technology & Engineering
//               Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Part of Kokkos, under the Apache License v2.0 with LLVM Exceptions.
// See https://kokkos.org/LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//@HEADER

#include <Kokkos_Macros.hpp>

#ifndef KOKKOS_IMPL_PUBLIC_INCLUDE
static_assert(false,
              "Including non-public Kokkos header files is not allowed.");
#endif

#ifndef KOKKOS_ENABLE_DEPRECATED_CODE_4
#error "The tasking framework is deprecated"
#endif

#ifndef KOKKOS_FUTURE_HPP
#define KOKKOS_FUTURE_HPP

//----------------------------------------------------------------------------

#include <Kokkos_Macros.hpp>
#if defined(KOKKOS_ENABLE_TASKDAG)

#include <Kokkos_Core_fwd.hpp>
#include <Kokkos_TaskScheduler_fwd.hpp>
//----------------------------------------------------------------------------

#include <impl/Kokkos_TaskQueue.hpp>
#include <impl/Kokkos_TaskResult.hpp>
#include <impl/Kokkos_TaskBase.hpp>
#include <Kokkos_Atomic.hpp>

#include <Kokkos_Concepts.hpp>  // is_space

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#ifdef KOKKOS_ENABLE_DEPRECATION_WARNINGS
// We allow using deprecated classes in this file
KOKKOS_IMPL_DISABLE_DEPRECATED_WARNINGS_PUSH()
#endif

namespace Kokkos {

// For now, hack this in as a partial specialization
// TODO @tasking @cleanup Make this the "normal" class template and make the old
// code the specialization
template <typename ValueType, typename ExecutionSpace, typename QueueType>
class KOKKOS_DEPRECATED
    BasicFuture<ValueType, SimpleTaskScheduler<ExecutionSpace, QueueType>> {
 public:
  using value_type      = ValueType;
  using execution_space = ExecutionSpace;
  using scheduler_type  = SimpleTaskScheduler<ExecutionSpace, QueueType>;
  using queue_type      = typename scheduler_type::task_queue_type;

 private:
  template <class, class>
  friend class SimpleTaskScheduler;
  template <class, class>
  friend class BasicFuture;

  using task_base_type  = typename scheduler_type::task_base_type;
  using task_queue_type = typename scheduler_type::task_queue_type;

  using task_queue_traits = typename scheduler_type::task_queue_traits;
  using task_scheduling_info_type =
      typename scheduler_type::task_scheduling_info_type;

  using result_storage_type = Impl::TaskResultStorage<
      ValueType,
      Impl::SchedulingInfoStorage<Impl::RunnableTaskBase<task_queue_traits>,
                                  task_scheduling_info_type>>;

  OwningRawPtr<task_base_type> m_task = nullptr;

  KOKKOS_INLINE_FUNCTION
  explicit BasicFuture(task_base_type* task) : m_task(task) {
    // Note: reference count starts at 2 to account for initial increment
    // TODO @tasking @minor DSH verify reference count here and/or encapsulate
    // starting reference count closer to here
  }

 public:
  KOKKOS_INLINE_FUNCTION
  BasicFuture() noexcept : m_task(nullptr) {}

  KOKKOS_INLINE_FUNCTION
  BasicFuture(BasicFuture&& rhs) noexcept : m_task(std::move(rhs.m_task)) {
    rhs.m_task = nullptr;
  }

  KOKKOS_INLINE_FUNCTION
  BasicFuture(BasicFuture const& rhs)
      //  : m_task(rhs.m_task)
      : m_task(nullptr) {
    *static_cast<task_base_type* volatile*>(&m_task) = rhs.m_task;
    if (m_task) m_task->increment_reference_count();
  }

  KOKKOS_INLINE_FUNCTION
  BasicFuture& operator=(BasicFuture&& rhs) noexcept {
    if (m_task != rhs.m_task) {
      clear();
      // m_task = std::move(rhs.m_task);
      *static_cast<task_base_type* volatile*>(&m_task) = rhs.m_task;
      // rhs.m_task reference count is unchanged, since this is a move
    } else {
      // They're the same, but this is a move, so 1 fewer references now
      rhs.clear();
    }
    rhs.m_task = nullptr;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION
  BasicFuture& operator=(BasicFuture const& rhs) {
    if (m_task != rhs.m_task) {
      clear();
      // m_task = rhs.m_task;
      *static_cast<task_base_type* volatile*>(&m_task) = rhs.m_task;
    }
    if (m_task != nullptr) {
      m_task->increment_reference_count();
    }
    return *this;
  }

  //----------------------------------------

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture(
      BasicFuture<T, S>&& rhs) noexcept  // NOLINT(google-explicit-constructor)
      : m_task(std::move(rhs.m_task)) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Moved Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Moved Futures must have the same value_type");

    // reference counts are unchanged, since this is a move
    rhs.m_task = nullptr;
  }

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture(
      BasicFuture<T, S> const& rhs)  // NOLINT(google-explicit-constructor)
                                     //: m_task(rhs.m_task)
      : m_task(nullptr) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Copied Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Copied Futures must have the same value_type");

    *static_cast<task_base_type* volatile*>(&m_task) = rhs.m_task;
    if (m_task) m_task->increment_reference_count();
  }

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture& operator=(BasicFuture<T, S> const& rhs) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Assigned Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Assigned Futures must have the same value_type");

    if (m_task != rhs.m_task) {
      clear();
      // m_task = rhs.m_task;
      *static_cast<task_base_type* volatile*>(&m_task) = rhs.m_task;
      if (m_task != nullptr) {
        m_task->increment_reference_count();
      }
    }
    return *this;
  }

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture& operator=(BasicFuture<T, S>&& rhs) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Assigned Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Assigned Futures must have the same value_type");

    if (m_task != rhs.m_task) {
      clear();
      // m_task = std::move(rhs.m_task);
      *static_cast<task_base_type* volatile*>(&m_task) = rhs.m_task;
      // rhs.m_task reference count is unchanged, since this is a move
    } else {
      // They're the same, but this is a move, so 1 fewer references now
      rhs.clear();
    }
    rhs.m_task = nullptr;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION
  ~BasicFuture() noexcept { clear(); }

  //----------------------------------------

  KOKKOS_INLINE_FUNCTION
  void clear() noexcept {
    if (m_task) {
      bool should_delete = m_task->decrement_and_check_reference_count();
      if (should_delete) {
        static_cast<task_queue_type*>(m_task->ready_queue_base_ptr())
            ->deallocate(std::move(*m_task));
      }
    }
    // m_task = nullptr;
    *static_cast<task_base_type* volatile*>(&m_task) = nullptr;
  }

  KOKKOS_INLINE_FUNCTION
  bool is_null() const noexcept { return m_task == nullptr; }

  KOKKOS_INLINE_FUNCTION
  bool is_ready() const noexcept {
    return (m_task == nullptr) || m_task->wait_queue_is_consumed();
  }

  KOKKOS_INLINE_FUNCTION
  const typename Impl::TaskResult<ValueType>::reference_type get() const {
    KOKKOS_EXPECTS(is_ready());
    return static_cast<result_storage_type*>(m_task)->value_reference();
    // return Impl::TaskResult<ValueType>::get(m_task);
  }
};

////////////////////////////////////////////////////////////////////////////////
// OLD CODE
////////////////////////////////////////////////////////////////////////////////

template <typename ValueType, typename Scheduler>
class KOKKOS_DEPRECATED BasicFuture {
 private:
  template <typename, typename>
  friend class BasicTaskScheduler;
  template <typename, typename>
  friend class BasicFuture;
  friend class Impl::TaskBase;
  template <typename, typename, typename>
  friend class Impl::Task;

  //----------------------------------------

 public:
  //----------------------------------------

  using scheduler_type  = Scheduler;
  using queue_type      = typename scheduler_type::queue_type;
  using execution_space = typename scheduler_type::execution_space;
  using value_type      = ValueType;

  //----------------------------------------

 private:
  //----------------------------------------

  using task_base = Impl::TaskBase;

  task_base* m_task;

  KOKKOS_INLINE_FUNCTION explicit BasicFuture(task_base* task)
      : m_task(nullptr) {
    if (task) queue_type::assign(&m_task, task);
  }

  //----------------------------------------

 public:
  //----------------------------------------

  KOKKOS_INLINE_FUNCTION
  bool is_null() const { return nullptr == m_task; }

  KOKKOS_INLINE_FUNCTION
  int reference_count() const {
    return nullptr != m_task ? m_task->reference_count() : 0;
  }

  //----------------------------------------

  KOKKOS_INLINE_FUNCTION
  void clear() {
    if (m_task) queue_type::assign(&m_task, nullptr);
  }

  //----------------------------------------

  KOKKOS_INLINE_FUNCTION
  ~BasicFuture() { clear(); }

  //----------------------------------------

  KOKKOS_INLINE_FUNCTION
  BasicFuture() noexcept : m_task(nullptr) {}

  KOKKOS_INLINE_FUNCTION
  BasicFuture(BasicFuture&& rhs) noexcept : m_task(rhs.m_task) {
    rhs.m_task = nullptr;
  }

  KOKKOS_INLINE_FUNCTION
  BasicFuture(const BasicFuture& rhs) : m_task(nullptr) {
    if (rhs.m_task) queue_type::assign(&m_task, rhs.m_task);
  }

  KOKKOS_INLINE_FUNCTION
  BasicFuture& operator=(BasicFuture&& rhs) noexcept {
    clear();
    m_task     = rhs.m_task;
    rhs.m_task = nullptr;
    return *this;
  }

  KOKKOS_INLINE_FUNCTION
  BasicFuture& operator=(BasicFuture const& rhs) {
    if (&rhs == this) return *this;
    if (m_task || rhs.m_task) queue_type::assign(&m_task, rhs.m_task);
    return *this;
  }

  //----------------------------------------

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture(
      BasicFuture<T, S>&& rhs) noexcept  // NOLINT(google-explicit-constructor)
      : m_task(rhs.m_task) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Assigned Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Assigned Futures must have the same value_type");

    rhs.m_task = 0;
  }

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture(
      BasicFuture<T, S> const& rhs)  // NOLINT(google-explicit-constructor)
      : m_task(nullptr) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Assigned Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Assigned Futures must have the same value_type");

    if (rhs.m_task) queue_type::assign(&m_task, rhs.m_task);
  }

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture& operator=(BasicFuture<T, S> const& rhs) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Assigned Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Assigned Futures must have the same value_type");

    if (m_task || rhs.m_task) queue_type::assign(&m_task, rhs.m_task);
    return *this;
  }

  template <class T, class S>
  KOKKOS_INLINE_FUNCTION BasicFuture& operator=(BasicFuture<T, S>&& rhs) {
    static_assert(
        std::is_void_v<scheduler_type> || std::is_same_v<scheduler_type, S>,
        "Assigned Futures must have the same scheduler");

    static_assert(std::is_void_v<value_type> || std::is_same_v<value_type, T>,
                  "Assigned Futures must have the same value_type");

    clear();
    m_task     = rhs.m_task;
    rhs.m_task = 0;
    return *this;
  }

  //----------------------------------------

  KOKKOS_INLINE_FUNCTION
  int is_ready() const noexcept {
    return (nullptr == m_task) ||
           (reinterpret_cast<task_base*>(task_base::LockTag) == m_task->m_wait);
  }

  KOKKOS_INLINE_FUNCTION
  const typename Impl::TaskResult<ValueType>::reference_type get() const {
    if (nullptr == m_task) {
      Kokkos::abort("Kokkos:::Future::get ERROR: is_null()");
    }
    return Impl::TaskResult<ValueType>::get(m_task);
  }
};

// Is a Future with the given execution space
template <typename, typename ExecSpace = void>
struct KOKKOS_DEPRECATED is_future : public std::false_type {};

template <typename ValueType, typename Scheduler, typename ExecSpace>
struct KOKKOS_DEPRECATED is_future<BasicFuture<ValueType, Scheduler>, ExecSpace>
    : std::bool_constant<
          std::is_same_v<ExecSpace, typename Scheduler::execution_space> ||
          std::is_void_v<ExecSpace>> {};

////////////////////////////////////////////////////////////////////////////////
// END OLD CODE
////////////////////////////////////////////////////////////////////////////////

namespace Impl {

template <class Arg1, class Arg2>
class ResolveFutureArgOrder {
 private:
  enum { Arg1_is_space = Kokkos::is_space<Arg1>::value };
  enum { Arg2_is_space = Kokkos::is_space<Arg2>::value };
  enum { Arg1_is_value = !Arg1_is_space && !std::is_void_v<Arg1> };
  enum { Arg2_is_value = !Arg2_is_space && !std::is_void_v<Arg2> };

  static_assert(!(Arg1_is_space && Arg2_is_space),
                "Future cannot be given two spaces");

  static_assert(!(Arg1_is_value && Arg2_is_value),
                "Future cannot be given two value types");

  using value_type =
      std::conditional_t<Arg1_is_value, Arg1,
                         std::conditional_t<Arg2_is_value, Arg2, void>>;

  using execution_space = typename std::conditional_t<
      Arg1_is_space, Arg1,
      std::conditional_t<Arg2_is_space, Arg2, void>>::execution_space;

 public:
  using type = BasicFuture<value_type, TaskScheduler<execution_space>>;
};

}  // end namespace Impl

/**
 *
 *  Future< space >  // value_type == void
 *  Future< value >  // space == Default
 *  Future< value , space >
 *
 */
template <class Arg1 = void, class Arg2 = void>
using Future KOKKOS_DEPRECATED =
    typename Impl::ResolveFutureArgOrder<Arg1, Arg2>::type;

}  // namespace Kokkos

#ifdef KOKKOS_ENABLE_DEPRECATION_WARNINGS
KOKKOS_IMPL_DISABLE_DEPRECATED_WARNINGS_POP()
#endif

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#endif /* #if defined( KOKKOS_ENABLE_TASKDAG ) */
#endif /* #ifndef KOKKOS_FUTURE */
