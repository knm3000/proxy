#pragma once

#include "envoy/event/file_event.h"
#include "envoy/event/timer.h"
#include "envoy/extensions/filters/listener/tls_inspector/v3/tls_inspector.pb.h"
#include "envoy/network/filter.h"
#include "envoy/stats/scope.h"
#include "envoy/stats/stats_macros.h"

#include "source/common/common/logger.h"

#include "bssl_wrapper/bssl_wrapper.h"
#include "openssl/ssl.h"
#include "ssl/ssl_locl.h"

#define SSL_CLIENT_HELLO  CLIENTHELLO_MSG

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace TlsInspector {

/**
 * All stats for the TLS inspector. @see stats_macros.h
 */
#define ALL_TLS_INSPECTOR_STATS(COUNTER)                                                           \
  COUNTER(connection_closed)                                                                       \
  COUNTER(client_hello_too_large)                                                                  \
  COUNTER(read_error)                                                                              \
  COUNTER(tls_found)                                                                               \
  COUNTER(tls_not_found)                                                                           \
  COUNTER(alpn_found)                                                                              \
  COUNTER(alpn_not_found)                                                                          \
  COUNTER(sni_found)                                                                               \
  COUNTER(sni_not_found)

/**
 * Definition of all stats for the TLS inspector. @see stats_macros.h
 */
struct TlsInspectorStats {
  ALL_TLS_INSPECTOR_STATS(GENERATE_COUNTER_STRUCT)
};

enum class ParseState {
  // Parse result is out. It could be tls or not.
  Done,
  // Parser expects more data.
  Continue,
  // Parser reports unrecoverable error.
  Error
};
/**
 * Global configuration for TLS inspector.
 */
class Config {
public:
  Config(Stats::Scope& scope,
         const envoy::extensions::filters::listener::tls_inspector::v3::TlsInspector& proto_config,
         uint32_t max_client_hello_size = TLS_MAX_CLIENT_HELLO);

  const TlsInspectorStats& stats() const { return stats_; }
  bssl::UniquePtr<SSL> newSsl();
  bool enableJA3Fingerprinting() const { return enable_ja3_fingerprinting_; }
  void setEnableJA3Fingerprinting(bool enable) { enable_ja3_fingerprinting_ = enable; }
  uint32_t maxClientHelloSize() const { return max_client_hello_size_; }
  static constexpr size_t TLS_MAX_CLIENT_HELLO = 64 * 1024;
  static const unsigned TLS_MIN_SUPPORTED_VERSION;
  static const unsigned TLS_MAX_SUPPORTED_VERSION;
private:
  TlsInspectorStats stats_;
  bssl::UniquePtr<SSL_CTX> ssl_ctx_;
  bool enable_ja3_fingerprinting_;
  const uint32_t max_client_hello_size_;
};

using ConfigSharedPtr = std::shared_ptr<Config>;

/**
 * TLS inspector listener filter.
 */
class Filter : public Network::ListenerFilter, Logger::Loggable<Logger::Id::filter> {
public:
  Filter(const ConfigSharedPtr config);

  // Network::ListenerFilter
  Network::FilterStatus onAccept(Network::ListenerFilterCallbacks& cb) override;
  void onCert();

private:
  std::vector<absl::string_view> getAlpnProtocols(const unsigned char* data, unsigned int len);
  ParseState parseClientHello(const void* data, size_t len);
  ParseState onRead();
  void done(bool success);
  void onALPN(const unsigned char* data, unsigned int len);
  void onServername(absl::string_view name);
  // JA3 fingerprinting
  void createJA3Hash(const SSL_CLIENT_HELLO* ssl_client_hello);

  ConfigSharedPtr config_;
  Network::ListenerFilterCallbacks* cb_{};

  bssl::UniquePtr<SSL> ssl_;
  uint64_t read_{0};
  bool alpn_found_{false};
  bool clienthello_success_{false};

  static thread_local uint8_t buf_[Config::TLS_MAX_CLIENT_HELLO];

  // Allows callbacks on the SSL_CTX to set fields in this class.
  friend class Config;
};

} // namespace TlsInspector
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
