#include "crypto.hpp"
#include "base58.hpp"
#include "configdir.hpp"

#include "cryptlib.h"
#include "sha3.h"
#include "filters.h"

using namespace Crowd;
using namespace CryptoPP;
using namespace base58;

std::string Crypto::sha256_create(std::string &msg)
{
    std::string digest;

    SHA256 hash;
    StringSource(msg, true, new HashFilter(hash, new StringSink(digest)));

    return digest;
}

std::string Crypto::base58_encode_sha256(std::string &hash)
{
    std::string str = sha256_create(hash);
    std::vector<uint8_t> vec(str.begin(), str.end());
    return base58::EncodeBase58(vec);
}

std::string Crypto::base58_decode(std::string &b58)
{
    std::vector<uint8_t> vec;

    if (base58::DecodeBase58(b58, vec))
    {
        std::string str(vec.begin(), vec.end());
        return str;
    }
    else
    {
        return "false";
    }
}

int Crypto::ecdsa_generate_and_save_keypair()
{
    // Scratch result
    bool result = false; 

    // Generate Keys
    result = ecdsa_generate_private_key( CryptoPP::ASN1::secp160r1(), ecdsa_private_key_ );
    assert( true == result );
    if( !result ) { return -1; }

    result = ecdsa_generate_public_key( ecdsa_private_key_, ecdsa_public_key_ );
    assert( true == result );
    if( !result ) { return -2; }

    return 0;
}

bool Crypto::ecdsa_generate_private_key( const OID& oid, ECDSA<ECP, SHA256>::PrivateKey& key )
{
    AutoSeededRandomPool prng;

    key.Initialize( prng, oid );
    assert( key.Validate( prng, 3 ) );
     
    return key.Validate( prng, 3 );
}

bool Crypto::ecdsa_generate_public_key( const ECDSA<ECP, SHA256>::PrivateKey& ecdsa_private_key_, ECDSA<ECP, SHA256>::PublicKey& ecdsa_public_key_ )
{
    AutoSeededRandomPool prng;

    // Sanity check
    assert( ecdsa_private_key_.Validate( prng, 3 ) );

    ecdsa_private_key_.MakePublicKey(ecdsa_public_key_);
    assert( ecdsa_public_key_.Validate( prng, 3 ) );

    return ecdsa_public_key_.Validate( prng, 3 );
}

void Crypto::ecdsa_save_private_key( const ECDSA<ECP, SHA256>::PrivateKey& key )
{
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_priv_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_priv_key";
        key.Save( FileSink( filename.c_str(), true /*binary*/ ).Ref() );
    }
    else
    {
        std::cout << "Ecdsa_priv_key existed already!!" << std::endl;
    }
}

void Crypto::ecdsa_save_public_key( const ECDSA<ECP, SHA256>::PublicKey& key )
{
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_pub_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_pub_key";
        key.Save( FileSink( filename.c_str(), true /*binary*/ ).Ref() );
    }
    else
    {
        std::cout << "Ecdsa_pub_key existed already!!" << std::endl;
    }
}

void Crypto::ecdsa_load_private_key( ECDSA<ECP, SHA256>::PrivateKey& key )
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_priv_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_priv_key";
        key.Load( FileSource( filename.c_str(), true /*pump all*/ ).Ref() );
    }
    else
    {
        std::cout << "Ecdsa_priv_key doesn't exist!!" << std::endl;
    }
}

void Crypto::ecdsa_load_public_key( ECDSA<ECP, SHA256>::PublicKey& key )
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_pub_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_pub_key";
        key.Load( FileSource( filename.c_str(), true /*pump all*/ ).Ref() );
    }
    else
    {
        std::cout << "Ecdsa_pub_key doesn't exist!!" << std::endl;
    }
}

bool Crypto::ecdsa_sign_message( const ECDSA<ECP, SHA256>::PrivateKey& key, const string& message, string& signature )
{
    AutoSeededRandomPool prng;
    
    signature.erase();    

    StringSource( message, true,
        new SignerFilter( prng,
            ECDSA<ECP,SHA256>::Signer(key),
            new StringSink( signature )
        ) // SignerFilter
    ); // StringSource
    
    return !signature.empty();
}

bool Crypto::ecdsa_verify_message( const ECDSA<ECP, SHA256>::PublicKey& key, const string& message, const string& signature )
{
    bool result = false;

    StringSource( signature+message, true,
        new SignatureVerificationFilter(
            ECDSA<ECP,SHA256>::Verifier(key),
            new ArraySink( (byte*)&result, sizeof(result) )
        ) // SignatureVerificationFilter
    );

    return result;
}