#include "views/view_crypto.hpp"

#include "providers/provider.hpp"

namespace hex {

    ViewCrypto::ViewCrypto(prv::Provider* &dataProvider) : View("Cryptography"), m_dataProvider(dataProvider) {

    }

    ViewCrypto::~ViewCrypto() {

    }

    void ViewCrypto::addDecryptionInfo() {
        this->m_decryptionInfo.emplace_back([] {
            DecryptionInfo info = { 0 };
            info.function = DecryptionInfo::Function::None;
            info.keySize = 16;
            info.ivSize = 16;
            info.key.resize(static_cast<u32>(std::ceil((info.keySize / 3.0F) * 4.0F) + 0x03) & ~0x04);
            info.decodedKey.resize(info.keySize);
            info.iv.resize(static_cast<u32>(std::ceil((info.ivSize / 3.0F) * 4.0F) + 0x03) & ~0x04);
            info.decodedIv.resize(info.ivSize);
            info.params = 0;

            return info;
        }());

        this->m_dataProvider->addOverlay([&info = this->m_decryptionInfo.back()](u64 offset, u8 value) -> std::optional<u8> {
            if (offset >= info.region[0] && offset <= info.region[1]) {
                return 0x69;
            } else
                return { };
        });
    }

    void ViewCrypto::createView() {
        if (ImGui::Begin("Cryptography", &this->getWindowOpenState(), ImGuiWindowFlags_NoCollapse)) {
            if (ImGui::BeginChild( "##scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav)) {

                if (ImGui::Button("Add Decryption info"))
                    this->addDecryptionInfo();

                ImGui::Separator();

                u32 index = 1;
                for (auto &decryptionInfo : this->m_decryptionInfo) {
                    ImGui::NewLine();
                    if (ImGui::BeginChild(index, ImVec2(0, 230), false)) {
                        ImGui::Text("Algorithm");
                        ImGui::Separator();
                        ImGui::Combo("##nolabel", reinterpret_cast<int*>(&decryptionInfo.function), CryptoFunctionNames, 4);
                        ImGui::NewLine();
                        ImGui::Text("Region");
                        ImGui::Separator();
                        ImGui::InputScalarN("##nolabel", ImGuiDataType_U64, &decryptionInfo.region, 2, nullptr, nullptr, "%lX", ImGuiInputTextFlags_CharsHexadecimal);
                        ImGui::NewLine();
                        ImGui::Text("Settings");
                        ImGui::Separator();
                        ImGui::InputText("Key", decryptionInfo.key.data(), decryptionInfo.key.size());
                        ImGui::InputScalar("Key Size", ImGuiDataType_U32, &decryptionInfo.keySize);
                        ImGui::InputText("IV", decryptionInfo.iv.data(), decryptionInfo.iv.size());
                        ImGui::InputScalar("IV Size", ImGuiDataType_U32, &decryptionInfo.ivSize);
                    }
                    ImGui::EndChild();
                    if (index < this->m_decryptionInfo.size()) {
                        ImGui::NewLine();
                        ImGui::Separator();
                        ImGui::Separator();
                    }

                    index++;
                }

            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

    void ViewCrypto::createMenu() {

    }

}