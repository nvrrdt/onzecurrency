#pragma once

#include "stdafx.h"

#include <assert.h>

#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include "osrng.h"
// using CryptoPP::AutoSeededX917RNG;
using CryptoPP::AutoSeededRandomPool;

#include "aes.h"
using CryptoPP::AES;

#include "integer.h"
using CryptoPP::Integer;

#include "sha.h"
using CryptoPP::SHA256;

#include "filters.h"
using CryptoPP::StringSource;
using CryptoPP::StringSink;
using CryptoPP::ArraySink;
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;

#include "files.h"
using CryptoPP::FileSource;
using CryptoPP::FileSink;

#include "eccrypto.h"
using CryptoPP::ECDSA;
using CryptoPP::ECP;
using CryptoPP::DL_GroupParameters_EC;

#if _MSC_VER <= 1200 // VS 6.0
using KeyPair = CryptoPP::ECDSA<ECP, SHA256>;
using GroupParams = CryptoPP::DL_GroupParameters_EC<ECP>;
#endif

#include "oids.h"
using CryptoPP::OID;

namespace Crowd
{
    class Crypto
    {
    public:
        std::string sha256_create(std::string &msg);
        std::string base58_encode_sha256(std::string &hash);
        std::string base58_decode(std::string &b58);
        int ecdsa_generate_and_save_keypair();
        bool ecdsa_generate_private_key( const OID& oid, ECDSA<ECP, SHA256>::PrivateKey& key );
        bool ecdsa_generate_public_key( const ECDSA<ECP, SHA256>::PrivateKey& ecdsa_private_key_, ECDSA<ECP, SHA256>::PublicKey& ecdsa_public_key_ );
        void ecdsa_save_private_key( const ECDSA<ECP, SHA256>::PrivateKey& key );
        void ecdsa_save_public_key( const ECDSA<ECP, SHA256>::PublicKey& key );
        void ecdsa_load_private_key( ECDSA<ECP, SHA256>::PrivateKey& key );
        void ecdsa_load_public_key( ECDSA<ECP, SHA256>::PublicKey& key );
        bool ecdsa_sign_message( const ECDSA<ECP, SHA256>::PrivateKey& key, const string& message, string& signature );
        bool ecdsa_verify_message( const ECDSA<ECP, SHA256>::PublicKey& key, const string& message, const string& signature );
        // ecdsa::Key get_keypair_with_priv_key(std::string &priv_key);
        // std::string get_priv_key();
        // std::string get_pub_key();
        // std::tuple<std::vector<uint8_t>, bool> sign(const std::string &string);
        // bool verify(const std::string &string, std::string &signature_base58, std::string &pub_key_base58);
    private:
        // Private and Public keys
        ECDSA<ECP, SHA256>::PrivateKey ecdsa_private_key_;
        ECDSA<ECP, SHA256>::PublicKey ecdsa_public_key_;
    };
}