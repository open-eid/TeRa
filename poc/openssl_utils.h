/*
 * TeRa
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

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
