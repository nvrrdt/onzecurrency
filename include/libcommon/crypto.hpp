#pragma once

#include "stdafx.h"

#include <assert.h>

#include <iostream>
using std::cout;
using std::cerr;
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

using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;

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

#include "rsa.h"
using CryptoPP::RSA;
using CryptoPP::InvertibleRSAFunction;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;

#include "secblock.h"
using CryptoPP::SecByteBlock;

#include "cryptlib.h"
using CryptoPP::Exception;
using CryptoPP::DecodingResult;

#include <exception>
using std::exception;

#include "hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

namespace Crowd
{
    class Crypto
    {
    public:
        // SHA256:
        std::string sha256_create(std::string &msg);

        // BASE58/
        std::string base58_encode_sha256(std::string &hash);
        std::string base58_encode(std::string &str);
        std::string base58_decode(std::string &b58);

        // ECDSA:
        int ecdsa_generate_and_save_keypair();
        bool ecdsa_generate_private_key( const OID& oid, ECDSA<ECP, SHA256>::PrivateKey& key );
        bool ecdsa_generate_public_key( const ECDSA<ECP, SHA256>::PrivateKey& ecdsa_private_key_, ECDSA<ECP, SHA256>::PublicKey& ecdsa_public_key_ );
        void ecdsa_save_private_key( const ECDSA<ECP, SHA256>::PrivateKey& key );
        void ecdsa_save_public_key( const ECDSA<ECP, SHA256>::PublicKey& key );
        void ecdsa_load_private_key( ECDSA<ECP, SHA256>::PrivateKey& key );
        void ecdsa_load_public_key( ECDSA<ECP, SHA256>::PublicKey& key );
        void ecdsa_load_private_key_from_string( ECDSA<ECP, SHA256>::PrivateKey& private_key );
        void ecdsa_load_public_key_from_string( ECDSA<ECP, SHA256>::PublicKey& public_key );
        void ecdsa_string_to_public_key( std::string public_key_in, ECDSA<ECP, SHA256>::PublicKey& public_key_out );
        void ecdsa_save_private_key_as_string(const ECDSA<ECP, SHA256>::PrivateKey& key);
        void ecdsa_save_public_key_as_string(const ECDSA<ECP, SHA256>::PublicKey& key);
        void ecdsa_load_private_key_as_string(std::string &private_key);
        void ecdsa_load_public_key_as_string(std::string &public_key);
        bool ecdsa_sign_message( const ECDSA<ECP, SHA256>::PrivateKey& key, const string& message, string& signature );
        bool ecdsa_verify_message( const ECDSA<ECP, SHA256>::PublicKey& key, const string& message, const string& signature );
        
        // RSA:
        void rsa_generate_and_save_keypair();
        void rsa_save_private_key( RSA::PrivateKey& key );
        void rsa_save_public_key( RSA::PublicKey& key );
        void rsa_load_private_key( RSA::PrivateKey& key );
        void rsa_load_public_key( RSA::PublicKey& key );
        void rsa_save_private_key_as_string(RSA::PrivateKey& key);
        void rsa_save_public_key_as_string(RSA::PublicKey& key);
        void rsa_load_public_key_as_string_from_file(std::string &rsa_pub_key);
        void rsa_encrypt_message( RSA::PublicKey& rsa_public_key, string& message, string& cipher );
        void rsa_decrypt_message( RSA::PrivateKey& rsa_private_key, string& cipher, string& recovered_message );
    private:
        // Private and Public keys
        ECDSA<ECP, SHA256>::PrivateKey ecdsa_private_key_;
        ECDSA<ECP, SHA256>::PublicKey ecdsa_public_key_;
    };
}