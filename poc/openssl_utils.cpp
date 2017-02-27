/**
 * Based on apps/ts.c in OpenSSL
 */
// TODO ll
#include "openssl_utils.h"

#include <openssl/ssl.h>
#include <openssl/ts.h>

#include <iostream>


#if (OPENSSL_VERSION_NUMBER & 0xFFFF00000) == 0x010000000
    #define TERA_OLD_OPENSSL
#endif

namespace ria_tera {

    // apps/ts.c    TODO
    // static TS_REQ *create_query(BIO *data_bio, const char *digest, const EVP_MD *md,
    //                             const char *policy, int no_nonce, int cert)


/* Request nonce length, in bits (must be a multiple of 8). */
# define NONCE_LENGTH            64

static ASN1_INTEGER *create_nonce(int bits)
{
	unsigned char buf[20];
	ASN1_INTEGER *nonce = NULL;
	int len = (bits - 1) / 8 + 1;
	int i;

	if (len > (int)sizeof(buf))
		goto err;
//TODO    if (RAND_bytes(buf, len) <= 0)
//        goto err;

	/* Find the first non-zero byte and creating ASN1_INTEGER object. */
	for (i = 0; i < len && !buf[i]; ++i)
		continue;
	if ((nonce = ASN1_INTEGER_new()) == NULL)
		goto err;
	OPENSSL_free(nonce->data);
	nonce->length = len - i;
	nonce->data = new unsigned char[nonce->length + 1];// TODO app_malloc(nonce->length + 1, "nonce buffer");
	memcpy(nonce->data, buf + i, nonce->length);
	return nonce;

 err:
//    BIO_printf(bio_err, "could not create nonce\n");
std::cout << "could not create nonce\n" << std::endl;
	ASN1_INTEGER_free(nonce);
	return NULL;
}

static int create_digest(BIO *input, const char *digest, const EVP_MD *md,
    unsigned char **md_value)
{
    int md_value_len;
    int rv = 0;
#ifdef TERA_OLD_OPENSSL
#else
    EVP_MD_CTX *md_ctx = NULL;
#endif

    md_value_len = EVP_MD_size(md);
    if (md_value_len < 0)
        return 0;

    if (input) {
#ifdef TERA_OLD_OPENSSL
        /* Digest must be computed from an input file. */
        EVP_MD_CTX md_ctx__obj;
        EVP_MD_CTX* md_ctx = &md_ctx__obj;
#else
        md_ctx = EVP_MD_CTX_new();
        if (md_ctx == NULL)
            return 0;
#endif
        unsigned char buffer[4096];
        int length;

// TODO freed later...
        *md_value = new unsigned char[md_value_len]; //app_malloc(md_value_len, "digest buffer");
        if (!EVP_DigestInit(md_ctx, md))
            goto err;
        while ((length = BIO_read(input, buffer, sizeof(buffer))) > 0) {
            if (!EVP_DigestUpdate(md_ctx, buffer, length))
                goto err;
        }
        if (!EVP_DigestFinal(md_ctx, *md_value, NULL))
            goto err;
        md_value_len = EVP_MD_size(md);
    }
    else {
        long digest_len;
#ifdef TERA_OLD_OPENSSL
        *md_value = string_to_hex(digest, &digest_len);
#else
        *md_value = OPENSSL_hexstr2buf(digest, &digest_len);
#endif
        if (!*md_value || md_value_len != digest_len) {
            OPENSSL_free(*md_value);
            *md_value = NULL;
            std::cout << "could not create query aa\n" << std::endl;
            //            BIO_printf(bio_err, "bad digest, %d bytes "
            //                       "must be specified\n", md_value_len);
            goto err;
        }
    }
    rv = md_value_len;
err:
#ifdef TERA_OLD_OPENSSL
#else
    EVP_MD_CTX_free(md_ctx);
#endif
    return rv;
}

static TS_REQ *create_query(BIO *data_bio, const char *digest, const EVP_MD *md,
							const char *policy, int no_nonce, int cert)
{
	int ret = 0;
	TS_REQ *ts_req = NULL;
	int len;
	TS_MSG_IMPRINT *msg_imprint = NULL;
	X509_ALGOR *algo = NULL;
	unsigned char *data = NULL;
	ASN1_OBJECT *policy_obj = NULL;
	ASN1_INTEGER *nonce_asn1 = NULL;

	if (md == NULL && (md = EVP_get_digestbyname("sha1")) == NULL)
		goto err;
	if ((ts_req = TS_REQ_new()) == NULL)
		goto err;
	if (!TS_REQ_set_version(ts_req, 1))
		goto err;
	if ((msg_imprint = TS_MSG_IMPRINT_new()) == NULL)
		goto err;
	if ((algo = X509_ALGOR_new()) == NULL)
		goto err;
	if ((algo->algorithm = OBJ_nid2obj(EVP_MD_type(md))) == NULL)
		goto err;
	if ((algo->parameter = ASN1_TYPE_new()) == NULL)
		goto err;
	algo->parameter->type = V_ASN1_NULL;
	if (!TS_MSG_IMPRINT_set_algo(msg_imprint, algo))
		goto err;
// TODO not needed, only checks for length
	if ((len = create_digest(data_bio, digest, md, &data)) == 0)
		goto err;
	if (!TS_MSG_IMPRINT_set_msg(msg_imprint, data, len))
		goto err;
	if (!TS_REQ_set_msg_imprint(ts_req, msg_imprint))
		goto err;
// TODO
//    if (policy && (policy_obj = txt2obj(policy)) == NULL)
//        goto err;
	if (policy_obj && !TS_REQ_set_policy_id(ts_req, policy_obj))
		goto err;

	/* Setting nonce if requested. */
// TODO???
	if (!no_nonce && (nonce_asn1 = create_nonce(NONCE_LENGTH)) == NULL)
		goto err;
	if (nonce_asn1 && !TS_REQ_set_nonce(ts_req, nonce_asn1))
		goto err;
	if (!TS_REQ_set_cert_req(ts_req, cert))
		goto err;

	ret = 1;
 err:
	if (!ret) {
		TS_REQ_free(ts_req);
		ts_req = NULL;
std::cout << "could not create query\n" << std::endl;
// TODO
//        BIO_printf(bio_err, "could not create query\n");
//        ERR_print_errors(bio_err);
	}
	TS_MSG_IMPRINT_free(msg_imprint);
	X509_ALGOR_free(algo);
	OPENSSL_free(data);
	ASN1_OBJECT_free(policy_obj);
	ASN1_INTEGER_free(nonce_asn1);
	return ts_req;
}
/////////////////////////////



static int reply_command(CONF *conf, const char *section, const char *engine,
                         const char *queryfile, const char *passin, const char *inkey,
                         const EVP_MD *md, const char *signer, const char *chain,
                         const char *policy, QByteArray const& in, int token_in,
                         QByteArray& out, int token_out, int text)
{
    int ret = 0;
    TS_RESP *response = NULL;
    BIO *in_bio = NULL;
    BIO *query_bio = NULL;
    BIO *inkey_bio = NULL;
    BIO *signer_bio = NULL;
    BIO *out_bio = NULL;

//    if (in != NULL) {
//        if ((in_bio = BIO_new_file(in, "rb")) == NULL)
//          return 1; // TODO goto end;
//QFile resp("xresp.tsr");
//std::cout << "ff " << in.length() << " " << resp.open(QIODevice::WriteOnly | QIODevice::Truncate) << std::endl;
//resp.write(in);
//resp.close();

    in_bio = BIO_new_mem_buf(const_cast<char*>(in.constData()), in.length());//BIO_new(BIO_s_mem());
//    in_bio = BIO_new_file("response.tsr", "rb");
//    in_bio = BIO_new_file("xresp.tsr", "rb");
//        if (token_in) {
//            response = read_PKCS7(in_bio);
//        } else {
            response = d2i_TS_RESP_bio(in_bio, NULL);
//        }
//    } else {
//        response = create_response(conf, section, engine, queryfile,
//                                   passin, inkey, md, signer, chain, policy);
//        if (response)
//            BIO_printf(bio_err, "Response has been generated.\n");
//        else
//            BIO_printf(bio_err, "Response is not generated.\n");
//    }
    if (response == NULL) {
std::cout <<  "reply_command  response is NULL" << std::endl;
        return ret; // TODO goto end;
    }

//    /* Write response. */
//    if (text) {
//        if ((out_bio = bio_open_default(out, 'w', FORMAT_TEXT)) == NULL)
//        goto end;
//        if (token_out) {
//            TS_TST_INFO *tst_info = TS_RESP_get_tst_info(response);
//            if (!TS_TST_INFO_print_bio(out_bio, tst_info))
//                goto end;
//        } else {
//            if (!TS_RESP_print_bio(out_bio, response))
//                goto end;
//        }
//    } else {
    out_bio = BIO_new(BIO_s_mem());
//        if ((out_bio = bio_open_default(out, 'w', FORMAT_ASN1)) == NULL) // TODO FORMAT_ASN1 ??????????
//            return 1; // TODO goto end;
//        if (token_out) {
            PKCS7 *token = TS_RESP_get_token(response);
            if (!i2d_PKCS7_bio(out_bio, token))
                return 1; // TODO goto end;
//        } else {
//            if (!i2d_TS_RESP_bio(out_bio, response))
//                goto end;
//        }
//    }

    BUF_MEM *bptr = NULL;
    BIO_get_mem_ptr(out_bio, &bptr);
//  BIO_set_close(out_bio, BIO_NOCLOSE); /* So BIO_free() leaves BUF_MEM alone */
//  BIO_free(out_bio);
QByteArray xxx(bptr->data, bptr->length);
out = xxx;

    ret = 1;

 end:
// TODO    ERR_print_errors(bio_err);
    BIO_free_all(in_bio);
    BIO_free_all(query_bio);
    BIO_free_all(inkey_bio);
    BIO_free_all(signer_bio);
    BIO_free_all(out_bio);
    TS_RESP_free(response);
    return ret;
}

bool extract_timestamp_from_ts_response(QByteArray const& timeserverResponse, QByteArray& timestamp) {
    CONF *conf; // TODO
    const char *section = NULL;
    const char *engine = NULL;
    const char *queryfile = NULL;
    const char *passin = NULL;
    const char *inkey = NULL;
    const EVP_MD *md = NULL;
    const char *signer = NULL;
    const char *chain = NULL;
    const char *policy = NULL;
    const char *in = NULL; // TODO
    int token_in = 0;
    //const char *out = NULL; // TODO
    int token_out = 1;
    int text = 0;

    int result = reply_command(conf, section, engine,
                             queryfile, passin, inkey,
                             md, signer, chain,
                             policy, timeserverResponse, token_in,
                             timestamp, token_out, text);
//std::cout << "extractTimestamp-> " << result << std::endl;
    return result == 1;
}


/* Reads a PKCS7 token and adds default 'granted' status info to it. */
static TS_RESP *read_PKCS7(BIO *in_bio)
{
    int ret = 0;
    PKCS7 *token = NULL;
    TS_TST_INFO *tst_info = NULL;
    TS_RESP *resp = NULL;
    TS_STATUS_INFO *si = NULL;

    if ((token = d2i_PKCS7_bio(in_bio, NULL)) == NULL)
        goto end;
    if ((tst_info = PKCS7_to_TS_TST_INFO(token)) == NULL)
        goto end;

    /* Creating response object. */
    if (!(resp = TS_RESP_new()))
        goto end;

    /* Create granted status info. */
    if ((si = TS_STATUS_INFO_new()) == NULL)
        goto end;
#ifdef TERA_OLD_OPENSSL
    if (!(ASN1_INTEGER_set(si->status, TS_STATUS_GRANTED)))
        goto end;
#else
    if (!TS_STATUS_INFO_set_status(si, TS_STATUS_GRANTED))
        return resp; // goto end;
#endif
    if (!TS_RESP_set_status_info(resp, si))
        goto end;

    /* Setting encapsulated token. */
    TS_RESP_set_tst_info(resp, token, tst_info);
    token = NULL;               /* Ownership is lost. */
    tst_info = NULL;            /* Ownership is lost. */

    ret = 1;

end:
    PKCS7_free(token);
    TS_TST_INFO_free(tst_info);
    if (!ret) {
        TS_RESP_free(resp);
        resp = NULL;
    }
    TS_STATUS_INFO_free(si);
    return resp;
}

QByteArray create_timestamp_request(QByteArray const& sha256) // TODO check size
{
    TS_REQ *query = NULL; // TODO

    BIO *data_bio = NULL;
    QByteArray hashHex = sha256.toHex();
    const char *digest = hashHex.constData();

    const EVP_MD *md = EVP_get_digestbyname("sha256"); // TODO
    const char *policy = NULL; // TODO
    int no_nonce = 0;
    int cert = 1;

    query = create_query(data_bio, digest, md, policy, no_nonce, cert);

    // https://www.openssl.org/docs/manmaster/crypto/bio.html
    BIO *out_bio = NULL;
    out_bio = BIO_new(BIO_s_mem());

    // typedef struct buf_mem_st BUF_MEM;
    if (!i2d_TS_REQ_bio(out_bio, query)) {
//      goto end;
//std::cout << "WTF i2d_TS_REQ_bio" << std::endl;
    }

    // free ressources
    BUF_MEM *bptr = NULL;
    BIO_get_mem_ptr(out_bio, &bptr);

    QByteArray res(bptr->data, bptr->length);

    BIO_set_close(out_bio, BIO_NOCLOSE); /* So BIO_free() leaves BUF_MEM alone */
    BIO_free(out_bio);

    return res;
}

} // namespace
