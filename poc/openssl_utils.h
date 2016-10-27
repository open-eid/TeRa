/*
 * openssl_utils.h
 *
 *  Created on: Oct 26, 2016
 */

#ifndef OPENSSL_UTILS_H_
#define OPENSSL_UTILS_H_

#include <QByteArray>

namespace ria_tera {

QByteArray create_timestamp_request(QByteArray const& sha256);
bool extract_timestamp_from_ts_response(QByteArray const& response, QByteArray& timestamp);

} // namespace

#endif /* OPENSSL_UTILS_H_ */
