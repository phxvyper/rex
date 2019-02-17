#include <string.h> // strcmp

#include <rx/core/statics.h>

#include <rx/core/debug.h> // RX_MESSAGE
#include <rx/core/concurrency/spin_lock.h> // spin_lock
#include <rx/core/concurrency/scope_lock.h> // scope_lock

namespace rx {

using lock_guard = concurrency::scope_lock<concurrency::spin_lock>;

static concurrency::spin_lock g_lock;

static static_node* g_head; // protected by |g_lock|
static static_node* g_tail; // protected by |g_lock|

void static_node::link() {
  lock_guard locked(g_lock);

  if (!g_head) {
    g_head = this;
  }

  if (g_tail) {
    m_prev = g_tail;
    m_prev->m_next = this;
  }

  g_tail = this;
}

void static_globals::init() {
  lock_guard locked(g_lock);
  for (auto node{g_head}; node; node = node->m_next) {
    RX_MESSAGE("init static global %s\n", node->m_name);
    node->init();
  }
}

void static_globals::fini() {
  lock_guard locked(g_lock);
  for (auto node{g_tail}; node; node = node->m_prev) {
    RX_MESSAGE("fini static global %s\n", node->m_name);
    node->fini();
  }
}

static_node* static_globals::find(const char* name) {
  lock_guard locked(g_lock);
  for (auto node{g_head}; node; node = node->m_next) {
    if (!strcmp(node->m_name, name)) {
      return node;
    }
  }
  return nullptr;
}

void static_globals::remove(static_node* node) {
  lock_guard locked(g_lock);
  if (node->m_next) {
    node->m_next->m_prev = node->m_prev;
  }

  if (node->m_prev) {
    node->m_prev->m_next = node->m_next;
  }

  if (g_head == node) {
    g_head = node->m_next;
  }

  if (g_tail == node) {
    g_tail = node->m_prev;
  }
}

} // namespace rx