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

std::string Crypto::base58_encode(std::string &str)
{
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
    result = ecdsa_generate_private_key( CryptoPP::ASN1::secp256k1(), ecdsa_private_key_ );
    assert( true == result );
    if( !result ) { return -1; }

    ecdsa_save_private_key_as_string(ecdsa_private_key_);

    result = ecdsa_generate_public_key( ecdsa_private_key_, ecdsa_public_key_ );
    assert( true == result );
    if( !result ) { return -2; }

    ecdsa_save_public_key_as_string(ecdsa_public_key_);

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

void Crypto::ecdsa_load_private_key_from_string( ECDSA<ECP, SHA256>::PrivateKey& private_key )
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_priv_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_priv_key";
        std::fstream pk;
        std::string private_key_h;
        pk.open(filename, std::ios::in);
        if (pk.is_open())
        {
            std::string str;
            while(getline(pk, str))
            {
                std::cout << str << std::endl;
                private_key_h += str;
            }
            pk.close(); //close the file object.
        }

        HexDecoder decoderPrivate;
        decoderPrivate.Put((byte*)private_key_h.data(), private_key_h.size());
        decoderPrivate.MessageEnd();

        private_key.Load(decoderPrivate);
    }
    else
    {
        std::cout << "Ecdsa_priv_key doesn't exist!!" << std::endl;
    }
}

void Crypto::ecdsa_load_public_key_from_string( ECDSA<ECP, SHA256>::PublicKey& public_key )
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_pub_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_pub_key";
        std::fstream pk;
        std::string public_key_h;
        pk.open(filename, std::ios::in);
        if (pk.is_open())
        {
            std::string str;
            while(getline(pk, str))
            {
                std::cout << str << std::endl;
                public_key_h += str;
            }
            pk.close(); //close the file object.
        }
        
        HexDecoder decoderPublic;
        decoderPublic.Put((byte*)public_key_h.data(), public_key_h.size());
        decoderPublic.MessageEnd();

        public_key.Load(decoderPublic);
    }
    else
    {
        std::cout << "Ecdsa_pub_key doesn't exist!!" << std::endl;
    }
}

void Crypto::ecdsa_string_to_public_key( std::string public_key_in, ECDSA<ECP, SHA256>::PublicKey& public_key_out )
{
    HexDecoder decoderPublic;
    decoderPublic.Put((byte*)public_key_in.data(), public_key_in.size());
    decoderPublic.MessageEnd();

    public_key_out.Load(decoderPublic);
}

void Save(const string& filename, const BufferedTransformation& bt)
{
    FileSink file(filename.c_str());
    bt.CopyTo(file);
    file.MessageEnd();
}

void SaveHex(const string& filename, const BufferedTransformation& bt)
{
    HexEncoder encoder;
    bt.CopyTo(encoder);
    encoder.MessageEnd();
    Save(filename, encoder);
}

void SaveHexPrivateKey(const string& filename, const PrivateKey& key)
{
    ByteQueue queue;
    key.Save(queue);
    SaveHex(filename, queue);
}

void SaveHexPublicKey(const string& filename, const PublicKey& key)
{
    ByteQueue queue;
    key.Save(queue);
    SaveHex(filename, queue);
}

void Crypto::ecdsa_save_private_key_as_string(const ECDSA<ECP, SHA256>::PrivateKey& key)
{
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_priv_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_priv_key";

        SaveHexPrivateKey(filename, key);
    }
    else
    {
        std::cout << "Ecdsa_priv_key doesn't exist!!" << std::endl;
    }
}

void Crypto::ecdsa_save_public_key_as_string(const ECDSA<ECP, SHA256>::PublicKey& key)
{
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_pub_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_pub_key";
        
        SaveHexPublicKey(filename, key);
    }
    else
    {
        std::cout << "Ecdsa_pub_key doesn't exist!!" << std::endl;
    }
}

void Crypto::ecdsa_load_private_key_as_string(std::string &private_key)
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_priv_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_priv_key";
        std::fstream pk;
        pk.open(filename, std::ios::in);
        if (pk.is_open())
        {
            std::string str;
            while(getline(pk, str))
            {
                std::cout << str << std::endl;
                private_key += str;
            }
            pk.close(); //close the file object.
        }
    }
    else
    {
        std::cout << "Ecdsa_priv_key doesn't exist!!" << std::endl;
    }
}

void Crypto::ecdsa_load_public_key_as_string(std::string &public_key)
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "ecdsa_pub_key"))
    {
        const std::string filename = cd.GetConfigDir() + "ecdsa_pub_key";
        std::fstream pk;
        pk.open(filename, std::ios::in);
        if (pk.is_open())
        {
            std::string str;
            while(getline(pk, str))
            {
                std::cout << str << std::endl;
                public_key += str;
            }
            pk.close(); //close the file object.
        }
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

void Crypto::rsa_generate_and_save_keypair()
{
    // Generate keys
    AutoSeededRandomPool rng;

    InvertibleRSAFunction parameters;
    parameters.GenerateRandomWithKeySize( rng, 2048 );

    RSA::PrivateKey rsa_private_key( parameters );
    RSA::PublicKey rsa_public_key( parameters );

    rsa_save_private_key(rsa_private_key);
    rsa_save_public_key(rsa_public_key);
}

void Crypto::rsa_save_private_key( RSA::PrivateKey& key )
{
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "rsa_priv_key"))
    {
        const std::string filename = cd.GetConfigDir() + "rsa_priv_key";
        key.Save( FileSink( filename.c_str(), true /*binary*/ ).Ref() );
    }
    else
    {
        std::cout << "Rsa_priv_key existed already!!" << std::endl;
    }
}

void Crypto::rsa_save_public_key( RSA::PublicKey& key )
{
    ConfigDir cd;
    if (!boost::filesystem::exists(cd.GetConfigDir() + "rsa_pub_key"))
    {
        const std::string filename = cd.GetConfigDir() + "rsa_pub_key";
        key.Save( FileSink( filename.c_str(), true /*binary*/ ).Ref() );
    }
    else
    {
        std::cout << "Rsa_pub_key existed already!!" << std::endl;
    }
}

void Crypto::rsa_load_private_key( RSA::PrivateKey& key )
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "rsa_priv_key"))
    {
        const std::string filename = cd.GetConfigDir() + "rsa_priv_key";
        key.Load( FileSource( filename.c_str(), true /*pump all*/ ).Ref() );
    }
    else
    {
        std::cout << "Rsa_priv_key doesn't exist!!" << std::endl;
    }
}

void Crypto::rsa_load_public_key( RSA::PublicKey& key )
{
    ConfigDir cd;
    if (boost::filesystem::exists(cd.GetConfigDir() + "rsa_pub_key"))
    {
        const std::string filename = cd.GetConfigDir() + "rsa_pub_key";
        key.Load( FileSource( filename.c_str(), true /*pump all*/ ).Ref() );
    }
    else
    {
        std::cout << "Rsa_pub_key doesn't exist!!" << std::endl;
    }
}

void Crypto::rsa_encrypt_message( RSA::PublicKey& rsa_public_key, string& message, string& cipher )
{
    AutoSeededRandomPool rng;

    // Encryption
    RSAES_OAEP_SHA_Encryptor e( rsa_public_key );

    StringSource( message, true,
        new PK_EncryptorFilter( rng, e,
            new StringSink( cipher )
        ) // PK_EncryptorFilter
        ); // StringSource
}

void Crypto::rsa_decrypt_message( RSA::PrivateKey& rsa_private_key, string& cipher, string& recovered_message )
{
    AutoSeededRandomPool rng;

    // Decryption
    RSAES_OAEP_SHA_Decryptor d( rsa_private_key );

    StringSource( cipher, true,
        new PK_DecryptorFilter( rng, d,
            new StringSink( recovered_message )
        ) // PK_EncryptorFilter
        ); // StringSource
}