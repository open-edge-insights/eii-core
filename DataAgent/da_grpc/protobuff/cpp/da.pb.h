// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: da.proto

#ifndef PROTOBUF_INCLUDED_da_2eproto
#define PROTOBUF_INCLUDED_da_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_da_2eproto 

namespace protobuf_da_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[6];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_da_2eproto
namespace DataAgent {
class BlobReq;
class BlobReqDefaultTypeInternal;
extern BlobReqDefaultTypeInternal _BlobReq_default_instance_;
class Chunk;
class ChunkDefaultTypeInternal;
extern ChunkDefaultTypeInternal _Chunk_default_instance_;
class ConfigReq;
class ConfigReqDefaultTypeInternal;
extern ConfigReqDefaultTypeInternal _ConfigReq_default_instance_;
class ConfigResp;
class ConfigRespDefaultTypeInternal;
extern ConfigRespDefaultTypeInternal _ConfigResp_default_instance_;
class QueryReq;
class QueryReqDefaultTypeInternal;
extern QueryReqDefaultTypeInternal _QueryReq_default_instance_;
class QueryResp;
class QueryRespDefaultTypeInternal;
extern QueryRespDefaultTypeInternal _QueryResp_default_instance_;
}  // namespace DataAgent
namespace google {
namespace protobuf {
template<> ::DataAgent::BlobReq* Arena::CreateMaybeMessage<::DataAgent::BlobReq>(Arena*);
template<> ::DataAgent::Chunk* Arena::CreateMaybeMessage<::DataAgent::Chunk>(Arena*);
template<> ::DataAgent::ConfigReq* Arena::CreateMaybeMessage<::DataAgent::ConfigReq>(Arena*);
template<> ::DataAgent::ConfigResp* Arena::CreateMaybeMessage<::DataAgent::ConfigResp>(Arena*);
template<> ::DataAgent::QueryReq* Arena::CreateMaybeMessage<::DataAgent::QueryReq>(Arena*);
template<> ::DataAgent::QueryResp* Arena::CreateMaybeMessage<::DataAgent::QueryResp>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace DataAgent {

// ===================================================================

class BlobReq : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:DataAgent.BlobReq) */ {
 public:
  BlobReq();
  virtual ~BlobReq();

  BlobReq(const BlobReq& from);

  inline BlobReq& operator=(const BlobReq& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  BlobReq(BlobReq&& from) noexcept
    : BlobReq() {
    *this = ::std::move(from);
  }

  inline BlobReq& operator=(BlobReq&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const BlobReq& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const BlobReq* internal_default_instance() {
    return reinterpret_cast<const BlobReq*>(
               &_BlobReq_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(BlobReq* other);
  friend void swap(BlobReq& a, BlobReq& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline BlobReq* New() const final {
    return CreateMaybeMessage<BlobReq>(NULL);
  }

  BlobReq* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<BlobReq>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const BlobReq& from);
  void MergeFrom(const BlobReq& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(BlobReq* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string imgHandle = 1;
  void clear_imghandle();
  static const int kImgHandleFieldNumber = 1;
  const ::std::string& imghandle() const;
  void set_imghandle(const ::std::string& value);
  #if LANG_CXX11
  void set_imghandle(::std::string&& value);
  #endif
  void set_imghandle(const char* value);
  void set_imghandle(const char* value, size_t size);
  ::std::string* mutable_imghandle();
  ::std::string* release_imghandle();
  void set_allocated_imghandle(::std::string* imghandle);

  // @@protoc_insertion_point(class_scope:DataAgent.BlobReq)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr imghandle_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_da_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class Chunk : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:DataAgent.Chunk) */ {
 public:
  Chunk();
  virtual ~Chunk();

  Chunk(const Chunk& from);

  inline Chunk& operator=(const Chunk& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  Chunk(Chunk&& from) noexcept
    : Chunk() {
    *this = ::std::move(from);
  }

  inline Chunk& operator=(Chunk&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const Chunk& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const Chunk* internal_default_instance() {
    return reinterpret_cast<const Chunk*>(
               &_Chunk_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  void Swap(Chunk* other);
  friend void swap(Chunk& a, Chunk& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline Chunk* New() const final {
    return CreateMaybeMessage<Chunk>(NULL);
  }

  Chunk* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<Chunk>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const Chunk& from);
  void MergeFrom(const Chunk& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Chunk* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // bytes chunk = 1;
  void clear_chunk();
  static const int kChunkFieldNumber = 1;
  const ::std::string& chunk() const;
  void set_chunk(const ::std::string& value);
  #if LANG_CXX11
  void set_chunk(::std::string&& value);
  #endif
  void set_chunk(const char* value);
  void set_chunk(const void* value, size_t size);
  ::std::string* mutable_chunk();
  ::std::string* release_chunk();
  void set_allocated_chunk(::std::string* chunk);

  // @@protoc_insertion_point(class_scope:DataAgent.Chunk)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr chunk_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_da_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class QueryReq : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:DataAgent.QueryReq) */ {
 public:
  QueryReq();
  virtual ~QueryReq();

  QueryReq(const QueryReq& from);

  inline QueryReq& operator=(const QueryReq& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  QueryReq(QueryReq&& from) noexcept
    : QueryReq() {
    *this = ::std::move(from);
  }

  inline QueryReq& operator=(QueryReq&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const QueryReq& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const QueryReq* internal_default_instance() {
    return reinterpret_cast<const QueryReq*>(
               &_QueryReq_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  void Swap(QueryReq* other);
  friend void swap(QueryReq& a, QueryReq& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline QueryReq* New() const final {
    return CreateMaybeMessage<QueryReq>(NULL);
  }

  QueryReq* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<QueryReq>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const QueryReq& from);
  void MergeFrom(const QueryReq& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(QueryReq* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:DataAgent.QueryReq)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_da_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class QueryResp : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:DataAgent.QueryResp) */ {
 public:
  QueryResp();
  virtual ~QueryResp();

  QueryResp(const QueryResp& from);

  inline QueryResp& operator=(const QueryResp& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  QueryResp(QueryResp&& from) noexcept
    : QueryResp() {
    *this = ::std::move(from);
  }

  inline QueryResp& operator=(QueryResp&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const QueryResp& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const QueryResp* internal_default_instance() {
    return reinterpret_cast<const QueryResp*>(
               &_QueryResp_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    3;

  void Swap(QueryResp* other);
  friend void swap(QueryResp& a, QueryResp& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline QueryResp* New() const final {
    return CreateMaybeMessage<QueryResp>(NULL);
  }

  QueryResp* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<QueryResp>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const QueryResp& from);
  void MergeFrom(const QueryResp& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(QueryResp* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:DataAgent.QueryResp)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_da_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class ConfigReq : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:DataAgent.ConfigReq) */ {
 public:
  ConfigReq();
  virtual ~ConfigReq();

  ConfigReq(const ConfigReq& from);

  inline ConfigReq& operator=(const ConfigReq& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  ConfigReq(ConfigReq&& from) noexcept
    : ConfigReq() {
    *this = ::std::move(from);
  }

  inline ConfigReq& operator=(ConfigReq&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const ConfigReq& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const ConfigReq* internal_default_instance() {
    return reinterpret_cast<const ConfigReq*>(
               &_ConfigReq_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    4;

  void Swap(ConfigReq* other);
  friend void swap(ConfigReq& a, ConfigReq& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline ConfigReq* New() const final {
    return CreateMaybeMessage<ConfigReq>(NULL);
  }

  ConfigReq* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<ConfigReq>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const ConfigReq& from);
  void MergeFrom(const ConfigReq& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(ConfigReq* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:DataAgent.ConfigReq)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_da_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class ConfigResp : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:DataAgent.ConfigResp) */ {
 public:
  ConfigResp();
  virtual ~ConfigResp();

  ConfigResp(const ConfigResp& from);

  inline ConfigResp& operator=(const ConfigResp& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  ConfigResp(ConfigResp&& from) noexcept
    : ConfigResp() {
    *this = ::std::move(from);
  }

  inline ConfigResp& operator=(ConfigResp&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const ConfigResp& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const ConfigResp* internal_default_instance() {
    return reinterpret_cast<const ConfigResp*>(
               &_ConfigResp_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    5;

  void Swap(ConfigResp* other);
  friend void swap(ConfigResp& a, ConfigResp& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline ConfigResp* New() const final {
    return CreateMaybeMessage<ConfigResp>(NULL);
  }

  ConfigResp* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<ConfigResp>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const ConfigResp& from);
  void MergeFrom(const ConfigResp& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(ConfigResp* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:DataAgent.ConfigResp)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_da_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// BlobReq

// string imgHandle = 1;
inline void BlobReq::clear_imghandle() {
  imghandle_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& BlobReq::imghandle() const {
  // @@protoc_insertion_point(field_get:DataAgent.BlobReq.imgHandle)
  return imghandle_.GetNoArena();
}
inline void BlobReq::set_imghandle(const ::std::string& value) {
  
  imghandle_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:DataAgent.BlobReq.imgHandle)
}
#if LANG_CXX11
inline void BlobReq::set_imghandle(::std::string&& value) {
  
  imghandle_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:DataAgent.BlobReq.imgHandle)
}
#endif
inline void BlobReq::set_imghandle(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  imghandle_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:DataAgent.BlobReq.imgHandle)
}
inline void BlobReq::set_imghandle(const char* value, size_t size) {
  
  imghandle_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:DataAgent.BlobReq.imgHandle)
}
inline ::std::string* BlobReq::mutable_imghandle() {
  
  // @@protoc_insertion_point(field_mutable:DataAgent.BlobReq.imgHandle)
  return imghandle_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* BlobReq::release_imghandle() {
  // @@protoc_insertion_point(field_release:DataAgent.BlobReq.imgHandle)
  
  return imghandle_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void BlobReq::set_allocated_imghandle(::std::string* imghandle) {
  if (imghandle != NULL) {
    
  } else {
    
  }
  imghandle_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), imghandle);
  // @@protoc_insertion_point(field_set_allocated:DataAgent.BlobReq.imgHandle)
}

// -------------------------------------------------------------------

// Chunk

// bytes chunk = 1;
inline void Chunk::clear_chunk() {
  chunk_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& Chunk::chunk() const {
  // @@protoc_insertion_point(field_get:DataAgent.Chunk.chunk)
  return chunk_.GetNoArena();
}
inline void Chunk::set_chunk(const ::std::string& value) {
  
  chunk_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:DataAgent.Chunk.chunk)
}
#if LANG_CXX11
inline void Chunk::set_chunk(::std::string&& value) {
  
  chunk_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:DataAgent.Chunk.chunk)
}
#endif
inline void Chunk::set_chunk(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  chunk_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:DataAgent.Chunk.chunk)
}
inline void Chunk::set_chunk(const void* value, size_t size) {
  
  chunk_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:DataAgent.Chunk.chunk)
}
inline ::std::string* Chunk::mutable_chunk() {
  
  // @@protoc_insertion_point(field_mutable:DataAgent.Chunk.chunk)
  return chunk_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* Chunk::release_chunk() {
  // @@protoc_insertion_point(field_release:DataAgent.Chunk.chunk)
  
  return chunk_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void Chunk::set_allocated_chunk(::std::string* chunk) {
  if (chunk != NULL) {
    
  } else {
    
  }
  chunk_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), chunk);
  // @@protoc_insertion_point(field_set_allocated:DataAgent.Chunk.chunk)
}

// -------------------------------------------------------------------

// QueryReq

// -------------------------------------------------------------------

// QueryResp

// -------------------------------------------------------------------

// ConfigReq

// -------------------------------------------------------------------

// ConfigResp

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace DataAgent

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_da_2eproto
