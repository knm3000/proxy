#ifndef THIRD_PARTY_CEL_CPP_EVAL_PUBLIC_STRUCTS_CEL_PROTO_WRAPPER_H_
#define THIRD_PARTY_CEL_CPP_EVAL_PUBLIC_STRUCTS_CEL_PROTO_WRAPPER_H_

#include "google/protobuf/duration.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "eval/public/cel_value.h"
#include "internal/proto_util.h"

namespace google::api::expr::runtime {

class CelProtoWrapper {
 public:
  // CreateMessage creates CelValue from google::protobuf::Message.
  // As some of CEL basic types are subclassing google::protobuf::Message,
  // this method contains type checking and downcasts.
  static CelValue CreateMessage(const google::protobuf::Message* value,
                                google::protobuf::Arena* arena);

  // CreateDuration creates CelValue from a non-null protobuf duration value.
  static CelValue CreateDuration(const google::protobuf::Duration* value) {
    return CelValue(expr::internal::DecodeDuration(*value));
  }

  // CreateTimestamp creates CelValue from a non-null protobuf timestamp value.
  static CelValue CreateTimestamp(const google::protobuf::Timestamp* value) {
    return CelValue(expr::internal::DecodeTime(*value));
  }

  // MaybeWrapValue attempts to wrap the input value in a proto message with
  // the given type_name. If the value can be wrapped, it is returned as a
  // CelValue pointing to the protobuf message. Otherwise, the result will be
  // empty.
  //
  // This method is the complement to CreateMessage which may unwrap a protobuf
  // message to native CelValue representation during a protobuf field read.
  // Just as CreateMessage should only be used when reading protobuf values,
  // MaybeWrapValue should only be used when assigning protobuf fields.
  static absl::optional<CelValue> MaybeWrapValue(absl::string_view type_name,
                                                 const CelValue& value,
                                                 google::protobuf::Arena* arena);
};

}  // namespace google::api::expr::runtime

#endif  // THIRD_PARTY_CEL_CPP_EVAL_PUBLIC_STRUCTS_CEL_PROTO_WRAPPER_H_
