#include "crypto_main.hpp"
#include "p2p.hpp"

using namespace Crowd;

void CryptoMain::do_encrypt_rsa_message()
{
    // // TODO: figure out if nat traversal is needed
    // LookupPeer lp(hash_of_peer_);
    // std::string pub_key_str = lp.GetPubKeyPeer();
    // unsigned char *pub_key = reinterpret_cast<unsigned char*>(const_cast<char*>(pub_key_str.c_str()));
    // //const char *pub_key = pub_key_str.c_str();

    // Crypto crypto(pub_key, strlen(pub_key));
    // // std::string message = do_encrypt_aes_message();
    // std::string message =  "encryptie test"; // needs to become password symmetric encryption
    // unsigned char *encryptedMessage = NULL;
    // unsigned char *encryptedKey;
    // unsigned char *iv;
    // size_t encryptedKeyLength;
    // size_t ivLength;
    // int encryptedMessageLength = crypto.rsaEncrypt((const unsigned char*)message.c_str(), message.size()+1,
    //     &encryptedMessage, &encryptedKey, &encryptedKeyLength, &iv, &ivLength);

    // if(encryptedMessageLength == -1) {
    //     fprintf(stderr, "Encryption failed\n");
    //     return;
    // }

    // // Print the encrypted message as a base64 string
    // char* b64Message = base64Encode(encryptedMessage, encryptedMessageLength);
    // printf("Encrypted message: %s\n", b64Message);

    // // Clean up
    // free(encryptedMessage);
    // free(encryptedKey);
    // free(iv);
    // free(b64Message);

    // encryptedMessage = NULL;
    // encryptedKey = NULL;
    // iv = NULL;
    // b64Message = NULL;
}

void CryptoMain::do_decrypt_rsa_message()
{
    // Crypto crypto;

    // unsigned char *encryptedMessage = NULL;
    // unsigned char *encryptedKey;
    // unsigned char *iv;
    // size_t encryptedKeyLength;
    // size_t ivLength;

    // // Decrypt the message
    // char *decryptedMessage = NULL;

    // int decryptedMessageLength = crypto->rsaDecrypt(encryptedMessage, (size_t)encryptedMessageLength,
    //     encryptedKey, encryptedKeyLength, iv, ivLength, (unsigned char**)&decryptedMessage);

    // if(decryptedMessageLength == -1) {
    //     fprintf(stderr, "Decryption failed\n");
    //     return;
    // }

    // printf("Decrypted message: %s\n", decryptedMessage);
    
    // // Clean up
    // free(decryptedMessage);
    // free(encryptedKey);
    // free(iv);
    // free(b64Message);

    // decryptedMessage = NULL;
    // encryptedKey = NULL;
    // iv = NULL;
    // b64Message = NULL;
}

std::string CryptoMain::do_encrypt_aes_message()
{
    //


    
    // return message;
    return "";
}

void CryptoMain::do_decrypt_aes_message()
{
    //
}
