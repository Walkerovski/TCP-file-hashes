#include <string.h>
#include <strings.h>
#include <iostream>
#include <openssl/evp.h>

#include "hash.h"

using namespace std;

struct checksum_ctx {
	EVP_MD_CTX *ctx;
	const EVP_MD *md;
	uint8_t *salt;
	size_t len;
};


struct checksum_ctx * checksum_create(const uint8_t *salt, size_t len) {
		struct checksum_ctx *csm = static_cast<checksum_ctx*>(malloc(sizeof(*csm)));
	if (!csm) {
		goto err;
	}
	bzero(csm, sizeof(*csm));
	csm->ctx = EVP_MD_CTX_new();
	csm->md = EVP_sha256();
	csm->len = len;
	if (len > 0) {
		csm->salt = static_cast<uint8_t*>(malloc(len));
		if (!csm->salt) {
			goto err;
		}
		memcpy(csm->salt, salt, len);
	}
	if (checksum_reset(csm)) {
		goto err;
	}


	return csm;

  err:
	if (csm) {
		if (csm->salt) {
			free(csm->salt);
		}
		bzero(csm, sizeof(*csm));
		free(csm);
	}
	csm = NULL;

	return csm;
}

int checksum_update(struct checksum_ctx *csm, const uint8_t *payload) {
	return EVP_DigestUpdate(csm->ctx, payload, UPDATE_PAYLOAD_SIZE) != 1;
}

int checksum_finish(struct checksum_ctx *csm, const uint8_t *payload, size_t len, uint8_t *out) {
	int ret = 1;
	if (len) {
		ret = EVP_DigestUpdate(csm->ctx, payload, len);
	}
	if (ret == 1) {
		return EVP_DigestFinal_ex(csm->ctx, out, NULL) != 1;
	} else {
		return 1;
	}
}

int checksum_reset(struct checksum_ctx *csm) {
	EVP_DigestInit_ex(csm->ctx, csm->md, NULL);
	if (csm->len) {
		return EVP_DigestUpdate(csm->ctx, csm->salt, csm->len) != 1;
	} else {
		return 0;
	}
	
}

int checksum_destroy(struct checksum_ctx *csm) {
	EVP_MD_CTX_free(csm->ctx);
	free(csm->salt);
	bzero(csm, sizeof(*csm));
	free(csm);
	return 0;
}

array<uint8_t, 32> compute_checksum(const vector<uint8_t>& payload, const string& salt) {
	array<uint8_t, 32> digest{};  
	const uint8_t* salt_ptr = salt.empty() ? nullptr
                                           : reinterpret_cast<const uint8_t*>(salt.data());

    checksum_ctx* ctx = checksum_create(salt_ptr, salt.size());
    if (!ctx) {
        cerr << "Failed to create checksum context\n";
        return {};
    }

    size_t offset = 0;
	size_t length = payload.size();
    while (length - offset >= UPDATE_PAYLOAD_SIZE) {
        if (checksum_update(ctx, payload.data() + offset) != 0) {
            cerr << "checksum_update failed\n";
            checksum_destroy(ctx);
            return {};
        }
        offset += UPDATE_PAYLOAD_SIZE;
    }

    size_t remaining = length - offset;
    if (checksum_finish(ctx, payload.data() + offset, remaining, digest.data()) != 0) {
        cerr << "checksum_finish failed\n";
        checksum_destroy(ctx);
        return {};
    }

    checksum_destroy(ctx);

    return digest;
}
