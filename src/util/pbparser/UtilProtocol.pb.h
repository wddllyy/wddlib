// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: util/pbparser/UtilProtocol.proto

#ifndef PROTOBUF_util_2fpbparser_2fUtilProtocol_2eproto__INCLUDED
#define PROTOBUF_util_2fpbparser_2fUtilProtocol_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
#include "google/protobuf/descriptor.pb.h"
// @@protoc_insertion_point(includes)

namespace util {
namespace pbparser {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_util_2fpbparser_2fUtilProtocol_2eproto();
void protobuf_AssignDesc_util_2fpbparser_2fUtilProtocol_2eproto();
void protobuf_ShutdownFile_util_2fpbparser_2fUtilProtocol_2eproto();

class ProtocolPacketOptions;

enum ProtocolSwitch {
  protocol_no = 0,
  protocol_yes = 1
};
bool ProtocolSwitch_IsValid(int value);
const ProtocolSwitch ProtocolSwitch_MIN = protocol_no;
const ProtocolSwitch ProtocolSwitch_MAX = protocol_yes;
const int ProtocolSwitch_ARRAYSIZE = ProtocolSwitch_MAX + 1;

const ::google::protobuf::EnumDescriptor* ProtocolSwitch_descriptor();
inline const ::std::string& ProtocolSwitch_Name(ProtocolSwitch value) {
  return ::google::protobuf::internal::NameOfEnum(
    ProtocolSwitch_descriptor(), value);
}
inline bool ProtocolSwitch_Parse(
    const ::std::string& name, ProtocolSwitch* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ProtocolSwitch>(
    ProtocolSwitch_descriptor(), name, value);
}
// ===================================================================

class ProtocolPacketOptions : public ::google::protobuf::Message {
 public:
  ProtocolPacketOptions();
  virtual ~ProtocolPacketOptions();

  ProtocolPacketOptions(const ProtocolPacketOptions& from);

  inline ProtocolPacketOptions& operator=(const ProtocolPacketOptions& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ProtocolPacketOptions& default_instance();

  void Swap(ProtocolPacketOptions* other);

  // implements Message ----------------------------------------------

  ProtocolPacketOptions* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ProtocolPacketOptions& from);
  void MergeFrom(const ProtocolPacketOptions& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional .util.pbparser.ProtocolSwitch ProtocolPacket = 1 [default = protocol_no];
  inline bool has_protocolpacket() const;
  inline void clear_protocolpacket();
  static const int kProtocolPacketFieldNumber = 1;
  inline ::util::pbparser::ProtocolSwitch protocolpacket() const;
  inline void set_protocolpacket(::util::pbparser::ProtocolSwitch value);

  // @@protoc_insertion_point(class_scope:util.pbparser.ProtocolPacketOptions)
 private:
  inline void set_has_protocolpacket();
  inline void clear_has_protocolpacket();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  int protocolpacket_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_util_2fpbparser_2fUtilProtocol_2eproto();
  friend void protobuf_AssignDesc_util_2fpbparser_2fUtilProtocol_2eproto();
  friend void protobuf_ShutdownFile_util_2fpbparser_2fUtilProtocol_2eproto();

  void InitAsDefaultInstance();
  static ProtocolPacketOptions* default_instance_;
};
// ===================================================================

static const int kPacketOptionsFieldNumber = 50001;
extern ::google::protobuf::internal::ExtensionIdentifier< ::google::protobuf::MessageOptions,
    ::google::protobuf::internal::MessageTypeTraits< ::util::pbparser::ProtocolPacketOptions >, 11, false >
  PacketOptions;

// ===================================================================

// ProtocolPacketOptions

// optional .util.pbparser.ProtocolSwitch ProtocolPacket = 1 [default = protocol_no];
inline bool ProtocolPacketOptions::has_protocolpacket() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ProtocolPacketOptions::set_has_protocolpacket() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ProtocolPacketOptions::clear_has_protocolpacket() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ProtocolPacketOptions::clear_protocolpacket() {
  protocolpacket_ = 0;
  clear_has_protocolpacket();
}
inline ::util::pbparser::ProtocolSwitch ProtocolPacketOptions::protocolpacket() const {
  return static_cast< ::util::pbparser::ProtocolSwitch >(protocolpacket_);
}
inline void ProtocolPacketOptions::set_protocolpacket(::util::pbparser::ProtocolSwitch value) {
  assert(::util::pbparser::ProtocolSwitch_IsValid(value));
  set_has_protocolpacket();
  protocolpacket_ = value;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace pbparser
}  // namespace util

#ifndef SWIG
namespace google {
namespace protobuf {

template <>
inline const EnumDescriptor* GetEnumDescriptor< ::util::pbparser::ProtocolSwitch>() {
  return ::util::pbparser::ProtocolSwitch_descriptor();
}

}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_util_2fpbparser_2fUtilProtocol_2eproto__INCLUDED
