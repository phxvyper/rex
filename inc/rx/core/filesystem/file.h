#ifndef RX_CORE_FILESYSTEM_FILE_H
#define RX_CORE_FILESYSTEM_FILE_H

#include <rx/core/concepts/no_copy.h> // no_copy
#include <rx/core/string.h> // string

namespace rx::filesystem {

struct file : concepts::no_copy {
  file(const char* file_name, const char* mode);
  file(file&& other);
  ~file();

  // read |size| bytes from file into |data|
  rx_u64 read(rx_byte* data, rx_u64 size);

  // write |size| bytes from |data| into file
  rx_u64 write(const rx_byte* data, rx_u64 size);

  // print |fmt| with |args| to file using |alloc| for formatting
  template<typename... Ts>
  bool print(rx::memory::allocator* alloc, const char* fmt, Ts&&... args);

  // print |fmt| with |args| to file
  template<typename... Ts>
  bool print(const char* fmt, Ts&&... args);

  // seek to |where| in file
  bool seek(rx_u64 where);

  // flush to disk
  bool flush();

private:
  bool print(string&& contents);

  void* m_impl;
  const char* m_file_name;
  const char* m_mode;
};

template<typename... Ts>
inline bool file::print(rx::memory::allocator* alloc, const char* fmt, Ts&&... args) {
  return print({alloc, fmt, forward<Ts>(args)...});
}

template<typename... Ts>
inline bool file::print(const char* fmt, Ts&&... args) {
  return print({fmt, forward<Ts>(args)...});
}

} // namespace rx::filesystem

#endif // RX_CORE_FILESYSTEM_FILE_H
