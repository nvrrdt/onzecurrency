// using namespace std;

// #include <openssl/evp.h>
// #include <openssl/pem.h>

// #include <iostream>
// #include <string>

// https://gist.github.com/grejdi/9361828

// int main()
// {

//     // create private/public key pair
//     // init RSA context, so we can generate a key pair
//     EVP_PKEY_CTX *keyCtx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
//     EVP_PKEY_keygen_init(keyCtx);
//     EVP_PKEY_CTX_set_rsa_keygen_bits(keyCtx, 2048); // RSA 2048
//     // variable that will hold both private and public keys
//     EVP_PKEY *key = NULL;
//     // generate key
//     EVP_PKEY_keygen(keyCtx, &key);
//     // free up key context
//     EVP_PKEY_CTX_free(keyCtx);
 
    
//     // extract private key as string
//     // create a place to dump the IO, in this case in memory
//     BIO *privateBIO = BIO_new(BIO_s_mem());
//     // dump key to IO
//     PEM_write_bio_PrivateKey(privateBIO, key, NULL, NULL, 0, 0, NULL);
//     // get buffer length
//     int privateKeyLen = BIO_pending(privateBIO);
//     // create char reference of private key length
//     unsigned char *privateKeyChar = (unsigned char *) malloc(privateKeyLen);
//     // read the key from the buffer and put it in the char reference
//     BIO_read(privateBIO, privateKeyChar, privateKeyLen);
//     // at this point we can save the private key somewhere
    
    
//     // extract public key as string
//     // create a place to dump the IO, in this case in memory
//     BIO *publicBIO = BIO_new(BIO_s_mem());
//     // dump key to IO
//     PEM_write_bio_PUBKEY(publicBIO, key);
//     // get buffer length
//     int publicKeyLen = BIO_pending(publicBIO);
//     // create char reference of public key length
//     unsigned char *publicKeyChar = (unsigned char *) malloc(publicKeyLen);
//     // read the key from the buffer and put it in the char reference
//     BIO_read(publicBIO, publicKeyChar, publicKeyLen);
//     // at this point we can save the public somewhere

    
//     // pretend we are pulling the public key from some source and using it
//     // to encrypt a message
//     unsigned char *rsaPublicKeyChar = publicKeyChar;
//     // write char array to BIO
//     BIO *rsaPublicBIO = BIO_new_mem_buf(rsaPublicKeyChar, -1);
//     // create a RSA object from public key char array
//     RSA *rsaPublicKey = NULL;
//     PEM_read_bio_RSA_PUBKEY(rsaPublicBIO, &rsaPublicKey, NULL, NULL);
//     // create public key
//     EVP_PKEY *publicKey = EVP_PKEY_new();
//     EVP_PKEY_assign_RSA(publicKey, rsaPublicKey);
//     // initialize encrypt context
//     EVP_CIPHER_CTX *rsaEncryptCtx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
//     EVP_CIPHER_CTX_init(rsaEncryptCtx);
//     // variables for where the encrypted secret, length, and IV reside
//     unsigned char *ek = (unsigned char *) malloc(EVP_PKEY_size(publicKey));
//     int ekLen = 0;
//     unsigned char *iv = (unsigned char *) malloc(EVP_MAX_IV_LENGTH);
//     // generate AES secret, and encrypt it with public key
//     EVP_SealInit(rsaEncryptCtx, EVP_aes_256_cbc(), &ek, &ekLen, iv, &publicKey, 1);
//     // encrypt a message with AES secret
//     string message = "You can include the standard headers in any order, a standard header more than once, or two or more standard headers that define the same macro or the same type. Do not include a standard header within a declaration. Do not define macros that have the same names as keywords before you include a standard header.";
//     const unsigned char* messageChar = (const unsigned char*) message.c_str();
//     // length of message
//     int messageLen = message.size() + 1;
//     // create char reference for where the encrypted message will reside
//     unsigned char *encryptedMessage = (unsigned char *) malloc(messageLen + EVP_MAX_IV_LENGTH);
//     // the length of the encrypted message
//     int encryptedMessageLen = 0;
//     int encryptedBlockLen = 0;
//     // encrypt message with AES secret
//     EVP_SealUpdate(rsaEncryptCtx, encryptedMessage, &encryptedBlockLen, messageChar, messageLen);
//     encryptedMessageLen = encryptedBlockLen;
//     // finalize by encrypting the padding
//     EVP_SealFinal(rsaEncryptCtx, encryptedMessage + encryptedBlockLen, &encryptedBlockLen);
//     encryptedMessageLen += encryptedBlockLen;
    
    
//     // pretend we are decrypting a message we have received using a the private key we have
//     unsigned char *rsaPrivateKeyChar = privateKeyChar;
//     // write char array to BIO
//     BIO *rsaPrivateBIO = BIO_new_mem_buf(rsaPrivateKeyChar, -1);
//     // create a RSA object from private key char array
//     RSA *rsaPrivateKey = NULL;
//     PEM_read_bio_RSAPrivateKey(rsaPrivateBIO, &rsaPrivateKey, NULL, NULL);
//     // create private key
//     EVP_PKEY *privateKey = EVP_PKEY_new();
//     EVP_PKEY_assign_RSA(privateKey, rsaPrivateKey);
//     // initialize decrypt context
//     EVP_CIPHER_CTX *rsaDecryptCtx = (EVP_CIPHER_CTX *) malloc(sizeof(EVP_CIPHER_CTX));
//     EVP_CIPHER_CTX_init(rsaDecryptCtx);
//     // decrypt EK with private key, and get AES secretp
//     EVP_OpenInit(rsaDecryptCtx, EVP_aes_256_cbc(), ek, ekLen, iv, privateKey);
//     // variable for where the decrypted message with be outputed to
//     unsigned char *decryptedMessage = (unsigned char *) malloc(encryptedMessageLen + EVP_MAX_IV_LENGTH);
//     // the length of the encrypted message
//     int decryptedMessageLen = 0;
//     int decryptedBlockLen = 0;
//     // decrypt message with AES secret
//     EVP_OpenUpdate(rsaDecryptCtx, decryptedMessage, &decryptedBlockLen, encryptedMessage, encryptedMessageLen);
//     decryptedMessageLen = decryptedBlockLen;
//     // finalize by decrypting padding
//     EVP_OpenFinal(rsaDecryptCtx, decryptedMessage + decryptedBlockLen, &decryptedBlockLen);
//     decryptedMessageLen += decryptedBlockLen;
    
// printf("%s\n", encryptedMessage);
// printf("\"%s\"\n", decryptedMessage);
    
// }