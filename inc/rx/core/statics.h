#ifndef RX_CORE_STATICS_H
#define RX_CORE_STATICS_H

#include <rx/core/concepts/no_copy.h> // no_copy
#include <rx/core/concepts/no_move.h> // no_move

#include <rx/core/debug.h> // RX_MESSAGE
#include <rx/core/memory/uninitialized_storage.h> // uninitialized_storage

namespace rx {

struct static_node
  : concepts::no_copy
  , concepts::no_move
{
  template<typename T, typename... Ts>
  constexpr static_node(const char* name, memory::uninitialized_storage<T>& data, Ts&&... args);

  void init();
  void fini();

  const char* name() const;

private:
  friend struct static_globals;
  void link();
  const char *m_name;
  static_node *m_next;
  static_node *m_prev;
  type_eraser m_data;
};

template<typename T, typename... Ts>
inline constexpr static_node::static_node(const char* name, memory::uninitialized_storage<T>& data, Ts&&... args)
  : m_name{name}
  , m_next{nullptr}
  , m_prev{nullptr}
  , m_data{data.type_erase(forward<Ts>(args)...)}
{
  link();
}

inline void static_node::init() {
  RX_MESSAGE("init static global %s\n", m_name);
  m_data.init();
}

inline void static_node::fini() {
  RX_MESSAGE("fini static global %s\n", m_name);
  m_data.fini();
}

inline const char* static_node::name() const {
  return m_name;
}

template<typename T>
struct static_global
  : concepts::no_copy
  , concepts::no_move
{
  template<typename... Ts>
  constexpr static_global(const char* name, Ts&&... args);

  T& operator*();
  T* operator&();
  T* operator->();
  const T& operator*() const;
  const T* operator&() const;
  const T* operator->() const;

  template<typename... Ts>
  auto operator()(Ts&&... args) {
    return (*m_data.data())(forward<Ts>(args)...);
  }

private:
  static_node m_node;
  memory::uninitialized_storage<T> m_data;
};

template<typename T>
template<typename... Ts>
inline constexpr static_global<T>::static_global(const char* name, Ts&&... args)
  : m_node{name, m_data, forward<Ts>(args)...}
{
}

template<typename T>
T& static_global<T>::operator*() {
  return *m_data.data();
}

template<typename T>
T* static_global<T>::operator&() {
  return m_data.data();
}

template<typename T>
T *static_global<T>::operator->() {
  return m_data.data();
}

template<typename T>
const T& static_global<T>::operator*() const {
  return *m_data.data();
}

template<typename T>
const T* static_global<T>::operator&() const {
  return m_data.data();
}

template<typename T>
const T* static_global<T>::operator->() const {
  return m_data.data();
}

struct static_globals {
  static void init();
  static void fini();

  static static_node* find(const char* name);
  static void remove(static_node* node);

  template<typename F>
  static bool each(F&& function);

private:
  // linked-list manipulation
  static void lock();
  static void unlock();

  static static_node* head();
  static static_node* tail();
};

template<typename F>
bool static_globals::each(F&& function) {
  lock();
  for (auto node{head()}; node; node = node->m_next) {
    unlock();
    if (!function(node)) {
      return false;
    }
    lock();
  }
  unlock();
  return true;
}

} // namespace rx

#endif // RX_CORE_STATICS_H
