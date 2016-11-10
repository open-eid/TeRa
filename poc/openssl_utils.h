/**
 * Wrapper for functionality provided by OpenSSL.
 */
// TODO ll
#ifndef _TERA_OPENSSL_UTILS_H_
#define _TERA_OPENSSL_UTILS_H_

#include <QByteArray>

namespace ria_tera {

///
/// \brief Converts sha256 hash into timestamp request suitable for sending to time server.
///
QByteArray create_timestamp_request(QByteArray const& sha256);

///
/// \brief Extracts timestamp from time server response
/// \param[in] response response form time server
/// \param[out] timestamp timestamp
/// \return true if extraction was successful
///
bool extract_timestamp_from_ts_response(QByteArray const& response, QByteArray& timestamp);

} // namespace

#endif /* _TERA_OPENSSL_UTILS_H_ */
