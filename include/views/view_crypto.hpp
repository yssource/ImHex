#pragma once

#include "views/view.hpp"

#include <cstdio>
#include <list>

#include "helpers/utils.hpp"

namespace hex {

    namespace prv { class Provider; }

    struct DecryptionInfo {
        u64 region[2];
        enum class Function : int { None = 0, AES = 1, Blowfish = 2, Chacha = 3 } function;
        std::vector<char> key;
        std::vector<char> iv;
        std::vector<u8> decodedKey;
        std::vector<u8> decodedIv;
        u32 keySize;
        u32 ivSize;
        u64 params;
    };

    class ViewCrypto : public View {
    public:
        explicit ViewCrypto(prv::Provider* &dataProvider);
        ~ViewCrypto() override;

        void createView() override;
        void createMenu() override;

    private:
        prv::Provider* &m_dataProvider;

        constexpr static const char* CryptoFunctionNames[] = { "None", "AES", "Blowfish", "Chacha" };
        std::list<DecryptionInfo> m_decryptionInfo;

        void addDecryptionInfo();
    };

}